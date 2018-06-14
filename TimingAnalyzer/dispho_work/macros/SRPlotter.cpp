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

void SRPlotter::ScaleCRtoSR()
{
  std::cout << "Scaling data hists in CR to SR using MC scale factors..." << std::endl;

  // Create MC SFs
  std::map<TString,Float_t> mcSFMap;
  mcSFMap["GJets"] = (GJetsHistMC_SR->Integral((fXVarBins?"width":"")))/(GJetsHistMC_CR->Integral((fXVarBins?"width":"")));
  mcSFMap["QCD"]   = (QCDHistMC_SR  ->Integral((fXVarBins?"width":"")))/(QCDHistMC_CR  ->Integral((fXVarBins?"width":"")));
  
  // Scale Data CR by MC SFs --> yields SR prediction for each CR
  for (const auto & mcSFPair : mcSFMap)
  {
    const auto & sample = mcSFPair.first;
    const auto & mcsf   = mcSFPair.second;

    HistMap[sample]->Scale(mcsf);
  }
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
  TreePlotter::DumpIntegrals(fOutFileText);

  // delete memory
  SRPlotter::DeleteMemory();
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

  // store misc config
  Common::AddTextFromInputConfig(TreePlotter::fConfigPave,"Miscellaneous Config",fMiscConfig); 

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

void SRPlotter::DeleteMemory()
{
  TreePlotter::DeleteMemory(false);
  
  delete QCDHistMC_SR;
  delete GJetsHistMC_SR;
  delete fSRFile;

  delete QCDHistMC_CR;
  delete fQCDFile;

  delete GJetsHistMC_CR;
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

  /////////////////////////
  // GJets CR Histograms //
  /////////////////////////

  fGJetsFile = TFile::Open(Form("%s",fGJetsFileName.Data()));
  Common::CheckValidFile(fGJetsFile,fGJetsFileName);

  TreePlotter::HistMap["GJets"] = (TH1F*)fGJetsFile->Get(Form("%s",Common::HistNameMap["Data"] .Data()));
  GJetsHistMC_CR                = (TH1F*)fGJetsFile->Get(Form("%s",Common::HistNameMap["GJets"].Data()));
  Common::CheckValidTH1F(TreePlotter::HistMap["GJets"],Common::HistNameMap["Data"] ,fGJetsFileName);
  Common::CheckValidTH1F(GJetsHistMC_CR               ,Common::HistNameMap["GJets"],fGJetsFileName);

  // Rename histograms
  TreePlotter::HistMap["GJets"]->SetName(Form("CR_Data_%s",Common::HistNameMap["GJets"].Data()));
  GJetsHistMC_CR               ->SetName(Form("CR_MC_%s"  ,Common::HistNameMap["GJets"].Data()));

  ////////////
  // QCD CR //
  ////////////

  fQCDFile = TFile::Open(Form("%s",fQCDFileName.Data()));
  Common::CheckValidFile(fQCDFile,fQCDFileName);

  TreePlotter::HistMap["QCD"] = (TH1F*)fQCDFile->Get(Form("%s",Common::HistNameMap["Data"].Data()));
  QCDHistMC_CR                = (TH1F*)fQCDFile->Get(Form("%s",Common::HistNameMap["QCD"] .Data()));
  Common::CheckValidTH1F(TreePlotter::HistMap["QCD"],Common::HistNameMap["Data"],fQCDFileName);
  Common::CheckValidTH1F(QCDHistMC_CR               ,Common::HistNameMap["QCD"] ,fQCDFileName);

  // Rename histograms
  TreePlotter::HistMap["QCD"]->SetName(Form("CR_Data_%s",Common::HistNameMap["QCD"].Data()));
  QCDHistMC_CR               ->SetName(Form("CR_MC_%s"  ,Common::HistNameMap["QCD"].Data()));

  //////////////////////////////////////////////////////////////////////////////
  // SR (read in GJets and QCD MC into separate histograms, not into HistMap) //
  //////////////////////////////////////////////////////////////////////////////

  fSRFile = TFile::Open(Form("%s",fSRFileName.Data()));
  Common::CheckValidFile(fSRFile,fSRFileName);

  GJetsHistMC_SR = (TH1F*)fSRFile->Get(Form("%s",Common::HistNameMap["GJets"].Data()));
  QCDHistMC_SR   = (TH1F*)fSRFile->Get(Form("%s",Common::HistNameMap["QCD"].Data()));
  Common::CheckValidTH1F(GJetsHistMC_SR,Common::HistNameMap["GJets"],fSRFileName);
  Common::CheckValidTH1F(QCDHistMC_SR  ,Common::HistNameMap["QCD"]  ,fSRFileName);

  // Rename histograms
  GJetsHistMC_SR->SetName(Form("SR_MC_%s",Common::HistNameMap["GJets"].Data()));
  QCDHistMC_SR  ->SetName(Form("SR_MC_%s",Common::HistNameMap["QCD"  ].Data()));

  //////////////////////////////////////
  // Read in the remaining histograms //
  //////////////////////////////////////

  for (const auto & HistNamePair : Common::HistNameMap)
  {
    const auto & sample   = HistNamePair.first;
    const auto & histname = HistNamePair.second;

    if (sample.Contains("GJets",TString::kExact) || sample.Contains("QCD",TString::kExact)) continue;

    TreePlotter::HistMap[sample] = (TH1F*)fSRFile->Get(Form("%s",histname.Data()));
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

  GJetsHistMC_CR->Write(GJetsHistMC_CR->GetName(),TObject::kWriteDelete);
  GJetsHistMC_SR->Write(GJetsHistMC_SR->GetName(),TObject::kWriteDelete);

  QCDHistMC_CR->Write(QCDHistMC_CR->GetName(),TObject::kWriteDelete);
  QCDHistMC_SR->Write(QCDHistMC_SR->GetName(),TObject::kWriteDelete);
}
