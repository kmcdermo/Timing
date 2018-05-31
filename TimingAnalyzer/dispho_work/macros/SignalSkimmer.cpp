#include "SignalSkimmer.hh"

SignalSkimmer::SignalSkimmer(const TString & cutflowconfig, const TString & outtext)
  : fCutFlowConfig(cutflowconfig), fOutFileText(outtext)
{
  std::cout << "Initializing SignalSkimmer..." << std::endl;

  ////////////////
  //            //
  // Initialize //
  //            //
  ////////////////

  // setup samples and such
  SignalSkimmer::SetupConfig();

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
    const TString infilename = Form("%s/%s/%s/%s",Common::eosDir.Data(),Common::baseDir.Data(),input.Data(),Common::tupleFileName.Data());
    auto infile = TFile::Open(Form("%s",infilename.Data()));
    Common::CheckValidFile(infile,infilename);
	
    // Get TTree
    auto intree = (TTree*)infile->Get(Form("%s",Common::disphotreename.Data()));
    Common::CheckValidTree(intree,Common::disphotreename,infilename);
    intree->SetName(Form("%s",Common::TreeNameMap[sample].Data()));

    // Get Input Cut Flow Histogram 
    auto inhist = (TH1F*)infile->Get(Form("%s",Common::h_cutflowname.Data()));
    Common::CheckValidTH1F(inhist,Common::h_cutflowname,infilename);

    // Init Output Cut Flow Histogram 
    fOutFile->cd();
    std::map<TString,Int_t> binlabels;
    auto outhist = SignalSkimmer::InitOutCutFlowHist(inhist,Common::SignalCutFlowHistNameMap[sample],binlabels);

    // Initialize map of lists
    std::map<TString,TEntryList*> listmap;
    SignalSkimmer::InitListMap(listmap,sample);

    // Loop over cuts, and make entry list for each cut, 
    for (const auto & SignalCutFlowPair : Common::CutFlowPairVec)
    {
      // Get entry list
      const auto & label = SignalCutFlowPair.first;
      auto & list = listmap[label];
      list->SetDirectory(infile);

      //////////////// **************** HACK FOR NOW!!!! **************** ////////////////
      if (label.Contains("HLT",TString::kExact) && sample.EqualTo("GMSB_L200_CTau400")) continue;
    
      std::cout << "Computing entries for cut: " << label.Data() << std::endl;

      // use ttree::draw() to generate entry list
      intree->Draw(Form(">>%s",list->GetName()),Form("%s",SignalCutFlowPair.second.Data()),"entrylist");

      // store result of number of entries into cutflow th1
      outhist->SetBinContent(binlabels[label],list->GetN()*1.0);
      outhist->SetBinError  (binlabels[label],std::sqrt(list->GetN()*1.0));

      // recursively set entry list for input tree
      intree->SetEntryList(list);
    } // end loop over cuts

    std::cout << "Writing out..." << std::endl;

    // Write out a copy of the last skim
    fOutFile->cd();
    auto outtree = intree->CopyTree("");
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
  std::string str; // tmp string
  
  // give grand title
  fConfigPave->AddText("***** SignalSkimmer Config *****");

  // dump cut config
  fConfigPave->AddText(Form("SignalSkimmer Cut Config: %s",fCutFlowConfig.Data()));
  std::ifstream cutfile(Form("%s",fCutFlowConfig.Data()),std::ios::in);
  while (std::getline(cutfile,str))
  {
    fConfigPave->AddText(str.c_str());
  }

  // save to output file
  fOutFile->cd();
  fConfigPave->Write(fConfigPave->GetName(),TObject::kWriteDelete);
}

void SignalSkimmer::SetupConfig()
{
  Common::SetupSignalSamples();
  Common::SetupGroups();
  Common::SetupTreeNames();
  Common::SetupSignalCutFlowHistNames();
  Common::SetupCutFlow(fCutFlowConfig);
}

void SignalSkimmer::InitListMap(std::map<TString,TEntryList*> & listmap, const TString & signal)
{
  // loop over vector of cuts, make new list for each
  for (const auto & SignalCutFlowPair : Common::CutFlowPairVec)
  {
    const auto & label  = SignalCutFlowPair.first;
    listmap[label] = new TEntryList(Form("%s_%s_EntryList",signal.Data(),label.Data()),"EntryList");
  }
}

TH1F * SignalSkimmer::InitOutCutFlowHist(const TH1F * inhist, const TString & outname, std::map<TString,Int_t> & binlabels)
{
  // get cut flow labels
  const auto inNbinsX = inhist->GetNbinsX();
  for (auto ibin = 1; ibin <= inNbinsX; ibin++) binlabels[inhist->GetXaxis()->GetBinLabel(ibin)] = ibin;

  Int_t inNbinsX_new = inNbinsX;
  for (const auto & SignalCutFlowPair : Common::CutFlowPairVec) binlabels[SignalCutFlowPair.first] = ++inNbinsX_new;
  
  // make new cut flow
  auto outhist = new TH1F(outname.Data(),inhist->GetTitle(),binlabels.size(),0,binlabels.size());
  outhist->Sumw2();

  for (const auto & binlabel : binlabels)
  {
    const auto & cut = binlabel.first;
    const auto ibin = binlabel.second;

    outhist->GetXaxis()->SetBinLabel(ibin,cut.Data());

    if (ibin > inNbinsX) continue;

    outhist->SetBinContent(ibin,inhist->GetBinContent(ibin));
    outhist->SetBinError(ibin,inhist->GetBinError(ibin));
  }

  outhist->GetYaxis()->SetTitle(inhist->GetYaxis()->GetTitle());

  return outhist;
}
