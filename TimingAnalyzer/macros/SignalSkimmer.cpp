#include "SignalSkimmer.hh"

SignalSkimmer::SignalSkimmer(const TString & cutflowconfig, const TString & inskimdir, const TString & outtext)
  : fCutFlowConfig(cutflowconfig), fInSkimDir(inskimdir), fOutFileText(outtext)
{
  std::cout << "Initializing SignalSkimmer..." << std::endl;

  ////////////////
  //            //
  // Initialize //
  //            //
  ////////////////

  // setup samples and such
  SignalSkimmer::SetupCommon();

  // output root file containing every tree
  fOutFile = TFile::Open(Form("%s.root",fOutFileText.Data()),"UPDATE");
}

SignalSkimmer::~SignalSkimmer()
{
  // delete everything else already not deleted
  delete fConfigPave;
  delete fOutFile;
}

void SignalSkimmer::MakeSkims()
{
  // Make TEntryLists for each cut, clone and save final tree
  SignalSkimmer::MakeSkimsFromTrees();

  // Write Out Config
  SignalSkimmer::MakeConfigPave();
}

void SignalSkimmer::MakeSkimsFromTrees()
{
  std::cout << "Skimming trees from cut flow vector..." << std::endl;

  for (const auto & SamplePair : Common::SampleMap)
  {
    // Init
    const auto & input  = SamplePair.first;
    const auto & sample = SamplePair.second;
    std::cout << "Working on input: " << input.Data() << std::endl;

    // Get File
    const TString infilename = Form("%s/%s/%s/%s/%s",Common::eosDir.Data(),Common::baseDir.Data(),fInSkimDir.Data(),input.Data(),Common::tupleFileName.Data());
    auto infile = TFile::Open(Form("%s",infilename.Data()));
    Common::CheckValidFile(infile,infilename);
	
    // Get TTree
    auto intree = (TTree*)infile->Get(Form("%s",Common::disphotreename.Data()));
    Common::CheckValidTree(intree,Common::disphotreename,infilename);

    // Get Input Cut Flow Histogram 
    auto inhist = (TH1F*)infile->Get(Form("%s",Common::h_cutflow_scaledname.Data()));
    Common::CheckValidHist(inhist,Common::h_cutflow_scaledname,infilename);

    // Init Output Cut Flow Histogram 
    std::map<TString,Int_t> binlabels;
    auto outhist = Common::SetupOutCutFlowHist(inhist,Common::SignalCutFlowHistNameMap[sample],binlabels);

    // Initialize map of lists
    std::map<TString,TEntryList*> listmap;
    fOutFile->cd();
    SignalSkimmer::InitListMap(listmap,sample);

    // use evt wgt to fill cutflow hist
    Float_t evtwgt = 0;
    TBranch * b_evtwgt = 0;
    intree->SetBranchAddress("evtwgt",&evtwgt,&b_evtwgt);

    // Loop over cuts, and make entry list for each cut, 
    for (const auto & CutFlowPair : Common::CutFlowPairVec)
    {
      // Get entry list
      const auto & label = CutFlowPair.first;
      auto & list = listmap[label];
      list->SetDirectory(infile);
      infile->cd();

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
    outtree->SetName(Form("%s",Common::TreeNameMap[sample].Data()));
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
    delete infile;
  } // end loop over signal samples
}

void SignalSkimmer::MakeConfigPave()
{
  std::cout << "Dumping config to a pave..." << std::endl;

  // create the pave
  fConfigPave = new TPaveText();
  fConfigPave->SetName(Form("%s",Common::pavename.Data()));
  
  // give grand title
  fConfigPave->AddText("***** SignalSkimmer Config *****");

  // dump cut config
  Common::AddTextFromInputConfig(fConfigPave,"SignalSkimmer Cut Config",fCutFlowConfig);

  // save to output file
  fOutFile->cd();
  fConfigPave->Write(fConfigPave->GetName(),TObject::kWriteDelete);
}

void SignalSkimmer::SetupCommon()
{
  std::cout << "Setting up Common..." << std::endl;

  Common::SetupSignalSamples();
  Common::SetupGroups();
  Common::SetupTreeNames();
  Common::SetupSignalCutFlowHistNames();
  Common::SetupCutFlow(fCutFlowConfig);
}

void SignalSkimmer::InitListMap(std::map<TString,TEntryList*> & listmap, const TString & signal)
{
  // loop over vector of cuts, make new list for each
  for (const auto & CutFlowPair : Common::CutFlowPairVec)
  {
    const auto & label  = CutFlowPair.first;
    listmap[label] = new TEntryList(Form("%s_%s_EntryList",signal.Data(),label.Data()),"EntryList");
  }
}
