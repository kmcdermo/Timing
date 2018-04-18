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
  delete fConfigPave;
  delete fOutFile;
}

void FastSkimmer::MakeSkim()
{
  // Clone TTrees Passing Cuts
  FastSkimmer::MakeSkimFromTrees();

  // Generate Merged Trees
  FastSkimmer::MergeSkims();

  // Write Out Config
  FastSkimmer::MakeConfigPave();
}

void FastSkimmer::MakeSkimFromTrees()
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

    std::cout << "Skimming into a smaller tree..." << std::endl;

    // Fill from tree into smaller tree
    fOutFile->cd();
    TreeMap[input] = tree->CopyTree(Form("%s",Config::CutMap[sample].Data()));
    
    // Add tree to list
    ListMap[sample]->Add(TreeMap[input]);

    delete tree;
    delete file;
  }
}

void FastSkimmer::MergeSkims()
{
  std::cout << "Merging skimmed trees for each sample..." << std::endl;

  // loop over all samples, merging subsamples and saving
  for (auto & ListPair : ListMap)
  {
    // merge trees for all subsamples in sample
    fOutFile->cd();
    TTree * outtree = TTree::MergeTrees(ListPair.second);
    outtree->SetName(Form("%s",Config::TreeNameMap[ListPair.first].Data()));

    // write out merged tree
    fOutFile->cd();
    outtree->Write(outtree->GetName(),TObject::kWriteDelete);
    
    // now start deleting stuff to reduce memory footprint
    delete outtree;
    delete ListPair.second;
    for (auto & TreePair : TreeMap)
    {
      if (Config::SampleMap[TreePair.first] != ListPair.first) continue;
      delete TreePair.second;
    }
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
  Config::SetupTreeNames();
  Config::SetupCuts(fCutConfig);
}

void FastSkimmer::SetupLists()
{
  // instantiate each list for each sample
  for (const auto & TreeNamePair : Config::TreeNameMap)
  {
    ListMap[TreeNamePair.first] = new TList();
  }
}
