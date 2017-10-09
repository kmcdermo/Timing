#include "Timing/TimingAnalyzer/plugins/HLTDump.hh"

HLTDump::HLTDump(const edm::ParameterSet& iConfig): 
  // cuts
  phpTmin(iConfig.existsAs<double>("phpTmin") ? iConfig.getParameter<double>("phpTmin") : 20.f),
  jetpTmin(iConfig.existsAs<double>("jetpTmin") ? iConfig.getParameter<double>("jetpTmin") : 15.f),
  dRmin(iConfig.existsAs<double>("dRmin") ? iConfig.getParameter<double>("dRmin") : 0.4),
  pTres(iConfig.existsAs<double>("pTres") ? iConfig.getParameter<double>("pTres") : 0.5),
  saveTrigObjs(iConfig.existsAs<bool>("saveTrigObjs") ? iConfig.getParameter<bool>("saveTrigObjs") : false),
  trackpTmin(iConfig.existsAs<double>("trackpTmin") ? iConfig.getParameter<double>("trackpTmin") : 5.f),
  trackdRmin(iConfig.existsAs<double>("trackdRmin") ? iConfig.getParameter<double>("trackdRmin") : 0.2),
  
  // triggers
  inputPaths       (iConfig.existsAs<std::string>("inputPaths")   ? iConfig.getParameter<std::string>("inputPaths") : ""),
  inputFilters     (iConfig.existsAs<std::string>("inputFilters") ? iConfig.getParameter<std::string>("inputFilters") : ""),
  triggerResultsTag(iConfig.getParameter<edm::InputTag>("triggerResults")),
  triggerObjectsTag(iConfig.getParameter<edm::InputTag>("triggerObjects")),

  // vertices
  verticesTag(iConfig.getParameter<edm::InputTag>("vertices")),

  // rhos
  rhosTag(iConfig.getParameter<edm::InputTag>("rhos")),

  // mets
  metsTag(iConfig.getParameter<edm::InputTag>("mets")),  

  // jets
  jetsTag(iConfig.getParameter<edm::InputTag>("jets")),  

  // photons
  photonsTag(iConfig.getParameter<edm::InputTag>("photons")),  

  // ootPhotons
  ootPhotonsTag(iConfig.getParameter<edm::InputTag>("ootPhotons")),  
  
  // recHits
  recHitsEBTag(iConfig.getParameter<edm::InputTag>("recHitsEB")),  
  recHitsEETag(iConfig.getParameter<edm::InputTag>("recHitsEE")),

  // tracks
  tracksTag(iConfig.getParameter<edm::InputTag>("tracks"))  
{
  usesResource();
  usesResource("TFileService");

  // triggers
  triggerResultsToken = consumes<edm::TriggerResults> (triggerResultsTag);
  triggerObjectsToken = consumes<std::vector<pat::TriggerObjectStandAlone> > (triggerObjectsTag);

  // read in from a stream the trigger paths for saving
  oot::ReadInTriggerNames(inputPaths,pathNames,triggerBitMap);
  triggerBits.resize(pathNames.size());

  // read in from a stream the hlt objects/labels to match to
  oot::ReadInFilterNames(inputFilters,filterNames,triggerObjectsByFilterMap);

  //vertex
  verticesToken = consumes<std::vector<reco::Vertex> > (verticesTag);

  // rhos
  rhosToken = consumes<double> (rhosTag);

  // mets
  metsToken = consumes<std::vector<pat::MET> > (metsTag);

  // jets
  jetsToken = consumes<std::vector<pat::Jet> > (jetsTag);

  // photons
  photonsToken = consumes<std::vector<pat::Photon> > (photonsTag);
  if (not ootPhotonsTag.label().empty())
  {
    ootPhotonsToken = consumes<std::vector<pat::Photon> > (ootPhotonsTag);
  }

  // rechits
  recHitsEBToken = consumes<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > (recHitsEBTag);
  recHitsEEToken = consumes<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > (recHitsEETag);

  // tracks 
  tracksToken = consumes<std::vector<reco::Track> > (tracksTag);
}

HLTDump::~HLTDump() {}

void HLTDump::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) 
{
  // VERTICES
  edm::Handle<std::vector<reco::Vertex> > verticesH;
  iEvent.getByToken(verticesToken, verticesH);
  
  // RHOS
  edm::Handle<double> rhosH;
  iEvent.getByToken(rhosToken, rhosH);
  const float rho = rhosH.isValid() ? *(rhosH.product()) : 0.f;

  // TRIGGERS
  edm::Handle<edm::TriggerResults> triggerResultsH;
  iEvent.getByToken(triggerResultsToken, triggerResultsH);

  edm::Handle<std::vector<pat::TriggerObjectStandAlone> > triggerObjectsH;
  iEvent.getByToken(triggerObjectsToken, triggerObjectsH);

  // MET
  edm::Handle<std::vector<pat::MET> > metsH;
  iEvent.getByToken(metsToken, metsH);

  // JETS
  edm::Handle<std::vector<pat::Jet> > jetsH;
  iEvent.getByToken(jetsToken, jetsH);
  std::vector<pat::Jet> jets; jets.reserve(jetsH->size());

  // PHOTONS
  edm::Handle<std::vector<pat::Photon> > photonsH;
  iEvent.getByToken(photonsToken, photonsH);
  int phosize = photonsH->size();

  edm::Handle<std::vector<pat::Photon> > ootPhotonsH;
  if (not ootPhotonsToken.isUninitialized())
  {
    iEvent.getByToken(ootPhotonsToken, ootPhotonsH);
    phosize += ootPhotonsH->size();
  }
  std::vector<oot::Photon> photons; photons.reserve(phosize);

  // RecHits
  edm::Handle<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > recHitsEBH;
  iEvent.getByToken(recHitsEBToken, recHitsEBH);
  edm::Handle<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > recHitsEEH;
  iEvent.getByToken(recHitsEEToken, recHitsEEH);

  // Tracks
  edm::Handle<std::vector<reco::Track> > tracksH;
  iEvent.getByToken(tracksToken, tracksH);

  // geometry (from ECAL ELF)
  edm::ESHandle<CaloGeometry> calogeoH;
  iSetup.get<CaloGeometryRecord>().get(calogeoH);
  const CaloSubdetectorGeometry * barrelGeometry = calogeoH->getSubdetectorGeometry(DetId::Ecal, EcalBarrel);
  const CaloSubdetectorGeometry * endcapGeometry = calogeoH->getSubdetectorGeometry(DetId::Ecal, EcalEndcap);

  // do some prepping of objects
  oot::PrepTriggerBits(triggerResultsH,iEvent,triggerBitMap);
  oot::PrepTriggerObjects(triggerResultsH,triggerObjectsH,iEvent,triggerObjectsByFilterMap);
  oot::PrepJets(jetsH,jets,jetpTmin);
  oot::PrepPhotons(photonsH,ootPhotonsH,photons,rho,phpTmin);

  ///////////////////////////
  //                       //
  // Event, lumi, run info //
  //                       //
  ///////////////////////////
  run   = iEvent.id().run();
  lumi  = iEvent.luminosityBlock();
  event = iEvent.id().event();

  /////////////////////////
  //                     //   
  // Primary Vertex info //
  //                     //
  /////////////////////////
  HLTDump::InitializePVBranches();
  if (verticesH.isValid()) 
  {
    nvtx = verticesH->size();
    const reco::Vertex & primevtx = (*verticesH)[0];
    vtxX = primevtx.position().x();
    vtxY = primevtx.position().y();
    vtxZ = primevtx.position().z();
  }

  //////////////////
  //              //
  // Trigger Info //
  //              //
  //////////////////
  HLTDump::InitializeTriggerBranches();
  if (triggerResultsH.isValid())
  {
    for (std::size_t ipath = 0; ipath < pathNames.size(); ipath++)
    {
      triggerBits[ipath] = triggerBitMap[pathNames[ipath]];
    }
  } // end check over valid TriggerResults

  /////////////////
  //             //
  // HLT Objects //
  //             //
  /////////////////
  if (saveTrigObjs) 
  {
    HLTDump::ClearTriggerObjectBranches();
    for (std::size_t ifilter = 0; ifilter < filterNames.size(); ifilter++)
    {
      for (std::size_t iobject = 0; iobject < triggerObjectsByFilterMap[filterNames[ifilter]].size(); iobject++)
      {
	const auto & triggerObject = triggerObjectsByFilterMap[filterNames[ifilter]][iobject];
	trigobjE  [ifilter][iobject] = triggerObject.energy();
	trigobjeta[ifilter][iobject] = triggerObject.eta();
	trigobjphi[ifilter][iobject] = triggerObject.phi();
	trigobjpt [ifilter][iobject] = triggerObject.pt();
      } // save trig objects
    } // end loop over filter objects
  } // end loop over filters

  //////////////////
  //              //
  // Type1 PF Met //
  //              //
  //////////////////
  HLTDump::InitializeMETBranches();
  if (metsH.isValid())
  {
    const pat::MET & t1pfMET = (*metsH)[0];

    // Type1 PF MET (corrected)
    t1pfMETpt    = t1pfMET.pt();
    t1pfMETphi   = t1pfMET.phi();
    t1pfMETsumEt = t1pfMET.sumEt();
  }

  /////////////////////////
  //                     //
  // Jets (AK4 standard) //
  //                     //
  /////////////////////////
  
  HLTDump::ClearJetBranches();
  if (jetsH.isValid()) // check to make sure reco jets exist
  {
    njets = jets.size();
    if (njets > 0) HLTDump::InitializeJetBranches();

    int ijet = 0;
    for (std::vector<pat::Jet>::const_iterator jetiter = jets.begin(); jetiter != jets.end(); ++jetiter)
    {
      jetE  [ijet] = jetiter->energy();
      jetpt [ijet] = jetiter->pt();
      jetphi[ijet] = jetiter->phi();
      jeteta[ijet] = jetiter->eta();
      
      const float nhf = jetiter->neutralHadronEnergyFraction();
      const float chf = jetiter->chargedHadronEnergyFraction();

      const float nef = jetiter->neutralEmEnergyFraction();
      const float cef = jetiter->chargedEmEnergyFraction();

      const int cm = jetiter->chargedMultiplicity();
      const int nm = jetiter->neutralMultiplicity();

      const float eta = std::abs(jeteta[ijet]);

      if      (eta <= 2.4) jetidL[ijet] = nhf<0.99 && nef<0.99 && (cm+nm)>1 && chf > 0 && cm > 0 && cef < 0.99;
      else if (eta <= 2.7) jetidL[ijet] = nhf<0.99 && nef<0.99 && (cm+nm)>1;
      else if (eta <= 3.0) jetidL[ijet] = nef>0.01 && nhf<0.98 && nm>2;
      else                 jetidL[ijet] = nef<0.90 && nm>10;
      
      ijet++;
    } // end loop over reco jets
  } // end check over reco jets
  
  //////////////////
  //              //
  // Reco Photons //
  //              //
  //////////////////
  HLTDump::ClearRecoPhotonBranches();
  if (photonsH.isValid() || ootPhotonsH.isValid()) // standard handle check
  {
    nphotons = photons.size();
    if (nphotons > 0) HLTDump::InitializeRecoPhotonBranches();

    int iph = 0;
    for (std::vector<oot::Photon>::const_iterator phiter = photons.begin(); phiter != photons.end(); ++phiter) // loop over photon vector
    {
      const pat::Photon& photon = phiter->photon();

      // from ootPhoton collection
      phisOOT[iph] = phiter->isOOT();
      
      // standard photon branches
      phE  [iph] = photon.energy();
      phpt [iph] = photon.pt();
      phphi[iph] = photon.phi();
      pheta[iph] = photon.eta();

      // check for HLT filter matches!
      strBitMap isHLTMatched;
      for (const auto & filter : filterNames) isHLTMatched[filter] = false;
      oot::HLTToObjectMatching(triggerObjectsByFilterMap,isHLTMatched,*phiter,pTres,dRmin);
      for (std::size_t ifilter = 0; ifilter < filterNames.size(); ifilter++)
      {
	phIsHLTMatched[iph][ifilter] = isHLTMatched[filterNames[ifilter]];
      }

      // check for simple track veto
      phIsTrack[iph] = oot::TrackToObjectMatching(tracksH,*phiter,trackpTmin,trackdRmin);

      // super cluster from photon
      const reco::SuperClusterRef& phsc = photon.superCluster().isNonnull() ? photon.superCluster() : photon.parentSuperCluster();
      phscE  [iph] = phsc->energy();
      phsceta[iph] = phsc->eta();
      phscphi[iph] = phsc->phi();

      // Shower Shape Objects
      const reco::Photon::ShowerShape& phshape = photon.full5x5_showerShapeVariables(); // photon.showerShapeVariables();

      // ID-like variables
      phHoE[iph] = photon.hadTowOverEm(); // ID + trigger == single tower
      phr9 [iph] = photon.r9();

      // pseudo-track veto
      phPixSeed[iph] = photon.passElectronVeto();
      phEleVeto[iph] = photon.hasPixelSeed();

      // cluster shape variables
      phsieie[iph] = phshape.sigmaIetaIeta;
      phsipip[iph] = phshape.sigmaIphiIphi;
      phsieip[iph] = phshape.sigmaIetaIphi;

      // PF Isolations
      const float sceta = std::abs(phsceta[iph]);
      phChgIso[iph] = std::max(photon.chargedHadronIso() - (rho * oot::GetChargedHadronEA(sceta)),0.f);
      phNeuIso[iph] = std::max(photon.neutralHadronIso() - (rho * oot::GetNeutralHadronEA(sceta)),0.f);
      phIso   [iph] = std::max(photon.photonIso()        - (rho * oot::GetGammaEA        (sceta)),0.f);

      // PF Cluster Isolations
      phPFClEcalIso[iph] = photon.ecalPFClusterIso();
      phPFClHcalIso[iph] = photon.hcalPFClusterIso();

      // Track Isolation (dR of outer cone < 0.3 as matching in trigger)
      phHollowTkIso[iph] = photon.trkSumPtHollowConeDR03();
     
      // use seed to get geometry and recHits
      const DetId seedDetId = phsc->seed()->seed(); //seed detid
      const bool isEB = (seedDetId.subdetId() == EcalBarrel); //which subdet
      const EcalRecHitCollection * recHits = (isEB ? recHitsEBH : recHitsEEH).product();
      const EcalRecHitCollection::const_iterator seedRecHit = recHits->find(seedDetId);
      
      if (seedRecHit != recHits->end())
      {
	// save position, energy, and time of each rechit to a vector
	const auto seedPos = isEB ? barrelGeometry->getGeometry(seedDetId)->getPosition() : endcapGeometry->getGeometry(seedDetId)->getPosition();
	phseedeta [iph] = seedPos.eta();
	phseedphi [iph] = seedPos.phi();

	phseedE   [iph] = seedRecHit->energy();
	phseedtime[iph] = seedRecHit->time();
	phseedID  [iph] = int(seedDetId.rawId());
	phseedOOT [iph] = int(seedRecHit->checkFlag(EcalRecHit::kOutOfTime));

      	if (recHits->size() > 0)
	{
	  // radius of semi-major,minor axis is the inverse square root of the eigenvalues of the covariance matrix
	  const Cluster2ndMoments ph2ndMoments = noZS::EcalClusterTools::cluster2ndMoments( *phsc, *recHits);
	  phsmaj [iph] = ph2ndMoments.sMaj;
	  phsmin [iph] = ph2ndMoments.sMin;
	  phalpha[iph] = ph2ndMoments.alpha;

	  // map of rec hit ids
	  uiiumap phrhIDmap;
      
	  // all rechits in superclusters
	  const DetIdPairVec hitsAndFractions = phsc->hitsAndFractions();

	  for (DetIdPairVec::const_iterator hafitr = hitsAndFractions.begin(); hafitr != hitsAndFractions.end(); ++hafitr) // loop over all rec hits in SC
          {
	    const DetId recHitId = hafitr->first; // get detid of crystal
	    EcalRecHitCollection::const_iterator recHit = recHits->find(recHitId); // get the underlying rechit
	    if (recHit != recHits->end()) // standard check
	    { 
	      phrhIDmap[recHitId.rawId()]++;
	    } // end standard check recHit
	  } // end loop over hits and fractions

	  phnrh[iph] = phrhIDmap.size();
	} // end check over recHits size
      } // end check over seed recHit exists

      iph++; // increment photon counter
    } // end loop over photon vector
  } // end check over photon handle valid

  ///////////////
  //           //
  // Fill Tree //
  //           //
  ///////////////
  tree->Fill();      
}    

void HLTDump::InitializeTriggerBranches()
{
  for (std::size_t ipath = 0; ipath < pathNames.size(); ipath++)
  { 
    triggerBits[ipath] = false;
  }
}

void HLTDump::ClearTriggerObjectBranches()
{
  trigobjE.clear();
  trigobjeta.clear();
  trigobjphi.clear();
  trigobjpt.clear();

  trigobjE.resize(triggerObjectsByFilterMap.size());
  trigobjeta.resize(triggerObjectsByFilterMap.size());
  trigobjphi.resize(triggerObjectsByFilterMap.size());
  trigobjpt.resize(triggerObjectsByFilterMap.size());

  for (std::size_t ifilter = 0; ifilter < filterNames.size(); ifilter++)
  {
    trigobjE  [ifilter].resize(triggerObjectsByFilterMap[filterNames[ifilter]].size());
    trigobjeta[ifilter].resize(triggerObjectsByFilterMap[filterNames[ifilter]].size());
    trigobjphi[ifilter].resize(triggerObjectsByFilterMap[filterNames[ifilter]].size());
    trigobjpt [ifilter].resize(triggerObjectsByFilterMap[filterNames[ifilter]].size());
    for (std::size_t iobject = 0; iobject < triggerObjectsByFilterMap[filterNames[ifilter]].size(); iobject++)
    {
      trigobjE  [ifilter][iobject] = -9999.f;
      trigobjeta[ifilter][iobject] = -9999.f;
      trigobjphi[ifilter][iobject] = -9999.f;
      trigobjpt [ifilter][iobject] = -9999.f;
    }
  }
}

void HLTDump::InitializePVBranches()
{
  nvtx = -9999; 
  vtxX = -9999.f; vtxY = -9999.f; vtxZ = -9999.f;
}

void HLTDump::InitializeMETBranches()
{
  t1pfMETpt = -9999.f; t1pfMETphi = -9999.f; t1pfMETsumEt = -9999.f;
}

void HLTDump::ClearJetBranches()
{
  njets = -9999;

  jetE.clear();
  jetpt.clear();
  jetphi.clear();
  jeteta.clear();
  jetidL.clear();
}

void HLTDump::InitializeJetBranches()
{
  jetE.resize(njets);
  jetpt.resize(njets);
  jetphi.resize(njets);
  jeteta.resize(njets);
  jetidL.resize(njets);

  for (int ijet = 0; ijet < njets; ijet++)
  {
    jetE  [ijet] = -9999.f;
    jetpt [ijet] = -9999.f;
    jetphi[ijet] = -9999.f;
    jeteta[ijet] = -9999.f;
    jetidL[ijet] = false;
  }
}

void HLTDump::ClearRecoPhotonBranches()
{
  nphotons = -9999;

  phisOOT.clear();

  phE.clear();
  phpt.clear();
  phphi.clear(); 
  pheta.clear(); 

  phscE.clear(); 
  phsceta.clear(); 
  phscphi.clear(); 

  phHoE.clear();
  phr9.clear();

  phPixSeed.clear();
  phEleVeto.clear();

  phChgIso.clear();
  phNeuIso.clear();
  phIso.clear();
  phPFClEcalIso.clear();
  phPFClHcalIso.clear();
  phHollowTkIso.clear();

  phsieie.clear();
  phsipip.clear();
  phsieip.clear();
  phsmaj.clear();
  phsmin.clear();
  phalpha.clear();

  phIsHLTMatched.clear();

  phIsTrack.clear();

  phnrh.clear();

  phseedeta.clear(); 
  phseedphi.clear(); 
  phseedE.clear(); 
  phseedtime.clear();
  phseedID.clear();
  phseedOOT.clear();
}

void HLTDump::InitializeRecoPhotonBranches()
{
  phisOOT.resize(nphotons);

  phE.resize(nphotons);
  phpt.resize(nphotons);
  phphi.resize(nphotons);
  pheta.resize(nphotons);

  phscE.resize(nphotons);
  phsceta.resize(nphotons);
  phscphi.resize(nphotons);

  phHoE.resize(nphotons);
  phr9.resize(nphotons);

  phPixSeed.resize(nphotons);
  phEleVeto.resize(nphotons);

  phChgIso.resize(nphotons);
  phNeuIso.resize(nphotons);
  phIso.resize(nphotons);
  phPFClEcalIso.resize(nphotons);
  phPFClHcalIso.resize(nphotons);
  phHollowTkIso.resize(nphotons);

  phsieie.resize(nphotons);
  phsipip.resize(nphotons);
  phsieip.resize(nphotons);
  phsmaj.resize(nphotons);
  phsmin.resize(nphotons);
  phalpha.resize(nphotons);

  phIsHLTMatched.resize(nphotons);

  phIsTrack.resize(nphotons);

  phnrh.resize(nphotons);

  phseedeta.resize(nphotons);
  phseedphi.resize(nphotons);
  phseedE.resize(nphotons);
  phseedtime.resize(nphotons);
  phseedID.resize(nphotons);
  phseedOOT.resize(nphotons);

  for (int iph = 0; iph < nphotons; iph++)
  {
    phisOOT[iph] = -1;

    phE  [iph] = -9999.f; 
    phpt [iph] = -9999.f; 
    phphi[iph] = -9999.f; 
    pheta[iph] = -9999.f; 

    phscE  [iph] = -9999.f; 
    phsceta[iph] = -9999.f; 
    phscphi[iph] = -9999.f; 

    phHoE[iph] = -9999.f;
    phr9 [iph] = -9999.f;

    phPixSeed[iph] = false;
    phEleVeto[iph] = false;

    phChgIso     [iph] = -9999.f;
    phNeuIso     [iph] = -9999.f;
    phIso        [iph] = -9999.f;
    phPFClEcalIso[iph] = -9999.f;
    phPFClHcalIso[iph] = -9999.f;
    phHollowTkIso[iph] = -9999.f;

    phsieie[iph] = -9999.f;
    phsipip[iph] = -9999.f;
    phsieip[iph] = -9999.f;
    phsmaj [iph] = -9999.f;
    phsmin [iph] = -9999.f;
    phalpha[iph] = -9999.f;

    phnrh    [iph] = -9999;

    phIsHLTMatched[iph].resize(filterNames.size());
    for (std::size_t ifilter = 0; ifilter < filterNames.size(); ifilter++)
    {
      phIsHLTMatched[iph][ifilter] = -1; // false
    }

    phIsTrack [iph] = -1;

    phseedeta [iph] = -9999.f;
    phseedphi [iph] = -9999.f;
    phseedE   [iph] = -9999.f;
    phseedtime[iph] = -9999.f;
    phseedID  [iph] = -9999.f;
    phseedOOT [iph] = -9999.f;
  }
}

void HLTDump::beginJob() 
{
  edm::Service<TFileService> fs;
  tree = fs->make<TTree>("tree","tree");

  // Run, Lumi, Event info
  tree->Branch("event"                , &event                , "event/l");
  tree->Branch("run"                  , &run                  , "run/i");
  tree->Branch("lumi"                 , &lumi                 , "lumi/i");
   
  // Trigger Info
  tree->Branch("triggerBits"          , &triggerBits);

  // Vertex info
  tree->Branch("nvtx"                 , &nvtx                 , "nvtx/I");
  tree->Branch("vtxX"                 , &vtxX                 , "vtxX/F");
  tree->Branch("vtxY"                 , &vtxY                 , "vtxY/F");
  tree->Branch("vtxZ"                 , &vtxZ                 , "vtxZ/F");

  // MET info
  tree->Branch("t1pfMETpt"            , &t1pfMETpt            , "t1pfMETpt/F");
  tree->Branch("t1pfMETphi"           , &t1pfMETphi           , "t1pfMETphi/F");
  tree->Branch("t1pfMETsumEt"         , &t1pfMETsumEt         , "t1pfMETsumEt/F");

  // Jet Info
  tree->Branch("njets"                , &njets                , "njets/I");

  tree->Branch("jetE"                 , &jetE);
  tree->Branch("jetpt"                , &jetpt);
  tree->Branch("jetphi"               , &jetphi);
  tree->Branch("jeteta"               , &jeteta);
  tree->Branch("jetidL"               , &jetidL);
   
  // Trigger Object Info
  if (saveTrigObjs)
  {
    tree->Branch("trigobjE"             , &trigobjE);
    tree->Branch("trigobjeta"           , &trigobjeta);
    tree->Branch("trigobjphi"           , &trigobjphi);
    tree->Branch("trigobjpt"            , &trigobjpt);
  }

  // Photon Info
  tree->Branch("nphotons"             , &nphotons             , "nphotons/I");

  tree->Branch("phisOOT"              , &phisOOT);

  tree->Branch("phE"                  , &phE);
  tree->Branch("phpt"                 , &phpt);
  tree->Branch("phphi"                , &phphi);
  tree->Branch("pheta"                , &pheta);

  tree->Branch("phscE"                , &phscE);
  tree->Branch("phsceta"              , &phsceta);
  tree->Branch("phscphi"              , &phscphi);

  tree->Branch("phHoE"                , &phHoE);
  tree->Branch("phr9"                 , &phr9);

  tree->Branch("phPixSeed"            , &phPixSeed);
  tree->Branch("phEleVeto"            , &phEleVeto);

  tree->Branch("phChgIso"             , &phChgIso);
  tree->Branch("phNeuIso"             , &phNeuIso);
  tree->Branch("phIso"                , &phIso);
  tree->Branch("phPFClEcalIso"        , &phPFClEcalIso);
  tree->Branch("phPFClHcalIso"        , &phPFClHcalIso);
  tree->Branch("phHollowTkIso"        , &phHollowTkIso);

  tree->Branch("phsieie"              , &phsieie);
  tree->Branch("phsipip"              , &phsipip);
  tree->Branch("phsieip"              , &phsieip);
  tree->Branch("phsmaj"               , &phsmaj);
  tree->Branch("phsmin"               , &phsmin);
  tree->Branch("phalpha"              , &phalpha);

  tree->Branch("phIsHLTMatched"       , &phIsHLTMatched);

  tree->Branch("phIsTrack"            , &phIsTrack);

  tree->Branch("phnrh"                , &phnrh);

  tree->Branch("phseedeta"            , &phseedeta);
  tree->Branch("phseedphi"            , &phseedphi);
  tree->Branch("phseedE"              , &phseedE);
  tree->Branch("phseedtime"           , &phseedtime);
  tree->Branch("phseedID"             , &phseedID);
  tree->Branch("phseedOOT"            , &phseedOOT);
}

void HLTDump::endJob() {}

void HLTDump::beginRun(edm::Run const& iRun, edm::EventSetup const& iSetup) {}

void HLTDump::endRun(edm::Run const&, edm::EventSetup const&) {}

void HLTDump::fillDescriptions(edm::ConfigurationDescriptions& descriptions) 
{
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

DEFINE_FWK_MODULE(HLTDump);
