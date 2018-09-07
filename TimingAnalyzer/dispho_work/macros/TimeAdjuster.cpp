// Class include
#include "TimeAdjuster.hh"

TimeAdjuster::TimeAdjuster(const TString & skimfilename, const TString & signalskimfilename, const TString & infilesconfig,
			   const Bool_t doshift, const Bool_t dosmear)
  : fSkimFileName(skimfilename), fSignalSkimFileName(signalskimfilename), fInFilesConfig(infilesconfig),
    fDoShift(doshift), fDoSmear(dosmear)
{
  std::cout << "Initializing TimeAdjuster..." << std::endl;

  ////////////////
  //            //
  // Initialize //
  //            //
  ////////////////

  // init configuration
  TimeAdjuster::SetupCommon();
  TimeAdjuster::SetupInFilesConfig();

  // open skim file
  fSkimFile = TFile::Open(Form("%s",fSkimFileName.Data()),"UPDATE");
  Common::CheckValidFile(fSkimFile,fSkimFileName);

  // open signal skim file
  fSignalSkimFile = TFile::Open(Form("%s",fSignalSkimFileName.Data()),"UPDATE");
  Common::CheckValidFile(fSignalSkimFile,fSignalSkimFileName);
}

TimeAdjuster::~TimeAdjuster()
{
  std::cout << "Tidying up in the destructor..." << std::endl;

  TimeAdjuster::DeleteMap(fInFileMap);

  delete fSignalSkimFile;
  delete fSkimFile;
}

void TimeAdjuster::AdjustTime()
{
  std::cout << "Adjusting time..." << std::endl;

  // prepare for time adjustments: data
  FitStruct DataInfo("Data");
  TimeAdjuster::PrepAdjustments(DataInfo);

  // prepare for time adjustments: mc
  FitStruct MCInfo("MC");
  TimeAdjuster::PrepAdjustments(MCInfo);

  // Correct data
  if (fDoShift) TimeAdjuster::CorrectData(DataInfo);

  // Correct MC
  if (fDoShift || fDoSmear) TimeAdjuster::CorrectMC(DataInfo,MCInfo);

  // dump meta info
  TimeAdjuster::MakeConfigPave(fSkimFile);
  TimeAdjuster::MakeConfigPave(fSignalSkimFile);

  // delete info
  TimeAdjuster::DeleteInfo(DataInfo);
  TimeAdjuster::DeleteInfo(MCInfo);
}

void TimeAdjuster::PrepAdjustments(FitStruct & FitInfo)
{
  const auto & label = FitInfo.label;
  std::cout << "Preparing time adjustments for: " << label.Data() << std::endl;

  // Get the input mu hists
  if (fDoShift) TimeAdjuster::GetInputMuHists(FitInfo);

  // Get input resolution fits
  if (fDoSmear) TimeAdjuster::GetInputSigmaFits(FitInfo);
}

void TimeAdjuster::CorrectData(FitStruct & DataInfo)
{
  std::cout << "Correcting data!" << std::endl;

  ////////////////////////////
  // Initialize corrections //
  ////////////////////////////

  // variables for looping
  Event ev;
  std::vector<Photon> phos(Common::nPhotons);

  // get tree
  fSkimFile->cd();
  auto tree = (TTree*)fSkimFile->Get(Form("%s",Common::TreeNameMap["Data"].Data()));
  
  // set input branches
  tree->SetBranchAddress(Form("%s",ev.s_run.c_str()),&ev.run,&ev.b_run);
  tree->SetBranchAddress(Form("%s",ev.s_nphotons.c_str()),&ev.nphotons,&ev.b_nphotons);
  for (auto ipho = 0; ipho < Common::nPhotons; ipho++)
  {
    // get pho
    auto & pho = phos[ipho];

    // set old
    tree->SetBranchAddress(Form("%s_%i",pho.s_pt.c_str(),ipho),&pho.pt,&pho.b_pt);
    tree->SetBranchAddress(Form("%s_%i",pho.s_isEB.c_str(),ipho),&pho.isEB,&pho.b_isEB);

    // make new
    pho.b_seedtimeSHIFT = tree->Branch(Form("%s_%i",pho.s_seedtimeSHIFT.c_str(),ipho),&pho.seedtimeSHIFT,Form("%s/F",pho.s_seedtimeSHIFT.c_str()));
  }

  /////////////////////////////
  // Get and set corrections //
  /////////////////////////////

  // loop over entries in tree
  const auto nEntries = tree->GetEntries();
  for (auto entry = 0U; entry < nEntries; entry++)
  {
    // dump status check
    if (entry%Common::nEvCheck == 0 || entry == 0) std::cout << "Processing Entry: " << entry << " out of " << nEntries << std::endl;

    // only need the entry for the single branches
    ev.b_run->GetEntry(entry);
    ev.b_nphotons->GetEntry(entry);

    // get era
    const auto & era = std::find_if(Common::EraMap.begin(),Common::EraMap.end(),
				    [=](const std::pair<TString,EraStruct> & EraPair)
				    {
				      return ( (EraPair.first != "Full") && ((ev.run >= EraPair.second.startRun) && (ev.run <= EraPair.second.endRun)) );
				    })->first;
    
    // loop over nphotons
    const auto nphos = std::min(ev.nphotons,Common::nPhotons);
    for (auto ipho = 0; ipho < nphos; ipho++)
    {
      auto & pho = phos[ipho];
      
      // get branch
      pho.b_pt->GetEntry(entry);
      pho.b_isEB->GetEntry(entry);
      
      // get the correction
      const TString key = Form("%s_%s",(pho.isEB?"EB":"EE"),era.Data());
      const auto & hist = DataInfo.MuHistMap[key];
      
      // set correction branch
      const auto shift = hist->GetBinContent(hist->FindBin(pho.pt));
      pho.seedtimeSHIFT = ((shift > 0.f) ? -shift : shift);

      // fill branch
      pho.b_seedtimeSHIFT->Fill();
    } // end loop over nphotons on file

    // store remainder photons
    for (auto ipho = nphos; ipho < Common::nPhotons; ipho++)
    {
      auto & pho = phos[ipho];

      // set remainder
      pho.seedtimeSHIFT = -9999.f;
      
      // fill branch
      pho.b_seedtimeSHIFT->Fill();
    } // end loop over remainder photons
    
  } // end loop over entries
  
  //////////////
  // Clean up //
  //////////////

  // overwrite tree
  tree->Write(tree->GetName(),TObject::kWriteDelete);
  
  // delete it all
  delete tree;
}

void TimeAdjuster::CorrectMC(FitStruct & DataInfo, FitStruct & MCInfo)
{
  std::cout << "Correcting MC!" << std::endl;

  //////////////////////////
  // Get random generator //
  //////////////////////////

  auto rand = new TRandomMixMax();

  ////////////////////////
  // Loop over MC trees //
  ////////////////////////

  for (const auto & TreeNamePair : Common::TreeNameMap)
  {
    // Init
    const auto & sample   = TreeNamePair.first;
    const auto & treename = TreeNamePair.second;
    
    // Skip data
    if (Common::GroupMap[sample] == SampleGroup::isData) continue;
    
    std::cout << "Working on tree: " << treename.Data() << std::endl;
	
    // Get infile
    auto & SkimFile = ((Common::GroupMap[sample] != SampleGroup::isSignal) ? fSkimFile : fSignalSkimFile);
    SkimFile->cd();

    // Get tree
    auto tree = (TTree*)SkimFile->Get(Form("%s",treename.Data()));
    const auto isnull = Common::IsNullTree(tree);

    if (!isnull)
    {
      ////////////////////////////
      // Initialize corrections //
      ////////////////////////////

      // variables for looping
      Event ev;
      std::vector<Photon> phos(Common::nPhotons);
  
      // set input branches
      tree->SetBranchAddress(Form("%s",ev.s_nphotons.c_str()),&ev.nphotons,&ev.b_nphotons);
      for (auto ipho = 0; ipho < Common::nPhotons; ipho++)
      {
	// get pho
	auto & pho = phos[ipho];
	
	// set old
	tree->SetBranchAddress(Form("%s_%i",pho.s_pt.c_str(),ipho),&pho.pt,&pho.b_pt);
	tree->SetBranchAddress(Form("%s_%i",pho.s_isEB.c_str(),ipho),&pho.isEB,&pho.b_isEB);

	// make new
	if (fDoShift) pho.b_seedtimeSHIFT = tree->Branch(Form("%s_%i",pho.s_seedtimeSHIFT.c_str(),ipho),&pho.seedtimeSHIFT,Form("%s/F",pho.s_seedtimeSHIFT.c_str()));
	if (fDoSmear) pho.b_seedtimeSMEAR = tree->Branch(Form("%s_%i",pho.s_seedtimeSMEAR.c_str(),ipho),&pho.seedtimeSMEAR,Form("%s/F",pho.s_seedtimeSMEAR.c_str()));
      }

      /////////////////////////////
      // Get and set corrections //
      /////////////////////////////

      // loop over entries in tree
      const auto nEntries = tree->GetEntries();
      for (auto entry = 0U; entry < nEntries; entry++)
      {
	// dump status check
	if (entry%Common::nEvCheck == 0 || entry == 0) std::cout << "Processing Entry: " << entry << " out of " << nEntries << std::endl;
	
	// only need the entry for the single branches
	ev.b_nphotons->GetEntry(entry);
    
	// loop over nphotons
	const auto nphos = std::min(ev.nphotons,Common::nPhotons);
	for (auto ipho = 0; ipho < nphos; ipho++)
        {
	  auto & pho = phos[ipho];
	  
	  // get branch
	  pho.b_pt->GetEntry(entry);
	  pho.b_isEB->GetEntry(entry);
      
	  // get the key
	  const TString key = Form("%s_Full",(pho.isEB?"EB":"EE"));

	  // set shift correction branch
	  if (fDoShift)
	  {
	    const auto & hist = MCInfo  .MuHistMap  [key];
	    const auto shift  = hist->GetBinContent(hist->FindBin(pho.pt));
	    pho.seedtimeSHIFT = ((shift > 0.f) ? -shift : shift);
	  }

	  // set smear correction branch
	  if (fDoSmear)
	  {
	    const auto & datafit = DataInfo.SigmaFitMap[key];
	    const auto & mcfit   = MCInfo  .SigmaFitMap[key];
	    const auto sigma     = std::sqrt(std::pow(datafit->Eval(pho.pt),2.f)-std::pow(mcfit->Eval(pho.pt),2.f));
	    pho.seedtimeSMEAR    = rand->Gaus(0.f,sigma);
	  }

	  // fill branches
	  if (fDoShift) pho.b_seedtimeSHIFT->Fill();
	  if (fDoSmear) pho.b_seedtimeSMEAR->Fill();
	} // end loop over nphotons on file

	// store remainder photons
	for (auto ipho = nphos; ipho < Common::nPhotons; ipho++)
	{
	  auto & pho = phos[ipho];

	  // set remainders
	  if (fDoShift) pho.seedtimeSHIFT = -9999.f;
	  if (fDoSmear) pho.seedtimeSMEAR = -9999.f;
      
	  // fill branches
	  if (fDoShift) pho.b_seedtimeSHIFT->Fill();
	  if (fDoSmear) pho.b_seedtimeSMEAR->Fill();
	} // end loop over remainder photons
    
      } // end loop over entries
  
      //////////////
      // Clean up //
      //////////////
      
      // overwrite tree
      tree->Write(tree->GetName(),TObject::kWriteDelete);
  
      // delete it all
      delete tree;

    } // tree is valid
    else
    {
      std::cout << "Skipping null tree..." << std::endl;
    } // isnull tree

  } // end loop over tree names
}

void TimeAdjuster::GetInputMuHists(FitStruct & FitInfo)
{
  const auto & label = FitInfo.label;
  std::cout << "Getting input mu histograms for: " << label.Data() << std::endl;

  // get inputs
  const TString muhistname = label+"_mu";
  auto & MuHistMap = FitInfo.MuHistMap;

  // loop over files, read in hists, then rename
  for (auto & InFilePair : fInFileMap)
  {
    const auto & key = InFilePair.first;
    auto & InFile = InFilePair.second;
    auto & MuHist = MuHistMap[key];

    // grab hist
    MuHist = (TH1F*)InFile->Get(muhistname.Data());

    // check to make sure ok
    Common::CheckValidHist(MuHist,muhistname,InFile->GetName());

    // rename
    MuHist->SetName(Form("%s_%s_Hist",muhistname.Data(),key.Data()));
  }
}

void TimeAdjuster::GetInputSigmaFits(FitStruct & FitInfo)
{
  const auto & label = FitInfo.label;
  std::cout << "Getting sigma fits for: " << label.Data() << std::endl;
  
  // get inputs/outputs
  const TString sigmafitname = label+"_sigma_fit";
  auto & SigmaFitMap = FitInfo.SigmaFitMap;

  // loop over sigma hists and fit!
  for (auto & InFilePair : fInFileMap)
  {
    const auto & key = InFilePair.first;
    auto & InFile = InFilePair.second;
    auto & SigmaFit = SigmaFitMap[key];

    // grab fit
    SigmaFit = (TF1*)InFile->Get(sigmafitname.Data());

    // check to make sure ok
    Common::CheckValidF1(SigmaFit,sigmafitname,InFile->GetName());

    // rename
    SigmaFit->SetName(Form("%s_%s_Fit",sigmafitname.Data(),key.Data()));
  }
}

void TimeAdjuster::MakeConfigPave(TFile *& SkimFile)
{
  std::cout << "Dumping config to a pave for: " << SkimFile->GetName() << std::endl;

  // create the pave, copying in old info
  SkimFile->cd();
  auto ConfigPave = (TPaveText*)SkimFile->Get(Form("%s",Common::pavename.Data()));

  // add some padding to new stuff
  Common::AddPaddingToPave(ConfigPave,3);

  // give grand title
  ConfigPave->AddText("***** TimeAdjuster Config *****");
  
  // store which correction performed
  ConfigPave->AddText(Form("DoShift: %s",Common::PrintBool(fDoShift).Data()));
  ConfigPave->AddText(Form("DoSmear: %s",Common::PrintBool(fDoSmear).Data()));

  // list of inputs
  Common::AddTextFromInputConfig(ConfigPave,"InFiles Config",fInFilesConfig);

  // padding
  Common::AddPaddingToPave(ConfigPave,3);

  // dump in old configs... will be a lot
  for (auto & InFilePair : fInFileMap)
  {
    const auto & key = InFilePair.first;
    auto & file = InFilePair.second;

    // save input file name
    ConfigPave->AddText(Form("InFile, key: %s, name: %s",key.Data(),file->GetName())); 
    Common::AddTextFromInputPave(ConfigPave,file);

    // padding
    Common::AddPaddingToPave(ConfigPave,3);
  }

  // save to output file
  SkimFile->cd();
  ConfigPave->Write(ConfigPave->GetName(),TObject::kWriteDelete);

  // delete pave
  delete ConfigPave;
}

void TimeAdjuster::SetupCommon()
{
  std::cout << "Setting up Common..." << std::endl;

  Common::SetupEras();
  Common::SetupSamples();
  Common::SetupSignalSamples();
  Common::SetupGroups();
  Common::SetupTreeNames();
}

void TimeAdjuster::SetupInFilesConfig()
{
  std::cout << "Setting up in files..." << std::endl;

  // read in filenames
  std::ifstream infile(Form("%s",fInFilesConfig.Data()),std::ios::in);
  TString line = "";
  while (infile >> line)
  {
    if (line.EqualTo("",TString::kExact)) continue; 
    
    // dissect the file name
    const TString match = "=";
    auto middle = line.Index(match);
    
    const TString key(line(0,middle)); // [EB/EE]_[2017B-F/Full] --> eta_era
    const TString infilename(line(middle+1,line.Length()-middle-1));

    fInFileNameMap[key] = infilename;
  }

  // now open the files!
  for (const auto & InFileNamePair : fInFileNameMap)
  {
    const auto & key      = InFileNamePair.first;
    const auto & filename = InFileNamePair.second;

    fInFileMap[key] = TFile::Open(filename.Data());
    Common::CheckValidFile(fInFileMap[key],filename);
  }
}

void TimeAdjuster::DeleteInfo(FitStruct & FitInfo)
{
  const auto & label = FitInfo.label;
  std::cout << "Deleting info for: " << label.Data() << std::endl;

  TimeAdjuster::DeleteMap(FitInfo.MuHistMap);
  TimeAdjuster::DeleteMap(FitInfo.SigmaFitMap);
}

template <typename T>
void TimeAdjuster::DeleteMap(T & Map)
{
  for (auto & Pair : Map) delete Pair.second;
}
