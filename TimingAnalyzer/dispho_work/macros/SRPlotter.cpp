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

  // set style
  TreePlotter::fTDRStyle = new TStyle("TDRStyle","Style for P-TDR");
  Common::SetTDRStyle(TreePlotter::fTDRStyle);

  // setup config
  TreePlotter::SetupDefaults();
  SRPlotter::SetupConfig();
  TreePlotter::SetupMiscConfig(fMiscConfig);
  SRPlotter::SetupSRPlotConfig();
  TreePlotter::SetupPlotConfig(fPlotConfig);

  // output root file for quick inspection
  TreePlotter::fOutFile = TFile::Open(Form("%s.root",fOutFileText.Data()),"UPDATE");

  // setup hists
  SRPlotter::SetupHists();
  TreePlotter::SetupHistsStyle();
}

void SRPlotter::MakeSRPlot()
{
  // Scale data CR hists via MC predictions
  SRPlotter::ScaleCRtoSR();

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
  TreePlotter::SaveOutput(fOutFileText);

  // Write Out Config
  SRPlotter::MakeConfigPave();

  // Dump integrals into text file
  const auto & filename = TreePlotter::DumpIntegrals(fOutFileText);

  // Dump constants
  SRPlotter::DumpFactors(filename);

  // delete memory
  SRPlotter::DeleteMemory();
}

void SRPlotter::ScaleCRtoSR()
{
  std::cout << "Scaling data hists in CR to SR using MC scale factors..." << std::endl;

  // Create k-factors
  std::map<TString,Float_t> crKFMap;
  crKFMap["GJets"] = SRPlotter::GetKFactor("GJets");
  crKFMap["QCD"]   = SRPlotter::GetKFactor("QCD");

  // Create CRtoSR scale factors : a bit redundant with K-Factors for now
  std::map<TString,Float_t> crSFMap;
  for (const auto & crKFPair : crKFMap)
  {
    const auto & CR    = crKFPair.first;
    const auto kFactor = crKFPair.second;

    crSFMap[CR] = SRPlotter::GetScaleFactor(CR,kFactor);
  }

  // Scale Data CR by MC SFs --> yields SR prediction for each CR
  for (const auto & crSFPair : crSFMap)
  {
    const auto & sample = crSFPair.first;
    const auto & crsf   = crSFPair.second;

    TreePlotter::HistMap[sample]->Scale(crsf);
  }
}

Float_t SRPlotter::GetKFactor(const TString & CR)
{
  std::cout << "Computing k-factor for: " << CR.Data() << std::endl;

  // numer = Data - !CR_MC
  Float_t numer = TreePlotter::HistMap[CR]->Integral(fXVarBins?"width":"");
  for (const auto & HistPair : SRPlotter::HistMap)
  {
    const auto & key  = HistPair.first;
    const auto & hist = HistPair.second;

    if (key.Contains("SR",TString::kExact)) continue; // skip SR plots
    if (!key.Contains(Form("%s_CR",CR.Data()),TString::kExact)) continue; // skip other CR
    if (key.Contains(Form("%s_CR_%s",CR.Data(),CR.Data()),TString::kExact)) continue; // skip MC Hist in CR
    
    // subtract everything in MC that is NOT CR MC
    numer -= hist->Integral(fXVarBins?"width":"");
  }

  return (numer / SRPlotter::HistMap[Form("%s_CR_%s",CR.Data(),CR.Data())]->Integral(fXVarBins?"width":""));
}

Float_t SRPlotter::GetScaleFactor(const TString & CR, const Float_t kFactor)
{
  return (kFactor * SRPlotter::HistMap[Form("%s_CR_%s",CR.Data(),CR.Data())]->Integral(fXVarBins?"width":""));
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

void SRPlotter::DumpFactors(const TString & filename)
{
  std::cout << "Dumping kFactors and scaleFactors into text file..." << std::endl;

  // reopen dumpfile object
  std::ofstream dumpfile(Form("%s",filename.Data()),std::ios_base::app);

  dumpfile << std::endl;
  dumpfile << "-------------------------------------" << std::endl;
  dumpfile << "-------------------------------------" << std::endl;
  dumpfile << "-------------------------------------" << std::endl;  
  dumpfile << std::endl;

  // Dump GJets info first
  SRPlotter::DumpIntegrals("GJets",fGJetsFile,dumpfile);

  // get factors
  const auto gjets_kf = SRPlotter::GetKFactor("GJets");
  const auto gjets_sf = SRPlotter::GetScaleFactor("GJets",gjets_kf);

  // dump factors
  std::cout << "GJets K-Factor    : " << gjets_kf << std::endl;
  std::cout << "GJets Scale-Factor: " << gjets_sf << std::endl;

  // Dump QCD info second
  SRPlotter::DumpIntegrals("QCD",fQCDFile,dumpfile);

  // get factors
  const auto qcd_kf = SRPlotter::GetKFactor("QCD");
  const auto qcd_sf = SRPlotter::GetScaleFactor("QCD",qcd_kf);

  // dump factors
  std::cout << "QCD K-Factor      : " << qcd_kf << std::endl;
  std::cout << "QCD Scale-Factor  : " << qcd_sf << std::endl;
}

void SRPlotter::DumpIntegrals(const TString & CR, TFile *& file, std::ofstream & dumpfile)
{
  // Dump Data in CR first
  file->cd();
  auto data_hist = (TH1F*)file->Get(Form("%s",Common::HistNameMap["Data"].Data()));
  auto data_error = 0.;
  const auto data_integral = data_hist->IntegralAndError(1,data_hist->GetXaxis()->GetNbins(),data_error,(fXVarBins?"width":""));
  dumpfile << data_hist->GetName() << " : " << data_integral << " +/- " << data_error << std::endl;
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
}

void SRPlotter::DeleteMemory()
{
  TreePlotter::DeleteMemory(false);
  
  for (auto & HistPair : SRPlotter::HistMap) delete HistPair.second;

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
  Common::SetupSignalGroups();
  Common::SetupSignalSubGroups();
  Common::SetupHistNames();
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

  SRPlotter::SetupCRHists(fGJetsFile,"GJets");

  ////////////
  // QCD CR //
  ////////////

  fQCDFile = TFile::Open(Form("%s",fQCDFileName.Data()));
  Common::CheckValidFile(fQCDFile,fQCDFileName);
  fQCDFile->cd();

  SRPlotter::SetupCRHists(fQCDFile,"QCD");

  //////////////
  // SR Hists //
  //////////////

  fSRFile = TFile::Open(Form("%s",fSRFileName.Data()));
  Common::CheckValidFile(fSRFile,fSRFileName);
  fSRFile->cd();

  SRPlotter::HistMap["SR_GJets"] = (TH1F*)fSRFile->Get(Form("%s",Common::HistNameMap["GJets"].Data()));
  SRPlotter::HistMap["SR_QCD"]   = (TH1F*)fSRFile->Get(Form("%s",Common::HistNameMap["QCD"]  .Data()));
  Common::CheckValidTH1F(SRPlotter::HistMap["SR_GJets"],Common::HistNameMap["GJets"],fSRFileName);
  Common::CheckValidTH1F(SRPlotter::HistMap["SR_QCD"]  ,Common::HistNameMap["QCD"]  ,fSRFileName);

  SRPlotter::HistMap["SR_GJets"]->SetName(Form("SR_%s",Common::HistNameMap["GJets"].Data()));
  SRPlotter::HistMap["SR_QCD"]  ->SetName(Form("SR_%s",Common::HistNameMap["QCD"  ].Data()));

  /////////////////////////////////////////
  // Read in the remaining SR histograms //
  /////////////////////////////////////////

  for (const auto & HistNamePair : Common::HistNameMap)
  {
    const auto & sample   = HistNamePair.first;
    const auto & histname = HistNamePair.second;

    if (Common::IsCRMC(sample)) continue;

    TreePlotter::HistMap[sample] = (TH1F*)fSRFile->Get(Form("%s",histname.Data()));
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

void SRPlotter::SetupCRHists(TFile *& file, const TString & CR)
{
  std::cout << "Setting up CR hists for: " << CR.Data() << std::endl;

  // Get Histograms
  TreePlotter::HistMap[CR] = (TH1F*)file->Get(Form("%s",Common::HistNameMap["Data"].Data()));

  SRPlotter::HistMap[Form("%s_CR_GJets",CR.Data())] = (TH1F*)file->Get(Form("%s",Common::HistNameMap["GJets"].Data()));
  SRPlotter::HistMap[Form("%s_CR_QCD"  ,CR.Data())] = (TH1F*)file->Get(Form("%s",Common::HistNameMap["QCD"]  .Data()));
  SRPlotter::HistMap[Form("%s_CR_EWK"  ,CR.Data())] = (TH1F*)file->Get(Form("%s",Common::EWKHistName         .Data()));

  // Check hists are okay
  const TString filename = file->GetName();
  Common::CheckValidTH1F(TreePlotter::HistMap[CR],Common::HistNameMap["Data"],filename);

  Common::CheckValidTH1F(SRPlotter::HistMap[Form("%s_CR_GJets",CR.Data())],Common::HistNameMap["GJets"],filename);
  Common::CheckValidTH1F(SRPlotter::HistMap[Form("%s_CR_QCD"  ,CR.Data())],Common::HistNameMap["QCD"]  ,filename);
  Common::CheckValidTH1F(SRPlotter::HistMap[Form("%s_CR_EWK"  ,CR.Data())],Common::EWKHistName         ,filename);
  
  // Rename hists
  TreePlotter::HistMap[CR]->SetName(Form("%s_CR_%s",CR.Data(),TreePlotter::HistMap[CR]->GetName()));

  SRPlotter::HistMap[Form("%s_CR_GJets",CR.Data())]->SetName(Form("%s_CR_%s",CR.Data(),SRPlotter::HistMap[Form("%s_CR_GJets",CR.Data())]->GetName()));
  SRPlotter::HistMap[Form("%s_CR_QCD"  ,CR.Data())]->SetName(Form("%s_CR_%s",CR.Data(),SRPlotter::HistMap[Form("%s_CR_QCD"  ,CR.Data())]->GetName()));
  SRPlotter::HistMap[Form("%s_CR_EWK"  ,CR.Data())]->SetName(Form("%s_CR_%s",CR.Data(),SRPlotter::HistMap[Form("%s_CR_EWK"  ,CR.Data())]->GetName()));
}
