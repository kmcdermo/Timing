#include "EntryListMaker.hh"

EntryListMaker::EntryListMaker(const TString & cutflowconfig, const TString & filename, const TString & grouplabel)
  : fCutFlowConfig(cutflowconfig), fFileName(filename), fGroupLabel(grouplabel)
{
  std::cout << "Initializing EntryListMaker..." << std::endl;

  ////////////////
  //            //
  // Initialize //
  //            //
  ////////////////

  // setup config
  Common::SetupCutFlow(fCutFlowConfig);

  // input/output root file containing every tree
  fFile = TFile::Open(Form("%s",fFileName.Data()),"UPDATE");
}

EntryListMaker::~EntryListMaker()
{
  // delete everything else already not deleted
  delete fConfigPave;
  delete fFile;
}

void EntryListMaker::MakeLists()
{
  // Make TEntryLists for each cut and save each list in order
  EntryListMaker::MakeListsFromTrees();

  // Write Out Config
  EntryListMaker::MakeConfigPave();
}

void EntryListMaker::MakeListsFromTrees()
{
  std::cout << "Making list of entries from trees in order of cut flow vector..." << std::endl;
  
  // loop over keys in file, only keeping trees
  TIter keyIter(fFile->GetListOfKeys());
  TKey * key = 0;
  while ((key = (TKey*)keyIter()))
  {
    // get name
    const TString & name = key->GetName();
    if (!name.Contains("_Tree",TString::kExact)) continue;
    
    std::cout << "Working on input: " << name.Data() << std::endl;

    // get tree
    auto tree = (TTree*)key->ReadObj();

    // Initialize map of lists
    std::map<TString,TEntryList*> listmap;
    EntryListMaker::InitListMap(listmap,name);

    // loop over cut flow
    for (const auto & CutFlowPair : Common::CutFlowPairVec)
    {
      // get cut info
      const auto & label = CutFlowPair.first;
      const auto & cut   = CutFlowPair.second;

      // Get entry list
      auto & list = listmap[label];
      list->SetDirectory(fFile);

      //////////////// **************** HACK FOR NOW!!!! **************** ////////////////
      if (label.Contains("HLT",TString::kExact) && name.EqualTo("GMSB_L200_CTau400")) continue;
    
      std::cout << "Computing entries for cut: " << label.Data() << std::endl;

      // use ttree::draw() to generate entry list
      tree->Draw(Form(">>%s",list->GetName()),Form("%s",cut.Data()),"entrylist");

      // recursively set entry list for input tree
      tree->SetEntryList(list);
    } // end loop over cuts

    // Write out lists
    fFile->cd();
    for (auto & listpair : listmap)
    {
      auto & list = listpair.second;
      list->SetDirectory(fFile);
      list->SetTitle(Form("%s_EntryList",fGroupLabel.Data()));
      list->Write(list->GetName(),TObject::kWriteDelete);
      delete list;
    }
    
    // delete input tree
    delete tree;
  } // end loop over input trees
}

void EntryListMaker::MakeConfigPave()
{
  std::cout << "Dumping config to a pave..." << std::endl;

  // create the pave
  fConfigPave = new TPaveText();
  fConfigPave->SetName(Form("%s_%s",fGroupLabel.Data(),Common::pavename.Data()));
  std::string str; // tmp string
  
  // give grand title
  fConfigPave->AddText("***** EntryListMaker Config *****");

  // dump cut config
  fConfigPave->AddText(Form("EntryListMaker Cut Config: %s",fCutFlowConfig.Data()));
  std::ifstream cutfile(Form("%s",fCutFlowConfig.Data()),std::ios::in);
  while (std::getline(cutfile,str))
  {
    fConfigPave->AddText(str.c_str());
  }

  // save to output file
  fFile->cd();
  fConfigPave->Write(fConfigPave->GetName(),TObject::kWriteDelete);
}

void EntryListMaker::InitListMap(std::map<TString,TEntryList*> & listmap, const TString & name)
{
  // loop over vector of cuts, make new list for each
  for (const auto & CutFlowPair : Common::CutFlowPairVec)
  {
    const auto & label  = CutFlowPair.first;
    
    // get name of sample
    TString sample = name;
    sample.ReplaceAll("_Tree","");
    
    listmap[label] = new TEntryList(Form("%s_%s_%s_EntryList",fGroupLabel.Data(),sample.Data(),label.Data()),"EntryList");
  }
}
