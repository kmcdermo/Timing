#include "FastSkimmer.hh"

FastSkimmer::FastSkimmer(const TString & cutconfig, const TString & pdname, const TString & outtext)
  : fCutConfig(cutconfig), fPDName(pdname), fOutFileText(outtext)
{
  std::cout << "Initializing FastSkimmer..." << std::endl;

  ////////////////
  //            //
  // Initialize //
  //            //
  ////////////////

  // setup samples and such
  FastSkimmer::SetupConfig();
  FastSkimmer::SetupLists();

  // output root file containing every tree
  fOutFile = TFile::Open(Form("%s.root",fOutFileText.Data()),"UPDATE");
}

FastSkimmer::~FastSkimmer()
{
  // delete everything else already not deleted
  for (const auto & ListPair : ListMap) delete ListPair.second;
  delete fConfigPave;
  delete fOutFile;
}

void FastSkimmer::MakeSkim()
{
  // Make TEntryLists for each Subsample
  FastSkimmer::MakeListFromTrees();

  // Generate Merged Skims
  FastSkimmer::MakeMergedSkims();

  // Write Out Config
  FastSkimmer::MakeConfigPave();
}

void FastSkimmer::MakeListFromTrees()
{
  std::cout << "Making TEntryLists from trees..." << std::endl;

  for (const auto & SamplePair : Config::SampleMap)
  {
    // Init
    const auto & input  = SamplePair.first;
    const auto & sample = SamplePair.second;
    std::cout << "Working on input: " << input.Data() << std::endl;

    // Get File
    const TString filename = Form("%s/%s/%s/%s",Config::eosDir.Data(),Config::baseDir.Data(),input.Data(),Config::tupleFileName.Data());
    auto file = TFile::Open(Form("%s",filename.Data()));
    Config::CheckValidFile(file,filename);
    file->cd();
	
    // Get TTree
    auto tree = (TTree*)file->Get(Form("%s",Config::disphotreename.Data()));
    Config::CheckValidTree(tree,Config::disphotreename,filename);

    std::cout << "Computing list of entries..." << std::endl;

    // Get entry list
    auto listname = Config::ReplaceSlashWithUnderscore(input);
    auto & list = ListMap[listname];
    list->SetDirectory(file);
    
    // use ttree::draw() to generate entry list
    tree->Draw(Form(">>%s",list->GetName()),Form("%s",Config::CutMap[sample].Data()),"entrylist");

    // Write out list
    fOutFile->cd();
    list->SetTitle("EntryList");
    list->Write(list->GetName(),TObject::kWriteDelete);

    // delete everything
    delete tree;
    delete file;
  }
}

void FastSkimmer::MakeMergedSkims()
{
  std::cout << "Make skims and merge for each sample..." << std::endl;

  // loop over all samples, merging subsamples and saving
  for (const auto & TreeNamePair : Config::TreeNameMap)
  {
    // get main sample, 
    const auto & sample   = TreeNamePair.first;
    const auto & treename = TreeNamePair.second;

    std::cout << "Working on sample group: " << treename.Data() << std::endl;

    // Make a tmp list to eventually merge ttrees
    auto TreeList = new TList();
    std::map<TString,TTree*> TreeMap;

    // Make skims...
    FastSkimmer::MakeSkimsFromEntryLists(TreeMap,TreeList,sample);

    std::cout << "Merging skimmed trees..." << std::endl;
  
    // Now want to merge the list!
    fOutFile->cd();
    auto OutTree = TTree::MergeTrees(TreeList);
    OutTree->SetName(Form("%s",treename.Data()));

    // write out merged tree
    fOutFile->cd();
    OutTree->Write(OutTree->GetName(),TObject::kWriteDelete);
    
    // now start deleting stuff to reduce memory footprint
    delete OutTree;
    delete TreeList;
    for (auto & TreePair : TreeMap)
    {
      delete TreePair.second;
    }
  } // end loop over sample groups
}

void FastSkimmer::MakeSkimsFromEntryLists(std::map<TString,TTree*> & TreeMap, TList *& TreeList, const SampleType & sample)
{
  // loop over all subsamples to avoid too many maps, simple control statement to control subsample grouping
  for (const auto & SamplePair : Config::SampleMap)
  {
    const auto & input = SamplePair.first;
    if (sample != SamplePair.second) continue;
  
    std::cout << "Working on input: " << input.Data() << std::endl;
    
    // Get File
    const TString filename = Form("%s/%s/%s/%s",Config::eosDir.Data(),Config::baseDir.Data(),input.Data(),Config::tupleFileName.Data());
    auto file = TFile::Open(Form("%s",filename.Data()));
    Config::CheckValidFile(file,filename);
    file->cd();
    
    // Get TTree
    auto tree = (TTree*)file->Get(Form("%s",Config::disphotreename.Data()));
    Config::CheckValidTree(tree,Config::disphotreename,filename);
    
    // Set Entry List Needed
    auto listname = Config::ReplaceSlashWithUnderscore(input);
    tree->SetEntryList(ListMap[listname]);
    
    std::cout << "Skimming into a smaller tree..." << std::endl;
    
    // Fill from tree into smaller tree
    fOutFile->cd();
    TreeMap[input] = tree->CopyTree("");
    
    // Add tree to list
    TreeList->Add(TreeMap[input]);
    
    // Delete everything you can
    delete tree;
    delete file;
  }
}

void FastSkimmer::MakeConfigPave()
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

void FastSkimmer::SetupConfig()
{
  Config::SetupPrimaryDataset(fPDName);
  Config::SetupSamples();
  Config::SetupGroups();
  Config::SetupTreeNames();
  Config::SetupCuts(fCutConfig);
}

void FastSkimmer::SetupLists()
{
  // loop over all subsamples, create new list
  for (const auto & SamplePair : Config::SampleMap)
  {
    const auto & input  = SamplePair.first;
    auto listname = Config::ReplaceSlashWithUnderscore(input);
    ListMap[listname] = new TEntryList(Form("%s_EntryList",listname.Data()),"EntryList");
  }
}
