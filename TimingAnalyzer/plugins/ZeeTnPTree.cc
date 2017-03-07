#include "ZeeTnPTree.h"

ZeeTnPTree::ZeeTnPTree(const edm::ParameterSet& iConfig): 
  // triggers
  triggerResultsTag(iConfig.getParameter<edm::InputTag>("triggerResults")),
  
  // vertices
  verticesTag(iConfig.getParameter<edm::InputTag>("vertices")),
  
  // electrons
  electronVetoIdMapTag  (iConfig.getParameter<edm::InputTag>("vetoElectronID")),  
  electronLooseIdMapTag (iConfig.getParameter<edm::InputTag>("looseElectronID")),  
  electronMediumIdMapTag(iConfig.getParameter<edm::InputTag>("mediumElectronID")),  
  electronTightIdMapTag (iConfig.getParameter<edm::InputTag>("tightElectronID")),  
  electronsTag          (iConfig.getParameter<edm::InputTag>("electrons")),  

  // recHits
  recHitsEBTag(iConfig.getParameter<edm::InputTag>("recHitsEB")),  
  recHitsEETag(iConfig.getParameter<edm::InputTag>("recHitsEE"))
{
  usesResource();
  usesResource("TFileService");

  // triggers
  triggerResultsToken = consumes<edm::TriggerResults> (triggerResultsTag);
  
  // vertices
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
}

ZeeTnPTree::~ZeeTnPTree() {}

void ZeeTnPTree::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) 
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
  ZeeTnPTree::PrepElectrons(electronsH,electronVetoIdMap,electronLooseIdMap,electronMediumIdMap,electronTightIdMap,electrons);

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
  hltdoubleel23_12 = false;
  hltdoubleel33_33 = false;
  hltdoubleel37_27 = false;

  // Which triggers fired
  if (triggerResultsH.isValid())
  {
    for (std::size_t i = 0; i < triggerPathsVector.size(); i++) 
    {
      if (triggerPathsMap[triggerPathsVector[i]] == -1) continue;	
      if (i == 0 && triggerResultsH->accept(triggerPathsMap[triggerPathsVector[i]])) hltdoubleel23_12 = true; // Double electron trigger (23-12)
      if (i == 1 && triggerResultsH->accept(triggerPathsMap[triggerPathsVector[i]])) hltdoubleel23_12 = true; // Double electron trigger (23-12)
      if (i == 2 && triggerResultsH->accept(triggerPathsMap[triggerPathsVector[i]])) hltdoubleel33_33 = true; // Double electron trigger (33-33)
      if (i == 3 && triggerResultsH->accept(triggerPathsMap[triggerPathsVector[i]])) hltdoubleel33_33 = true; // Double electron trigger (33-33)
      if (i == 4 && triggerResultsH->accept(triggerPathsMap[triggerPathsVector[i]])) hltdoubleel37_27 = true; // Double electron trigger (37-27)
    }
  }

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
  // First build the tags
  // Then build tnp pairs with mass
  // Pick pair with mass closest to Z
  // save it all

  // electron tree vars (initialize)
  el1pid = -9999; el1vid = -9999; el1E = -9999.0; el1p = -9999; el1pt = -9999.0; el1eta = -9999.0; el1phi = -9999.0; 
  el2pid = -9999; el2vid = -9999; el2E = -9999.0; el2p = -9999; el2pt = -9999.0; el2eta = -9999.0; el2phi = -9999.0; 

  // supercluster info
  el1scX = -9999.0; el1scY = -9999.0; el1scZ = -9999.0; el1scE = -9999.0;
  el2scX = -9999.0; el2scY = -9999.0; el2scZ = -9999.0; el2scE = -9999.0;

  // rechits info
  el1seedX = -9999.0; el1seedY = -9999.0; el1seedZ = -9999.0; el1seedE = -9999.0; el1seedtime = -9999.0; 
  el2seedX = -9999.0; el2seedY = -9999.0; el2seedZ = -9999.0; el2seedE = -9999.0; el2seedtime = -9999.0; 

  el1seedid = -9999; el1seedOOT = -9999; el1seedgain1 = -9999; el1seedgain6 = -9999;
  el2seedid = -9999; el2seedOOT = -9999; el2seedgain1 = -9999; el2seedgain6 = -9999;

  // z variables
  zmass = -9999.0; zE = -9999.0; zp = -9999.0; zpt = -9999.0; zeta = -9999.0; zphi = -9999.0;

  // First build the tags
  std::vector<std::size_t> tagelectrons;
  for (std::size_t i = 0; i < electrons.size(); i++)
  {
    bool saveElectron = false;

    // get the electron
    const pat::Electron& electron = electrons[i];

    // fidicual cuts
    float eta = std::abs(electron.superCluster()->eta());
    if (eta > 2.5 || (eta > 1.4442 && eta < 1.566)) continue;

    // quality cuts
    if (electron.pt() < 35.f) continue;
    if (!electron.electronID("tight")) continue;

    // check to make sure seed exists!
    const reco::SuperClusterRef& elsc = electron.superCluster().isNonnull() ? electron.superCluster() : electron.parentSuperCluster();
    if (electron.ecalDrivenSeed() && elsc.isNonnull()) 
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
	
	if (recHit != recHits->end() && seedDetId == recHitId && !recHit->checkFlag(EcalRecHit::kOutOfTime)) // ensure seed actually exists and is marked in time
	{ 
	  saveElectron = true;
	  break;
	} // end check over seed id 
      } // end loop over all rec hits in SC
    } // end check over valid SC
    if (saveElectron) tagelectrons.push_back(i); // save index of tag electron
  } // end loop over all electrons

  // Now build the tnppairs
  tuplevec tnpelectrons;
  for (std::size_t i = 0; i < tagelectrons.size(); i++)
  {
    // get the electron
    const pat::Electron& tagelectron = electrons[tagelectrons[i]];

    for (std::size_t j = 0; j < electrons.size(); j++)
    {
      if (tagelectrons[i] == j) continue; // skip the electrons that are the same!
      const pat::Electron& electron = electrons[j]; // potential probe

      // fidicual cuts
      float eta = std::abs(electron.superCluster()->eta());
      if (eta > 2.5 || (eta > 1.4442 && eta < 1.566)) continue;
      
      // only want opposite signed pairs
      if (tagelectron.pdgId() != -electron.pdgId()) continue;
  
      // still check to make sure seed exists
      bool seedOK = false;
      const reco::SuperClusterRef& elsc = electron.superCluster().isNonnull() ? electron.superCluster() : electron.parentSuperCluster();
      if (electron.ecalDrivenSeed() && elsc.isNonnull()) 
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
	    seedOK = true;
	    break;
	  } // end check over seed id 
	} // end loop over all rec hits in SC
      } // end check over valid SC
      
      // need the seed to exist to be a good probe!
      if (!seedOK) continue;
      
      TLorentzVector el1vec; el1vec.SetPtEtaPhiE(tagelectron.pt(), tagelectron.eta(), tagelectron.phi(), tagelectron.energy());
      TLorentzVector el2vec; el2vec.SetPtEtaPhiE(   electron.pt(),    electron.eta(),    electron.phi(),    electron.energy());
      el1vec += el2vec;
      
      tnpelectrons.push_back(std::make_tuple(tagelectrons[i],j,std::abs(el1vec.M()-91.1876))); // save diff from Z mass
    } // end loop over potential probes
  } // end loop over tags

  // check to make sure at least one TnP pair exists
  if (tnpelectrons.size()>0)
  {
    // For now, get the best pair
    auto best = std::min_element(tnpelectrons.begin(),tnpelectrons.end(),minimizeByZmass); // keep the lowest! --> returns pointer to lowest element

    // Tag is el1
    pat::Electron el1 = electrons[std::get<0>(*best)];
  
    // set the individual electron variables
    el1pid = el1.pdgId(); 
    el1E   = el1.energy(); 
    el1p   = el1.p(); 
    el1pt  = el1.pt(); 
    el1eta = el1.eta(); 
    el1phi = el1.phi();   

    // set VID
    if      (el1.electronID("tight"))  el1vid = 4; 
    else if (el1.electronID("medium")) el1vid = 3;
    else if (el1.electronID("loose"))  el1vid = 2;
    else if (el1.electronID("veto"))   el1vid = 1;
    else                               el1vid = 0;

    // super cluster from electron 1
    const reco::SuperClusterRef& el1sc = el1.superCluster().isNonnull() ? el1.superCluster() : el1.parentSuperCluster();
    
    // save some supercluster info
    el1scX = el1sc->position().x();
    el1scY = el1sc->position().y();
    el1scZ = el1sc->position().z();
    el1scE = el1sc->energy();

    // use seed to get geometry and recHits
    const DetId el1seedDetId = el1sc->seed()->seed(); //seed detid
    const bool el1isEB = (el1seedDetId.subdetId() == EcalBarrel); //which subdet
    const GlobalPoint& el1recHitPos = el1isEB ? barrelGeometry->getGeometry(el1seedDetId)->getPosition() : endcapGeometry->getGeometry(el1seedDetId)->getPosition();
    EcalRecHitCollection::const_iterator el1recHit = el1isEB ? recHitsEB->find(el1seedDetId) : recHitsEE->find(el1seedDetId);
    
    // push back the values
    el1seedX     = el1recHitPos.x();
    el1seedY     = el1recHitPos.y();
    el1seedZ     = el1recHitPos.z();
    el1seedE     = el1recHit->energy();
    el1seedtime  = el1recHit->time();	 
    el1seedid    = int(el1seedDetId.rawId());
    el1seedOOT   = el1recHit->checkFlag(EcalRecHit::kOutOfTime);
    el1seedgain1 = el1recHit->checkFlag(EcalRecHit::kHasSwitchToGain1);
    el1seedgain6 = el1recHit->checkFlag(EcalRecHit::kHasSwitchToGain6);

    // Probe is el2
    pat::Electron el2 = electrons[std::get<1>(*best)];
  
    // set the individual electron variables
    el2pid = el2.pdgId(); 
    el2E   = el2.energy(); 
    el2p   = el2.p(); 
    el2pt  = el2.pt(); 
    el2eta = el2.eta(); 
    el2phi = el2.phi();   

    // set VID
    if      (el2.electronID("tight"))  el2vid = 4; 
    else if (el2.electronID("medium")) el2vid = 3;
    else if (el2.electronID("loose"))  el2vid = 2;
    else if (el2.electronID("veto"))   el2vid = 1;
    else                               el2vid = 0;

    // super cluster from electron 2
    const reco::SuperClusterRef& el2sc = el2.superCluster().isNonnull() ? el2.superCluster() : el2.parentSuperCluster();
    
    // save some supercluster info
    el2scX = el2sc->position().x();
    el2scY = el2sc->position().y();
    el2scZ = el2sc->position().z();
    el2scE = el2sc->energy();

    // use seed to get geometry and recHits
    const DetId el2seedDetId = el2sc->seed()->seed(); //seed detid
    const bool el2isEB = (el2seedDetId.subdetId() == EcalBarrel); //which subdet
    const GlobalPoint& el2recHitPos = el2isEB ? barrelGeometry->getGeometry(el2seedDetId)->getPosition() : endcapGeometry->getGeometry(el2seedDetId)->getPosition();
    EcalRecHitCollection::const_iterator el2recHit = el2isEB ? recHitsEB->find(el2seedDetId) : recHitsEE->find(el2seedDetId);

    // push back the values
    el2seedX     = el2recHitPos.x();
    el2seedY     = el2recHitPos.y();
    el2seedZ     = el2recHitPos.z();
    el2seedE     = el2recHit->energy();
    el2seedtime  = el2recHit->time();	 
    el2seedid    = int(el2seedDetId.rawId());
    el2seedOOT   = el2recHit->checkFlag(EcalRecHit::kOutOfTime);
    el2seedgain1 = el2recHit->checkFlag(EcalRecHit::kHasSwitchToGain1);
    el2seedgain6 = el2recHit->checkFlag(EcalRecHit::kHasSwitchToGain6);

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
  } // end section over tnp pair

  // fill the tree!
  tree->Fill();    
}    

void ZeeTnPTree::PrepElectrons(const edm::Handle<std::vector<pat::Electron> > & electronsH, 
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
    for (size_t iel = 0; iel < idpairs.size(); iel++)
    {
      idpairs[iel].resize(4);
      idpairs[iel][0] = {"veto"  ,false};
      idpairs[iel][1] = {"loose" ,false};
      idpairs[iel][2] = {"medium",false};
      idpairs[iel][3] = {"tight" ,false};
    }

    int ielH = 0; // dumb counter because iterators only work with VID
    for (std::vector<pat::Electron>::const_iterator phiter = electronsH->begin(); phiter != electronsH->end(); ++phiter) // loop over electron vector
    {
      // Get the VID of the electron
      const edm::Ptr<pat::Electron> electronPtr(electronsH, phiter - electronsH->begin());

      // store VID in temp struct
      // veto < loose < medium < tight
      if (electronVetoIdMap  [electronPtr]) idpairs[ielH][0].second = true;
      if (electronLooseIdMap [electronPtr]) idpairs[ielH][1].second = true;
      if (electronMediumIdMap[electronPtr]) idpairs[ielH][2].second = true;
      if (electronTightIdMap [electronPtr]) idpairs[ielH][3].second = true;
      
      ielH++;
    }
    
    // set the ID-value for each electron in other collection
    for (size_t iel = 0; iel < electrons.size(); iel++)
    {
      electrons[iel].setElectronIDs(idpairs[iel]);
    }
    
    // now finally sort vector by pT
    std::sort(electrons.begin(),electrons.end(),sortByElectronPt);
  }
}  

void ZeeTnPTree::beginJob() 
{
  edm::Service<TFileService> fs;
  tree = fs->make<TTree>("tree"       , "tree");

  // Run, Lumi, Event info
  tree->Branch("run"                  , &run                  , "run/i");
  tree->Branch("lumi"                 , &lumi                 , "lumi/i");
  tree->Branch("event"                , &event                , "event/l");
  
  // Triggers
  tree->Branch("hltdoubleel23_12"     , &hltdoubleel23_12     , "hltdoubleel23_12/O");
  tree->Branch("hltdoubleel33_33"     , &hltdoubleel33_33     , "hltdoubleel33_33/O");
  tree->Branch("hltdoubleel37_27"     , &hltdoubleel37_27     , "hltdoubleel37_27/O");

  // Vertex info
  tree->Branch("nvtx"                 , &nvtx                 , "nvtx/I");
  tree->Branch("vtxX"                 , &vtxX                 , "vtxX/F");
  tree->Branch("vtxY"                 , &vtxY                 , "vtxY/F");
  tree->Branch("vtxZ"                 , &vtxZ                 , "vtxZ/F");

  // Tag Lepton info
  tree->Branch("el1pid"               , &el1pid               , "el1pid/I");
  tree->Branch("el1vid"               , &el1vid               , "el1vid/I");
  tree->Branch("el1E"                 , &el1E                 , "el1E/F");
  tree->Branch("el1p"                 , &el1p                 , "el1p/F");
  tree->Branch("el1pt"                , &el1pt                , "el1pt/F");
  tree->Branch("el1eta"               , &el1eta               , "el1eta/F");
  tree->Branch("el1phi"               , &el1phi               , "el1phi/F");

  // Tag SuperCluster info
  tree->Branch("el1scX"               , &el1scX               , "el1scX/F");
  tree->Branch("el1scY"               , &el1scY               , "el1scY/F");
  tree->Branch("el1scZ"               , &el1scZ               , "el1scZ/F");
  tree->Branch("el1scE"               , &el1scE               , "el1scE/F");

  // Tag seed recHit info
  tree->Branch("el1seedX"             , &el1seedX               , "el1seedX/F");
  tree->Branch("el1seedY"             , &el1seedY               , "el1seedY/F");
  tree->Branch("el1seedZ"             , &el1seedZ               , "el1seedZ/F");
  tree->Branch("el1seedE"             , &el1seedE               , "el1seedE/F");
  tree->Branch("el1seedtime"          , &el1seedtime            , "el1seedtime/F");
  tree->Branch("el1seedid"            , &el1seedid              , "el1seedid/I");
  tree->Branch("el1seedOOT"           , &el1seedOOT             , "el1seedOOT/I");
  tree->Branch("el1seedgain1"         , &el1seedgain1           , "el1seedgain1/I");
  tree->Branch("el1seedgain6"         , &el1seedgain6           , "el1seedgain6/I");

  // Probe Lepton info
  tree->Branch("el2pid"               , &el2pid               , "el2pid/I");
  tree->Branch("el2vid"               , &el2vid               , "el2vid/I");
  tree->Branch("el2E"                 , &el2E                 , "el2E/F");
  tree->Branch("el2p"                 , &el2p                 , "el2p/F");
  tree->Branch("el2pt"                , &el2pt                , "el2pt/F");
  tree->Branch("el2eta"               , &el2eta               , "el2eta/F");
  tree->Branch("el2phi"               , &el2phi               , "el2phi/F");

  // Probe SuperCluster info
  tree->Branch("el2scX"               , &el2scX               , "el2scX/F");
  tree->Branch("el2scY"               , &el2scY               , "el2scY/F");
  tree->Branch("el2scZ"               , &el2scZ               , "el2scZ/F");
  tree->Branch("el2scE"               , &el2scE               , "el2scE/F");

  // Probe seed recHit info
  tree->Branch("el2seedX"             , &el2seedX               , "el2seedX/F");
  tree->Branch("el2seedY"             , &el2seedY               , "el2seedY/F");
  tree->Branch("el2seedZ"             , &el2seedZ               , "el2seedZ/F");
  tree->Branch("el2seedE"             , &el2seedE               , "el2seedE/F");
  tree->Branch("el2seedtime"          , &el2seedtime            , "el2seedtime/F");
  tree->Branch("el2seedid"            , &el2seedid              , "el2seedid/I");
  tree->Branch("el2seedOOT"           , &el2seedOOT             , "el2seedOOT/I");
  tree->Branch("el2seedgain1"         , &el2seedgain1           , "el2seedgain1/I");
  tree->Branch("el2seedgain6"         , &el2seedgain6           , "el2seedgain6/I");

  // Dilepton info
  tree->Branch("zmass"                , &zmass                , "zmass/F");
  tree->Branch("zpt"                  , &zpt                  , "zpt/F");
  tree->Branch("zeta"                 , &zeta                 , "zeta/F");
  tree->Branch("zphi"                 , &zphi                 , "zphi/F");
  tree->Branch("zE"                   , &zE                   , "zE/F");
  tree->Branch("zp"                   , &zp                   , "zp/F");
}

void ZeeTnPTree::endJob() {}

void ZeeTnPTree::beginRun(edm::Run const& iRun, edm::EventSetup const& iSetup) 
{
  // triggers for the Analysis -- all unprescaled till RunH
  triggerPathsVector.push_back("HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL_DZ_L1JetTauSeeded_v");
  triggerPathsVector.push_back("HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL_DZ_v");
  triggerPathsVector.push_back("HLT_DoubleEle33_CaloIdL_GsfTrkIdVL_MW_v");
  triggerPathsVector.push_back("HLT_DoubleEle33_CaloIdL_MW_v");
  triggerPathsVector.push_back("HLT_DoubleEle37_Ele27_CaloIdL_GsfTrkIdVL_v");

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

void ZeeTnPTree::endRun(edm::Run const&, edm::EventSetup const&) {}

void ZeeTnPTree::fillDescriptions(edm::ConfigurationDescriptions& descriptions) 
{
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

DEFINE_FWK_MODULE(ZeeTnPTree);
