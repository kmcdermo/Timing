// Class include
#include "TimeAdjuster.hh"

TimeAdjuster::TimeAdjuster(const TString & skimfilename, const TString & signalskimfilename, const TString & infilesconfig,
			   const TString & sadjustvar, const TString & stime, const Bool_t doshift, const Bool_t dosmear)
  : fSkimFileName(skimfilename), fSignalSkimFileName(signalskimfilename), fInFilesConfig(infilesconfig),
    fSAdjustVar(sadjustvar), fSTime(stime), fDoShift(doshift), fDoSmear(dosmear)
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
  TimeAdjuster::SetupStrings();

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

  Common::DeleteMap(fInFileMap);

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
  //  if (fDoSmear) TimeAdjuster::GetInputSigmaFits(FitInfo);
  if (fDoSmear) TimeAdjuster::GetInputSigmaHists(FitInfo);
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
  //  tree->SetBranchAddress(Form("%s",ev.s_run.c_str()),&ev.run,&ev.b_run);
  tree->SetBranchAddress(Form("%s",ev.s_nphotons.c_str()),&ev.nphotons,&ev.b_nphotons);
  for (auto ipho = 0; ipho < Common::nPhotons; ipho++)
  {
    // get pho
    auto & pho = phos[ipho];

    // set old
    tree->SetBranchAddress(Form("%s_%i",fSAdjustVar.Data(),ipho),&pho.adjustvar,&pho.b_adjustvar);
    tree->SetBranchAddress(Form("%s_%i",pho.s_isEB.c_str(),ipho),&pho.isEB,&pho.b_isEB);

    // make new
    pho.b_timeSHIFT = tree->Branch(Form("%s_%i",fSTimeSHIFT.Data(),ipho),&pho.timeSHIFT,Form("%s_%i/F",fSTimeSHIFT.Data(),ipho));
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
    //    ev.b_run->GetEntry(entry);
    ev.b_nphotons->GetEntry(entry);

    // get era
    // const auto & era = std::find_if(Common::EraMap.begin(),Common::EraMap.end(),
    // 				    [=](const std::pair<TString,EraStruct> & EraPair)
    // 				    {
    // 				      return ( (EraPair.first != "Full") && ((ev.run >= EraPair.second.startRun) && (ev.run <= EraPair.second.endRun)) );
    // 				    })->first;
    const TString era = "Full";

    // loop over nphotons
    const auto nphos = std::min(ev.nphotons,Common::nPhotons);
    for (auto ipho = 0; ipho < nphos; ipho++)
    {
      auto & pho = phos[ipho];
      
      // get branch
      pho.b_adjustvar->GetEntry(entry);
      pho.b_isEB->GetEntry(entry);
      
      // get the correction
      const TString key = Form("%s_%s",(pho.isEB?"EB":"EE"),era.Data());
      const auto & hist = DataInfo.MuHistMap[key];
      
      // get bin which corresponds to the pt of the object
      const auto bin = hist->FindBin(pho.adjustvar);

      // set correction branch if bin is found, else no correction
      if (bin != 0 && bin != hist->GetXaxis()->GetNbins()+1)
      {
	const auto shift = hist->GetBinContent(bin);
	pho.timeSHIFT = -shift;
      }
      else
      {
	pho.timeSHIFT = 0.f;
      }
      
      // fill branch
      pho.b_timeSHIFT->Fill();
    } // end loop over nphotons on file

    // store remainder photons
    for (auto ipho = nphos; ipho < Common::nPhotons; ipho++)
    {
      auto & pho = phos[ipho];

      // set remainder
      pho.timeSHIFT = -9999.f;
      
      // fill branch
      pho.b_timeSHIFT->Fill();
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
	tree->SetBranchAddress(Form("%s_%i",fSAdjustVar.Data(),ipho),&pho.adjustvar,&pho.b_adjustvar);
	tree->SetBranchAddress(Form("%s_%i",pho.s_isEB.c_str(),ipho),&pho.isEB,&pho.b_isEB);

	// make new
	if (fDoShift) pho.b_timeSHIFT = tree->Branch(Form("%s_%i",fSTimeSHIFT.Data(),ipho),&pho.timeSHIFT,Form("%s_%i/F",fSTimeSHIFT.Data(),ipho));
	if (fDoSmear) pho.b_timeSMEAR = tree->Branch(Form("%s_%i",fSTimeSMEAR.Data(),ipho),&pho.timeSMEAR,Form("%s_%i/F",fSTimeSMEAR.Data(),ipho));
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
	  pho.b_adjustvar->GetEntry(entry);
	  pho.b_isEB->GetEntry(entry);
      
	  // era to use
	  const TString era = "Full";

	  // get the key
	  const TString key = Form("%s_%s",(pho.isEB?"EB":"EE"),era.Data());

	  // set shift correction branch
	  if (fDoShift)
	  {
	    // get the right hist
	    const auto & hist = MCInfo.MuHistMap[key];

	    // get the bin for the pt of the object
	    const auto bin = hist->FindBin(pho.adjustvar);
	    
	    // set correction branch if bin is found, else no correction
	    if (bin != 0 && bin != hist->GetXaxis()->GetNbins()+1)
	    {
	      const auto shift = hist->GetBinContent(bin);
	      pho.timeSHIFT = -shift;
	    }
	    else
	    {
	      pho.timeSHIFT = 0.f;
	    }
	  }

	  // set smear correction branch
	  if (fDoSmear)
	  {
	    // const auto & datafit = DataInfo.SigmaFitMap[key];
	    // const auto & mcfit   = MCInfo  .SigmaFitMap[key];
	    // const auto sigma     = std::sqrt(std::pow(datafit->Eval(pho.pt),2.f)-std::pow(mcfit->Eval(pho.pt),2.f));

	    // get the right hists
	    const auto & datahist = DataInfo.SigmaHistMap[key];
	    const auto & mchist   = MCInfo  .SigmaHistMap[key];

	    // get the right bins based on pt
	    const auto & databin = datahist->FindBin(pho.adjustvar);
	    const auto & mcbin   = mchist  ->FindBin(pho.adjustvar);
	    
	    // make smear if bins within range
	    if ( (databin != 0 && databin != datahist->GetXaxis()->GetNbins()+1) && (mcbin != 0 && mcbin != mchist->GetXaxis()->GetNbins()+1) )
	    {
	      const auto sigma  = std::sqrt(std::pow(datahist->GetBinContent(databin),2.f)-std::pow(mchist->GetBinContent(mcbin),2.f));
	      pho.timeSMEAR = rand->Gaus(0.f,sigma);
	    }
	    else
	    {
	      pho.timeSMEAR = 0.f;
	    }
	  }

	  // fill branches
	  if (fDoShift) pho.b_timeSHIFT->Fill();
	  if (fDoSmear) pho.b_timeSMEAR->Fill();
	} // end loop over nphotons on file

	// store remainder photons
	for (auto ipho = nphos; ipho < Common::nPhotons; ipho++)
	{
	  auto & pho = phos[ipho];

	  // set remainders
	  if (fDoShift) pho.timeSHIFT = -9999.f;
	  if (fDoSmear) pho.timeSMEAR = -9999.f;
      
	  // fill branches
	  if (fDoShift) pho.b_timeSHIFT->Fill();
	  if (fDoSmear) pho.b_timeSMEAR->Fill();
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

void TimeAdjuster::GetInputSigmaHists(FitStruct & FitInfo)
{
  const auto & label = FitInfo.label;
  std::cout << "Getting input sigma histograms for: " << label.Data() << std::endl;

  // get inputs
  const TString sigmahistname = label+"_sigma";
  auto & SigmaHistMap = FitInfo.SigmaHistMap;

  // loop over files, read in hists, then rename
  for (auto & InFilePair : fInFileMap)
  {
    const auto & key = InFilePair.first;
    auto & InFile = InFilePair.second;
    auto & SigmaHist = SigmaHistMap[key];

    // grab hist
    SigmaHist = (TH1F*)InFile->Get(sigmahistname.Data());

    // check to make sure ok
    Common::CheckValidHist(SigmaHist,sigmahistname,InFile->GetName());

    // rename
    SigmaHist->SetName(Form("%s_%s_Hist",sigmahistname.Data(),key.Data()));
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

  // Store which adjust variable and time used
  ConfigPave->AddText(Form("Adjust var used: %s",fSAdjustVar.Data()));
  ConfigPave->AddText(Form("Time used: %s",fSTime.Data()));

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

void TimeAdjuster::SetupStrings()
{
  if (fDoShift) fSTimeSHIFT = "pho"+fSTime+"SHIFT";
  if (fDoSmear) fSTimeSMEAR = "pho"+fSTime+"SMEAR";
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

  Common::DeleteMap(FitInfo.MuHistMap);
  Common::DeleteMap(FitInfo.SigmaHistMap);
  //  Common::DeleteMap(FitInfo.SigmaFitMap);
}
