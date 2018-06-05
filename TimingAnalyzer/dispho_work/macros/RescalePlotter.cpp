#include "RescalePlotter.hh"

RescalePlotter::RescalePlotter(const TString & infilename, const TString & rescaleconfig, const TString & plotconfig,
			       const TString & miscconfig, const TString & outfiletext) 
  : fInFileName(infilename), fRescaleConfig(rescaleconfig), fPlotConfig(plotconfig),
    fMiscConfig(miscconfig), fOutFileText(outfiletext)
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

  // setup hists
  TreePlotter::SetupDefaults();
  RescalePlotter::SetupConfig();
  TreePlotter::SetupMiscConfig(fMiscConfig);
  if (TreePlotter::fSignalsOnly) Common::KeepOnlySignals();
  RescalePlotter::SetupRescaleConfig();
  TreePlotter::SetupPlotConfig(fPlotConfig);
  RescalePlotter::SetupHists();
}

RescalePlotter::~RescalePlotter()
{
  // delete everything
  delete TreePlotter::fConfigPave;

  delete TreePlotter::LowerPad;
  delete TreePlotter::UpperPad;
  delete TreePlotter::OutCanv;
  delete TreePlotter::Legend;
  delete TreePlotter::RatioLine;
  delete TreePlotter::RatioMCErrs;
  delete TreePlotter::RatioHist;
  delete TreePlotter::BkgdStack;
  delete TreePlotter::BkgdHist;
  delete TreePlotter::DataHist;

  delete TreePlotter::fOutFile;
  for (auto & HistPair : TreePlotter::HistMap) delete HistPair.second; // will delete RescaleHist as well
  delete TreePlotter::fTDRStyle;
  delete fInFile;
}

void RescalePlotter::MakeRescaledPlot()
{
  // Scale hists
  RescalePlotter::RescaleHists();

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
  RescalePlotter::MakeConfigPave();

  // Dump integrals into text file
  TreePlotter::DumpIntegrals(fOutFileText);
}

void RescalePlotter::RescaleHists()
{
  std::cout << "Rescaling hists..." << std::endl;

  // get references for ease of use
  const auto & InScaleHist = TreePlotter::HistMap[fScaleSample];
  auto & InShapeHist = TreePlotter::HistMap[fShapeSample];
  const auto sf = InScaleHist->Integral(fXVarBins?"width":"")/InShapeHist->Integral(fXVarBins?"width":"");

  // first add the shape hist to output scale
  RescaleHist = TreePlotter::SetupHist(Form("%s_Rescaled",Common::HistNameMap[fScaleSample]));
  RescaleHist->Add(InShapeHist);

  // then scale it by the integral of scale hist
  RescaleHist->Scale(sf);
  
  // erase original unscaled hist in hist map
  delete InScaleHist;
  Common::HistNameMap.erase(fScaleSample);

  // set scaled hist to hist map
  Common::HistNameMap[fScaleSample] = RescaleHist;

  // save a copy as _scaled
  fOutFile->cd();
  RescaleHist->Write(RescaleHist->GetName(),TObject::kWriteDelete);
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

  // dump rescale config first
  Common::AddTextFromInputConfig(TreePlotter::fConfigPave,"Rescale Config",fRescaleConfig); 

  // store misc config
  Common::AddTextFromInputConfig(TreePlotter::fConfigPave,"Miscellaneous Config",fMiscConfig); 

  // save name of infile, redundant
  TreePlotter::fConfigPave->AddText(Form("InFile name: %s",fInFileName.Data()));

  // dump in old config
  Common::AddTextFromInputPave(TreePlotter::fConfigPave,fInFile);
  
  // save to output file
  TreePlotter::fOutFile->cd();
  TreePlotter::fConfigPave->Write(TreePlotter::fConfigPave->GetName(),TObject::kWriteDelete);
}

void RescalePlotter::SetupConfig()
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
  TreePlotter::fOutFile->cd();
  for (const auto & HistPair : TreePlotter::HistMap)
  { 
    const auto & hist = HistPair.second;
    hist->Write(hist->GetName(),TObject::kWriteDelete);
  }
}
