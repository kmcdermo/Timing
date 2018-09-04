#include "EraTreePlotter2D.hh"

EraTreePlotter2D::EraTreePlotter2D(const TString & infilename, const TString & insignalfilename, const TString & cutconfig,
				   const TString & varwgtmapconfig, const TString & plotconfig, const TString & miscconfig,
				   const TString & era, const TString & outfiletext) 
  : fInFileName(infilename), fInSignalFileName(insignalfilename), fCutConfig(cutconfig),
    fVarWgtMapConfig(varwgtmapconfig), fPlotConfig(plotconfig), fMiscConfig(miscconfig),
    fEra(era), fOutFileText(outfiletext)
{
  std::cout << "Initializing EraTreePlotter2D..." << std::endl;

  ////////////////
  //            //
  // Initialize //
  //            //
  ////////////////

  // Get main input file
  fInFile = TFile::Open(Form("%s",fInFileName.Data()));
  Common::CheckValidFile(fInFile,fInFileName);

  // Get signal input file
  fInSignalFile = TFile::Open(Form("%s",fInSignalFileName.Data()));
  Common::CheckValidFile(fInSignalFile,fInSignalFileName);
 
  // set style
  TreePlotter2D::fTDRStyle = new TStyle("TDRStyle","Style for P-TDR");
  Common::SetTDRStyle(TreePlotter2D::fTDRStyle);

  // setup hists
  TreePlotter2D::SetupDefaults();
  EraTreePlotter2D::SetupConfig();
  TreePlotter2D::SetupPlotConfig(fPlotConfig);
  TreePlotter2D::SetupMiscConfig(fMiscConfig);
  TreePlotter2D::SetupHists();

  // output root file
  TreePlotter2D::fOutFile = TFile::Open(Form("%s.root",fOutFileText.Data()),"UPDATE");
}

void EraTreePlotter2D::MakeEraTreePlot2D()
{
  // Fill Hists from TTrees
  TreePlotter2D::MakeHistFromTrees(fInFile,fInSignalFile);

  // Make Data Output
  TreePlotter2D::MakeDataOutput();

  // Make Bkgd Output
  TreePlotter2D::MakeBkgdOutput();

  // Make Signal Output
  TreePlotter2D::MakeSignalOutput();

  // Make Ratio Output
  TreePlotter2D::MakeRatioOutput();

  // Write Out Config
  EraTreePlotter2D::MakeConfigPave();

  // Delete allocated memory
  EraTreePlotter2D::DeleteMemory();
}

void EraTreePlotter2D::MakeConfigPave()
{
  std::cout << "Dumping config to a pave..." << std::endl;

  // create the pave, copying in old info
  TreePlotter2D::fOutFile->cd();
  TreePlotter2D::fConfigPave = new TPaveText();
  TreePlotter2D::fConfigPave->SetName(Form("%s",Common::pavename.Data()));
  
  // give grand title
  TreePlotter2D::fConfigPave->AddText("***** EraTreePlotter2D Config *****");

  // save era info
  const auto & erainfo = Common::EraMap[fEra];
  TreePlotter2D::fConfigPave->AddText(Form("Era: %s, Runs: %i-%i, Lumi: %5.2f /fb",fEra.Data(),erainfo.startRun,erainfo.endRun,erainfo.lumi));
  
  // dump plot cut config first
  Common::AddTextFromInputConfig(TreePlotter2D::fConfigPave,"TreePlotter2D Cut Config",fCutConfig);

  // dump extra weights 
  Common::AddTextFromInputConfig(TreePlotter2D::fConfigPave,"VarWgtMap Config",fVarWgtMapConfig);

  // dump plot config
  Common::AddTextFromInputConfig(TreePlotter2D::fConfigPave,"Plot Config",fPlotConfig);

  // store last bits of info from misc
  Common::AddTextFromInputConfig(TreePlotter2D::fConfigPave,"Miscellaneous Config",fMiscConfig); 

  // padding
  Common::AddPaddingToPave(TreePlotter2D::fConfigPave,3);

  // save name of infile, redundant
  TreePlotter2D::fConfigPave->AddText(Form("InFile name: %s",fInFileName.Data()));

  // dump in old config
  Common::AddTextFromInputPave(TreePlotter2D::fConfigPave,fInFile);

  // save name of insignalfile, redundant
  TreePlotter2D::fConfigPave->AddText(Form("InSignalFile name: %s",fInSignalFileName.Data()));

  // dump in old signal config
  Common::AddTextFromInputPave(TreePlotter2D::fConfigPave,fInSignalFile);
  
  // save to output file
  TreePlotter2D::fOutFile->cd();
  TreePlotter2D::fConfigPave->Write(TreePlotter2D::fConfigPave->GetName(),TObject::kWriteDelete);
}

void EraTreePlotter2D::DeleteMemory()
{
  std::cout << "Deleting memory in EraTreePlotter2D..." << std::endl;

  TreePlotter2D::DeleteMemory(false);
  delete fInSignalFile;
  delete fInFile;
}

void EraTreePlotter2D::SetupConfig()
{
  std::cout << "Setting up Config..." << std::endl;

  Common::SetupEras();
  Common::SetupSamples();
  Common::SetupSignalSamples();
  Common::SetupGroups();
  Common::SetupTreeNames();
  Common::SetupHistNames();
  Common::SetupCuts(fCutConfig);
  Common::SetupEraCuts(fEra);
  Common::SetupVarWgts(fVarWgtMapConfig);
  Common::SetupWeights();
  Common::SetupEraWeights(fEra);
}
