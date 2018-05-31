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

  for (const auto & SamplePair : Common::SampleMap)
  {
    // Init
    const auto & input  = SamplePair.first;
    const auto & sample = SamplePair.second;
    std::cout << "Working on input: " << input.Data() << std::endl;

    // Get File
    const TString filename = Form("%s/%s/%s/%s",Common::eosDir.Data(),Common::baseDir.Data(),input.Data(),Common::tupleFileName.Data());
    auto file = TFile::Open(Form("%s",filename.Data()));
    Common::CheckValidFile(file,filename);
    file->cd();
	
    // Get TTree
    auto tree = (TTree*)file->Get(Form("%s",Common::disphotreename.Data()));
    Common::CheckValidTree(tree,Common::disphotreename,filename);

    std::cout << "Computing list of entries..." << std::endl;

    // Get entry list
    const auto samplename = Common::ReplaceSlashWithUnderscore(input);
    auto & list = ListMap[samplename];
    list->SetDirectory(file);
    
    // use ttree::draw() to generate entry list
    tree->Draw(Form(">>%s",list->GetName()),Form("%s",Common::CutMap[sample].Data()),"entrylist");

    // Write out list
    fOutFile->cd();
    list->SetDirectory(fOutFile);
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
  for (const auto & TreeNamePair : Common::TreeNameMap)
  {
    // get main sample
    const auto & sample   = TreeNamePair.first;
    const auto & treename = TreeNamePair.second;

    std::cout << "Working on sample group: " << treename.Data() << std::endl;

    // Make a tmp list to eventually merge ttrees
    auto TreeList = new TList();
    std::map<TString,TTree*> TreeMap;

    // make tmp file to keep trees from being memory resident and also not in the output file
    // https://root-forum.cern.ch/t/merging-trees-with-ttree-mergetrees/24301
    const TString TreeFileName = Form("tmpfile_%s.root",fOutFileText.Data());
    auto TreeFile = TFile::Open(Form("%s",TreeFileName.Data()),"RECREATE");

    // Make skims...
    FastSkimmer::MakeSkimsFromEntryLists(TreeFile,TreeMap,TreeList,sample,treename);

    if (TreeList->GetEntries() != 0)
    {
      std::cout << "Merging skimmed trees..." << std::endl;
      
      // Now want to merge the list!
      fOutFile->cd();
      auto OutTree = TTree::MergeTrees(TreeList);
      
      // write out merged tree
      fOutFile->cd();
      OutTree->Write(OutTree->GetName(),TObject::kWriteDelete);

      // delete to reduce memory footprint
      delete OutTree;
    }

    // delete other stuff to reduce memory footprint
    delete TreeList;
    for (auto & TreePair : TreeMap)
    {
      delete TreePair.second;
    }

    // delete tmp file and remove it locally, too!
    delete TreeFile;
    gSystem->Exec(Form("rm %s",TreeFileName.Data()));
  } // end loop over sample groups
}

void FastSkimmer::MakeSkimsFromEntryLists(TFile *& TreeFile, std::map<TString,TTree*> & TreeMap, TList *& TreeList, const TString & sample, const TString & treename)
{
  // loop over all subsamples to avoid too many maps, simple control statement to control subsample grouping
  for (const auto & SamplePair : Common::SampleMap)
  {
    const auto & input = SamplePair.first;
    if (sample != SamplePair.second) continue;
  
    std::cout << "Working on input: " << input.Data() << std::endl;
    
    // Get File
    const TString infilename = Form("%s/%s/%s/%s",Common::eosDir.Data(),Common::baseDir.Data(),input.Data(),Common::tupleFileName.Data());
    auto infile = TFile::Open(Form("%s",infilename.Data()));
    Common::CheckValidFile(infile,infilename);
    
    // Get TTree
    auto intree = (TTree*)infile->Get(Form("%s",Common::disphotreename.Data()));
    Common::CheckValidTree(intree,Common::disphotreename,infilename);
    
    // Set Entry List Needed
    const auto samplename = Common::ReplaceSlashWithUnderscore(input);
    const auto & list = ListMap[samplename];
    intree->SetEntryList(list);
    intree->SetName(treename.Data());

    if (list->GetN() != 0)
    {
      std::cout << "Skimming into a smaller tree..." << std::endl;
    
      // Fill from tree into smaller tree
      TreeFile->cd();
      TreeMap[input] = intree->CopyTree("");
      
      // Add tree to list
      TreeList->Add(TreeMap[input]);
    }
    else
    {
      std::cout << "Skipping as no entries in this tree..." << std::endl;
    }
    
    // Delete everything you can
    delete intree;
    delete infile;
  }
}

void FastSkimmer::MakeConfigPave()
{
  std::cout << "Dumping config to a pave..." << std::endl;

  // create the pave
  fConfigPave = new TPaveText();
  fConfigPave->SetName(Form("%s",Common::pavename.Data()));
  std::string str; // tmp string
  
  // give grand title
  fConfigPave->AddText("***** FastSkimmer Config *****");

  // dump cut config
  fConfigPave->AddText(Form("FastSkimmer Cut Config: %s",fCutConfig.Data()));
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
  Common::SetupPrimaryDataset(fPDName);
  Common::SetupSamples();
  Common::SetupGroups();
  Common::SetupTreeNames();
  Common::SetupCuts(fCutConfig);
}

void FastSkimmer::SetupLists()
{
  // loop over all subsamples, create new list
  for (const auto & SamplePair : Common::SampleMap)
  {
    const auto & input    = SamplePair.first;
    const auto samplename = Common::ReplaceSlashWithUnderscore(input);
    ListMap[samplename]   = new TEntryList(Form("%s_EntryList",samplename.Data()),"EntryList");
  }
}
