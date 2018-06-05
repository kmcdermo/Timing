#include "RescalePlotter.hh"

RescalePlotter::RescalePlotter(const TString & infilename, const TString & rescaleconfig,
			       const TString & miscconfig, const TString & outfiletext) 
  : fInFileName(infilename), fRescaleConfig(rescaleconfig),
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

  // setup hists
  RescalePlotter::SetupDefaults();
  RescalePlotter::SetupConfig();
  TreePlotter::SetupMiscConfig(fMiscConfig);
  if (TreePlotter::fSignalsOnly) Common::KeepOnlySignals();
  RescalePlotter::SetupHists();

  // output root file for quick inspection
  TreePlotter::fOutFile = TFile::Open(Form("%s.root",fOutFileText.Data()),"UPDATE");
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
  for (auto & HistPair : TreePlotter::HistMap) delete HistPair.second;
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
  TreePlotter::SaveOutput();

  // Write Out Config
  RescalePlotter::MakeConfigPave();

  // Dump integrals into text file
  TreePlotter::DumpIntegrals();
}

void RescalePlotter::RescaleHists()
{
  std::cout << "Rescaling hists..." << std::endl;
}

void RescalePlotter::MakeConfigPave()
{
  std::cout << "Dumping config into pave..." << std::endl;

}

void RescalePlotter::SetupDefaults()
{
  std::cout << "Setting up defaults..." << std::endl;
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

void RescalePlotter::SetupHists()
{
  std::cout << "Setting up input hists..." << std::endl;
}

