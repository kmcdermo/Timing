#include "ZeeTree.h"

ZeeTree::ZeeTree(const edm::ParameterSet& iConfig): 
  ///////////// TRIGGER and filter info INFO
  triggerResultsTag(iConfig.getParameter<edm::InputTag>("triggerResults")),
  applyHLTFilter(iConfig.existsAs<bool>("applyHLTFilter") ? iConfig.getParameter<bool>("applyHLTFilter") : false),
  
  // vertexes
  verticesTag(iConfig.getParameter<edm::InputTag>("vertices")),
  
  // electrons
  vetoelectronsTag(iConfig.getParameter<edm::InputTag>("vetoelectrons")),
  looseelectronsTag(iConfig.getParameter<edm::InputTag>("looseelectrons")),
  mediumelectronsTag(iConfig.getParameter<edm::InputTag>("mediumelectrons")),
  tightelectronsTag(iConfig.getParameter<edm::InputTag>("tightelectrons")),
  heepelectronsTag(iConfig.getParameter<edm::InputTag>("heepelectrons")),

  // more filters
  applyKinematicsFilter(iConfig.existsAs<bool>("applyKinematicsFilter") ? iConfig.getParameter<bool>("applyKinematicsFilter") : false),  

  //recHits
  recHitCollectionEBTAG(consumes<EcalRecHitCollection>(iConfig.getParameter<edm::InputTag>( "recHitCollectionEB" ))),
  recHitCollectionEETAG(consumes<EcalRecHitCollection>(iConfig.getParameter<edm::InputTag>( "recHitCollectionEE" ))),

  ///////////// GEN INFO
  // isMC or Data --> default Data
  isMC(iConfig.existsAs<bool>("isMC") ? iConfig.getParameter<bool>("isMC") : false),
  doZmassSort(iConfig.existsAs<bool>("doZmassSort") ? iConfig.getParameter<bool>("doZmassSort") : false)
{
  usesResource();
  usesResource("TFileService");

  // trigger tokens
  triggerResultsToken = consumes<edm::TriggerResults> (triggerResultsTag);
  
  //vertex
  verticesToken = consumes<std::vector<reco::Vertex> > (verticesTag);

  // electrons
  vetoelectronsToken   = consumes<pat::ElectronRefVector> (vetoelectronsTag);
  looseelectronsToken  = consumes<pat::ElectronRefVector> (looseelectronsTag);
  mediumelectronsToken = consumes<pat::ElectronRefVector> (mediumelectronsTag);
  tightelectronsToken  = consumes<pat::ElectronRefVector> (tightelectronsTag);
  heepelectronsToken   = consumes<pat::ElectronRefVector> (heepelectronsTag);

  // only for simulated samples
  if (isMC)
  {
    pileupInfoToken = consumes<std::vector<PileupSummaryInfo> > (iConfig.getParameter<edm::InputTag>("pileup"));
    genevtInfoToken = consumes<GenEventInfoProduct> (iConfig.getParameter<edm::InputTag>("genevt"));
    gensToken       = consumes<edm::View<reco::GenParticle> > (iConfig.getParameter<edm::InputTag>("gens"));   
  }
}

ZeeTree::~ZeeTree() {}

void ZeeTree::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) 
{
  // TRIGGER
  edm::Handle<edm::TriggerResults> triggerResultsH;
  iEvent.getByToken(triggerResultsToken, triggerResultsH);

  // VERTEX
  edm::Handle<std::vector<reco::Vertex> > verticesH;
  iEvent.getByToken(verticesToken, verticesH);

  // ELECTRONS
  edm::Handle<pat::ElectronRefVector> vetoelectronsH;
  iEvent.getByToken(vetoelectronsToken, vetoelectronsH);

  edm::Handle<pat::ElectronRefVector> looseelectronsH;
  iEvent.getByToken(looseelectronsToken, looseelectronsH);

  edm::Handle<pat::ElectronRefVector> mediumelectronsH;
  iEvent.getByToken(mediumelectronsToken, mediumelectronsH);

  edm::Handle<pat::ElectronRefVector> tightelectronsH;
  iEvent.getByToken(tightelectronsToken, tightelectronsH);
  pat::ElectronRefVector tightelectronsvec = *tightelectronsH;

  edm::Handle<pat::ElectronRefVector> heepelectronsH;
  iEvent.getByToken(heepelectronsToken, heepelectronsH);

  // geometry (from ECAL ELF)
  edm::ESHandle<CaloGeometry> geoHandle;
  iSetup.get<CaloGeometryRecord>().get(geoHandle);
  const CaloSubdetectorGeometry *barrelGeometry = geoHandle->getSubdetectorGeometry(DetId::Ecal, EcalBarrel);
  const CaloSubdetectorGeometry *endcapGeometry = geoHandle->getSubdetectorGeometry(DetId::Ecal, EcalEndcap);

  // Event, lumi, run info
  event = iEvent.id().event();
  run   = iEvent.id().run();
  lumi  = iEvent.luminosityBlock();
    
  // Trigger info
  hltdoubleel33 = false;
  hltdoubleel37 = false;

  // Which triggers fired
  if (triggerResultsH.isValid())
  {
    for (std::size_t i = 0; i < triggerPathsVector.size(); i++) 
    {
      if (triggerPathsMap[triggerPathsVector[i]] == -1) continue;	
      if (i == 0 && triggerResultsH->accept(triggerPathsMap[triggerPathsVector[i]])) hltdoubleel33 = true; // Double electron trigger (33-33)
      if (i == 1 && triggerResultsH->accept(triggerPathsMap[triggerPathsVector[i]])) hltdoubleel37 = true; // Double electron trigger (37-27)
    }
  }

  // skim on events that pass triggers
  bool triggered = false;
  if (hltdoubleel33 || hltdoubleel37) triggered = true;
  if (applyHLTFilter && !triggered) return;

  // Vertex info
  nvtx = -99; vtxX = -999.0; vtxY = -999.0; vtxZ = -999.0;
  if (verticesH.isValid()) 
  {
    nvtx = verticesH->size();
    const reco::Vertex primevtx = (*verticesH)[0];
    vtxX = primevtx.position().x();
    vtxY = primevtx.position().y();
    vtxZ = primevtx.position().z();
  }
  
  // ELECTRON ANALYSIS 
  // nelectrons AFTER PF cleaning (kinematic selection pT > 10, |eta| < 2.5
  nvetoelectrons = -99; nlooseelectrons = -99; nmediumelectrons = -99; ntightelectrons = -99; nheepelectrons = -99;
  if (vetoelectronsH.isValid())   nvetoelectrons   = vetoelectronsH->size();
  if (looseelectronsH.isValid())  nlooseelectrons  = looseelectronsH->size();
  if (mediumelectronsH.isValid()) nmediumelectrons = mediumelectronsH->size();
  if (tightelectronsH.isValid())  ntightelectrons  = tightelectronsH->size();
  if (heepelectronsH.isValid())   nheepelectrons   = heepelectronsH->size();
  
  // electron tree vars (initialize)
  el1pid  = -99;   el1pt   = -99.0; el1eta = -99.0; el1phi = -99.0; 
  el2pid  = -99;   el2pt   = -99.0; el2eta = -99.0; el2phi = -99.0; 
  el1E    = -99.0; el2E    = -99.0; el1p   = -99.0; el2p   = -99.0;

  // supercluster, rec hits, and seed info
  el1scX = -999.0; el1scY = -999.0; el1scZ = -999.0; el1scE = -999.0;
  el2scX = -999.0; el2scY = -999.0; el2scZ = -999.0; el2scE = -999.0;

  el1rhXs.clear(); el1rhYs.clear(); el1rhZs.clear(); el1rhEs.clear(); el1rhtimes.clear(); 
  el2rhXs.clear(); el2rhYs.clear(); el2rhZs.clear(); el2rhEs.clear(); el2rhtimes.clear(); 

  el1rhids.clear(); el1rhOOTs.clear(); el1rhgain1s.clear(); el1rhgain6s.clear();
  el2rhids.clear(); el2rhOOTs.clear(); el2rhgain1s.clear(); el2rhgain6s.clear();

  el1seedX = -999.0; el1seedY = -999.0; el1seedZ = -999.0; el1seedE = -999.0; el1seedtime = -999.0; 
  el2seedX = -999.0; el2seedY = -999.0; el2seedZ = -999.0; el2seedE = -999.0; el2seedtime = -999.0; 

  el1seedid = -99; el1seedOOT = -99; el1seedgain1 = -99; el1seedgain6 = -99;
  el2seedid = -99; el2seedOOT = -99; el2seedgain1 = -99; el2seedgain6 = -99;

  el1nrh = -99; el2nrh = -99;

  // z variables
  zmass = -99.0; zpt = -99.0; zeta = -99.0; zphi = -99.0; zE = -99.0; zp = -99.0;

       // save only really pure electrons
  std::vector<pat::ElectronRef> tightelectrons;
  for (std::size_t i = 0; i < tightelectronsvec.size(); i++) 
  {
    if (tightelectronsvec[i]->pt() > 35.) 
    {
      tightelectrons.push_back(tightelectronsvec[i]); 
    }
  }

  // Z matching + filling of variables
  if (tightelectrons.size()>1)  // need at least two electrons that pass id and pt cuts! 
  {
    // First sort on pT --> should be redudant, as already sorted this way in miniAOD
    std::sort(tightelectrons.begin(), tightelectrons.end(), sortElectronsByPt);

    pat::ElectronRef el1;
    pat::ElectronRef el2;

    if (doZmassSort) 
    {
      triplevec invmasspairs; // store i-j tight el index + invariant mass
      // only want pair of tight electrons that yield closest zmass diff   
      for (std::size_t i = 0; i < tightelectrons.size(); i++) 
      {
	const pat::ElectronRef el1 = tightelectrons[i];
	for (std::size_t j = i+1; j < tightelectrons.size(); j++) 
	{
	  const pat::ElectronRef el2 = tightelectrons[j];
	  TLorentzVector el1vec; el1vec.SetPtEtaPhiE(el1->pt(), el1->eta(), el1->phi(), el1->energy());
	  TLorentzVector el2vec; el2vec.SetPtEtaPhiE(el2->pt(), el2->eta(), el2->phi(), el2->energy());
	  el1vec += el2vec;
	  invmasspairs.push_back(std::make_tuple(i,j,std::abs(el1vec.M()-91.1876))); 
	}
      }
      auto best = std::min_element(invmasspairs.begin(),invmasspairs.end(),minimizeByZmass); // keep the lowest! --> returns pointer to lowest element
      el1 = tightelectrons[std::get<0>(*best)];
      el2 = tightelectrons[std::get<1>(*best)];
    }
    else
    { // just take highest pt electrons in event
      el1 = tightelectrons[0];
      el2 = tightelectrons[1];
    }

    // set the individual electron variables
    el1pid = el1->pdgId();  el2pid = el2->pdgId();
    el1pt  = el1->pt();     el2pt  = el2->pt();
    el1eta = el1->eta();    el2eta = el2->eta();
    el1phi = el1->phi();    el2phi = el2->phi();
    el1E   = el1->energy(); el2E   = el2->energy();
    el1p   = el1->p();      el2p   = el2->p();

    // ecal cluster tools --> stolen from ECAL ELF
    clustertools = new EcalClusterLazyTools (iEvent, iSetup, recHitCollectionEBTAG, recHitCollectionEETAG);

    // super cluster from electron 1
    const reco::SuperClusterRef& el1sc = el1->superCluster().isNonnull() ? el1->superCluster() : el1->parentSuperCluster();
    if (el1->ecalDrivenSeed() && el1sc.isNonnull()) 
    {
      // save some supercluster info
      el1scX = el1sc->position().x();
      el1scY = el1sc->position().y();
      el1scZ = el1sc->position().z();
      el1scE = el1sc->energy();

      // use seed to get geometry and recHits
      const DetId seedDetId = el1sc->seed()->seed(); //seed detid
      const bool isEB = (seedDetId.subdetId() == EcalBarrel); //which subdet
      const EcalRecHitCollection *recHits = isEB ? clustertools->getEcalEBRecHitCollection() : clustertools->getEcalEERecHitCollection();

      // loop over all crystals
      const DetIdPairVec hitsAndFractions = el1sc->hitsAndFractions(); // all crystals in SC
      for (DetIdPairVec::const_iterator hafitr = hitsAndFractions.begin(); hafitr != hitsAndFractions.end(); ++hafitr) 
      {
	const DetId recHitId = hafitr->first; // get detid of crystal
	EcalRecHitCollection::const_iterator recHit = recHits->find(recHitId); // get the underlying rechit

	if (recHit != recHits->end()) // standard check
        { 
	  // save position, energy, and time of each crystal to a vector
	  const auto recHitPos = isEB ? barrelGeometry->getGeometry(recHitId)->getPosition() : endcapGeometry->getGeometry(recHitId)->getPosition();
	  el1rhXs.push_back(recHitPos.x());
	  el1rhYs.push_back(recHitPos.y());
	  el1rhZs.push_back(recHitPos.z());
	  el1rhEs.push_back(recHit->energy());
	  el1rhtimes.push_back(recHit->time());	 
	  el1rhids.push_back(int(recHitId.rawId()));
	  el1rhOOTs.push_back(recHit->checkFlag(EcalRecHit::kOutofTime));
	  el1rhgain1s.push_back(recHit->checkFlag(EcalRecHit::kHasSwitchToGain1));
	  el1rhgain6s.push_back(recHit->checkFlag(EcalRecHit::kHasSwitchToGain6));

	  // save seed info in a flat branch
	  if (seedDetId == recHitId) 
          { 
	    el1seedX = el1rhXs.back();
	    el1seedY = el1rhYs.back();
	    el1seedZ = el1rhZs.back();
	    el1seedE = el1rhEs.back();
	    el1seedtime = el1rhtimes.back();
	    el1seedid = el1rhids.back();
	    el1seedOOT = el1rhOOTs.back();
	    el1seedgain1 = el1rhgain1s.back();
	    el1seedgain6 = el1rhgain6s.back();
	  }
	}
      } // end loop over all crystals
      el1nrh = el1rhtimes.size(); // save the number of valid rechits
    } // end check over supercluster

    // super cluster from electron 2
    const reco::SuperClusterRef& el2sc = el2->superCluster().isNonnull() ? el2->superCluster() : el2->parentSuperCluster();
    if (el2->ecalDrivenSeed() && el2sc.isNonnull()) 
    {
      // save some supercluster info
      el2scX = el2sc->position().x();
      el2scY = el2sc->position().y();
      el2scZ = el2sc->position().z();
      el2scE = el2sc->energy();

      // use seed to get geometry and recHits
      const DetId seedDetId = el2sc->seed()->seed(); //seed detid
      const bool isEB = (seedDetId.subdetId() == EcalBarrel); //which subdet
      const EcalRecHitCollection *recHits = isEB ? clustertools->getEcalEBRecHitCollection() : clustertools->getEcalEERecHitCollection();

      // loop over all crystals
      const DetIdPairVec hitsAndFractions = el2sc->hitsAndFractions(); // all crystals in SC
      for (DetIdPairVec::const_iterator hafitr = hitsAndFractions.begin(); hafitr != hitsAndFractions.end(); ++hafitr)
      {
	const DetId recHitId = hafitr->first; // get detid of crystal
	EcalRecHitCollection::const_iterator recHit = recHits->find(recHitId); // get the underlying rechit

	if (recHit != recHits->end()) // standard check
	{
	  // save position, energy, and time of each crystal to a vector
	  const auto recHitPos = isEB ? barrelGeometry->getGeometry(recHitId)->getPosition() : endcapGeometry->getGeometry(recHitId)->getPosition();
	  el2rhXs.push_back(recHitPos.x());
	  el2rhYs.push_back(recHitPos.y());
	  el2rhZs.push_back(recHitPos.z());
	  el2rhEs.push_back(recHit->energy());
	  el2rhtimes.push_back(recHit->time());	 
	  el2rhids.push_back(int(recHitId.rawId()));
	  el2rhOOTs.push_back(recHit->checkFlag(EcalRecHit::kOutofTime));
	  el2rhgain1s.push_back(recHit->checkFlag(EcalRecHit::kHasSwitchToGain1));
	  el2rhgain6s.push_back(recHit->checkFlag(EcalRecHit::kHasSwitchToGain6));

	  // save seed info in a flat branch
	  if (seedDetId == recHitId) 
          { 
	    el2seedX = el2rhXs.back();
	    el2seedY = el2rhYs.back();
	    el2seedZ = el2rhZs.back();
	    el2seedE = el2rhEs.back();
	    el2seedtime = el2rhtimes.back();
	    el2seedid = el2rhids.back();
	    el2seedOOT = el2rhOOTs.back();
	    el2seedgain1 = el2rhgain1s.back();
	    el2seedgain6 = el2rhgain6s.back();
	  }
	}
      } // end loop over all crystals
      el2nrh = el2rhtimes.size(); // save the number of valid rechits
    } // end check over supercluster

    // store Z information
    TLorentzVector el1vec; el1vec.SetPtEtaPhiE(el1pt, el1eta, el1phi, el1E);
    TLorentzVector el2vec; el2vec.SetPtEtaPhiE(el2pt, el2eta, el2phi, el2E);
      
    TLorentzVector zvec(el1vec);
    zvec += el2vec;
      
    zpt   = zvec.Pt();
    zeta  = zvec.Eta();
    zphi  = zvec.Phi();
    zmass = zvec.M();
    zE    = zvec.Energy();
    zp    = zvec.P();

    delete clustertools;
  } // end section over tight electrons
  else
  {
    if (applyKinematicsFilter) return;
  }

  // MC INFO    
  if (isMC) 
  {
    edm::Handle<std::vector<PileupSummaryInfo> > pileupInfoH;
    edm::Handle<GenEventInfoProduct>             genevtInfoH;
    edm::Handle<edm::View<reco::GenParticle> >         gensH;
    iEvent.getByToken(pileupInfoToken, pileupInfoH);
    iEvent.getByToken(genevtInfoToken, genevtInfoH);
    iEvent.getByToken(gensToken, gensH);

    // Pileup info
    puobs  = 0;
    putrue = 0;
    if (pileupInfoH.isValid()) 
    {
      for (auto pileupInfo_iter = pileupInfoH->begin(); pileupInfo_iter != pileupInfoH->end(); ++pileupInfo_iter) 
      {
	if (pileupInfo_iter->getBunchCrossing() == 0) 
        {
	  puobs  = pileupInfo_iter->getPU_NumInteractions();
	  putrue = pileupInfo_iter->getTrueNumInteractions();
	}
      }
    }
    
    // Event weight info
    wgt = 1.0;
    if (genevtInfoH.isValid()) {wgt = genevtInfoH->weight();}
  
    // Gen particles info
    genzpid   = -99;   genzpt   = -99.0; genzeta   = -99.0; genzphi   = -99.0; genzmass = -99.0; genzE = -99.0; genzp = -99.0;
    genel1pid = -99;   genel1pt = -99.0; genel1eta = -99.0; genel1phi = -99.0;
    genel2pid = -99;   genel2pt = -99.0; genel2eta = -99.0; genel2phi = -99.0;
    genel1E   = -99.0; genel2E  = -99.0; genel1p   = -99.0; genel2p   = -99.0;

    if (gensH.isValid())
    {
      // try to get the final state z
      for (auto gens_iter = gensH->begin(); gens_iter != gensH->end(); ++gens_iter) 
      {
  	if (gens_iter->pdgId() == 23 && gens_iter->numberOfDaughters() == 2 && abs(gens_iter->daughter(0)->pdgId()) == 11) // Final state Z --> ee
	{ 
	  // Z info
	  genzpid  = gens_iter->pdgId();
	  genzpt   = gens_iter->pt();
	  genzeta  = gens_iter->eta();
	  genzphi  = gens_iter->phi();
	  genzmass = gens_iter->mass();
	  genzE    = gens_iter->energy();
	  genzp    = gens_iter->p();
	  
	  // electron 1 info
	  genel1pid  = gens_iter->daughter(0)->pdgId();
	  genel1pt   = gens_iter->daughter(0)->pt();
	  genel1eta  = gens_iter->daughter(0)->eta();
	  genel1phi  = gens_iter->daughter(0)->phi();
	  genel1E    = gens_iter->daughter(0)->energy();
	  genel1p    = gens_iter->daughter(0)->p();
	  
	  // electron 2 info
	  genel2pid  = gens_iter->daughter(1)->pdgId();
	  genel2pt   = gens_iter->daughter(1)->pt();
	  genel2eta  = gens_iter->daughter(1)->eta();
	  genel2phi  = gens_iter->daughter(1)->phi();
	  genel2E    = gens_iter->daughter(1)->energy();
	  genel2p    = gens_iter->daughter(1)->p();

	} // end check over decay
      } // end loop over gen particles
    
      // if a Z is not found look for a pair of leptons ... 
      // this way when the pdgId is not guaranteed that you catch a Z boson, you can still recover DY production
      if (genzpid == -99) 
      {
  	for (auto gens_iter = gensH->begin(); gens_iter != gensH->end(); ++gens_iter) 
        {
  	  if (gens_iter->isPromptFinalState() || gens_iter->isPromptDecayed()) 
          {
  	    if (gens_iter->pdgId() == 11) 
            {
  	      genel1pid = gens_iter->pdgId();
  	      genel1pt  = gens_iter->pt();
  	      genel1eta = gens_iter->eta();
  	      genel1phi = gens_iter->phi();
  	      genel1E   = gens_iter->energy();
  	      genel1p   = gens_iter->p();
  	    }
  	    else if (gens_iter->pdgId() == -11) 
            {
  	      genel2pid = gens_iter->pdgId();
  	      genel2pt  = gens_iter->pt();
  	      genel2eta = gens_iter->eta();
  	      genel2phi = gens_iter->phi();
  	      genel2E   = gens_iter->energy();
  	      genel2p   = gens_iter->p();
  	    }
  	  } // end check over final state 
	} // end loop over gen particles
  	if (genel1pid == 11 && genel2pid == -11) 
        {
  	  TLorentzVector el1vec; el1vec.SetPtEtaPhiE(genel1pt, genel1eta, genel1phi, genel1E);
  	  TLorentzVector el2vec; el2vec.SetPtEtaPhiE(genel2pt, genel2eta, genel2phi, genel2E);

  	  TLorentzVector zvec(el1vec);
  	  zvec    += el2vec;

	  genzpid  = 23;
  	  genzpt   = zvec.Pt();
  	  genzeta  = zvec.Eta();
  	  genzphi  = zvec.Phi();
  	  genzmass = zvec.M();
  	  genzE    = zvec.Energy();
  	  genzp    = zvec.P();
  	}
      } // end recovery of DY check
    } // end check over gen particles are valid
  } // end check over isMC

  tree->Fill();    
}    

void ZeeTree::beginJob() 
{
  edm::Service<TFileService> fs;
  tree = fs->make<TTree>("tree"       , "tree");

  // Run, Lumi, Event info
  tree->Branch("event"                , &event                , "event/I");
  tree->Branch("run"                  , &run                  , "run/I");
  tree->Branch("lumi"                 , &lumi                 , "lumi/I");
  
  // Triggers
  tree->Branch("hltdoubleel37"        , &hltdoubleel37        , "hltdoubleel37/O");
  tree->Branch("hltdoubleel33"        , &hltdoubleel33        , "hltdoubleel33/O");

  // Vertex info
  tree->Branch("nvtx"                 , &nvtx                 , "nvtx/I");
  tree->Branch("vtxX"                 , &vtxX                 , "vtxX/F");
  tree->Branch("vtxY"                 , &vtxY                 , "vtxY/F");
  tree->Branch("vtxZ"                 , &vtxZ                 , "vtxZ/F");

  // Object counts
  tree->Branch("nvetoelectrons"       , &nvetoelectrons       , "nvetoelectrons/I");
  tree->Branch("nlooseelectrons"      , &nlooseelectrons      , "nlooseelectrons/I");
  tree->Branch("nmediumelectrons"     , &nmediumelectrons     , "nmediumelectrons/I");
  tree->Branch("ntightelectrons"      , &ntightelectrons      , "ntightelectrons/I");
  tree->Branch("nheepelectrons"       , &nheepelectrons       , "nheepelectrons/I");

  // Lepton info
  tree->Branch("el1pid"               , &el1pid               , "el1pid/I");
  tree->Branch("el1pt"                , &el1pt                , "el1pt/F");
  tree->Branch("el1eta"               , &el1eta               , "el1eta/F");
  tree->Branch("el1phi"               , &el1phi               , "el1phi/F");
  tree->Branch("el1E"                 , &el1E                 , "el1E/F");
  tree->Branch("el1p"                 , &el1p                 , "el1p/F");

  tree->Branch("el2pid"               , &el2pid               , "el2pid/I");
  tree->Branch("el2pt"                , &el2pt                , "el2pt/F");
  tree->Branch("el2eta"               , &el2eta               , "el2eta/F");
  tree->Branch("el2phi"               , &el2phi               , "el2phi/F");
  tree->Branch("el2E"                 , &el2E                 , "el2E/F");
  tree->Branch("el2p"                 , &el2p                 , "el2p/F");

  // supercluster stuff
  tree->Branch("el1scX"               , &el1scX               , "el1scX/F");
  tree->Branch("el1scY"               , &el1scY               , "el1scY/F");
  tree->Branch("el1scZ"               , &el1scZ               , "el1scZ/F");
  tree->Branch("el1scE"               , &el1scE               , "el1scE/F");

  tree->Branch("el2scX"               , &el2scX               , "el2scX/F");
  tree->Branch("el2scY"               , &el2scY               , "el2scY/F");
  tree->Branch("el2scZ"               , &el2scZ               , "el2scZ/F");
  tree->Branch("el2scE"               , &el2scE               , "el2scE/F");

  // all rechits
  tree->Branch("el1rhXs"              , "std::vector<float>"  , &el1rhXs);
  tree->Branch("el1rhYs"              , "std::vector<float>"  , &el1rhYs);
  tree->Branch("el1rhZs"              , "std::vector<float>"  , &el1rhZs);
  tree->Branch("el1rhEs"              , "std::vector<float>"  , &el1rhEs);
  tree->Branch("el1rhtimes"           , "std::vector<float>"  , &el1rhtimes);

  tree->Branch("el2rhXs"              , "std::vector<float>"  , &el2rhXs);
  tree->Branch("el2rhYs"              , "std::vector<float>"  , &el2rhYs);
  tree->Branch("el2rhZs"              , "std::vector<float>"  , &el2rhZs);
  tree->Branch("el2rhEs"              , "std::vector<float>"  , &el2rhEs);
  tree->Branch("el2rhtimes"           , "std::vector<float>"  , &el2rhtimes);

  tree->Branch("el1rhids"             , "std::vector<int>"    , &el1rhids);
  tree->Branch("el1rhOOTs"            , "std::vector<int>"    , &el1rhOOTs);
  tree->Branch("el1rhgain1s"          , "std::vector<int>"    , &el1rhgain1s);
  tree->Branch("el1rhgain6s"          , "std::vector<int>"    , &el1rhgain6s);

  tree->Branch("el2rhids"             , "std::vector<int>"    , &el2rhids);
  tree->Branch("el2rhOOTs"            , "std::vector<int>"    , &el2rhOOTs);
  tree->Branch("el2rhgain1s"          , "std::vector<int>"    , &el2rhgain1s);
  tree->Branch("el2rhgain6s"          , "std::vector<int>"    , &el2rhgain6s);

  // seed crystal info
  tree->Branch("el1seedX"             , &el1seedX             , "el1seedX/F");
  tree->Branch("el1seedY"             , &el1seedY             , "el1seedY/F");
  tree->Branch("el1seedZ"             , &el1seedZ             , "el1seedZ/F");
  tree->Branch("el1seedE"             , &el1seedE             , "el1seedE/F");
  tree->Branch("el1seedtime"          , &el1seedtime          , "el1seedtime/F");

  tree->Branch("el2seedX"             , &el2seedX             , "el2seedX/F");
  tree->Branch("el2seedY"             , &el2seedY             , "el2seedY/F");
  tree->Branch("el2seedZ"             , &el2seedZ             , "el2seedZ/F");
  tree->Branch("el2seedE"             , &el2seedE             , "el2seedE/F");
  tree->Branch("el2seedtime"          , &el2seedtime          , "el2seedtime/F");

  tree->Branch("el1seedid"            , &el1seedid            , "el1seedid/I");
  tree->Branch("el1seedOOT"           , &el1seedOOT           , "el1seedOOT/I");
  tree->Branch("el1seedgain1"         , &el1seedgain1         , "el1seedgain1/I");
  tree->Branch("el1seedgain6"         , &el1seedgain6         , "el1seedgain6/I");

  tree->Branch("el2seedid"            , &el2seedid            , "el2seedid/I");
  tree->Branch("el2seedOOT"           , &el2seedOOT           , "el2seedOOT/I");
  tree->Branch("el2seedgain1"         , &el2seedgain1         , "el2seedgain1/I");
  tree->Branch("el2seedgain6"         , &el2seedgain6         , "el2seedgain6/I");

  tree->Branch("el1nrh"               , &el1nrh               , "el1nrh/I");  
  tree->Branch("el2nrh"               , &el2nrh               , "el2nrh/I");  

  // Dilepton info
  tree->Branch("zmass"                , &zmass                , "zmass/F");
  tree->Branch("zpt"                  , &zpt                  , "zpt/F");
  tree->Branch("zeta"                 , &zeta                 , "zeta/F");
  tree->Branch("zphi"                 , &zphi                 , "zphi/F");
  tree->Branch("zE"                   , &zE                   , "zE/F");
  tree->Branch("zp"                   , &zp                   , "zp/F");
  
  // Z gen-level info: leptonic 
  if (isMC) 
  {
    // Pileup info
    tree->Branch("puobs"                , &puobs                , "puobs/I");
    tree->Branch("putrue"               , &putrue               , "putrue/I");

    // Event weights
    tree->Branch("wgt"                  , &wgt                  , "wgt/F");

    //Gen particles info
    tree->Branch("genzpid"              , &genzpid              , "genzpid/I");
    tree->Branch("genzpt"               , &genzpt               , "genzpt/F");
    tree->Branch("genzeta"              , &genzeta              , "genzeta/F");
    tree->Branch("genzphi"              , &genzphi              , "genzphi/F");
    tree->Branch("genzmass"             , &genzmass             , "genzmass/F");
    tree->Branch("genzE"                , &genzE                , "genzE/F");
    tree->Branch("genzp"                , &genzp                , "genzp/F");

    tree->Branch("genel1pid"            , &genel1pid            , "genel1pid/I");
    tree->Branch("genel1pt"             , &genel1pt             , "genel1pt/F");
    tree->Branch("genel1eta"            , &genel1eta            , "genel1eta/F");
    tree->Branch("genel1phi"            , &genel1phi            , "genel1phi/F");
    tree->Branch("genel1E"              , &genel1E              , "genel1E/F");
    tree->Branch("genel1p"              , &genel1p              , "genel1p/F");

    tree->Branch("genel2pid"            , &genel2pid            , "genel2pid/I");
    tree->Branch("genel2pt"             , &genel2pt             , "genel2pt/F");
    tree->Branch("genel2eta"            , &genel2eta            , "genel2eta/F");
    tree->Branch("genel2phi"            , &genel2phi            , "genel2phi/F");
    tree->Branch("genel2E"              , &genel2E              , "genel2E/F");
    tree->Branch("genel2p"              , &genel2p              , "genel2p/F");
  }
}

void ZeeTree::endJob() {}

void ZeeTree::beginRun(edm::Run const& iRun, edm::EventSetup const& iSetup) 
{
  // triggers for the Analysis
  //  triggerPathsVector.push_back("HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL_DZ");
  triggerPathsVector.push_back("HLT_DoubleEle33_CaloIdL_GsfTrkIdVL_v");
  triggerPathsVector.push_back("HLT_DoubleEle37_Ele27_CaloIdL_GsfTrkIdVL");

  triggerPathsVector.push_back("HLT_Diphoton30_18_R9Id_OR_IsoCaloId_AND_HE_R9Id_Mass90");
  triggerPathsVector.push_back("HLT_Diphoton30_18_R9Id_OR_IsoCaloId_AND_HE_R9Id_DoublePixelSeedMatch_Mass70");
  triggerPathsVector.push_back("HLT_Diphoton30PV_18PV_R9Id_AND_IsoCaloId_AND_HE_R9Id_DoublePixelVeto_Mass55");
  
  HLTConfigProvider hltConfig;
  bool changedConfig = false;
  hltConfig.init(iRun, iSetup, triggerResultsTag.process(), changedConfig);
  
  for (size_t i = 0; i < triggerPathsVector.size(); i++) 
  {
    triggerPathsMap[triggerPathsVector[i]] = -1;
  }
  
  for(size_t i = 0; i < triggerPathsVector.size(); i++)
  {
    TPRegexp pattern(triggerPathsVector[i]);
    for(size_t j = 0; j < hltConfig.triggerNames().size(); j++)
    {
      std::string pathName = hltConfig.triggerNames()[j];
      if(TString(pathName).Contains(pattern))
      {
	triggerPathsMap[triggerPathsVector[i]] = j;
      }
    }
  }
}

void ZeeTree::endRun(edm::Run const&, edm::EventSetup const&) {}

void ZeeTree::fillDescriptions(edm::ConfigurationDescriptions& descriptions) 
{
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

DEFINE_FWK_MODULE(ZeeTree);
