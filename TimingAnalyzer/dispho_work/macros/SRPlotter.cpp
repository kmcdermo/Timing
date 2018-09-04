#include "SRPlotter.hh"

SRPlotter::SRPlotter(const TString & srplotconfig, const TString & miscconfig, const TString & outfiletext) 
  : fSRPlotConfig(srplotconfig), fMiscConfig(miscconfig), fOutFileText(outfiletext)
{
  std::cout << "Initializing SRPlotter..." << std::endl;

  ////////////////
  //            //
  // Initialize //
  //            //
  ////////////////

  // setup config
  TreePlotter::SetupDefaults();
  SRPlotter::SetupConfig();
  TreePlotter::SetupMiscConfig(fMiscConfig);
  SRPlotter::SetupSRPlotConfig();
  TreePlotter::SetupPlotConfig(fPlotConfig);
}

void SRPlotter::MakePlots()
{
  std::cout << "Making all plots..." << std::endl;

  SRPlotter::MakeSRPlot();
  SRPlotter::MakeScaledCRPlot("GJets",fGJetsFile);
  SRPlotter::MakeScaledCRPlot("QCD"  ,fQCDFile);

  SRPlotter::DeleteInputFiles();
}

void SRPlotter::MakeSRPlot()
{
  std::cout << "Making SR plot..." << std::endl;

  // set style
  TreePlotter::fTDRStyle = new TStyle("TDRStyle","Style for P-TDR");
  Common::SetTDRStyle(TreePlotter::fTDRStyle);

  // output root file for quick inspection
  TreePlotter::fOutFile = TFile::Open(Form("%s.root",fOutFileText.Data()),"UPDATE");

  // setup hists
  SRPlotter::SetupHists();
  TreePlotter::SetupHistsStyle();

  // Scale data CR hists via MC predictions
  Common::GetSRPredFromCRs(TreePlotter::HistMap,SRPlotter::HistMap,fXVarBins,fCRKFMap,fCRXFMap);

  // Bulk hist plotter
  SRPlotter::CommonPlotter(fOutFileText);

  // Dump integrals into text file
  const auto & outfilename = TreePlotter::DumpIntegrals(fOutFileText);

  // Dump constants
  SRPlotter::DumpFactors(outfilename);

  // delete memory
  SRPlotter::DeleteMemory(true);
}

void SRPlotter::MakeScaledCRPlot(const TString & CR, TFile *& infile)
{
  std::cout << "Making CR-only plot for: " << CR.Data() << std::endl;

  // outtext
  const TString outfiletext = fOutFileText+"_CR_"+CR+"_kFScaled";

  // set style
  TreePlotter::fTDRStyle = new TStyle("TDRStyle","Style for P-TDR");
  Common::SetTDRStyle(TreePlotter::fTDRStyle);

  // output root file for quick inspection
  TreePlotter::fOutFile = TFile::Open(Form("%s.root",outfiletext.Data()),"UPDATE");

  // Quick hack for unblinding CRs
  TreePlotter::fBlindData = false;

  // setup hists
  SRPlotter::SetupCROnlyHists(CR, infile);
  TreePlotter::SetupHistsStyle();

  // Scale data CR hists via MC predictions
  SRPlotter::ScaleCRByKFOnly(CR);

  // Bulk hist plotter
  SRPlotter::CommonPlotter(outfiletext);

  // Dump integrals into text file
  TreePlotter::DumpIntegrals(outfiletext);

  // delete memory
  SRPlotter::DeleteMemory(false);
}

void SRPlotter::CommonPlotter(const TString & outfiletext)
{
  std::cout << "Building plot by using commong TreePlotter functions..." << std::endl;

  // Make Data Output
  TreePlotter::MakeDataOutput();

  // Make Bkgd Output
  TreePlotter::MakeBkgdOutput();

  // Make Signal Output
  TreePlotter::MakeSignalOutput();

  // Make Ratio Output
  TreePlotter::MakeRatioOutput();

  // Make Legend
  TreePlotter::MakeLegend();

  // Init Output Canv+Pads
  TreePlotter::InitOutputCanvPads();

  // Draw Upper Pad
  TreePlotter::DrawUpperPad();

  // Draw Lower Pad
  TreePlotter::DrawLowerPad();

  // Save Output
  TreePlotter::SaveOutput(outfiletext);

  // Write Out Config
  SRPlotter::MakeConfigPave();
}

void SRPlotter::ScaleCRByKFOnly(const TString & CR)
{
  std::cout << "Scaling MC hists in CR-only plots by k-factor for: " << CR.Data()  << std::endl;
  
  for (auto & HistPair : TreePlotter::HistMap)
  {
    const auto & sample = HistPair.first;
    auto & hist = HistPair.second;
    
    // skip data and signal, only want to scale bkgd MC!
    if (Common::GroupMap[sample] != SampleGroup::isBkgd) continue;
    
    hist->Scale(fCRKFMap[CR]); // already stored, but can recalculate Data/MC with loop over HistMap
  }
}

void SRPlotter::MakeConfigPave()
{
  std::cout << "Dumping config to a pave..." << std::endl;

  // create the pave, copying in old info
  TreePlotter::fOutFile->cd();
  TreePlotter::fConfigPave = new TPaveText();
  TreePlotter::fConfigPave->SetName(Form("%s",Common::pavename.Data()));

  // give grand title
  TreePlotter::fConfigPave->AddText("***** SRPlotter Config *****");

  // dump rescale config first
  Common::AddTextFromInputConfig(TreePlotter::fConfigPave,"SRPlot Config",fSRPlotConfig); 

  // dump plot config: a bit redundant
  Common::AddTextFromInputConfig(TreePlotter::fConfigPave,"Plot Config",fPlotConfig); 

  // store misc config
  Common::AddTextFromInputConfig(TreePlotter::fConfigPave,"Miscellaneous Config",fMiscConfig); 

  // padding
  Common::AddPaddingToPave(TreePlotter::fConfigPave,3);

  // dump in old config
  fConfigPave->AddText("***** GJets CR Config *****");
  Common::AddTextFromInputPave(TreePlotter::fConfigPave,fGJetsFile);

  fConfigPave->AddText("***** QCD CR Config *****");
  Common::AddTextFromInputPave(TreePlotter::fConfigPave,fQCDFile);

  fConfigPave->AddText("***** SR Config *****");
  Common::AddTextFromInputPave(TreePlotter::fConfigPave,fSRFile);
  
  // save to output file
  TreePlotter::fOutFile->cd();
  TreePlotter::fConfigPave->Write(TreePlotter::fConfigPave->GetName(),TObject::kWriteDelete);
}

void SRPlotter::DumpFactors(const TString & outfilename)
{
  std::cout << "Dumping kFactors and scaleFactors into text file..." << std::endl;

  // reopen dumpfile object
  std::ofstream dumpfile(Form("%s",outfilename.Data()),std::ios_base::app);

  // padding
  dumpfile << std::endl;
  Common::AddPaddingToFile(dumpfile,3);
  dumpfile << std::endl;

  // Dump GJets info first
  SRPlotter::DumpIntegrals("GJets",fGJetsFile,dumpfile);

  // padding
  Common::AddPaddingToFile(dumpfile,2);

  // Dump QCD info second
  SRPlotter::DumpIntegrals("QCD",fQCDFile,dumpfile);
}

void SRPlotter::DumpIntegrals(const TString & CR, TFile *& infile, std::ofstream & dumpfile)
{
  std::cout << "Dumping integrals for: " << CR.Data() << std::endl;

  // Dump Data in CR first
  infile->cd();
  auto data_hist = (TH1F*)infile->Get(Form("%s",Common::HistNameMap["Data"].Data()));
  auto data_error = 0.;
  const auto data_integral = data_hist->IntegralAndError(1,data_hist->GetXaxis()->GetNbins(),data_error,(fXVarBins?"width":""));
  dumpfile << CR.Data() << "_CR_" << data_hist->GetName() << " : " << data_integral << " +/- " << data_error << std::endl;
  delete data_hist;

  // Dump CR plots second
  for (const auto & HistPair : SRPlotter::HistMap)
  {
    const auto & key     = HistPair.first;
    const auto & cr_hist = HistPair.second;

    if (key.Contains("SR",TString::kExact)) continue; // skip SR plots
    if (!key.Contains(Form("%s_CR",CR.Data()),TString::kExact)) continue; // skip other CR

    auto cr_error = 0.;
    const auto cr_integral = cr_hist->IntegralAndError(1,cr_hist->GetXaxis()->GetNbins(),cr_error,(fXVarBins?"width":""));
    dumpfile << cr_hist->GetName() << " : " << cr_integral << " +/- " << cr_error << std::endl;
  }
 
  // Dump SR MC last
  const auto & sr_hist = SRPlotter::HistMap[Form("SR_%s",CR.Data())];
  auto sr_error = 0.;
  const auto sr_integral = sr_hist->IntegralAndError(1,sr_hist->GetXaxis()->GetNbins(),sr_error,(fXVarBins?"width":""));
  dumpfile << sr_hist->GetName() << " : " << sr_integral << " +/- " << sr_error << std::endl;

  // padding
  Common::AddPaddingToFile(dumpfile,1);

  // dump factors
  dumpfile << Form("%s k-Factor: ",CR.Data()) << fCRKFMap[CR] << std::endl;
  dumpfile << Form("%s x-Factor: ",CR.Data()) << fCRXFMap[CR] << std::endl;
}

void SRPlotter::DeleteMemory(const Bool_t deleteSRHists)
{
  std::cout << "Deleting memory in SRPlotter... deleting SR plot inputs: " << Common::PrintBool(deleteSRHists).Data() << std::endl; 

  TreePlotter::DeleteMemory(false);
  
  if (deleteSRHists)
  {
    for (auto & HistPair : SRPlotter::HistMap) delete HistPair.second;
    SRPlotter::HistMap.clear();
  }
}

void SRPlotter::DeleteInputFiles()
{
  std::cout << "Final delete of input files..." << std::endl;

  delete fSRFile;
  delete fQCDFile;
  delete fGJetsFile;
}

void SRPlotter::SetupConfig()
{
  std::cout << "Setting up Config..." << std::endl;

  Common::SetupSamples();
  Common::SetupSignalSamples();
  Common::SetupGroups();
  Common::SetupBkgdGroups();
  Common::SetupSignalGroups();
  Common::SetupSignalSubGroups();
  Common::SetupHistNames();
  Common::SetupBkgdHistNames();
  Common::SetupSignalSubGroupColors();
  Common::SetupColors();
  Common::SetupLabels();
}

void SRPlotter::SetupSRPlotConfig()
{
  std::cout << "Reading SRPlot config..." << std::endl;

  std::ifstream infile(Form("%s",fSRPlotConfig.Data()),std::ios::in);
  std::string str;
  while (std::getline(infile,str))
  {
    if (str == "") continue;
    else if (str.find("CR_GJets_in=") != std::string::npos)
    {
      fGJetsFileName = Common::RemoveDelim(str,"CR_GJets_in=");
    }
    else if (str.find("CR_QCD_in=") != std::string::npos)
    {
      fQCDFileName = Common::RemoveDelim(str,"CR_QCD_in=");
    }
    else if (str.find("SR_in=") != std::string::npos)
    {
      fSRFileName = Common::RemoveDelim(str,"SR_in=");
    }
    else if (str.find("plot_config=") != std::string::npos)
    {
      fPlotConfig = Common::RemoveDelim(str,"plot_config=");
    }
    else 
    {
      std::cerr << "Aye... your srplot config is messed up, try again!" << std::endl;
      std::cerr << "Offending line: " << str.c_str() << std::endl;
      exit(1);
    }
  }
}

void SRPlotter::SetupHists()
{
  std::cout << "Setting up input hists..." << std::endl;
  
  //////////////
  // GJets CR //
  //////////////

  fGJetsFile = TFile::Open(Form("%s",fGJetsFileName.Data()));
  Common::CheckValidFile(fGJetsFile,fGJetsFileName);
  fGJetsFile->cd();

  Common::SetupCRHists("GJets",fGJetsFile,TreePlotter::HistMap,SRPlotter::HistMap);

  ////////////
  // QCD CR //
  ////////////

  fQCDFile = TFile::Open(Form("%s",fQCDFileName.Data()));
  Common::CheckValidFile(fQCDFile,fQCDFileName);
  fQCDFile->cd();

  Common::SetupCRHists("QCD",fQCDFile,TreePlotter::HistMap,SRPlotter::HistMap);

  //////////////
  // SR Hists //
  //////////////

  fSRFile = TFile::Open(Form("%s",fSRFileName.Data()));
  Common::CheckValidFile(fSRFile,fSRFileName);
  fSRFile->cd();

  Common::SetupSRMCHists(fSRFile,SRPlotter::HistMap);

  /////////////////////////////////////////////////////////////
  // Read in the remaining SR histograms (for plotting only) //
  /////////////////////////////////////////////////////////////

  for (const auto & HistNamePair : Common::HistNameMap)
  {
    const auto & sample   = HistNamePair.first;
    const auto & histname = HistNamePair.second;

    if (Common::IsCR(sample)) continue;

    TreePlotter::HistMap[sample] = (TH1F*)fSRFile->Get(Form("%s",histname.Data()));
    Common::CheckValidHist(TreePlotter::HistMap[sample],histname,fSRFileName);
  }

  /////////////////////////
  // Save to output file //
  /////////////////////////

  TreePlotter::fOutFile->cd();

  // Real HistMap
  for (const auto & HistPair : TreePlotter::HistMap)
  { 
    const auto & hist = HistPair.second;
    hist->Write(hist->GetName(),TObject::kWriteDelete);
  }

  // Tmp HistMap
  for (const auto & HistPair : SRPlotter::HistMap)
  { 
    const auto & hist = HistPair.second;
    hist->Write(hist->GetName(),TObject::kWriteDelete);
  }
}

void SRPlotter::SetupCROnlyHists(const TString & CR, TFile *& infile)
{
  std::cout << "Getting hists from CR-only file: " << CR.Data() << std::endl;

  /////////////////////////
  // Emplace input hists //
  /////////////////////////
  
  // get hists
  for (const auto & HistNamePair : Common::HistNameMap)
  {
    const auto & sample   = HistNamePair.first;
    const auto & histname = HistNamePair.second;

    TreePlotter::HistMap[sample] = (TH1F*)infile->Get(Form("%s",histname.Data()));
    Common::CheckValidHist(TreePlotter::HistMap[sample],histname,infile->GetName());
  }

  /////////////////////////
  // Save to output file //
  /////////////////////////

  TreePlotter::fOutFile->cd();

  for (const auto & HistPair : TreePlotter::HistMap)
  { 
    const auto & hist = HistPair.second;
    hist->Write(hist->GetName(),TObject::kWriteDelete);
  }
}
