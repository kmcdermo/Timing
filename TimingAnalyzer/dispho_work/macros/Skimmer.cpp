#include "Skimmer.hh"
#include "TROOT.h"

#include <iostream>

Skimmer::Skimmer(const TString & indir, const TString & outdir, const TString & filename, const Float_t sumwgts, const Bool_t redophoid) :
  fInDir(indir), fOutDir(outdir), fFileName(filename), fSumWgts(sumwgts), fRedoPhoID(redophoid)
{
  // because root is dumb?
  gROOT->ProcessLine("#include <vector>");

  ////////////////////////
  // Get all the inputs //
  ////////////////////////
  std::cout << "Setting up inputs for skim" << std::endl;

  // Get input file
  const TString infilename = Form("%s/%s", fInDir.Data(), fFileName.Data());
  fInFile = TFile::Open(infilename.Data());
  Config::CheckValidFile(fInFile,infilename);

  // Get input config tree + sample weight
  const TString inconfigtreename = Form("%s%s",Config::rootdir.Data(),Config::configtreename.Data());
  fInConfigTree = (TTree*)fInFile->Get(inconfigtreename.Data());
  Config::CheckValidTree(fInConfigTree,inconfigtreename,infilename);
  Skimmer::GetInConfig();
  if (fIsMC) Skimmer::GetSampleWeight();

  // Get main input tree and initialize it
  const TString indisphotreename = Form("%s%s",Config::rootdir.Data(),Config::disphotreename.Data());
  fInTree = (TTree*)fInFile->Get(indisphotreename.Data());
  Config::CheckValidTree(fInTree,indisphotreename,infilename);
  Skimmer::InitInTree();

  // Get the cut flow + event weight histogram --> set the wgtsum
  const TString inh_cutflowname = Form("%s%s",Config::rootdir.Data(),Config::h_cutflowname.Data());
  fInCutFlow = (TH1F*)fInFile->Get(inh_cutflowname.Data());
  Config::CheckValidTH1F(fInCutFlow,inh_cutflowname,infilename);

  // Get PU weights input
  if (fIsMC)
  {
    const TString pufilename = Form("%s/%s/%s.root",Config::eosDir.Data(),Config::baseDir.Data(),Config::puwgtFileName.Data());
    fInPUWgtFile = TFile::Open(pufilename.Data());
    Config::CheckValidFile(fInPUWgtFile,pufilename);
    fInPUWgtHist = (TH1F*)fInPUWgtFile->Get(Config::puwgtHistName.Data());
    Config::CheckValidTH1F(fInPUWgtHist,Config::puwgtHistName,pufilename);
    Skimmer::GetPUWeights();
  }

  /////////////////////////
  // Set all the outputs //
  /////////////////////////
  std::cout << "Setting up output skim" << std::endl;

  // Make the output file, make trees, then init them
  fOutFile = TFile::Open(Form("%s/%s", fOutDir.Data(), fFileName.Data()),"recreate");
  fOutFile->cd();
  
  fOutConfigTree = new TTree(Config::configtreename.Data(),Config::configtreename.Data());
  fOutTree = new TTree(Config::disphotreename.Data(),Config::disphotreename.Data());

  // Init output info
  Skimmer::InitAndSetOutConfig();
  Skimmer::InitOutTree();
  Skimmer::InitOutCutFlow();
}

Skimmer::~Skimmer()
{
  if (fIsMC)
  {
    fPUWeights.clear();
    delete fInPUWgtHist;
    delete fInPUWgtFile;
  }

  delete fInCutFlow;
  delete fInTree;
  delete fInConfigTree;
  delete fInFile;

  delete fOutCutFlow;
  delete fOutTree;
  delete fOutConfigTree;
  delete fOutFile;
}

void Skimmer::EventLoop()
{
  // do loop over events, filling histos
  const UInt_t nEntries = fInTree->GetEntries();
  for (UInt_t entry = 0; entry < nEntries; entry++)
  {
    // read in tree
    fInTree->GetEntry(entry);

    // dump status check
    if (entry%Config::nEvCheck == 0 || entry == 0) std::cout << "Processing Entry: " << entry << " out of " << nEntries << std::endl;
    
    // get event weight: no scaling by BR, xsec, lumi, etc.
    const Float_t evtwgt = (fIsMC ? fInEvent.genwgt : 1.f);

    // perform skim
    if (!fInConfig.isToy) // do not apply skim selection on toy config
    {
      if (fInEvent.nphotons <= 0) continue;
      if (!fInPhos[0].isEB) continue;
      if (fInPhos[0].pt < 70.f) continue;
      
      // fill cutflow
      fOutCutFlow->Fill((cutLabels["skim"]*1.f)-0.5f,evtwgt);
      
      // filter on MET Flags
      if (!fInEvent.metPV || !fInEvent.metBeamHalo || !fInEvent.metHBHENoise || !fInEvent.metHBHEisoNoise || 
	  !fInEvent.metECALTP || !fInEvent.metPFMuon || !fInEvent.metPFChgHad) continue;
      if (!fInConfig.isGMSB && !fInEvent.metECALCalib) continue;
      if (!fIsMC && !fInEvent.metEESC) continue;
      
      // fill cutflow
      fOutCutFlow->Fill((cutLabels["METFlag"]*1.f)-0.5f,evtwgt);
      
      // cut on crappy pileup... eventually genputrue
      if (fIsMC && ((fInEvent.nvtx < 0) || (UInt_t(fInEvent.nvtx) >= fPUWeights.size()))) continue;
      
      // fill cutflow
      fOutCutFlow->Fill((cutLabels["badPU"]*1.f)-0.5f,evtwgt);
    }

    // end of skim, now copy... dropping rechits
    if (fOutConfig.isGMSB) Skimmer::FillOutGMSBs();
    if (fOutConfig.isHVDS) Skimmer::FillOutHVDSs();
    if (fOutConfig.isToy)  Skimmer::FillOutToys();
    Skimmer::FillOutEvent();
    Skimmer::FillOutJets();
    Skimmer::FillOutPhos();

    // fill the tree
    fOutTree->Fill();
  } // end loop over events

  // write out the output!
  fOutFile->cd();
  fOutCutFlow->Write();
  fOutConfigTree->Write();
  fOutTree->Write();
}

void Skimmer::FillOutGMSBs()
{
  for (Int_t igmsb = 0; igmsb < Config::nGMSBs; igmsb++)
  {
    const auto & ingmsb = fInGMSBs[igmsb];
    auto & outgmsb = fOutGMSBs[igmsb];

    outgmsb.genNmass = ingmsb.genNmass;
    outgmsb.genNE = ingmsb.genNE;
    outgmsb.genNpt = ingmsb.genNpt;
    outgmsb.genNphi = ingmsb.genNphi;
    outgmsb.genNeta = ingmsb.genNeta;
    outgmsb.genNprodvx = ingmsb.genNprodvx;
    outgmsb.genNprodvy = ingmsb.genNprodvy;
    outgmsb.genNprodvz = ingmsb.genNprodvz;
    outgmsb.genNdecayvx = ingmsb.genNdecayvx;
    outgmsb.genNdecayvy = ingmsb.genNdecayvy;
    outgmsb.genNdecayvz = ingmsb.genNdecayvz;
    outgmsb.genphE = ingmsb.genphE;
    outgmsb.genphpt = ingmsb.genphpt;
    outgmsb.genphphi = ingmsb.genphphi;
    outgmsb.genpheta = ingmsb.genpheta;
    outgmsb.genphmatch = ingmsb.genphmatch;
    outgmsb.gengrmass = ingmsb.gengrmass;
    outgmsb.gengrE = ingmsb.gengrE;
    outgmsb.gengrpt = ingmsb.gengrpt;
    outgmsb.gengrphi = ingmsb.gengrphi;
    outgmsb.gengreta = ingmsb.gengreta;
  }
}

void Skimmer::FillOutHVDSs()
{
  for (Int_t ihvds = 0; ihvds < Config::nHVDSs; ihvds++)
  {
    const auto & inhvds = fInHVDSs[ihvds];
    auto & outhvds = fOutHVDSs[ihvds];

    outhvds.genvPionmass = inhvds.genvPionmass;
    outhvds.genvPionE = inhvds.genvPionE;
    outhvds.genvPionpt = inhvds.genvPionpt;
    outhvds.genvPionphi = inhvds.genvPionphi;
    outhvds.genvPioneta = inhvds.genvPioneta;
    outhvds.genvPionprodvx = inhvds.genvPionprodvx;
    outhvds.genvPionprodvy = inhvds.genvPionprodvy;
    outhvds.genvPionprodvz = inhvds.genvPionprodvz;
    outhvds.genvPiondecayvx = inhvds.genvPiondecayvx;
    outhvds.genvPiondecayvy = inhvds.genvPiondecayvy;
    outhvds.genvPiondecayvz = inhvds.genvPiondecayvz;
    outhvds.genHVph0E = inhvds.genHVph0E;
    outhvds.genHVph0pt = inhvds.genHVph0pt;
    outhvds.genHVph0phi = inhvds.genHVph0phi;
    outhvds.genHVph0eta = inhvds.genHVph0eta;
    outhvds.genHVph0match = inhvds.genHVph0match;
    outhvds.genHVph1E = inhvds.genHVph1E;
    outhvds.genHVph1pt = inhvds.genHVph1pt;
    outhvds.genHVph1phi = inhvds.genHVph1phi;
    outhvds.genHVph1eta = inhvds.genHVph1eta;
    outhvds.genHVph1match = inhvds.genHVph1match;
  }
}

void Skimmer::FillOutToys()
{
  for (Int_t itoy = 0; itoy < Config::nToys; itoy++)
  {
    const auto & intoy = fInToys[itoy];
    auto & outtoy = fOutToys[itoy];

    outtoy.genphE = intoy.genphE;
    outtoy.genphpt = intoy.genphpt;
    outtoy.genphphi = intoy.genphphi;
    outtoy.genpheta = intoy.genpheta;

    outtoy.genphmatch = intoy.genphmatch;
    outtoy.genphmatch_ptres = intoy.genphmatch_ptres;
    outtoy.genphmatch_status = intoy.genphmatch_status;
  }
}

void Skimmer::FillOutEvent()
{
  fOutEvent.run = fInEvent.run;
  fOutEvent.lumi = fInEvent.lumi;
  fOutEvent.event = fInEvent.event;
  fOutEvent.hltSignal = fInEvent.hltSignal;
  fOutEvent.hltRefPhoID = fInEvent.hltRefPhoID;
  fOutEvent.hltRefDispID = fInEvent.hltRefDispID;
  fOutEvent.hltRefHT = fInEvent.hltRefHT;
  fOutEvent.hltPho50 = fInEvent.hltPho50;
  fOutEvent.hltPho200 = fInEvent.hltPho200;
  fOutEvent.hltDiPho70 = fInEvent.hltDiPho70;
  fOutEvent.hltDiPho3022M90 = fInEvent.hltDiPho3022M90;
  fOutEvent.hltDiPho30PV18PV = fInEvent.hltDiPho30PV18PV;
  fOutEvent.hltDiEle33MW = fInEvent.hltDiEle33MW;
  fOutEvent.hltDiEle27WPT = fInEvent.hltDiEle27WPT;
  fOutEvent.hltJet500 = fInEvent.hltJet500;
  fOutEvent.nvtx = fInEvent.nvtx;
  fOutEvent.vtxX = fInEvent.vtxX;
  fOutEvent.vtxY = fInEvent.vtxY;
  fOutEvent.vtxZ = fInEvent.vtxZ;
  fOutEvent.rho = fInEvent.rho;
  fOutEvent.t1pfMETpt = fInEvent.t1pfMETpt;
  fOutEvent.t1pfMETphi = fInEvent.t1pfMETphi;
  fOutEvent.t1pfMETsumEt = fInEvent.t1pfMETsumEt;
  fOutEvent.jetHT = fInEvent.jetHT;
  fOutEvent.njets = fInEvent.njets;
  fOutEvent.jetHTpt15 = fInEvent.jetHTpt15;
  fOutEvent.njetspt15 = fInEvent.njetspt15;
  fOutEvent.jetHTeta3 = fInEvent.jetHTeta3;
  fOutEvent.njetseta3 = fInEvent.njetseta3;
  fOutEvent.jetHTidL = fInEvent.jetHTidL;
  fOutEvent.njetsidL = fInEvent.njetsidL;
  fOutEvent.jetHTnopho = fInEvent.jetHTnopho;
  fOutEvent.njetsnopho = fInEvent.njetsnopho;
  fOutEvent.jetHTidT = fInEvent.jetHTidT;
  fOutEvent.njetsidT = fInEvent.njetsidT;
  fOutEvent.nrechits = fInEvent.nrechits;
  fOutEvent.nphotons = fInEvent.nphotons;
  fOutEvent.evtwgt   = (fIsMC ? fSampleWeight * fInEvent.genwgt : 1.f);

  if (fIsMC)
  {
    fOutEvent.genx0 = fInEvent.genx0;
    fOutEvent.geny0 = fInEvent.geny0;
    fOutEvent.genz0 = fInEvent.genz0;
    fOutEvent.gent0 = fInEvent.gent0;
    fOutEvent.puwgt = fPUWeights[fInEvent.nvtx]; // == 0 for tmp skims --> genputrue eventually
    if (fOutConfig.isGMSB)
    {
      fOutEvent.nNeutoPhGr = fInEvent.nNeutoPhGr;
    }
    if (fOutConfig.isHVDS)
    {
      fOutEvent.nvPions = fInEvent.nvPions;
    }
    if (fOutConfig.isToy)
    {
      fOutEvent.nToyPhs = fInEvent.nToyPhs;
    }
  }
}

void Skimmer::FillOutJets()
{
  // Jets Info
  for (Int_t ijet = 0; ijet < Config::nJets; ijet++) 
  {
    const auto & injet = fInJets[ijet];
    auto & outjet = fOutJets[ijet];
    
    outjet.E = injet.E;
    outjet.pt = injet.pt;
    outjet.eta = injet.eta;
    outjet.phi = injet.phi;
  }
}

void Skimmer::FillOutPhos()
{  
  // Photon Info
  for (Int_t ipho = 0; ipho < Config::nPhotons; ipho++) 
  {
    const auto & inpho = fInPhos[ipho];
    auto & outpho = fOutPhos[ipho];
    
    outpho.E = inpho.E;
    outpho.pt = inpho.pt;
    outpho.eta = inpho.eta;
    outpho.phi = inpho.phi;
    outpho.scE = inpho.scE;
    outpho.sceta = inpho.sceta;
    outpho.scphi = inpho.scphi;
    outpho.HoE = inpho.HoE;
    outpho.r9 = inpho.r9;
    outpho.ChgHadIso = inpho.ChgHadIso;
    outpho.NeuHadIso = inpho.NeuHadIso;
    outpho.PhoIso = inpho.PhoIso;
    outpho.EcalPFClIso = inpho.EcalPFClIso;
    outpho.HcalPFClIso = inpho.HcalPFClIso;
    outpho.TrkIso = inpho.TrkIso;
    outpho.sieie = inpho.sieie;
    outpho.sipip = inpho.sipip;
    outpho.sieip = inpho.sieip;
    outpho.smaj = inpho.smaj;
    outpho.smin = inpho.smin;
    outpho.alpha = inpho.alpha;
    //    outpho.suisseX = inpho.suisseX;
    outpho.isOOT = inpho.isOOT;
    outpho.isEB = inpho.isEB;
    outpho.isHLT = inpho.isHLT;
    outpho.isTrk = inpho.isTrk;
    outpho.passEleVeto = inpho.passEleVeto;
    outpho.hasPixSeed = inpho.hasPixSeed;

    if (fRedoPhoID)
    {
      inpho.gedID >= 0 ? Skimmer::GetGEDPhoVID(outpho) : -1;
      inpho.ootID >= 0 ? Skimmer::GetOOTPhoVID(outpho) : -1;
    }
    else
    {
      outpho.gedID = inpho.gedID;
      outpho.ootID = inpho.ootID;
    }

    if (fInConfig.storeRecHits)
    {
      if (inpho.seed >= 0)
      {
	outpho.seedtime = (*fInRecHits.time)[inpho.seed];
	outpho.seedE    = (*fInRecHits.E)   [inpho.seed];
	outpho.seedID   = (*fInRecHits.ID)  [inpho.seed];
      }
      else
      {
	outpho.seedtime = -9999.f;
	outpho.seedE    = -9999.f;
	outpho.seedID   = 0;
      }
    }
    else
    {
      outpho.seedtime = inpho.seedtime;
      outpho.seedE    = inpho.seedE;
      outpho.seedID   = inpho.seedID;
    }
    
    if (fIsMC)
    {
      outpho.isGen = inpho.isGen;
      if (fOutConfig.isGMSB || fOutConfig.isHVDS)
      {
	outpho.isSignal = inpho.isSignal;
      }
    }
  }
}

void Skimmer::GetInConfig()
{
  // Get Input Config
  Skimmer::InitInConfigStrings();
  Skimmer::InitInConfigBranches();

  // read in first entry (will be the same for all entries in a given file)
  fInConfigTree->GetEntry(0);

  // set isMC
  fIsMC = (fInConfig.isGMSB || fInConfig.isHVDS || fInConfig.isBkgd || fInConfig.isToy);
}

void Skimmer::InitInConfigStrings()
{
  fInConfig.phIDmin = 0;
  fInConfig.phgoodIDmin = 0;
  fInConfig.inputPaths = 0;
  fInConfig.inputFilters = 0;
  fInConfig.inputFlags = 0;
}

void Skimmer::InitInConfigBranches()
{
  fInConfigTree->SetBranchAddress(fInConfig.s_blindSF.c_str(), &fInConfig.blindSF);
  fInConfigTree->SetBranchAddress(fInConfig.s_applyBlindSF.c_str(), &fInConfig.applyBlindSF);
  fInConfigTree->SetBranchAddress(fInConfig.s_blindMET.c_str(), &fInConfig.blindMET);
  fInConfigTree->SetBranchAddress(fInConfig.s_applyBlindMET.c_str(), &fInConfig.applyBlindMET);
  fInConfigTree->SetBranchAddress(fInConfig.s_jetpTmin.c_str(), &fInConfig.jetpTmin);
  fInConfigTree->SetBranchAddress(fInConfig.s_jetEtamax.c_str(), &fInConfig.jetEtamax);
  fInConfigTree->SetBranchAddress(fInConfig.s_jetIDmin.c_str(), &fInConfig.jetIDmin);
  fInConfigTree->SetBranchAddress(fInConfig.s_rhEmin.c_str(), &fInConfig.rhEmin);
  fInConfigTree->SetBranchAddress(fInConfig.s_phpTmin.c_str(), &fInConfig.phpTmin);
  fInConfigTree->SetBranchAddress(fInConfig.s_phIDmin.c_str(), &fInConfig.phIDmin);
  fInConfigTree->SetBranchAddress(fInConfig.s_seedTimemin.c_str(), &fInConfig.seedTimemin);
  fInConfigTree->SetBranchAddress(fInConfig.s_jetIDStoremin.c_str(), &fInConfig.jetIDStoremin);
  fInConfigTree->SetBranchAddress(fInConfig.s_splitPho.c_str(), &fInConfig.splitPho);
  fInConfigTree->SetBranchAddress(fInConfig.s_onlyGED.c_str(), &fInConfig.onlyGED);
  fInConfigTree->SetBranchAddress(fInConfig.s_onlyOOT.c_str(), &fInConfig.onlyOOT);
  fInConfigTree->SetBranchAddress(fInConfig.s_storeRecHits.c_str(), &fInConfig.storeRecHits);
  fInConfigTree->SetBranchAddress(fInConfig.s_applyTrigger.c_str(), &fInConfig.applyTrigger);
  fInConfigTree->SetBranchAddress(fInConfig.s_minHT.c_str(), &fInConfig.minHT);
  fInConfigTree->SetBranchAddress(fInConfig.s_applyHT.c_str(), &fInConfig.applyHT);
  fInConfigTree->SetBranchAddress(fInConfig.s_phgoodpTmin.c_str(), &fInConfig.phgoodpTmin);
  fInConfigTree->SetBranchAddress(fInConfig.s_phgoodIDmin.c_str(), &fInConfig.phgoodIDmin);
  fInConfigTree->SetBranchAddress(fInConfig.s_applyPhGood.c_str(), &fInConfig.applyPhGood);
  fInConfigTree->SetBranchAddress(fInConfig.s_dRmin.c_str(), &fInConfig.dRmin);
  fInConfigTree->SetBranchAddress(fInConfig.s_pTres.c_str(), &fInConfig.pTres);
  fInConfigTree->SetBranchAddress(fInConfig.s_genpTres.c_str(), &fInConfig.genpTres);
  fInConfigTree->SetBranchAddress(fInConfig.s_trackdRmin.c_str(), &fInConfig.trackdRmin);
  fInConfigTree->SetBranchAddress(fInConfig.s_trackpTmin.c_str(), &fInConfig.trackpTmin);
  fInConfigTree->SetBranchAddress(fInConfig.s_inputPaths.c_str(), &fInConfig.inputPaths);
  fInConfigTree->SetBranchAddress(fInConfig.s_inputFilters.c_str(), &fInConfig.inputFilters);
  fInConfigTree->SetBranchAddress(fInConfig.s_inputFlags.c_str(), &fInConfig.inputFlags);
  fInConfigTree->SetBranchAddress(fInConfig.s_isGMSB.c_str(), &fInConfig.isGMSB);
  fInConfigTree->SetBranchAddress(fInConfig.s_isHVDS.c_str(), &fInConfig.isHVDS);
  fInConfigTree->SetBranchAddress(fInConfig.s_isBkgd.c_str(), &fInConfig.isBkgd);
  fInConfigTree->SetBranchAddress(fInConfig.s_isToy.c_str(), &fInConfig.isToy);
  fInConfigTree->SetBranchAddress(fInConfig.s_xsec.c_str(), &fInConfig.xsec);
  fInConfigTree->SetBranchAddress(fInConfig.s_filterEff.c_str(), &fInConfig.filterEff);
  fInConfigTree->SetBranchAddress(fInConfig.s_BR.c_str(), &fInConfig.BR);
}

void Skimmer::InitInTree() 
{
  Skimmer::InitInStructs();
  Skimmer::InitInBranchVecs();
  Skimmer::InitInBranches();
}

void Skimmer::InitInStructs()
{
  if (fIsMC)
  {
    if (fInConfig.isGMSB)
    {
      fInGMSBs.clear(); 
      fInGMSBs.resize(Config::nGMSBs);
    }
    if (fInConfig.isHVDS)
    {
      fInHVDSs.clear(); 
      fInHVDSs.resize(Config::nHVDSs);
    }
    if (fInConfig.isToy)
    {
      fInToys.clear(); 
      fInToys.resize(Config::nToys);
    }
  }

  fInJets.clear();
  fInJets.resize(Config::nJets);

  fInPhos.clear();
  fInPhos.resize(Config::nPhotons);
}

void Skimmer::InitInBranchVecs()
{
  if (fInConfig.storeRecHits) 
  {
    fInRecHits.eta = 0;
    fInRecHits.phi = 0;
    fInRecHits.E = 0;
    fInRecHits.time = 0;
    fInRecHits.OOT = 0;
    fInRecHits.ID = 0;

    for (Int_t ipho = 0; ipho < Config::nPhotons; ipho++) 
    {
      fInPhos[ipho].recHits = 0;
    }  
  }
}

void Skimmer::InitInBranches()
{
  if (fIsMC)
  {
    fInTree->SetBranchAddress(fInEvent.s_genwgt.c_str(), &fInEvent.genwgt);
    fInTree->SetBranchAddress(fInEvent.s_genx0.c_str(), &fInEvent.genx0);
    fInTree->SetBranchAddress(fInEvent.s_geny0.c_str(), &fInEvent.geny0);
    fInTree->SetBranchAddress(fInEvent.s_genz0.c_str(), &fInEvent.genz0);
    fInTree->SetBranchAddress(fInEvent.s_gent0.c_str(), &fInEvent.gent0);
    fInTree->SetBranchAddress(fInEvent.s_genpuobs.c_str(), &fInEvent.genpuobs);
    fInTree->SetBranchAddress(fInEvent.s_genputrue.c_str(), &fInEvent.genputrue);
    
    if (fInConfig.isGMSB)
    {
      fInTree->SetBranchAddress(fInEvent.s_nNeutoPhGr.c_str(), &fInEvent.nNeutoPhGr);
      for (Int_t igmsb = 0; igmsb < Config::nGMSBs; igmsb++) 
      {
	auto & gmsb = fInGMSBs[igmsb];
	fInTree->SetBranchAddress(Form("%s_%i",gmsb.s_genNmass.c_str(),igmsb), &gmsb.genNmass);
	fInTree->SetBranchAddress(Form("%s_%i",gmsb.s_genNE.c_str(),igmsb), &gmsb.genNE);
	fInTree->SetBranchAddress(Form("%s_%i",gmsb.s_genNpt.c_str(),igmsb), &gmsb.genNpt);
	fInTree->SetBranchAddress(Form("%s_%i",gmsb.s_genNphi.c_str(),igmsb), &gmsb.genNphi);
	fInTree->SetBranchAddress(Form("%s_%i",gmsb.s_genNeta.c_str(),igmsb), &gmsb.genNeta);
	fInTree->SetBranchAddress(Form("%s_%i",gmsb.s_genNprodvx.c_str(),igmsb), &gmsb.genNprodvx);
	fInTree->SetBranchAddress(Form("%s_%i",gmsb.s_genNprodvy.c_str(),igmsb), &gmsb.genNprodvy);
	fInTree->SetBranchAddress(Form("%s_%i",gmsb.s_genNprodvz.c_str(),igmsb), &gmsb.genNprodvz);
	fInTree->SetBranchAddress(Form("%s_%i",gmsb.s_genNdecayvx.c_str(),igmsb), &gmsb.genNdecayvx);
	fInTree->SetBranchAddress(Form("%s_%i",gmsb.s_genNdecayvy.c_str(),igmsb), &gmsb.genNdecayvy);
	fInTree->SetBranchAddress(Form("%s_%i",gmsb.s_genNdecayvz.c_str(),igmsb), &gmsb.genNdecayvz);
	fInTree->SetBranchAddress(Form("%s_%i",gmsb.s_genphE.c_str(),igmsb), &gmsb.genphE);
	fInTree->SetBranchAddress(Form("%s_%i",gmsb.s_genphpt.c_str(),igmsb), &gmsb.genphpt);
	fInTree->SetBranchAddress(Form("%s_%i",gmsb.s_genphphi.c_str(),igmsb), &gmsb.genphphi);
	fInTree->SetBranchAddress(Form("%s_%i",gmsb.s_genpheta.c_str(),igmsb), &gmsb.genpheta);
	fInTree->SetBranchAddress(Form("%s_%i",gmsb.s_genphmatch.c_str(),igmsb), &gmsb.genphmatch);
	fInTree->SetBranchAddress(Form("%s_%i",gmsb.s_gengrmass.c_str(),igmsb), &gmsb.gengrmass);
	fInTree->SetBranchAddress(Form("%s_%i",gmsb.s_gengrE.c_str(),igmsb), &gmsb.gengrE);
	fInTree->SetBranchAddress(Form("%s_%i",gmsb.s_gengrpt.c_str(),igmsb), &gmsb.gengrpt);
	fInTree->SetBranchAddress(Form("%s_%i",gmsb.s_gengrphi.c_str(),igmsb), &gmsb.gengrphi);
	fInTree->SetBranchAddress(Form("%s_%i",gmsb.s_gengreta.c_str(),igmsb), &gmsb.gengreta);
      } // end loop over neutralinos
    } // end block over gmsb

    if (fInConfig.isHVDS)
    {
      fInTree->SetBranchAddress(fInEvent.s_nvPions.c_str(), &fInEvent.nvPions);
      for (Int_t ihvds = 0; ihvds < Config::nHVDSs; ihvds++) 
      {
	auto & hvds = fInHVDSs[ihvds]; 
	fInTree->SetBranchAddress(Form("%s_%i",hvds.s_genvPionmass.c_str(),ihvds), &hvds.genvPionmass);
	fInTree->SetBranchAddress(Form("%s_%i",hvds.s_genvPionE.c_str(),ihvds), &hvds.genvPionE);
	fInTree->SetBranchAddress(Form("%s_%i",hvds.s_genvPionpt.c_str(),ihvds), &hvds.genvPionpt);
	fInTree->SetBranchAddress(Form("%s_%i",hvds.s_genvPionphi.c_str(),ihvds), &hvds.genvPionphi);
	fInTree->SetBranchAddress(Form("%s_%i",hvds.s_genvPioneta.c_str(),ihvds), &hvds.genvPioneta);
	fInTree->SetBranchAddress(Form("%s_%i",hvds.s_genvPionprodvx.c_str(),ihvds), &hvds.genvPionprodvx);
	fInTree->SetBranchAddress(Form("%s_%i",hvds.s_genvPionprodvy.c_str(),ihvds), &hvds.genvPionprodvy);
	fInTree->SetBranchAddress(Form("%s_%i",hvds.s_genvPionprodvz.c_str(),ihvds), &hvds.genvPionprodvz);
	fInTree->SetBranchAddress(Form("%s_%i",hvds.s_genvPiondecayvx.c_str(),ihvds), &hvds.genvPiondecayvx);
	fInTree->SetBranchAddress(Form("%s_%i",hvds.s_genvPiondecayvy.c_str(),ihvds), &hvds.genvPiondecayvy);
	fInTree->SetBranchAddress(Form("%s_%i",hvds.s_genvPiondecayvz.c_str(),ihvds), &hvds.genvPiondecayvz);
	fInTree->SetBranchAddress(Form("%s_%i",hvds.s_genHVph0E.c_str(),ihvds), &hvds.genHVph0E);
	fInTree->SetBranchAddress(Form("%s_%i",hvds.s_genHVph0pt.c_str(),ihvds), &hvds.genHVph0pt);
	fInTree->SetBranchAddress(Form("%s_%i",hvds.s_genHVph0phi.c_str(),ihvds), &hvds.genHVph0phi);
	fInTree->SetBranchAddress(Form("%s_%i",hvds.s_genHVph0eta.c_str(),ihvds), &hvds.genHVph0eta);
	fInTree->SetBranchAddress(Form("%s_%i",hvds.s_genHVph0match.c_str(),ihvds), &hvds.genHVph0match);
	fInTree->SetBranchAddress(Form("%s_%i",hvds.s_genHVph1E.c_str(),ihvds), &hvds.genHVph1E);
	fInTree->SetBranchAddress(Form("%s_%i",hvds.s_genHVph1pt.c_str(),ihvds), &hvds.genHVph1pt);
	fInTree->SetBranchAddress(Form("%s_%i",hvds.s_genHVph1phi.c_str(),ihvds), &hvds.genHVph1phi);
	fInTree->SetBranchAddress(Form("%s_%i",hvds.s_genHVph1eta.c_str(),ihvds), &hvds.genHVph1eta);
	fInTree->SetBranchAddress(Form("%s_%i",hvds.s_genHVph1match.c_str(),ihvds), &hvds.genHVph1match);
      } // end loop over nvpions 
    } // end block over hvds

    if (fInConfig.isToy)
    {
      fInTree->SetBranchAddress(fInEvent.s_nToyPhs.c_str(), &fInEvent.nToyPhs);
      for (Int_t itoy = 0; itoy < Config::nToys; itoy++) 
      {
	auto & toy = fInToys[itoy]; 
	fInTree->SetBranchAddress(Form("%s_%i",toy.s_genphE.c_str(),itoy), &toy.genphE);
	fInTree->SetBranchAddress(Form("%s_%i",toy.s_genphpt.c_str(),itoy), &toy.genphpt);
	fInTree->SetBranchAddress(Form("%s_%i",toy.s_genphphi.c_str(),itoy), &toy.genphphi);
	fInTree->SetBranchAddress(Form("%s_%i",toy.s_genpheta.c_str(),itoy), &toy.genpheta);
	fInTree->SetBranchAddress(Form("%s_%i",toy.s_genphmatch.c_str(),itoy), &toy.genphmatch);
	fInTree->SetBranchAddress(Form("%s_%i",toy.s_genphmatch_ptres.c_str(),itoy), &toy.genphmatch_ptres);
	fInTree->SetBranchAddress(Form("%s_%i",toy.s_genphmatch_status.c_str(),itoy), &toy.genphmatch_status);
      } // end loop over toy phos
    } // end block over toyMC

  } // end block over isMC

  fInTree->SetBranchAddress(fInEvent.s_run.c_str(), &fInEvent.run);
  fInTree->SetBranchAddress(fInEvent.s_lumi.c_str(), &fInEvent.lumi);
  fInTree->SetBranchAddress(fInEvent.s_event.c_str(), &fInEvent.event);

  fInTree->SetBranchAddress(fInEvent.s_hltSignal.c_str(), &fInEvent.hltSignal);
  fInTree->SetBranchAddress(fInEvent.s_hltRefPhoID.c_str(), &fInEvent.hltRefPhoID);
  fInTree->SetBranchAddress(fInEvent.s_hltRefDispID.c_str(), &fInEvent.hltRefDispID);
  fInTree->SetBranchAddress(fInEvent.s_hltRefHT.c_str(), &fInEvent.hltRefHT);
  fInTree->SetBranchAddress(fInEvent.s_hltPho50.c_str(), &fInEvent.hltPho50);
  fInTree->SetBranchAddress(fInEvent.s_hltPho200.c_str(), &fInEvent.hltPho200);
  fInTree->SetBranchAddress(fInEvent.s_hltDiPho70.c_str(), &fInEvent.hltDiPho70);
  fInTree->SetBranchAddress(fInEvent.s_hltDiPho3022M90.c_str(), &fInEvent.hltDiPho3022M90);
  fInTree->SetBranchAddress(fInEvent.s_hltDiPho30PV18PV.c_str(), &fInEvent.hltDiPho30PV18PV);
  fInTree->SetBranchAddress(fInEvent.s_hltDiEle33MW.c_str(), &fInEvent.hltDiEle33MW);
  fInTree->SetBranchAddress(fInEvent.s_hltDiEle27WPT.c_str(), &fInEvent.hltDiEle27WPT);
  fInTree->SetBranchAddress(fInEvent.s_hltJet500.c_str(), &fInEvent.hltJet500);
  
  fInTree->SetBranchAddress(fInEvent.s_metPV.c_str(), &fInEvent.metPV);
  fInTree->SetBranchAddress(fInEvent.s_metBeamHalo.c_str(), &fInEvent.metBeamHalo);
  fInTree->SetBranchAddress(fInEvent.s_metHBHENoise.c_str(), &fInEvent.metHBHENoise);
  fInTree->SetBranchAddress(fInEvent.s_metHBHEisoNoise.c_str(), &fInEvent.metHBHEisoNoise);
  fInTree->SetBranchAddress(fInEvent.s_metECALTP.c_str(), &fInEvent.metECALTP);
  fInTree->SetBranchAddress(fInEvent.s_metPFMuon.c_str(), &fInEvent.metPFMuon);
  fInTree->SetBranchAddress(fInEvent.s_metPFChgHad.c_str(), &fInEvent.metPFChgHad);
  fInTree->SetBranchAddress(fInEvent.s_metEESC.c_str(), &fInEvent.metEESC);
  fInTree->SetBranchAddress(fInEvent.s_metECALCalib.c_str(), &fInEvent.metECALCalib);

  fInTree->SetBranchAddress(fInEvent.s_nvtx.c_str(), &fInEvent.nvtx);
  fInTree->SetBranchAddress(fInEvent.s_vtxX.c_str(), &fInEvent.vtxX);
  fInTree->SetBranchAddress(fInEvent.s_vtxY.c_str(), &fInEvent.vtxY);
  fInTree->SetBranchAddress(fInEvent.s_vtxZ.c_str(), &fInEvent.vtxZ);
  fInTree->SetBranchAddress(fInEvent.s_rho.c_str(), &fInEvent.rho);

  fInTree->SetBranchAddress(fInEvent.s_t1pfMETpt.c_str(), &fInEvent.t1pfMETpt);
  fInTree->SetBranchAddress(fInEvent.s_t1pfMETphi.c_str(), &fInEvent.t1pfMETphi);
  fInTree->SetBranchAddress(fInEvent.s_t1pfMETsumEt.c_str(), &fInEvent.t1pfMETsumEt);

  fInTree->SetBranchAddress(fInEvent.s_jetHT.c_str(), &fInEvent.jetHT);
  fInTree->SetBranchAddress(fInEvent.s_njets.c_str(), &fInEvent.njets);
  fInTree->SetBranchAddress(fInEvent.s_jetHTpt15.c_str(), &fInEvent.jetHTpt15);
  fInTree->SetBranchAddress(fInEvent.s_njetspt15.c_str(), &fInEvent.njetspt15);
  fInTree->SetBranchAddress(fInEvent.s_jetHTeta3.c_str(), &fInEvent.jetHTeta3);
  fInTree->SetBranchAddress(fInEvent.s_njetseta3.c_str(), &fInEvent.njetseta3);
  fInTree->SetBranchAddress(fInEvent.s_jetHTidL.c_str(), &fInEvent.jetHTidL);
  fInTree->SetBranchAddress(fInEvent.s_njetsidL.c_str(), &fInEvent.njetsidL);
  fInTree->SetBranchAddress(fInEvent.s_jetHTnopho.c_str(), &fInEvent.jetHTnopho);
  fInTree->SetBranchAddress(fInEvent.s_njetsnopho.c_str(), &fInEvent.njetsnopho);
  fInTree->SetBranchAddress(fInEvent.s_jetHTidT.c_str(), &fInEvent.jetHTidT);
  fInTree->SetBranchAddress(fInEvent.s_njetsidT.c_str(), &fInEvent.njetsidT);

  for (Int_t ijet = 0; ijet < Config::nJets; ijet++) 
  {
    auto & jet = fInJets[ijet];
    fInTree->SetBranchAddress(Form("%s_%i",jet.s_E.c_str(),ijet), &jet.E);    
    fInTree->SetBranchAddress(Form("%s_%i",jet.s_pt.c_str(),ijet), &jet.pt);    
    fInTree->SetBranchAddress(Form("%s_%i",jet.s_phi.c_str(),ijet), &jet.phi);    
    fInTree->SetBranchAddress(Form("%s_%i",jet.s_eta.c_str(),ijet), &jet.eta);    
  }

  fInTree->SetBranchAddress(fInEvent.s_nrechits.c_str(), &fInEvent.nrechits);
  if (fInConfig.storeRecHits)
  {
    fInTree->SetBranchAddress(fInRecHits.s_eta.c_str(), &fInRecHits.eta);
    fInTree->SetBranchAddress(fInRecHits.s_phi.c_str(), &fInRecHits.phi);
    fInTree->SetBranchAddress(fInRecHits.s_E.c_str(), &fInRecHits.E);
    fInTree->SetBranchAddress(fInRecHits.s_time.c_str(), &fInRecHits.time);
    fInTree->SetBranchAddress(fInRecHits.s_OOT.c_str(), &fInRecHits.OOT);
    fInTree->SetBranchAddress(fInRecHits.s_ID.c_str(), &fInRecHits.ID);
  }

  fInTree->SetBranchAddress(fInEvent.s_nphotons.c_str(), &fInEvent.nphotons);
  for (Int_t ipho = 0; ipho < Config::nPhotons; ipho++) 
  {
    auto & pho = fInPhos[ipho];
    fInTree->SetBranchAddress(Form("%s_%i",pho.s_E.c_str(),ipho), &pho.E);
    fInTree->SetBranchAddress(Form("%s_%i",pho.s_pt.c_str(),ipho), &pho.pt);
    fInTree->SetBranchAddress(Form("%s_%i",pho.s_eta.c_str(),ipho), &pho.eta);
    fInTree->SetBranchAddress(Form("%s_%i",pho.s_phi.c_str(),ipho), &pho.phi);
    fInTree->SetBranchAddress(Form("%s_%i",pho.s_scE.c_str(),ipho), &pho.scE);
    fInTree->SetBranchAddress(Form("%s_%i",pho.s_sceta.c_str(),ipho), &pho.sceta);
    fInTree->SetBranchAddress(Form("%s_%i",pho.s_scphi.c_str(),ipho), &pho.scphi);
    fInTree->SetBranchAddress(Form("%s_%i",pho.s_HoE.c_str(),ipho), &pho.HoE);
    fInTree->SetBranchAddress(Form("%s_%i",pho.s_r9.c_str(),ipho), &pho.r9);
    fInTree->SetBranchAddress(Form("%s_%i",pho.s_ChgHadIso.c_str(),ipho), &pho.ChgHadIso);
    fInTree->SetBranchAddress(Form("%s_%i",pho.s_NeuHadIso.c_str(),ipho), &pho.NeuHadIso);
    fInTree->SetBranchAddress(Form("%s_%i",pho.s_PhoIso.c_str(),ipho), &pho.PhoIso);
    fInTree->SetBranchAddress(Form("%s_%i",pho.s_EcalPFClIso.c_str(),ipho), &pho.EcalPFClIso);
    fInTree->SetBranchAddress(Form("%s_%i",pho.s_HcalPFClIso.c_str(),ipho), &pho.HcalPFClIso);
    fInTree->SetBranchAddress(Form("%s_%i",pho.s_TrkIso.c_str(),ipho), &pho.TrkIso);
    fInTree->SetBranchAddress(Form("%s_%i",pho.s_sieie.c_str(),ipho), &pho.sieie);
    fInTree->SetBranchAddress(Form("%s_%i",pho.s_sipip.c_str(),ipho), &pho.sipip);
    fInTree->SetBranchAddress(Form("%s_%i",pho.s_sieip.c_str(),ipho), &pho.sieip);
    fInTree->SetBranchAddress(Form("%s_%i",pho.s_smaj.c_str(),ipho), &pho.smaj);
    fInTree->SetBranchAddress(Form("%s_%i",pho.s_smin.c_str(),ipho), &pho.smin);
    fInTree->SetBranchAddress(Form("%s_%i",pho.s_alpha.c_str(),ipho), &pho.alpha);
    if (fInConfig.storeRecHits)
    {
      fInTree->SetBranchAddress(Form("%s_%i",pho.s_seed.c_str(),ipho), &pho.seed);
      fInTree->SetBranchAddress(Form("%s_%i",pho.s_recHits.c_str(),ipho), &pho.recHits);
    }
    else
    {
      fInTree->SetBranchAddress(Form("%s_%i",pho.s_seedtime.c_str(),ipho), &pho.seedtime);
      fInTree->SetBranchAddress(Form("%s_%i",pho.s_seedE.c_str(),ipho), &pho.seedE);
      fInTree->SetBranchAddress(Form("%s_%i",pho.s_seedID.c_str(),ipho), &pho.seedID);;
    }
    //    fInTree->SetBranchAddress(Form("%s_%i",pho.s_suisseX.c_str(),ipho), &pho.suisseX);
    fInTree->SetBranchAddress(Form("%s_%i",pho.s_isOOT.c_str(),ipho), &pho.isOOT);
    fInTree->SetBranchAddress(Form("%s_%i",pho.s_isEB.c_str(),ipho), &pho.isEB);
    fInTree->SetBranchAddress(Form("%s_%i",pho.s_isHLT.c_str(),ipho), &pho.isHLT);
    fInTree->SetBranchAddress(Form("%s_%i",pho.s_isTrk.c_str(),ipho), &pho.isTrk);
    fInTree->SetBranchAddress(Form("%s_%i",pho.s_passEleVeto.c_str(),ipho), &pho.passEleVeto);
    fInTree->SetBranchAddress(Form("%s_%i",pho.s_hasPixSeed.c_str(),ipho), &pho.hasPixSeed);
    fInTree->SetBranchAddress(Form("%s_%i",pho.s_gedID.c_str(),ipho), &pho.gedID);
    fInTree->SetBranchAddress(Form("%s_%i",pho.s_ootID.c_str(),ipho), &pho.ootID);
    
    if (fIsMC)
    {
      fInTree->SetBranchAddress(Form("%s_%i",pho.s_isGen.c_str(),ipho), &pho.isGen);
      if (fInConfig.isGMSB || fInConfig.isHVDS)
      {
	fInTree->SetBranchAddress(Form("%s_%i",pho.s_isSignal.c_str(),ipho), &pho.isSignal);
      }
    }
  }
}

void Skimmer::InitAndSetOutConfig()
{
  // Make the branches
  fOutConfigTree->Branch(fOutConfig.s_blindSF.c_str(), &fOutConfig.blindSF);
  fOutConfigTree->Branch(fOutConfig.s_applyBlindSF.c_str(), &fOutConfig.applyBlindSF);
  fOutConfigTree->Branch(fOutConfig.s_blindMET.c_str(), &fOutConfig.blindMET);
  fOutConfigTree->Branch(fOutConfig.s_applyBlindMET.c_str(), &fOutConfig.applyBlindMET);
  fOutConfigTree->Branch(fOutConfig.s_jetpTmin.c_str(), &fOutConfig.jetpTmin);
  fOutConfigTree->Branch(fOutConfig.s_jetEtamax.c_str(), &fOutConfig.jetEtamax);
  fOutConfigTree->Branch(fOutConfig.s_jetIDmin.c_str(), &fOutConfig.jetIDmin);
  fOutConfigTree->Branch(fOutConfig.s_rhEmin.c_str(), &fOutConfig.rhEmin);
  fOutConfigTree->Branch(fOutConfig.s_phpTmin.c_str(), &fOutConfig.phpTmin);
  fOutConfigTree->Branch(fOutConfig.s_phIDmin.c_str(), &fOutConfig.phIDmin_s);
  fOutConfigTree->Branch(fOutConfig.s_seedTimemin.c_str(), &fOutConfig.seedTimemin);
  fOutConfigTree->Branch(fOutConfig.s_jetIDStoremin.c_str(), &fOutConfig.jetIDStoremin);
  fOutConfigTree->Branch(fOutConfig.s_splitPho.c_str(), &fOutConfig.splitPho);
  fOutConfigTree->Branch(fOutConfig.s_onlyGED.c_str(), &fOutConfig.onlyGED);
  fOutConfigTree->Branch(fOutConfig.s_onlyOOT.c_str(), &fOutConfig.onlyOOT);
  fOutConfigTree->Branch(fOutConfig.s_storeRecHits.c_str(), &fOutConfig.storeRecHits);
  fOutConfigTree->Branch(fOutConfig.s_applyTrigger.c_str(), &fOutConfig.applyTrigger);
  fOutConfigTree->Branch(fOutConfig.s_minHT.c_str(), &fOutConfig.minHT);
  fOutConfigTree->Branch(fOutConfig.s_applyHT.c_str(), &fOutConfig.applyHT);
  fOutConfigTree->Branch(fOutConfig.s_phgoodpTmin.c_str(), &fOutConfig.phgoodpTmin);
  fOutConfigTree->Branch(fOutConfig.s_phgoodIDmin.c_str(), &fOutConfig.phgoodIDmin_s);
  fOutConfigTree->Branch(fOutConfig.s_applyPhGood.c_str(), &fOutConfig.applyPhGood);
  fOutConfigTree->Branch(fOutConfig.s_dRmin.c_str(), &fOutConfig.dRmin);
  fOutConfigTree->Branch(fOutConfig.s_pTres.c_str(), &fOutConfig.pTres);
  fOutConfigTree->Branch(fOutConfig.s_genpTres.c_str(), &fOutConfig.genpTres);
  fOutConfigTree->Branch(fOutConfig.s_trackdRmin.c_str(), &fOutConfig.trackdRmin);
  fOutConfigTree->Branch(fOutConfig.s_trackpTmin.c_str(), &fOutConfig.trackpTmin);
  fOutConfigTree->Branch(fOutConfig.s_inputPaths.c_str(), &fOutConfig.inputPaths_s);
  fOutConfigTree->Branch(fOutConfig.s_inputFilters.c_str(), &fOutConfig.inputFilters_s);
  fOutConfigTree->Branch(fOutConfig.s_inputFlags.c_str(), &fOutConfig.inputFlags_s);
  fOutConfigTree->Branch(fOutConfig.s_isGMSB.c_str(), &fOutConfig.isGMSB);
  fOutConfigTree->Branch(fOutConfig.s_isHVDS.c_str(), &fOutConfig.isHVDS);
  fOutConfigTree->Branch(fOutConfig.s_isBkgd.c_str(), &fOutConfig.isBkgd);
  fOutConfigTree->Branch(fOutConfig.s_isToy.c_str(), &fOutConfig.isToy);
  fOutConfigTree->Branch(fOutConfig.s_xsec.c_str(), &fOutConfig.xsec);
  fOutConfigTree->Branch(fOutConfig.s_filterEff.c_str(), &fOutConfig.filterEff);
  fOutConfigTree->Branch(fOutConfig.s_BR.c_str(), &fOutConfig.BR);

  // Now set the values of the branches
  fOutConfig.blindSF = fInConfig.blindSF;
  fOutConfig.applyBlindSF = fInConfig.applyBlindSF;
  fOutConfig.blindMET = fInConfig.blindMET;
  fOutConfig.applyBlindMET = fInConfig.applyBlindMET;
  fOutConfig.jetpTmin = fInConfig.jetpTmin;
  fOutConfig.jetEtamax = fInConfig.jetEtamax;
  fOutConfig.jetIDmin = fInConfig.jetIDmin;
  fOutConfig.rhEmin = fInConfig.rhEmin;
  fOutConfig.phpTmin = fInConfig.phpTmin;
  fOutConfig.phIDmin_s = fInConfig.phIDmin->c_str();
  fOutConfig.seedTimemin = fInConfig.seedTimemin;
  fOutConfig.jetIDStoremin = fInConfig.jetIDStoremin;
  fOutConfig.splitPho = fInConfig.splitPho;
  fOutConfig.onlyGED = fInConfig.onlyGED;
  fOutConfig.onlyOOT = fInConfig.onlyOOT;
  fOutConfig.storeRecHits = false; // drop these now!
  fOutConfig.applyTrigger = fInConfig.applyTrigger;
  fOutConfig.minHT = fInConfig.minHT;
  fOutConfig.applyHT = fInConfig.applyHT;
  fOutConfig.phgoodpTmin = fInConfig.phgoodpTmin;
  fOutConfig.phgoodIDmin_s = fInConfig.phgoodIDmin->c_str();
  fOutConfig.applyPhGood = fInConfig.applyPhGood;
  fOutConfig.dRmin = fInConfig.dRmin;
  fOutConfig.pTres = fInConfig.pTres;
  fOutConfig.genpTres = fInConfig.genpTres;
  fOutConfig.trackdRmin = fInConfig.trackdRmin;
  fOutConfig.trackpTmin = fInConfig.trackpTmin;
  fOutConfig.inputPaths_s = fInConfig.inputPaths->c_str();
  fOutConfig.inputFilters_s = fInConfig.inputFilters->c_str();
  fOutConfig.inputFlags_s = fInConfig.inputFlags->c_str();
  fOutConfig.isGMSB = fInConfig.isGMSB;
  fOutConfig.isHVDS = fInConfig.isHVDS;
  fOutConfig.isBkgd = fInConfig.isBkgd;
  fOutConfig.isToy = fInConfig.isToy;
  fOutConfig.xsec = fInConfig.xsec;
  fOutConfig.filterEff = fInConfig.filterEff;
  fOutConfig.BR = fInConfig.BR;

  // and fill it once
  fOutConfigTree->Fill();
}

void Skimmer::InitOutTree()
{
  if (fIsMC)
  {
    fOutTree->Branch(fOutEvent.s_puwgt.c_str(), &fOutEvent.puwgt);
    fOutTree->Branch(fOutEvent.s_genx0.c_str(), &fOutEvent.genx0);
    fOutTree->Branch(fOutEvent.s_geny0.c_str(), &fOutEvent.geny0);
    fOutTree->Branch(fOutEvent.s_genz0.c_str(), &fOutEvent.genz0);
    fOutTree->Branch(fOutEvent.s_gent0.c_str(), &fOutEvent.gent0);
    
    if (fOutConfig.isGMSB)
    {
      fOutTree->Branch(fOutEvent.s_nNeutoPhGr.c_str(), &fOutEvent.nNeutoPhGr);
      fOutGMSBs.resize(Config::nGMSBs);
      for (Int_t igmsb = 0; igmsb < Config::nGMSBs; igmsb++) 
      {
	auto & gmsb = fOutGMSBs[igmsb];
	fOutTree->Branch(Form("%s_%i",gmsb.s_genNmass.c_str(),igmsb), &gmsb.genNmass);
	fOutTree->Branch(Form("%s_%i",gmsb.s_genNE.c_str(),igmsb), &gmsb.genNE);
	fOutTree->Branch(Form("%s_%i",gmsb.s_genNpt.c_str(),igmsb), &gmsb.genNpt);
	fOutTree->Branch(Form("%s_%i",gmsb.s_genNphi.c_str(),igmsb), &gmsb.genNphi);
	fOutTree->Branch(Form("%s_%i",gmsb.s_genNeta.c_str(),igmsb), &gmsb.genNeta);
	fOutTree->Branch(Form("%s_%i",gmsb.s_genNprodvx.c_str(),igmsb), &gmsb.genNprodvx);
	fOutTree->Branch(Form("%s_%i",gmsb.s_genNprodvy.c_str(),igmsb), &gmsb.genNprodvy);
	fOutTree->Branch(Form("%s_%i",gmsb.s_genNprodvz.c_str(),igmsb), &gmsb.genNprodvz);
	fOutTree->Branch(Form("%s_%i",gmsb.s_genNdecayvx.c_str(),igmsb), &gmsb.genNdecayvx);
	fOutTree->Branch(Form("%s_%i",gmsb.s_genNdecayvy.c_str(),igmsb), &gmsb.genNdecayvy);
	fOutTree->Branch(Form("%s_%i",gmsb.s_genNdecayvz.c_str(),igmsb), &gmsb.genNdecayvz);
	fOutTree->Branch(Form("%s_%i",gmsb.s_genphE.c_str(),igmsb), &gmsb.genphE);
	fOutTree->Branch(Form("%s_%i",gmsb.s_genphpt.c_str(),igmsb), &gmsb.genphpt);
	fOutTree->Branch(Form("%s_%i",gmsb.s_genphphi.c_str(),igmsb), &gmsb.genphphi);
	fOutTree->Branch(Form("%s_%i",gmsb.s_genpheta.c_str(),igmsb), &gmsb.genpheta);
	fOutTree->Branch(Form("%s_%i",gmsb.s_genphmatch.c_str(),igmsb), &gmsb.genphmatch);
	fOutTree->Branch(Form("%s_%i",gmsb.s_gengrmass.c_str(),igmsb), &gmsb.gengrmass);
	fOutTree->Branch(Form("%s_%i",gmsb.s_gengrE.c_str(),igmsb), &gmsb.gengrE);
	fOutTree->Branch(Form("%s_%i",gmsb.s_gengrpt.c_str(),igmsb), &gmsb.gengrpt);
	fOutTree->Branch(Form("%s_%i",gmsb.s_gengrphi.c_str(),igmsb), &gmsb.gengrphi);
	fOutTree->Branch(Form("%s_%i",gmsb.s_gengreta.c_str(),igmsb), &gmsb.gengreta);
      } // end loop over neutralinos
    } // end block over gmsb

    if (fOutConfig.isHVDS)
    {
      fOutTree->Branch(fOutEvent.s_nvPions.c_str(), &fOutEvent.nvPions);
      fOutHVDSs.resize(Config::nHVDSs);
      for (Int_t ihvds = 0; ihvds < Config::nHVDSs; ihvds++) 
      {
	auto & hvds = fOutHVDSs[ihvds]; 
	fOutTree->Branch(Form("%s_%i",hvds.s_genvPionmass.c_str(),ihvds), &hvds.genvPionmass);
	fOutTree->Branch(Form("%s_%i",hvds.s_genvPionE.c_str(),ihvds), &hvds.genvPionE);
	fOutTree->Branch(Form("%s_%i",hvds.s_genvPionpt.c_str(),ihvds), &hvds.genvPionpt);
	fOutTree->Branch(Form("%s_%i",hvds.s_genvPionphi.c_str(),ihvds), &hvds.genvPionphi);
	fOutTree->Branch(Form("%s_%i",hvds.s_genvPioneta.c_str(),ihvds), &hvds.genvPioneta);
	fOutTree->Branch(Form("%s_%i",hvds.s_genvPionprodvx.c_str(),ihvds), &hvds.genvPionprodvx);
	fOutTree->Branch(Form("%s_%i",hvds.s_genvPionprodvy.c_str(),ihvds), &hvds.genvPionprodvy);
	fOutTree->Branch(Form("%s_%i",hvds.s_genvPionprodvz.c_str(),ihvds), &hvds.genvPionprodvz);
	fOutTree->Branch(Form("%s_%i",hvds.s_genvPiondecayvx.c_str(),ihvds), &hvds.genvPiondecayvx);
	fOutTree->Branch(Form("%s_%i",hvds.s_genvPiondecayvy.c_str(),ihvds), &hvds.genvPiondecayvy);
	fOutTree->Branch(Form("%s_%i",hvds.s_genvPiondecayvz.c_str(),ihvds), &hvds.genvPiondecayvz);
	fOutTree->Branch(Form("%s_%i",hvds.s_genHVph0E.c_str(),ihvds), &hvds.genHVph0E);
	fOutTree->Branch(Form("%s_%i",hvds.s_genHVph0pt.c_str(),ihvds), &hvds.genHVph0pt);
	fOutTree->Branch(Form("%s_%i",hvds.s_genHVph0phi.c_str(),ihvds), &hvds.genHVph0phi);
	fOutTree->Branch(Form("%s_%i",hvds.s_genHVph0eta.c_str(),ihvds), &hvds.genHVph0eta);
	fOutTree->Branch(Form("%s_%i",hvds.s_genHVph0match.c_str(),ihvds), &hvds.genHVph0match);
	fOutTree->Branch(Form("%s_%i",hvds.s_genHVph1E.c_str(),ihvds), &hvds.genHVph1E);
	fOutTree->Branch(Form("%s_%i",hvds.s_genHVph1pt.c_str(),ihvds), &hvds.genHVph1pt);
	fOutTree->Branch(Form("%s_%i",hvds.s_genHVph1phi.c_str(),ihvds), &hvds.genHVph1phi);
	fOutTree->Branch(Form("%s_%i",hvds.s_genHVph1eta.c_str(),ihvds), &hvds.genHVph1eta);
	fOutTree->Branch(Form("%s_%i",hvds.s_genHVph1match.c_str(),ihvds), &hvds.genHVph1match);
      } // end loop over nvpions 
    } // end block over hvds

    if (fOutConfig.isToy)
    {
      fOutTree->Branch(fOutEvent.s_nToyPhs.c_str(), &fOutEvent.nToyPhs);
      fOutToys.resize(Config::nToys);
      for (Int_t itoy = 0; itoy < Config::nToys; itoy++) 
      {
	auto & toy = fOutToys[itoy]; 
	fOutTree->Branch(Form("%s_%i",toy.s_genphE.c_str(),itoy), &toy.genphE);
	fOutTree->Branch(Form("%s_%i",toy.s_genphpt.c_str(),itoy), &toy.genphpt);
	fOutTree->Branch(Form("%s_%i",toy.s_genphphi.c_str(),itoy), &toy.genphphi);
	fOutTree->Branch(Form("%s_%i",toy.s_genpheta.c_str(),itoy), &toy.genpheta);
	fOutTree->Branch(Form("%s_%i",toy.s_genphmatch.c_str(),itoy), &toy.genphmatch);
	fOutTree->Branch(Form("%s_%i",toy.s_genphmatch_ptres.c_str(),itoy), &toy.genphmatch_ptres);
	fOutTree->Branch(Form("%s_%i",toy.s_genphmatch_status.c_str(),itoy), &toy.genphmatch_status);
      } // end loop over toy phos
    } // end block over toyMC

  } // end block over isMC

  fOutTree->Branch(fOutEvent.s_run.c_str(), &fOutEvent.run);
  fOutTree->Branch(fOutEvent.s_lumi.c_str(), &fOutEvent.lumi);
  fOutTree->Branch(fOutEvent.s_event.c_str(), &fOutEvent.event);

  fOutTree->Branch(fOutEvent.s_hltSignal.c_str(), &fOutEvent.hltSignal);
  fOutTree->Branch(fOutEvent.s_hltRefPhoID.c_str(), &fOutEvent.hltRefPhoID);
  fOutTree->Branch(fOutEvent.s_hltRefDispID.c_str(), &fOutEvent.hltRefDispID);
  fOutTree->Branch(fOutEvent.s_hltRefHT.c_str(), &fOutEvent.hltRefHT);
  fOutTree->Branch(fOutEvent.s_hltPho50.c_str(), &fOutEvent.hltPho50);
  fOutTree->Branch(fOutEvent.s_hltPho200.c_str(), &fOutEvent.hltPho200);
  fOutTree->Branch(fOutEvent.s_hltDiPho70.c_str(), &fOutEvent.hltDiPho70);
  fOutTree->Branch(fOutEvent.s_hltDiPho3022M90.c_str(), &fOutEvent.hltDiPho3022M90);
  fOutTree->Branch(fOutEvent.s_hltDiPho30PV18PV.c_str(), &fOutEvent.hltDiPho30PV18PV);
  fOutTree->Branch(fOutEvent.s_hltDiEle33MW.c_str(), &fOutEvent.hltDiEle33MW);
  fOutTree->Branch(fOutEvent.s_hltDiEle27WPT.c_str(), &fOutEvent.hltDiEle27WPT);
  fOutTree->Branch(fOutEvent.s_hltJet500.c_str(), &fOutEvent.hltJet500);

  fOutTree->Branch(fOutEvent.s_nvtx.c_str(), &fOutEvent.nvtx);
  fOutTree->Branch(fOutEvent.s_vtxX.c_str(), &fOutEvent.vtxX);
  fOutTree->Branch(fOutEvent.s_vtxY.c_str(), &fOutEvent.vtxY);
  fOutTree->Branch(fOutEvent.s_vtxZ.c_str(), &fOutEvent.vtxZ);
  fOutTree->Branch(fOutEvent.s_rho.c_str(), &fOutEvent.rho);

  fOutTree->Branch(fOutEvent.s_t1pfMETpt.c_str(), &fOutEvent.t1pfMETpt);
  fOutTree->Branch(fOutEvent.s_t1pfMETphi.c_str(), &fOutEvent.t1pfMETphi);
  fOutTree->Branch(fOutEvent.s_t1pfMETsumEt.c_str(), &fOutEvent.t1pfMETsumEt);

  fOutTree->Branch(fOutEvent.s_jetHT.c_str(), &fOutEvent.jetHT);
  fOutTree->Branch(fOutEvent.s_njets.c_str(), &fOutEvent.njets);
  fOutTree->Branch(fOutEvent.s_jetHTpt15.c_str(), &fOutEvent.jetHTpt15);
  fOutTree->Branch(fOutEvent.s_njetspt15.c_str(), &fOutEvent.njetspt15);
  fOutTree->Branch(fOutEvent.s_jetHTeta3.c_str(), &fOutEvent.jetHTeta3);
  fOutTree->Branch(fOutEvent.s_njetseta3.c_str(), &fOutEvent.njetseta3);
  fOutTree->Branch(fOutEvent.s_jetHTidL.c_str(), &fOutEvent.jetHTidL);
  fOutTree->Branch(fOutEvent.s_njetsidL.c_str(), &fOutEvent.njetsidL);
  fOutTree->Branch(fOutEvent.s_jetHTnopho.c_str(), &fOutEvent.jetHTnopho);
  fOutTree->Branch(fOutEvent.s_njetsnopho.c_str(), &fOutEvent.njetsnopho);
  fOutTree->Branch(fOutEvent.s_jetHTidT.c_str(), &fOutEvent.jetHTidT);
  fOutTree->Branch(fOutEvent.s_njetsidT.c_str(), &fOutEvent.njetsidT);

  fOutJets.resize(Config::nJets);
  for (Int_t ijet = 0; ijet < Config::nJets; ijet++) 
  {
    auto & jet = fOutJets[ijet];
    fOutTree->Branch(Form("%s_%i",jet.s_E.c_str(),ijet), &jet.E);    
    fOutTree->Branch(Form("%s_%i",jet.s_pt.c_str(),ijet), &jet.pt);    
    fOutTree->Branch(Form("%s_%i",jet.s_phi.c_str(),ijet), &jet.phi);    
    fOutTree->Branch(Form("%s_%i",jet.s_eta.c_str(),ijet), &jet.eta);    
  }

  fOutTree->Branch(fOutEvent.s_nrechits.c_str(), &fOutEvent.nrechits);

  fOutTree->Branch(fOutEvent.s_nphotons.c_str(), &fOutEvent.nphotons);
  fOutPhos.resize(Config::nPhotons);
  for (Int_t ipho = 0; ipho < Config::nPhotons; ipho++) 
  {
    auto & pho = fOutPhos[ipho];
    fOutTree->Branch(Form("%s_%i",pho.s_E.c_str(),ipho), &pho.E);
    fOutTree->Branch(Form("%s_%i",pho.s_pt.c_str(),ipho), &pho.pt);
    fOutTree->Branch(Form("%s_%i",pho.s_eta.c_str(),ipho), &pho.eta);
    fOutTree->Branch(Form("%s_%i",pho.s_phi.c_str(),ipho), &pho.phi);
    fOutTree->Branch(Form("%s_%i",pho.s_scE.c_str(),ipho), &pho.scE);
    fOutTree->Branch(Form("%s_%i",pho.s_sceta.c_str(),ipho), &pho.sceta);
    fOutTree->Branch(Form("%s_%i",pho.s_scphi.c_str(),ipho), &pho.scphi);
    fOutTree->Branch(Form("%s_%i",pho.s_HoE.c_str(),ipho), &pho.HoE);
    fOutTree->Branch(Form("%s_%i",pho.s_r9.c_str(),ipho), &pho.r9);
    fOutTree->Branch(Form("%s_%i",pho.s_ChgHadIso.c_str(),ipho), &pho.ChgHadIso);
    fOutTree->Branch(Form("%s_%i",pho.s_NeuHadIso.c_str(),ipho), &pho.NeuHadIso);
    fOutTree->Branch(Form("%s_%i",pho.s_PhoIso.c_str(),ipho), &pho.PhoIso);
    fOutTree->Branch(Form("%s_%i",pho.s_EcalPFClIso.c_str(),ipho), &pho.EcalPFClIso);
    fOutTree->Branch(Form("%s_%i",pho.s_HcalPFClIso.c_str(),ipho), &pho.HcalPFClIso);
    fOutTree->Branch(Form("%s_%i",pho.s_TrkIso.c_str(),ipho), &pho.TrkIso);
    fOutTree->Branch(Form("%s_%i",pho.s_sieie.c_str(),ipho), &pho.sieie);
    fOutTree->Branch(Form("%s_%i",pho.s_sipip.c_str(),ipho), &pho.sipip);
    fOutTree->Branch(Form("%s_%i",pho.s_sieip.c_str(),ipho), &pho.sieip);
    fOutTree->Branch(Form("%s_%i",pho.s_smaj.c_str(),ipho), &pho.smaj);
    fOutTree->Branch(Form("%s_%i",pho.s_smin.c_str(),ipho), &pho.smin);
    fOutTree->Branch(Form("%s_%i",pho.s_alpha.c_str(),ipho), &pho.alpha);
    //    fOutTree->Branch(Form("%s_%i",pho.s_suisseX.c_str(),ipho), &pho.suisseX);
    fOutTree->Branch(Form("%s_%i",pho.s_seedtime.c_str(),ipho), &pho.seedtime);
    fOutTree->Branch(Form("%s_%i",pho.s_seedE.c_str(),ipho), &pho.seedE);
    fOutTree->Branch(Form("%s_%i",pho.s_seedID.c_str(),ipho), &pho.seedID);;
    fOutTree->Branch(Form("%s_%i",pho.s_isOOT.c_str(),ipho), &pho.isOOT);
    fOutTree->Branch(Form("%s_%i",pho.s_isEB.c_str(),ipho), &pho.isEB);
    fOutTree->Branch(Form("%s_%i",pho.s_isHLT.c_str(),ipho), &pho.isHLT);
    fOutTree->Branch(Form("%s_%i",pho.s_isTrk.c_str(),ipho), &pho.isTrk);
    fOutTree->Branch(Form("%s_%i",pho.s_passEleVeto.c_str(),ipho), &pho.passEleVeto);
    fOutTree->Branch(Form("%s_%i",pho.s_hasPixSeed.c_str(),ipho), &pho.hasPixSeed);
    fOutTree->Branch(Form("%s_%i",pho.s_gedID.c_str(),ipho), &pho.gedID);
    fOutTree->Branch(Form("%s_%i",pho.s_ootID.c_str(),ipho), &pho.ootID);

    if (fIsMC)
    {
      fOutTree->Branch(Form("%s_%i",pho.s_isGen.c_str(),ipho), &pho.isGen);
      if (fOutConfig.isGMSB || fOutConfig.isHVDS)
      {
	fOutTree->Branch(Form("%s_%i",pho.s_isSignal.c_str(),ipho), &pho.isSignal);
      }
    }

    // add event weight
    fOutTree->Branch(fOutEvent.s_evtwgt.c_str(), &fOutEvent.evtwgt);
  }
} 

void Skimmer::InitOutCutFlow()
{
  // get cut flow labels
  const Int_t inNbinsX = fInCutFlow->GetNbinsX();
  for (Int_t ibin = 1; ibin <= inNbinsX; ibin++)
  {
    cutLabels[fInCutFlow->GetXaxis()->GetBinLabel(ibin)] = ibin;
  }
  Int_t inNbinsX_new = inNbinsX;
  cutLabels["skim"] = ++inNbinsX_new;
  cutLabels["METFlag"] = ++inNbinsX_new;
  cutLabels["badPU"] = ++inNbinsX_new;

  // make new cut flow
  fOutCutFlow = new TH1F(Config::h_cutflowname.Data(),fInCutFlow->GetTitle(),cutLabels.size(),0,cutLabels.size());
  fOutCutFlow->Sumw2();

  for (const auto & cutlabel : cutLabels)
  {
    const Int_t ibin = cutlabel.second;

    fOutCutFlow->GetXaxis()->SetBinLabel(ibin,cutlabel.first.c_str());

    if (ibin > inNbinsX) continue;

    fOutCutFlow->SetBinContent(ibin,fInCutFlow->GetBinContent(ibin));
    fOutCutFlow->SetBinError(ibin,fInCutFlow->GetBinError(ibin));
  }
  fOutCutFlow->GetYaxis()->SetTitle(fInCutFlow->GetYaxis()->GetTitle());
}

void Skimmer::GetSampleWeight()
{
  fSampleWeight = fInConfig.xsec * fInConfig.filterEff * Config::lumi * Config::invfbToinvpb / fSumWgts; // include normalization to lumi!!! ( do we need to multiply by * fInConfig.BR)
}

void Skimmer::GetPUWeights()
{
  fPUWeights.clear();
  for (Int_t ibin = 1; ibin <= fInPUWgtHist->GetNbinsX(); ibin++)
  {
    fPUWeights.emplace_back(fInPUWgtHist->GetBinContent(ibin));
  }
}

/////////////////////
//                 //
// Effective Areas //
//                 //
/////////////////////

Float_t Skimmer::GetChargedHadronEA(const Float_t eta)
{
  if      (eta <  1.0)                  return 0.0385;
  else if (eta >= 1.0   && eta < 1.479) return 0.0468;
  else if (eta >= 1.479 && eta < 2.0  ) return 0.0435;
  else if (eta >= 2.0   && eta < 2.2  ) return 0.0378;
  else if (eta >= 2.2   && eta < 2.3  ) return 0.0338;
  else if (eta >= 2.3   && eta < 2.4  ) return 0.0314;
  else if (eta >= 2.4)                  return 0.0269;
  else                                  return 0.;
}
  
Float_t Skimmer::GetNeutralHadronEA(const Float_t eta) 
{
  if      (eta <  1.0)                  return 0.0636;
  else if (eta >= 1.0   && eta < 1.479) return 0.1103;
  else if (eta >= 1.479 && eta < 2.0  ) return 0.0759;
  else if (eta >= 2.0   && eta < 2.2  ) return 0.0236;
  else if (eta >= 2.2   && eta < 2.3  ) return 0.0151;
  else if (eta >= 2.3   && eta < 2.4  ) return 0.00007;
  else if (eta >= 2.4)                  return 0.0132;
  else                                  return 0.;
}
  
Float_t Skimmer::GetGammaEA(const Float_t eta) 
{
  if      (eta <  1.0)                  return 0.1240;
  else if (eta >= 1.0   && eta < 1.479) return 0.1093;
  else if (eta >= 1.479 && eta < 2.0  ) return 0.0631;
  else if (eta >= 2.0   && eta < 2.2  ) return 0.0779;
  else if (eta >= 2.2   && eta < 2.3  ) return 0.0999;
  else if (eta >= 2.3   && eta < 2.4  ) return 0.1155;
  else if (eta >= 2.4)                  return 0.1373;
  else                                  return 0.;
}

Float_t Skimmer::GetEcalPFClEA(const Float_t eta)
{
  if   (eta < Config::etaEBcutoff) return 0.167;
  else                             return 0.f;
}

Float_t Skimmer::GetHcalPFClEA(const Float_t eta)
{
  if   (eta < Config::etaEBcutoff) return 0.108;
  else                             return 0.f;
}

Float_t Skimmer::GetTrackEA(const Float_t eta)
{
  if   (eta < Config::etaEBcutoff) return 0.113;
  else                             return 0.f;
}

////////////////
//            //
// pT scaling //
//            //
////////////////

Float_t Skimmer::GetNeutralHadronPtScale(const Float_t eta, const Float_t pt)
{
  if      (eta <  Config::etaEBcutoff)                           return 0.0126*pt+0.000026*pt*pt;
  else if (eta >= Config::etaEBcutoff && eta < Config::etaEEmax) return 0.0119*pt+0.000025*pt*pt;
  else                                                           return 0.f;
}

Float_t Skimmer::GetGammaPtScale(const Float_t eta, const Float_t pt)
{
  if      (eta <  Config::etaEBcutoff)                           return 0.0035*pt;
  else if (eta >= Config::etaEBcutoff && eta < Config::etaEEmax) return 0.0040*pt;
  else                                                           return 0.f;
}

Float_t Skimmer::GetEcalPFClPtScale(const Float_t eta, const Float_t pt)
{
  if   (eta <  Config::etaEBcutoff) return 0.0028*pt;
  else                              return 0.f;
}

Float_t Skimmer::GetHcalPFClPtScale(const Float_t eta, const Float_t pt)
{
  if   (eta <  Config::etaEBcutoff) return 0.0087*pt;
  else                              return 0.f;
}

Float_t Skimmer::GetTrackPtScale(const Float_t eta, const Float_t pt)
{
  if   (eta <  Config::etaEBcutoff) return 0.0056*pt;
  else                              return 0.f;
}

////////////////////
//                //
// GED Photon VID //
//                //
////////////////////

Int_t Skimmer::GetGEDPhoVID(const Pho & outpho)
{
  // needed for cuts
  const Float_t eta = std::abs(outpho.sceta);
  const Float_t pt  = outpho.pt;
  
  // cut variables
  const Float_t HoverE    = outpho.HoE;
  const Float_t Sieie     = outpho.sieie;
  const Float_t ChgHadIso = std::max(outpho.ChgHadIso - (fOutEvent.rho * Skimmer::GetChargedHadronEA(eta))                                         ,0.f);
  const Float_t NeuHadIso = std::max(outpho.NeuHadIso - (fOutEvent.rho * Skimmer::GetNeutralHadronEA(eta)) - (Skimmer::GetNeutralHadronPtScale(eta,pt)),0.f);
  const Float_t PhoIso    = std::max(outpho.PhoIso    - (fOutEvent.rho * Skimmer::GetGammaEA        (eta)) - (Skimmer::GetGammaPtScale        (eta,pt)),0.f);
  
  if (eta < Config::etaEBcutoff)
  {
    if      ((HoverE < 0.020) && (Sieie < 0.0103) && (ChgHadIso < 1.158) && (NeuHadIso < 1.267) && (PhoIso < 2.065)) 
    {
      return 3;
    }
    else if ((HoverE < 0.035) && (Sieie < 0.0103) && (ChgHadIso < 1.416) && (NeuHadIso < 2.491) && (PhoIso < 2.952)) 
    {
      return 2;
    }   
    else if ((HoverE < 0.105) && (Sieie < 0.0103) && (ChgHadIso < 2.839) && (NeuHadIso < 9.188) && (PhoIso < 2.956)) 
    {
      return 1;
    } 
    else
    {
      return 0;
    }
  }
  else if (eta >= Config::etaEBcutoff && eta < Config::etaEEmax)
  {
    if      ((HoverE < 0.025) && (Sieie < 0.0271) && (ChgHadIso < 0.575) && (NeuHadIso < 8.916) && (PhoIso < 3.272)) 
    {
      return 3;
    }
    else if ((HoverE < 0.027) && (Sieie < 0.0271) && (ChgHadIso < 1.012) && (NeuHadIso < 9.131) && (PhoIso < 4.095)) 
    {
      return 2;
    }   
    else if ((HoverE < 0.029) && (Sieie < 0.0276) && (ChgHadIso < 2.150) && (NeuHadIso < 10.471) && (PhoIso < 4.895)) 
    {
      return 1;
    }   
    else
    {
      return 0;
    }
  }
  else
  {
    return -1;
  }
}

///////////////////
//               //
// OOT Photon ID //
//               //
///////////////////

Int_t Skimmer::GetOOTPhoVID(const Pho & outpho)
{
  // needed for cuts
  const Float_t eta = std::abs(outpho.sceta);
  const Float_t pt  = outpho.pt;
  
  // cut variables
  const Float_t HoverE      = outpho.HoE;
  const Float_t Sieie       = outpho.sieie;
  const Float_t EcalPFClIso = std::max(outpho.EcalPFClIso - (fOutEvent.rho * Skimmer::GetEcalPFClEA(eta)) - (Skimmer::GetEcalPFClPtScale(eta,pt)),0.f);
  const Float_t HcalPFClIso = std::max(outpho.HcalPFClIso - (fOutEvent.rho * Skimmer::GetHcalPFClEA(eta)) - (Skimmer::GetHcalPFClPtScale(eta,pt)),0.f);
  const Float_t TrkIso      = std::max(outpho.TrkIso      - (fOutEvent.rho * Skimmer::GetTrackEA   (eta)) - (Skimmer::GetTrackPtScale   (eta,pt)),0.f);
  
  if (eta < Config::etaEBcutoff)
  {
    if      ((HoverE < 0.020) && (Sieie < 0.0103) && (EcalPFClIso < 2.f) && (HcalPFClIso < 5.f) && (TrkIso < 3.f)) 
    {
      return 3;
    }   
    else if ((HoverE < 0.105) && (Sieie < 0.0103) && (EcalPFClIso < 5.f) && (HcalPFClIso < 10.f) && (TrkIso < 6.f)) 
    {
      return 1;
    }   
    else
    {
      return 0;
    }
  }
  else if (eta >= Config::etaEBcutoff && eta < Config::etaEEmax)
  {
    if      ((HoverE < 0.025) && (Sieie < 0.0271) && (EcalPFClIso < 2.f) && (HcalPFClIso < 5.f) && (TrkIso < 3.f)) 
    {
      return 3;
    }   
    else if ((HoverE < 0.029) && (Sieie < 0.0276) && (EcalPFClIso < 5.f) && (HcalPFClIso < 10.f) && (TrkIso < 6.f)) 
    {
      return 1;
    }   
    else
    {
      return 0;
    }
  }
  else
  {
    return -1;
  }
}
