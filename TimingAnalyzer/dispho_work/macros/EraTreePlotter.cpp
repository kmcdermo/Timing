#include "EraTreePlotter.hh"

EraTreePlotter::EraTreePlotter(const TString & infilename, const TString & insignalfilename, const TString & cutconfig,
			       const TString & varwgtmapconfig, const TString & plotconfig, const TString & miscconfig,
			       const TString & era, const TString & outfiletext) 
  : fInFileName(infilename), fInSignalFileName(insignalfilename), fCutConfig(cutconfig),
    fVarWgtMapConfig(varwgtmapconfig), fPlotConfig(plotconfig), fMiscConfig(miscconfig),
    fEra(era), fOutFileText(outfiletext)
{
  std::cout << "Initializing EraTreePlotter..." << std::endl;

  ////////////////
  //            //
  // Initialize //
  //            //
  ////////////////

  // Get input file
  fInFile = TFile::Open(Form("%s",fInFileName.Data()));
  Common::CheckValidFile(fInFile,fInFileName);
 
  // Get signal input file
  fInSignalFile = TFile::Open(Form("%s",fInSignalFileName.Data()));
  Common::CheckValidFile(fInSignalFile,fInSignalFileName);

  // set style
  TreePlotter::fTDRStyle = new TStyle("TDRStyle","Style for P-TDR");
  Common::SetTDRStyle(TreePlotter::fTDRStyle);

  // output root file for quick inspection
  TreePlotter::fOutFile = TFile::Open(Form("%s.root",fOutFileText.Data()),"UPDATE");

  // setup config
  TreePlotter::SetupDefaults();
  EraTreePlotter::SetupConfig();
  TreePlotter::SetupMiscConfig(fMiscConfig);
  if (fSignalsOnly) Common::KeepOnlySignals();
  TreePlotter::SetupPlotConfig(fPlotConfig);

  // setup hists
  TreePlotter::SetupHists();
  TreePlotter::SetupHistsStyle();
}

void EraTreePlotter::MakeEraTreePlot()
{
  // Fill Hists from TTrees
  TreePlotter::MakeHistFromTrees(fInFile,fInSignalFile);

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
  TreePlotter::SaveOutput(fOutFileText,Common::EraMap[fEra].lumi);

  // Write Out Config
  EraTreePlotter::MakeConfigPave();

  // Dump integrals into text file
  TreePlotter::DumpIntegrals(fOutFileText);

  // Delete allocated memory
  EraTreePlotter::DeleteMemory();
}

void EraTreePlotter::MakeConfigPave()
{
  std::cout << "Dumping config to a pave..." << std::endl;

  // create the pave, copying in old info
  TreePlotter::fOutFile->cd();
  TreePlotter::fConfigPave = new TPaveText();
  TreePlotter::fConfigPave->SetName(Form("%s",Common::pavename.Data()));
  
  // give grand title
  TreePlotter::fConfigPave->AddText("***** EraTreePlotter Config *****");

  // save era info
  const auto & erainfo = Common::EraMap[fEra];
  TreePlotter::fConfigPave->AddText(Form("Era: %s, Runs: %i-%i, Lumi: %5.2f /fb",fEra.Data(),erainfo.startRun,erainfo.endRun,erainfo.lumi));
  
  // dump plot cut config first
  Common::AddTextFromInputConfig(TreePlotter::fConfigPave,"TreePlotter Cut Config",fCutConfig);

  // dump extra weights 
  Common::AddTextFromInputConfig(TreePlotter::fConfigPave,"VarWgtMap Config",fVarWgtMapConfig);

  // dump plot config
  Common::AddTextFromInputConfig(TreePlotter::fConfigPave,"Plot Config",fPlotConfig);

  // store last bits of info from misc
  Common::AddTextFromInputConfig(TreePlotter::fConfigPave,"Miscellaneous Config",fMiscConfig); 

  // padding
  Common::AddPaddingToPave(TreePlotter::fConfigPave,3);

  // save name of infile, redundant
  TreePlotter::fConfigPave->AddText(Form("InFile name: %s",fInFileName.Data()));

  // dump in old config
  Common::AddTextFromInputPave(TreePlotter::fConfigPave,fInFile);

  // save name of insignalfile, redundant
  TreePlotter::fConfigPave->AddText(Form("InSignalFile name: %s",fInSignalFileName.Data()));

  // dump in old signal config
  Common::AddTextFromInputPave(TreePlotter::fConfigPave,fInSignalFile);
  
  // save to output file
  TreePlotter::fOutFile->cd();
  TreePlotter::fConfigPave->Write(TreePlotter::fConfigPave->GetName(),TObject::kWriteDelete);
}

void EraTreePlotter::DeleteMemory()
{
  std::cout << "Deleting memory in EraTreePlotter..." << std::endl;

  TreePlotter::DeleteMemory(false);
  delete fInSignalFile;
  delete fInFile;
}

void EraTreePlotter::SetupConfig()
{
  std::cout << "Setting up Config..." << std::endl;

  Common::SetupEras();
  Common::SetupSamples();
  Common::SetupSignalSamples();
  Common::SetupGroups();
  Common::SetupSignalGroups();
  Common::SetupSignalSubGroups();
  Common::SetupTreeNames();
  Common::SetupHistNames();
  Common::SetupSignalSubGroupColors();
  Common::SetupColors();
  Common::SetupLabels();
  Common::SetupCuts(fCutConfig);
  Common::SetupEraCuts(fEra);
  Common::SetupVarWgts(fVarWgtMapConfig);
  Common::SetupWeights();
  Common::SetupEraWeights(fEra);
}
