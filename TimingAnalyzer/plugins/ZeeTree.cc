#include "ZeeTree.h"

ZeeTree::ZeeTree(const edm::ParameterSet& iConfig): 
  ///////////// TRIGGER and filter info INFO
  triggerResultsTag(iConfig.getParameter<edm::InputTag>("triggerResults")),
  applyHLTFilter(iConfig.existsAs<bool>("applyHLTFilter") ? iConfig.getParameter<bool>("applyHLTFilter") : false),
  
  // vertexes
  verticesTag(iConfig.getParameter<edm::InputTag>("vertices")),
  
  // electrons
  electronVetoIdMapTag  (iConfig.getParameter<edm::InputTag>("vetoElectronID")),  
  electronLooseIdMapTag (iConfig.getParameter<edm::InputTag>("looseElectronID")),  
  electronMediumIdMapTag(iConfig.getParameter<edm::InputTag>("mediumElectronID")),  
  electronTightIdMapTag (iConfig.getParameter<edm::InputTag>("tightElectronID")),  
  electronsTag          (iConfig.getParameter<edm::InputTag>("electrons")),  

  // more filters
  applyKinematicsFilter(iConfig.existsAs<bool>("applyKinematicsFilter") ? iConfig.getParameter<bool>("applyKinematicsFilter") : false),  

  //recHits
  recHitsEBTag(iConfig.getParameter<edm::InputTag>("recHitsEB")),  
  recHitsEETag(iConfig.getParameter<edm::InputTag>("recHitsEE")),

  ///////////// GEN INFO
  // isMC or Data --> default Data
  isMC(iConfig.existsAs<bool>("isMC") ? iConfig.getParameter<bool>("isMC") : false)
{
  usesResource();
  usesResource("TFileService");

  // trigger tokens
  triggerResultsToken = consumes<edm::TriggerResults> (triggerResultsTag);
  
  //vertex
  verticesToken = consumes<std::vector<reco::Vertex> > (verticesTag);

  // electrons
  electronVetoIdMapToken   = consumes<edm::ValueMap<bool> > (electronVetoIdMapTag);
  electronLooseIdMapToken  = consumes<edm::ValueMap<bool> > (electronLooseIdMapTag);
  electronMediumIdMapToken = consumes<edm::ValueMap<bool> > (electronMediumIdMapTag);
  electronTightIdMapToken  = consumes<edm::ValueMap<bool> > (electronTightIdMapTag);
  electronsToken           = consumes<std::vector<pat::Electron> > (electronsTag);

  // recHits
  recHitsEBToken = consumes<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > (recHitsEBTag);
  recHitsEEToken = consumes<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > (recHitsEETag);

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

  // ELECTRONS + ID
  edm::Handle<edm::ValueMap<bool> > electronVetoIdMapH;
  iEvent.getByToken(electronVetoIdMapToken, electronVetoIdMapH);
  edm::ValueMap<bool> electronVetoIdMap = *electronVetoIdMapH;

  edm::Handle<edm::ValueMap<bool> > electronLooseIdMapH;
  iEvent.getByToken(electronLooseIdMapToken, electronLooseIdMapH);
  edm::ValueMap<bool> electronLooseIdMap = *electronLooseIdMapH;

  edm::Handle<edm::ValueMap<bool> > electronMediumIdMapH;
  iEvent.getByToken(electronMediumIdMapToken, electronMediumIdMapH);
  edm::ValueMap<bool> electronMediumIdMap = *electronMediumIdMapH;

  edm::Handle<edm::ValueMap<bool> > electronTightIdMapH;
  iEvent.getByToken(electronTightIdMapToken, electronTightIdMapH);
  edm::ValueMap<bool> electronTightIdMap = *electronTightIdMapH;

  edm::Handle<std::vector<pat::Electron> > electronsH;
  iEvent.getByToken(electronsToken, electronsH);
  std::vector<pat::Electron> electrons = *electronsH;

  // Prep electrons
  ZeeTree::PrepElectrons(electronsH,electronVetoIdMap,electronLooseIdMap,electronMediumIdMap,electronTightIdMap,electrons);

  // recHits
  edm::Handle<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > recHitsEBH;
  iEvent.getByToken(recHitsEBToken, recHitsEBH);
  const EcalRecHitCollection * recHitsEB = recHitsEBH.product();

  edm::Handle<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > recHitsEEH;
  iEvent.getByToken(recHitsEEToken, recHitsEEH);
  const EcalRecHitCollection * recHitsEE = recHitsEEH.product();

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
  nvtx = -9999; vtxX = -9999.0; vtxY = -9999.0; vtxZ = -9999.0;
  if (verticesH.isValid()) 
  {
    nvtx = verticesH->size();
    const reco::Vertex primevtx = (*verticesH)[0];
    vtxX = primevtx.position().x();
    vtxY = primevtx.position().y();
    vtxZ = primevtx.position().z();
  }
  
  // ELECTRON ANALYSIS 
  // electron tree vars (initialize)
  el1pid  = -9999;   el1pt   = -9999.0; el1eta = -9999.0; el1phi = -9999.0; 
  el2pid  = -9999;   el2pt   = -9999.0; el2eta = -9999.0; el2phi = -9999.0; 
  el1E    = -9999.0; el2E    = -9999.0; el1p   = -9999.0; el2p   = -9999.0;

  // supercluster info
  el1scX = -9999.0; el1scY = -9999.0; el1scZ = -9999.0; el1scE = -9999.0;
  el2scX = -9999.0; el2scY = -9999.0; el2scZ = -9999.0; el2scE = -9999.0;

  // rechits info
  el1nrh = -9999; el1seedpos = -9999;
  el2nrh = -9999; el2seedpos = -9999;

  el1rhXs.clear(); el1rhYs.clear(); el1rhZs.clear(); el1rhEs.clear(); el1rhtimes.clear(); 
  el2rhXs.clear(); el2rhYs.clear(); el2rhZs.clear(); el2rhEs.clear(); el2rhtimes.clear(); 

  el1rhids.clear(); el1rhOOTs.clear(); el1rhgain1s.clear(); el1rhgain6s.clear();
  el2rhids.clear(); el2rhOOTs.clear(); el2rhgain1s.clear(); el2rhgain6s.clear();

  // z variables
  zmass = -9999.0; zpt = -9999.0; zeta = -9999.0; zphi = -9999.0; zE = -9999.0; zp = -9999.0;

  // save only really pure electrons
  std::vector<pat::Electron> goodelectrons;
  for (std::vector<pat::Electron>::const_iterator eliter = electrons.begin(); eliter != electrons.end(); ++eliter)
  {
    bool saveElectron = false;

    if (eliter->pt() > 35.f && std::abs(eliter->superCluster()->eta()) < 2.5 && eliter->electronID("tight"))  
    {
      const reco::SuperClusterRef& elsc = eliter->superCluster().isNonnull() ? eliter->superCluster() : eliter->parentSuperCluster();
      if (eliter->ecalDrivenSeed() && elsc.isNonnull()) 
      {
	// use seed to get geometry and recHits
	const DetId seedDetId = elsc->seed()->seed(); //seed detid
	const bool isEB = (seedDetId.subdetId() == EcalBarrel); //which subdet
	const EcalRecHitCollection *recHits = isEB ? recHitsEB : recHitsEE;

	// loop over all crystals
	const DetIdPairVec hitsAndFractions = elsc->hitsAndFractions(); // all crystals in SC
	for (DetIdPairVec::const_iterator hafitr = hitsAndFractions.begin(); hafitr != hitsAndFractions.end(); ++hafitr) 
        {
	  const DetId recHitId = hafitr->first; // get detid of crystal
	  EcalRecHitCollection::const_iterator recHit = recHits->find(recHitId); // get the underlying rechit

	  if (recHit != recHits->end() && seedDetId == recHitId) // standard check
	  { 
	    saveElectron = true;
	    break;
	  }
	} 
      }
    }
    if (saveElectron) goodelectrons.push_back((*eliter));   
  }

  // Z matching + filling of variables
  triplevec invmasspairs; // store i-j good el index + invariant mass
  if (goodelectrons.size()>1)  // need at least two electrons that pass id and pt cuts! 
  {
    // only want pair of good electrons that yield closest zmass diff   
    for (std::size_t i = 0; i < goodelectrons.size(); i++) 
    {
      const pat::Electron& el1_tmp = goodelectrons[i];
      for (std::size_t j = i+1; j < goodelectrons.size(); j++) 
      {
	const pat::Electron& el2_tmp = goodelectrons[j];
	int el1pdgId = el1_tmp.pdgId();
	int el2pdgId = el2_tmp.pdgId();
	if (el1pdgId == -el2pdgId)
        {
	  TLorentzVector el1vec; el1vec.SetPtEtaPhiE(el1_tmp.pt(), el1_tmp.eta(), el1_tmp.phi(), el1_tmp.energy());
	  TLorentzVector el2vec; el2vec.SetPtEtaPhiE(el2_tmp.pt(), el2_tmp.eta(), el2_tmp.phi(), el2_tmp.energy());
	  el1vec += el2vec;
	  invmasspairs.push_back(std::make_tuple(i,j,std::abs(el1vec.M()-91.1876))); 
	}
      }
    }
  }
  else
  {
    if (applyKinematicsFilter) return;
  }

  if (invmasspairs.size()>0)
  {
    // Get the best pair (if it exists)
    auto best = std::min_element(invmasspairs.begin(),invmasspairs.end(),minimizeByZmass); // keep the lowest! --> returns pointer to lowest element
    pat::Electron el1 = goodelectrons[std::get<0>(*best)];
    pat::Electron el2 = goodelectrons[std::get<1>(*best)];
  
    // set the individual electron variables
    el1pid = el1.pdgId();  el2pid = el2.pdgId();
    el1pt  = el1.pt();     el2pt  = el2.pt();
    el1eta = el1.eta();    el2eta = el2.eta();
    el1phi = el1.phi();    el2phi = el2.phi();
    el1E   = el1.energy(); el2E   = el2.energy();
    el1p   = el1.p();      el2p   = el2.p();

    // super cluster from electron 1
    const reco::SuperClusterRef& el1sc = el1.superCluster().isNonnull() ? el1.superCluster() : el1.parentSuperCluster();
    if (el1.ecalDrivenSeed() && el1sc.isNonnull()) 
    {
      // save some supercluster info
      el1scX = el1sc->position().x();
      el1scY = el1sc->position().y();
      el1scZ = el1sc->position().z();
      el1scE = el1sc->energy();

      // use seed to get geometry and recHits
      const DetId seedDetId = el1sc->seed()->seed(); //seed detid
      const bool isEB = (seedDetId.subdetId() == EcalBarrel); //which subdet
      const EcalRecHitCollection *recHits = isEB ? recHitsEB : recHitsEE;

      // map of rec hit ids
      uiiumap el1rhIDmap;
      
      // all rechits in superclusters
      const DetIdPairVec hitsAndFractions = el1sc->hitsAndFractions();

      // only count each recHit once
      for (DetIdPairVec::const_iterator hafitr = hitsAndFractions.begin(); hafitr != hitsAndFractions.end(); ++hafitr) // loop over all rec hits in SC
      {
	const DetId recHitId = hafitr->first; // get detid of crystal
	EcalRecHitCollection::const_iterator recHit = recHits->find(recHitId); // get the underlying rechit
	if (recHit != recHits->end()) // standard check
        { 
	  el1rhIDmap[recHitId.rawId()]++;
	} // end standard check recHit
      } // end loop over hits and fractions

      for (uiiumap::const_iterator rhiter = el1rhIDmap.begin(); rhiter != el1rhIDmap.end(); ++rhiter) // loop over only good rec hit ids
      {
	// get the underlying recHit
	const uint32_t rhID = rhiter->first;
	const DetId recHitId(rhID);
	EcalRecHitCollection::const_iterator recHit = recHits->find(recHitId); // get the underlying rechit
	
	// get the position
	const auto recHitPos = isEB ? barrelGeometry->getGeometry(recHitId)->getPosition() : endcapGeometry->getGeometry(recHitId)->getPosition();

	// push back the values
	el1rhXs.push_back(recHitPos.x());
	el1rhYs.push_back(recHitPos.y());
	el1rhZs.push_back(recHitPos.z());
	el1rhEs.push_back(recHit->energy());
	el1rhtimes.push_back(recHit->time());	 
	el1rhids.push_back(int(rhID));
	el1rhOOTs.push_back(recHit->checkFlag(EcalRecHit::kOutOfTime));
	el1rhgain1s.push_back(recHit->checkFlag(EcalRecHit::kHasSwitchToGain1));
	el1rhgain6s.push_back(recHit->checkFlag(EcalRecHit::kHasSwitchToGain6));
	
	// extra info from the SEED
	if (seedDetId.rawId() == recHitId) 
	{ 
	  el1seedpos = el1rhEs.size()-1;
	}
      } // end loop over rec hit id map
    } // end check over electron1 supercluster

    // super cluster from electron 2
    const reco::SuperClusterRef& el2sc = el2.superCluster().isNonnull() ? el2.superCluster() : el2.parentSuperCluster();
    if (el2.ecalDrivenSeed() && el2sc.isNonnull()) 
    {
      // save some supercluster info
      el2scX = el2sc->position().x();
      el2scY = el2sc->position().y();
      el2scZ = el2sc->position().z();
      el2scE = el2sc->energy();

      // use seed to get geometry and recHits
      const DetId seedDetId = el2sc->seed()->seed(); //seed detid
      const bool isEB = (seedDetId.subdetId() == EcalBarrel); //which subdet
      const EcalRecHitCollection *recHits = isEB ? recHitsEB : recHitsEE;

      // map of rec hit ids
      uiiumap el2rhIDmap;
      
      // all rechits in superclusters
      const DetIdPairVec hitsAndFractions = el2sc->hitsAndFractions();

      // only count each recHit once
      for (DetIdPairVec::const_iterator hafitr = hitsAndFractions.begin(); hafitr != hitsAndFractions.end(); ++hafitr) // loop over all rec hits in SC
      {
	const DetId recHitId = hafitr->first; // get detid of crystal
	EcalRecHitCollection::const_iterator recHit = recHits->find(recHitId); // get the underlying rechit
	if (recHit != recHits->end()) // standard check
        { 
	  el2rhIDmap[recHitId.rawId()]++;
	} // end standard check recHit
      } // end loop over hits and fractions

      for (uiiumap::const_iterator rhiter = el2rhIDmap.begin(); rhiter != el2rhIDmap.end(); ++rhiter) // loop over only good rec hit ids
      {
	// get the underlying recHit
	const uint32_t rhID = rhiter->first;
	const DetId recHitId(rhID);
	EcalRecHitCollection::const_iterator recHit = recHits->find(recHitId); // get the underlying rechit
	
	// get the position
	const auto recHitPos = isEB ? barrelGeometry->getGeometry(recHitId)->getPosition() : endcapGeometry->getGeometry(recHitId)->getPosition();

	// push back the values
	el2rhXs.push_back(recHitPos.x());
	el2rhYs.push_back(recHitPos.y());
	el2rhZs.push_back(recHitPos.z());
	el2rhEs.push_back(recHit->energy());
	el2rhtimes.push_back(recHit->time());	 
	el2rhids.push_back(int(rhID));
	el2rhOOTs.push_back(recHit->checkFlag(EcalRecHit::kOutOfTime));
	el2rhgain1s.push_back(recHit->checkFlag(EcalRecHit::kHasSwitchToGain1));
	el2rhgain6s.push_back(recHit->checkFlag(EcalRecHit::kHasSwitchToGain6));
	
	// extra info from the SEED
	if (seedDetId.rawId() == recHitId) 
	{ 
	  el2seedpos = el2rhEs.size()-1;
	}
      } // end loop over rec hit id map
    } // end check over electron2 supercluster

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
  } // end section over two good electrons
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
    genzpid   = -9999;   genzpt   = -9999.0; genzeta   = -9999.0; genzphi   = -9999.0; genzmass = -9999.0; genzE = -9999.0; genzp = -9999.0;
    genel1pid = -9999;   genel1pt = -9999.0; genel1eta = -9999.0; genel1phi = -9999.0;
    genel2pid = -9999;   genel2pt = -9999.0; genel2eta = -9999.0; genel2phi = -9999.0;
    genel1E   = -9999.0; genel2E  = -9999.0; genel1p   = -9999.0; genel2p   = -9999.0;

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

void ZeeTree::PrepElectrons(const edm::Handle<std::vector<pat::Electron> > & electronsH, 
			    const edm::ValueMap<bool> & electronVetoIdMap, 
			    const edm::ValueMap<bool> & electronLooseIdMap, 
			    const edm::ValueMap<bool> & electronMediumIdMap, 
			    const edm::ValueMap<bool> & electronTightIdMap, 
			    std::vector<pat::Electron> & electrons)
{
  if (electronsH.isValid()) // standard handle check
  {
    // create and initialize temp id-value vector
    std::vector<std::vector<pat::Electron::IdPair> > idpairs(electrons.size());
    for (size_t iph = 0; iph < idpairs.size(); iph++)
    {
      idpairs[iph].resize(4);
      idpairs[iph][0] = {"veto"  ,false};
      idpairs[iph][1] = {"loose" ,false};
      idpairs[iph][2] = {"medium",false};
      idpairs[iph][3] = {"tight" ,false};
    }

    int iphH = 0; // dumb counter because iterators only work with VID
    for (std::vector<pat::Electron>::const_iterator phiter = electronsH->begin(); phiter != electronsH->end(); ++phiter) // loop over electron vector
    {
      // Get the VID of the electron
      const edm::Ptr<pat::Electron> electronPtr(electronsH, phiter - electronsH->begin());

      // store VID in temp struct
      // veto < loose < medium < tight
      if (electronVetoIdMap  [electronPtr]) idpairs[iphH][0].second = true;
      if (electronLooseIdMap [electronPtr]) idpairs[iphH][1].second = true;
      if (electronMediumIdMap[electronPtr]) idpairs[iphH][2].second = true;
      if (electronTightIdMap [electronPtr]) idpairs[iphH][3].second = true;
      
      iphH++;
    }
    
    // set the ID-value for each electron in other collection
    for (size_t iph = 0; iph < electrons.size(); iph++)
    {
      electrons[iph].setElectronIDs(idpairs[iph]);
    }
    
    // now finally sort vector by pT
    std::sort(electrons.begin(),electrons.end(),sortByElectronPt);
  }
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

  // seed crystal info
  tree->Branch("el1nrh"               , &el1nrh               , "el1nrh/I");  
  tree->Branch("el1seedpos"           , &el1seedpos           , "el1seedpos/I");  
  tree->Branch("el2nrh"               , &el2nrh               , "el2nrh/I");  
  tree->Branch("el2seedpos"           , &el2seedpos           , "el2seedpos/I");  

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
