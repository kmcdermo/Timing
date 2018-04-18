#include "EntryLister.hh"

EntryLister::EntryLister(const TString & cutconfig, const TString & pdname, const TString & outtext)
  : fCutConfig(cutconfig), fPDName(pdname), fOutFileText(outtext)
{
  std::cout << "Initializing EntryLister..." << std::endl;

  ////////////////
  //            //
  // Initialize //
  //            //
  ////////////////

  // setup samples and such
  EntryLister::SetupConfig();

  // output root file containing every tree
  fOutFile = TFile::Open(Form("%s.root",fOutFileText.Data()),"UPDATE");
}

EntryLister::~EntryLister()
{
  // delete everything else already not deleted
  delete fConfigPave;
  delete fOutFile;
}

void EntryLister::MakeList()
{
  // Get list of events passing cuts
  EntryLister::MakeListFromTrees();

  // Write Out Config
  EntryLister::MakeConfigPave();
}

void EntryLister::MakeListFromTrees()
{
  for (const auto & SamplePair : Config::SampleMap)
  {
    // Init
    const auto & input  = SamplePair.first;
    const auto & sample = SamplePair.second;
    std::cout << "Working on input: " << input.Data() << std::endl;

    // Get File
    const TString filename = Form("%s/%s/%s/%s",Config::eosDir.Data(),Config::baseDir.Data(),input.Data(),Config::tupleFileName.Data());
    TFile * file = TFile::Open(Form("%s",filename.Data()));
    Config::CheckValidFile(file,filename);
    file->cd();
	
    // Get TTree
    TTree * tree = (TTree*)file->Get(Form("%s",Config::disphotreename.Data()));
    Config::CheckValidTree(tree,Config::disphotreename,filename);

    std::cout << "Getting list of entries..." << std::endl;

    // Get entry list
    TString listname = Config::ReplaceSlashWithUnderscore(input);
    TEntryList * list = new TEntryList(Form("%s",listname.Data()),Form("%s",listname.Data()));
    list->SetDirectory(file);
    tree->Draw(Form(">>%s",listname.Data()),Form("%s",Config::CutMap[sample].Data()),"entrylist");

    // Write out list
    fOutFile->cd();
    list->Write(list->GetName(),TObject::kWriteDelete);

    // delete everything
    delete list;
    delete tree;
    delete file;
  }
}

void EntryLister::MakeConfigPave()
{
  std::cout << "Dumping config to a pave..." << std::endl;

  // create the pave
  fConfigPave = new TPaveText();
  fConfigPave->SetName("Config");
  std::string str; // tmp string
  
  // dump cut config
  fConfigPave->AddText("Cut Config");
  std::ifstream cutfile(Form("%s",fCutConfig.Data()),std::ios::in);
  while (std::getline(cutfile,str))
  {
    fConfigPave->AddText(str.c_str());
  }

  // dump with PD
  fConfigPave->AddText(Form("Primary Dataset: %s",fPDName.Data()));

  // save to output file
  fOutFile->cd();
  fConfigPave->Write(fConfigPave->GetName(),TObject::kWriteDelete);
}

void EntryLister::SetupConfig()
{
  Config::SetupPrimaryDataset(fPDName);
  Config::SetupSamples();
  Config::SetupCuts(fCutConfig);
}
