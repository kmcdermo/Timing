#include "RescalePlotter.hh"

RescalePlotter::RescalePlotter(const TString & infilename, const TString & rescaleconfig, const TString & plotconfig, const TString & miscconfig,
			       const TString & era, const Bool_t savemetadata, const TString & outfiletext) 
  : fInFileName(infilename), fRescaleConfig(rescaleconfig), fPlotConfig(plotconfig),
    fMiscConfig(miscconfig), fEra(era), fOutFileText(outfiletext)
{
  std::cout << "Initializing RescalePlotter..." << std::endl;

  ////////////////
  //            //
  // Initialize //
  //            //
  ////////////////

  // Get input file
  fInFile = TFile::Open(Form("%s",fInFileName.Data()));
  Common::CheckValidFile(fInFile,fInFileName);
 
  // set style
  TreePlotter::fTDRStyle = new TStyle("TDRStyle","Style for P-TDR");
  Common::SetTDRStyle(TreePlotter::fTDRStyle);

  // output root file for quick inspection
  TreePlotter::fOutFile = TFile::Open(Form("%s.root",fOutFileText.Data()),"UPDATE");

  // setup config
  TreePlotter::SetupDefaults();
  TreePlotter::SetupSaveMetaData(savemetadata);
  RescalePlotter::SetupCommon();
  TreePlotter::SetupMiscConfig(fMiscConfig);
  if (TreePlotter::fSignalsOnly) Common::KeepOnlySignals();
  RescalePlotter::SetupRescaleConfig();
  TreePlotter::SetupPlotConfig(fPlotConfig);

  // setup hists
  RescalePlotter::SetupHists();
  TreePlotter::SetupHistsStyle();
}

void RescalePlotter::MakeRescaledPlot()
{
  // Scale hists
  RescalePlotter::RescaleHist();

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
  TreePlotter::SaveOutput(fOutFileText,fEra);

  // Write Out Config
  if (TreePlotter::fSaveMetaData) RescalePlotter::MakeConfigPave();

  // Dump integrals into text file
  TreePlotter::DumpIntegrals(fOutFileText);

  // delete memory
  RescalePlotter::DeleteMemory();
}

void RescalePlotter::RescaleHist()
{
  std::cout << "Rescaling hists..." << std::endl;

  // get references for ease of use
  auto       & InScaleHist = TreePlotter::HistMap[fScaleSample];
  const auto & InShapeHist = TreePlotter::HistMap[fShapeSample];
  const auto sf = InScaleHist->Integral(fXVarBins?"width":"")/InShapeHist->Integral(fXVarBins?"width":"");

  // first add the shape hist to output scale
  RescaledHist = TreePlotter::SetupHist(Form("%s_Rescaled",Common::HistNameMap[fScaleSample].Data()));
  RescaledHist->Add(InShapeHist);

  // set style for rescaled hist to match old scale histogram
  RescaledHist->SetLineColor(InScaleHist->GetLineColor());
  RescaledHist->SetMarkerColor(InScaleHist->GetMarkerColor());
  RescaledHist->SetFillColor(InScaleHist->GetFillColor());
  RescaledHist->SetFillStyle(InScaleHist->GetFillStyle());
  RescaledHist->SetLineWidth(InScaleHist->GetLineWidth());

  // then scale it by the integral of scale hist
  RescaledHist->Scale(sf);
  
  // erase original unscaled hist in hist map
  delete InScaleHist;
  TreePlotter::HistMap.erase(fScaleSample);

  // set scaled hist to hist map
  TreePlotter::HistMap[fScaleSample] = RescaledHist;

  // save a copy as _scaled
  Common::Write(TreePlotter::fOutFile,RescaledHist);
}

void RescalePlotter::MakeConfigPave()
{
  std::cout << "Dumping config to a pave..." << std::endl;

  // create the pave, copying in old info
  TreePlotter::fOutFile->cd();
  TreePlotter::fConfigPave = new TPaveText();
  TreePlotter::fConfigPave->SetName(Form("%s",Common::pavename.Data()));

  // give grand title
  TreePlotter::fConfigPave->AddText("***** RescalePlotter Config *****");

  // add era info
  Common::AddEraInfoToPave(TreePlotter::fConfigPave,fEra);

  // dump rescale config first
  Common::AddTextFromInputConfig(TreePlotter::fConfigPave,"Rescale Config",fRescaleConfig); 

  // dump plot config
  Common::AddTextFromInputConfig(TreePlotter::fConfigPave,"Plot Config",fPlotConfig); 

  // store misc config
  Common::AddTextFromInputConfig(TreePlotter::fConfigPave,"Miscellaneous Config",fMiscConfig); 

  // padding
  Common::AddPaddingToPave(Plotter::fConfigPave,3);

  // save name of infile, redundant
  TreePlotter::fConfigPave->AddText(Form("InFile name: %s",fInFileName.Data()));

  // dump in old config
  Common::AddTextFromInputPave(TreePlotter::fConfigPave,fInFile);
  
  // save to output file
  Common::Write(TreePlotter::fOutFile,TreePlotter::fConfigPave);
}

void RescalePlotter::DeleteMemory()
{
  TreePlotter::DeleteMemory(false);
  delete fInFile;
}

void RescalePlotter::SetupCommon()
{
  std::cout << "Setting up Common..." << std::endl;

  Common::SetupEras();
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

void RescalePlotter::SetupRescaleConfig()
{
  std::cout << "Reading rescale config..." << std::endl;

  std::ifstream infile(Form("%s",fRescaleConfig.Data()),std::ios::in);
  std::string str;
  while (std::getline(infile,str))
  {
    if (str == "") continue;
    else if (str.find("scale_sample=") != std::string::npos)
    {
      fScaleSample = Common::RemoveDelim(str,"scale_sample=");
    }
    else if (str.find("shape_sample=") != std::string::npos)
    {
      fShapeSample = Common::RemoveDelim(str,"shape_sample=");
    }
    else 
    {
      std::cerr << "Aye... your rescale config is messed up, try again!" << std::endl;
      std::cerr << "Offending line: " << str.c_str() << std::endl;
      exit(1);
    }
  }
}

void RescalePlotter::SetupHists()
{
  std::cout << "Setting up input hists..." << std::endl;
  
  // read in histograms
  for (const auto & HistNamePair : Common::HistNameMap)
  {
    const auto & sample   = HistNamePair.first;
    const auto & histname = HistNamePair.second;
    TreePlotter::HistMap[sample] = (TH1F*)fInFile->Get(Form("%s",histname.Data()));
  }

  // save to output file
  Common::WriteMap(TreePlotter::fOutFile,TreePlotter::HistMap);
}
