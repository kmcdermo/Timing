#include "SuperFastSkimmer.hh"

SuperFastSkimmer::SuperFastSkimmer(const TString & cutflowconfig, const TString & infilename, 
				   const Bool_t issignalfile, const TString & outtext)
  : fCutFlowConfig(cutflowconfig), fInFileName(infilename), 
    fIsSignalFile(issignalfile), fOutFileText(outtext)
{
  std::cout << "Initializing SuperFastSkimmer..." << std::endl;

  ////////////////
  //            //
  // Initialize //
  //            //
  ////////////////

  // get input file
  fInFile = TFile::Open(Form("%s",fInFileName.Data()));
  Common::CheckValidFile(fInFile,fInFileName);

  // setup samples and such
  SuperFastSkimmer::SetupCommon();

  // output root file containing every tree
  fOutFile = TFile::Open(Form("%s.root",fOutFileText.Data()),"UPDATE");
}

SuperFastSkimmer::~SuperFastSkimmer()
{
  // delete everything else already not deleted
  delete fConfigPave;
  delete fOutFile;

  // delete input
  delete fInFile;
}

void SuperFastSkimmer::MakeSkims()
{
  // Make TEntryLists for each cut, clone and save final tree
  SuperFastSkimmer::MakeSkimsFromTrees();

  // Write Out Config
  SuperFastSkimmer::MakeConfigPave();
}

void SuperFastSkimmer::MakeSkimsFromTrees()
{
  std::cout << "Skimming trees from cut flow vector..." << std::endl;

  for (const auto & GroupPair : Common::GroupMap)
  {
    // Init: get input/output names for hist and tree
    const auto & sample     = GroupPair.first;
    const auto & iotreename = Common::TreeNameMap[sample];
    const auto & iohistname = ((fIsSignalFile) ? Common::SignalCutFlowHistNameMap[sample] : Common::GroupCutFlowHistNameMap[sample]);

    std::cout << "Working on input: " << sample.Data() << std::endl;

    // Get Input TTree
    auto intree = (TTree*)fInFile->Get(Form("%s",iotreename.Data()));
    Common::CheckValidTree(intree,iotreename,fInFileName);

    // Get Input Cut Flow Histogram 
    auto inhist = (TH1F*)fInFile->Get(Form("%s",iohistname.Data()));
    Common::CheckValidHist(inhist,iohistname,fInFileName);

    // temporarily rename so as not to confuse things
    intree->SetName("tmpInTree");
    inhist->SetName("tmpInHist");

    // Init Output Cut Flow Histogram 
    std::map<TString,Int_t> binlabels;
    auto outhist = Common::SetupOutCutFlowHist(inhist,iohistname,binlabels);

    // Initialize map of lists
    std::map<TString,TEntryList*> listmap;
    fOutFile->cd();
    SuperFastSkimmer::InitListMap(listmap,sample);

    // Set tmp wgt for filling output histogram
    Float_t evtwgt = 0;
    TBranch * b_evtwgt = 0;
    intree->SetBranchAddress("evtwgt",&evtwgt,&b_evtwgt);

    // Loop over cuts, and make entry list for each cut, 
    for (const auto & CutFlowPair : Common::CutFlowPairVec)
    {
      // Get entry list
      const auto & label = CutFlowPair.first;
      auto & list = listmap[label];
      list->SetDirectory(fInFile);
      fInFile->cd();

      std::cout << "Computing entries for cut: " << label.Data() << std::endl;

      // get cut string
      const auto & cutstring = CutFlowPair.second;

      // use ttree::draw() to generate entry list
      intree->Draw(Form(">>%s",list->GetName()),Form("%s",cutstring.Data()),"entrylist");

      // store result of number of entries into cutflow th1
      for (auto ientry = 0U; ientry < intree->GetEntries(); ientry++)
      {
      	// from the wise words of philippe: https://root-forum.cern.ch/t/tentrylist-and-setentrylist-on-chain-not-registering/28286/6
      	// and also: https://root-forum.cern.ch/t/ttree-loadtree/14566/6
      	auto filteredEntry = intree->GetEntryNumber(ientry);
      	if (filteredEntry < 0) break;
      	auto localEntry = intree->LoadTree(filteredEntry);
      	if (localEntry < 0) break;
      
      	b_evtwgt->GetEntry(localEntry);
      	outhist->Fill((binlabels[label]*1.f)-0.5f,evtwgt);
      }

      // recursively set entry list for input tree
      intree->SetEntryList(list);
    } // end loop over cuts

    std::cout << "Writing out..." << std::endl;

    // Write out a copy of the last skim
    fOutFile->cd();
    auto outtree = intree->CopyTree("");
    outtree->SetName(Form("%s",iotreename.Data()));
    outtree->Write(outtree->GetName(),TObject::kWriteDelete);

    // Write out hist
    fOutFile->cd();
    outhist->Write(outhist->GetName(),TObject::kWriteDelete);

    // Write out lists
    fOutFile->cd();
    for (auto & listpair : listmap)
    {
      auto & list = listpair.second;
      list->SetDirectory(fOutFile);
      list->SetTitle("EntryList");
      list->Write(list->GetName(),TObject::kWriteDelete);
      delete list;
    }

    // delete everything
    delete outhist;
    delete outtree;
    delete inhist;
    delete intree;
  } // end loop over signal samples
}

void SuperFastSkimmer::MakeConfigPave()
{
  std::cout << "Dumping config to a pave..." << std::endl;

  // create the pave
  fConfigPave = new TPaveText();
  fConfigPave->SetName(Form("%s",Common::pavename.Data()));
  
  // give grand title
  fConfigPave->AddText("***** SuperFastSkimmer Config *****");

  // dump cut config
  Common::AddTextFromInputConfig(fConfigPave,"SuperFastSkimmer Cut Config",fCutFlowConfig);

  // padding
  Common::AddPaddingToPave(fConfigPave,3);

  // save name of infile
  fConfigPave->AddText(Form("InFile name: %s (isSignal : %s)",fInFileName.Data(),Common::PrintBool(fIsSignalFile).Data()));

  // dump in old config
  Common::AddTextFromInputPave(fConfigPave,fInFile);

  // save to output file
  fOutFile->cd();
  fConfigPave->Write(fConfigPave->GetName(),TObject::kWriteDelete);
}

void SuperFastSkimmer::SetupCommon()
{
  std::cout << "Setting up Common..." << std::endl;

  if (fIsSignalFile) Common::SetupSignalSamples();
  else               Common::SetupSamples();
  Common::SetupGroups();
  Common::SetupTreeNames();
  if (fIsSignalFile) Common::SetupSignalCutFlowHistNames();
  else               Common::SetupGroupCutFlowHistNames();
  Common::SetupCutFlow(fCutFlowConfig);
}

void SuperFastSkimmer::InitListMap(std::map<TString,TEntryList*> & listmap, const TString & sample)
{
  // loop over vector of cuts, make new list for each
  for (const auto & CutFlowPair : Common::CutFlowPairVec)
  {
    const auto & label  = CutFlowPair.first;
    listmap[label] = new TEntryList(Form("%s_%s_EntryList",sample.Data(),label.Data()),"EntryList");
  }
}
