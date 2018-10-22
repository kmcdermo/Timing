#include "FastSkimmer.hh"

FastSkimmer::FastSkimmer(const TString & cutflowconfig, const TString & pdname, const TString & inskimdir,
			 const TString & outtext, const Bool_t doskim, const TString & sampleconfig)
  : fCutFlowConfig(cutflowconfig), fPDName(pdname), fInSkimDir(inskimdir),
    fOutFileText(outtext), fDoSkim(doskim), fSampleConfig(sampleconfig)
{
  std::cout << "Initializing FastSkimmer..." << std::endl;

  ////////////////
  //            //
  // Initialize //
  //            //
  ////////////////

  // setup samples and such
  FastSkimmer::SetupCommon();
  FastSkimmer::SetupLists();

  // output root file containing every tree
  fOutFile = TFile::Open(Form("%s.root",fOutFileText.Data()),"UPDATE");
}

FastSkimmer::~FastSkimmer()
{
  // delete everything else already not deleted
  for (const auto & ListMapPair : ListMapMap) 
  {
    for (const auto & ListPair : ListMapPair.second) 
    {
      delete ListPair.second;
    }
  }
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
    const auto & input    = SamplePair.first;
    const auto samplename = Common::ReplaceSlashWithUnderscore(input);
    
    std::cout << "Working on sample name: " << samplename.Data() << std::endl;

    // Get File
    const TString filename = Form("%s/%s/%s/%s/%s",Common::eosDir.Data(),Common::baseDir.Data(),fInSkimDir.Data(),input.Data(),Common::tupleFileName.Data());
    auto file = TFile::Open(Form("%s",filename.Data()));
    Common::CheckValidFile(file,filename);
    file->cd();
	
    // Get TTree
    auto tree = (TTree*)file->Get(Form("%s",Common::disphotreename.Data()));
    Common::CheckValidTree(tree,Common::disphotreename,filename);

    // Get Input Cut Flow Histogram 
    auto inhist = (TH1F*)file->Get(Form("%s",Common::h_cutflow_scaledname.Data()));
    Common::CheckValidHist(inhist,Common::h_cutflow_scaledname,filename);

    // Init Output Cut Flow Histogram 
    std::map<TString,Int_t> binlabels;
    auto outhist = Common::SetupOutCutFlowHist(inhist,Common::SampleCutFlowHistNameMap[input],binlabels);

    // Set tmp evtwgt variables for filling cutflow histogram
    Float_t evtwgt = 0;
    TBranch * b_evtwgt = 0;
    tree->SetBranchAddress("evtwgt",&evtwgt,&b_evtwgt);

    // Loop over cuts, and make entry list for each cut
    for (const auto & CutFlowPair : Common::CutFlowPairVec)
    {
      const auto & label = CutFlowPair.first;
      const auto & cutstring = CutFlowPair.second;

      std::cout << "Computing entries for cut: " << label.Data() << std::endl;
      
      // Get entry list	
      file->cd();
      auto & list = ListMapMap[samplename][label];
      list->SetDirectory(file);

      // use ttree::draw() to generate entry list
      tree->Draw(Form(">>%s",list->GetName()),Form("%s",cutstring.Data()),"entrylist");

      // recursively set entry list for input tree
      tree->SetEntryList(list);
      
      // store result of number of entries into cutflow th1
      for (auto ientry = 0U; ientry < tree->GetEntries(); ientry++)
      {
	// from the wise words of philippe: https://root-forum.cern.ch/t/tentrylist-and-setentrylist-on-chain-not-registering/28286/6
	// and also: https://root-forum.cern.ch/t/ttree-loadtree/14566/6
	auto filteredEntry = tree->GetEntryNumber(ientry);
	if (filteredEntry < 0) break;
	auto localEntry = tree->LoadTree(filteredEntry);
	if (localEntry < 0) break;

	b_evtwgt->GetEntry(localEntry);
	outhist->Fill((binlabels[label]*1.f)-0.5f,evtwgt);
      }

      // Write out list
      fOutFile->cd();
      list->SetDirectory(fOutFile);
      list->SetTitle("EntryList");
      list->Write(list->GetName(),TObject::kWriteDelete);
    }

    // Write out hist
    fOutFile->cd();
    outhist->Write(outhist->GetName(),TObject::kWriteDelete);
    
    // delete everything
    delete outhist;
    delete inhist;
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

    // output cutflow histogram
    auto OutHist = FastSkimmer::SetupTotalCutFlowHist(sample);

    // Make skims...
    FastSkimmer::MakeSkimsFromEntryLists(TreeFile,TreeMap,TreeList,OutHist,sample,treename);

    // Now merge all the eligible skims
    if (fDoSkim && TreeList->GetEntries() != 0)
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

    // write out cutflow histogram (then delete it)
    fOutFile->cd();
    OutHist->Write(OutHist->GetName(),TObject::kWriteDelete);
    delete OutHist;

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

void FastSkimmer::MakeSkimsFromEntryLists(TFile *& TreeFile, std::map<TString,TTree*> & TreeMap, TList *& TreeList,
					  TH1F *& OutHist, const TString & sample, const TString & treename)
{
  // loop over all subsamples to avoid too many maps, simple control statement to control subsample grouping
  for (const auto & SamplePair : Common::SampleMap)
  {
    const auto & input = SamplePair.first;
    if (sample != SamplePair.second) continue;

    const auto samplename = Common::ReplaceSlashWithUnderscore(input);
    std::cout << "Working on sample name: " << samplename.Data() << std::endl;
    
    // Get File
    const TString infilename = Form("%s/%s/%s/%s/%s",Common::eosDir.Data(),Common::baseDir.Data(),fInSkimDir.Data(),input.Data(),Common::tupleFileName.Data());
    auto infile = TFile::Open(Form("%s",infilename.Data()));
    Common::CheckValidFile(infile,infilename);
    
    // Get TTree
    auto intree = (TTree*)infile->Get(Form("%s",Common::disphotreename.Data()));
    Common::CheckValidTree(intree,Common::disphotreename,infilename);
    
    // Get cutflow histogram and then add it up to total
    auto inhist = (TH1F*)fOutFile->Get(Form("%s",Common::SampleCutFlowHistNameMap[input].Data()));
    OutHist->Add(inhist);

    // do the skimming: otherwise skip this as we only care about adding histograms
    if (fDoSkim)
    {
      // Loop over lists of cuts, setting entrylist recursively in the same order cuts were originally computed!
      auto nentries = 0;
      for (const auto & CutFlowPair : Common::CutFlowPairVec)
      {
	const auto & label = CutFlowPair.first;
	const auto & list = ListMapMap[samplename][label];

	nentries = list->GetN();
	intree->SetEntryList(list);
      }

      // copy tree after final entry list set, only if it has more than 0 entries
      if (nentries != 0)
      {
	std::cout << "Skimming into a smaller tree..." << std::endl;

	// rename tree to output name to prevent double object
	// https://root-forum.cern.ch/t/merging-trees-with-ttree-mergetrees/24301
	intree->SetName(treename.Data());
	
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
    }
      
    // Delete everything you can
    delete inhist;
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
  
  // give grand title
  fConfigPave->AddText("***** FastSkimmer Config *****");

  // dump cut config
  Common::AddTextFromInputConfig(fConfigPave,"FastSkimmer CutFlow Config",fCutFlowConfig);

  // dump with PD
  fConfigPave->AddText(Form("Primary Dataset: %s",fPDName.Data()));

  // dump with optional select only certain samples
  fConfigPave->AddText(Form("Which samples to keep: %s",fSampleConfig.Data()));

  // dump with option to save skim (a bit redundant as it will be obvious with .ls once the file is attached
  fConfigPave->AddText(Form("Do skim bool: %s",Common::PrintBool(fDoSkim).Data()));

  // save to output file
  fOutFile->cd();
  fConfigPave->Write(fConfigPave->GetName(),TObject::kWriteDelete);
}

void FastSkimmer::SetupCommon()
{
  std::cout << "Setting up Common..." << std::endl;

  Common::SetupPrimaryDataset(fPDName);
  Common::SetupSamples();
  if (fSampleConfig != "")
  {
    Common::SetupWhichSamples(fSampleConfig,fSampleVec);
    Common::KeepOnlySamples(fSampleVec);
  }
  Common::SetupGroups();
  Common::SetupTreeNames();
  Common::SetupSampleCutFlowHistNames();
  Common::SetupGroupCutFlowHistNames();
  Common::SetupCutFlow(fCutFlowConfig);
}

void FastSkimmer::SetupLists()
{
  std::cout << "Setting up lists..." << std::endl;

  // loop over all subsamples, create new list
  for (const auto & SamplePair : Common::SampleMap)
  {
    const auto & input    = SamplePair.first;
    const auto samplename = Common::ReplaceSlashWithUnderscore(input);

    for (const auto & CutFlowPair : Common::CutFlowPairVec)
    {
      const auto & label = CutFlowPair.first;
    
      ListMapMap[samplename][label] = new TEntryList(Form("%s_%s_EntryList",samplename.Data(),label.Data()),"EntryList");
    }
  }
}

TH1F * FastSkimmer::SetupTotalCutFlowHist(const TString & sample)
{    
  // default constructor for output
  auto OutHist = new TH1F();

  OutHist->SetName(Form("%s",Common::GroupCutFlowHistNameMap[sample].Data()));
  OutHist->Sumw2();
  for (const auto & SamplePair : Common::SampleMap)
  {
    const auto & input = SamplePair.first;
    if (sample != SamplePair.second) continue;
    
    // get tmp hist info
    auto tmphist = (TH1F*)fOutFile->Get(Form("%s",Common::SampleCutFlowHistNameMap[input].Data()));
    const auto nbinsX = tmphist->GetXaxis()->GetNbins();
    
    // set OutHist Info (bins+titles)
    OutHist->SetTitle(tmphist->GetTitle());
    OutHist->GetXaxis()->SetTitle(tmphist->GetXaxis()->GetTitle());
    OutHist->GetYaxis()->SetTitle(tmphist->GetYaxis()->GetTitle());
    OutHist->SetBins(nbinsX,0,nbinsX*1.f);
    
    // set bin labels
    for (auto ibinX = 1; ibinX <= nbinsX; ibinX++)
    {
      OutHist->GetXaxis()->SetBinLabel(ibinX,tmphist->GetXaxis()->GetBinLabel(ibinX));
    }
    
    // delete temporary histogram
    delete tmphist;
    
    // only need first example
    break; 
  }

  return OutHist;
}
