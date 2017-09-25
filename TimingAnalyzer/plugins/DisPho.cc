#include "Timing/TimingAnalyzer/plugins/DisPho.hh"

DisPho::DisPho(const edm::ParameterSet& iConfig): 
  // standard cuts
  phpTmin(iConfig.existsAs<double>("phpTmin") ? iConfig.getParameter<double>("phpTmin") : 20.f),
  jetpTmin(iConfig.existsAs<double>("jetpTmin") ? iConfig.getParameter<double>("jetpTmin") : 15.f),
  jetIDmin(iConfig.existsAs<int>("jetIDmin") ? iConfig.getParameter<int>("jetIDmin") : 1),
  dRmin(iConfig.existsAs<double>("dRmin") ? iConfig.getParameter<double>("dRmin") : 0.4),
  pTres(iConfig.existsAs<double>("pTres") ? iConfig.getParameter<double>("pTres") : 0.5),

  // pre-selection
  applyTrigger(iConfig.existsAs<bool>("applyTrigger") ? iConfig.getParameter<bool>("applyTrigger") : false),
  minHT(iConfig.existsAs<double>("minHT") ? iConfig.getParameter<double>("minHT") : 400.f),
  applyHT(iConfig.existsAs<bool>("applyHT") ? iConfig.getParameter<bool>("applyHT") : false),

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

  // photons + ids
  photonsTag          (iConfig.getParameter<edm::InputTag>("photons")),
  photonLooseIdMapTag (iConfig.getParameter<edm::InputTag>("photonLooseID")),
  photonMediumIdMapTag(iConfig.getParameter<edm::InputTag>("photonMediumID")),
  photonTightIdMapTag (iConfig.getParameter<edm::InputTag>("photonTightID")),

  // ootPhotons + ids
  ootPhotonsTag          (iConfig.getParameter<edm::InputTag>("ootPhotons")),
  ootPhotonLooseIdMapTag (iConfig.getParameter<edm::InputTag>("ootPhotonLooseID")),
  ootPhotonMediumIdMapTag(iConfig.getParameter<edm::InputTag>("ootPhotonMediumID")),
  ootPhotonTightIdMapTag (iConfig.getParameter<edm::InputTag>("ootPhotonTightID")),
  
  //recHits
  recHitsEBTag(iConfig.getParameter<edm::InputTag>("recHitsEB")),  
  recHitsEETag(iConfig.getParameter<edm::InputTag>("recHitsEE")),

  ///////////// GEN INFO
  // isMC or Data --> default Data
  isGMSB (iConfig.existsAs<bool>("isGMSB")  ? iConfig.getParameter<bool>("isGMSB")  : false),
  isHVDS (iConfig.existsAs<bool>("isHVDS")  ? iConfig.getParameter<bool>("isHVDS")  : false),
  isBkg  (iConfig.existsAs<bool>("isBkg")   ? iConfig.getParameter<bool>("isBkg")   : false),
{
  usesResource();
  usesResource("TFileService");

  // triggers
  triggerResultsToken = consumes<edm::TriggerResults> (triggerResultsTag);
  triggerObjectsToken = consumes<std::vector<pat::TriggerObjectStandAlone> > (triggerObjectsTag);

  // read in from a stream the trigger paths for saving
  oot::ReadInTriggerNames(inputPaths,pathNames,triggerBits);

  // read in from a stream the hlt objects/labels to match to
  oot::ReadInFilterNames(inputFilters,filterNames,triggerObjectsByFilter);

  //vertex
  verticesToken = consumes<std::vector<reco::Vertex> > (verticesTag);

  // rhos
  rhosToken = consumes<double> (rhosTag);

  // mets
  metsToken = consumes<std::vector<pat::MET> > (metsTag);

  // jets
  jetsToken = consumes<std::vector<pat::Jet> > (jetsTag);

  // photons + ids
  photonsToken = consumes<std::vector<pat::Photon> > (photonsTag);
  photonLooseIdMapToken  = consumes<edm::ValueMap<bool> > (photonLooseIdMapTag);
  photonMediumIdMapToken = consumes<edm::ValueMap<bool> > (photonMediumIdMapTag);
  photonTightIdMapToken  = consumes<edm::ValueMap<bool> > (photonTightIdMapTag);
  if (not ootPhotonsTag.label().empty())
  {
    ootPhotonsToken = consumes<std::vector<pat::Photon> > (ootPhotonsTag);
    ootPhotonLooseIdMapToken  = consumes<edm::ValueMap<bool> > (ootPhotonLooseIdMapTag);
    ootPhotonMediumIdMapToken = consumes<edm::ValueMap<bool> > (ootPhotonMediumIdMapTag);
    ootPhotonTightIdMapToken  = consumes<edm::ValueMap<bool> > (ootPhotonTightIdMapTag);
  }

  // rechits
  recHitsEBToken = consumes<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > (recHitsEBTag);
  recHitsEEToken = consumes<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > (recHitsEETag);

  // only for simulated samples
  if (isGMSB || isHVDS || isBkg)
  {
    isMC = true;
    genevtInfoToken = consumes<GenEventInfoProduct>             (iConfig.getParameter<edm::InputTag>("genevt"));
    pileupInfoToken = consumes<std::vector<PileupSummaryInfo> > (iConfig.getParameter<edm::InputTag>("pileup"));
    genpartsToken   = consumes<std::vector<reco::GenParticle> > (iConfig.getParameter<edm::InputTag>("genparts"));   
  }
  else 
  {
    isMC = false;
  }
}

DisPho::~DisPho() {}

void DisPho::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) 
{
  ////////////////////////
  //                    //
  // Get Object Handles //
  //                    //
  ////////////////////////

  // TRIGGERS
  edm::Handle<edm::TriggerResults> triggerResultsH;
  iEvent.getByToken(triggerResultsToken, triggerResultsH);

  edm::Handle<std::vector<pat::TriggerObjectStandAlone> > triggerObjectsH;
  iEvent.getByToken(triggerObjectsToken, triggerObjectsH);

  // VERTICES
  edm::Handle<std::vector<reco::Vertex> > verticesH;
  iEvent.getByToken(verticesToken, verticesH);
  
  // MET
  edm::Handle<std::vector<pat::MET> > metsH;
  iEvent.getByToken(metsToken, metsH);

  // RHOS
  edm::Handle<double> rhosH;
  iEvent.getByToken(rhosToken, rhosH);

  // JETS
  edm::Handle<std::vector<pat::Jet> > jetsH;
  iEvent.getByToken(jetsToken, jetsH);
  std::vector<pat::Jet> jets; jets.reserve(jetsH->size());

  // PHOTONS + IDS
  edm::Handle<std::vector<pat::Photon> > photonsH;
  iEvent.getByToken(photonsToken, photonsH);
  int phosize = photonsH->size();
  edm::Handle<edm::ValueMap<bool> > photonLooseIdMapH;
  iEvent.getByToken(photonLooseIdMapToken, photonLooseIdMapH);
  edm::Handle<edm::ValueMap<bool> > photonMediumIdMapH;
  iEvent.getByToken(photonMediumIdMapToken, photonMediumIdMapH);
  edm::Handle<edm::ValueMap<bool> > photonTightIdMapH;
  iEvent.getByToken(photonTightIdMapToken, photonTightIdMapH);

  // OOTPHOTONS + IDS
  edm::Handle<std::vector<pat::Photon> > ootPhotonsH;
  edm::Handle<edm::ValueMap<bool> > ootPhotonLooseIdMapH;
  edm::Handle<edm::ValueMap<bool> > ootPhotonMediumIdMapH;
  edm::Handle<edm::ValueMap<bool> > ootPhotonTightIdMapH;
  if (not ootPhotonsToken.isUninitialized())
  {
    iEvent.getByToken(ootPhotonsToken, ootPhotonsH);
    phosize += ootPhotonsH->size();
    iEvent.getByToken(ootPhotonLooseIdMapToken, ootPhotonLooseIdMapH);
    iEvent.getByToken(ootPhotonMediumIdMapToken, ootPhotonMediumIdMapH);
    iEvent.getByToken(ootPhotonTightIdMapToken, ootPhotonTightIdMapH);
  }
  // total photons vector
  std::vector<oot::Photon> photons; photons.reserve(phosize);

  // RecHits
  edm::Handle<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > recHitsEBH;
  iEvent.getByToken(recHitsEBToken, recHitsEBH);
  const EcalRecHitCollection * recHitsEB = recHitsEBH.product();
  edm::Handle<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > recHitsEEH;
  iEvent.getByToken(recHitsEEToken, recHitsEEH);
  const EcalRecHitCollection * recHitsEE = recHitsEBH.product();
  uiiumap recHitMap;

  // geometry (from ECAL ELF)
  edm::ESHandle<CaloGeometry> calogeoH;
  iSetup.get<CaloGeometryRecord>().get(calogeoH);
  const CaloSubdetectorGeometry * barrelGeometry = calogeoH->getSubdetectorGeometry(DetId::Ecal, EcalBarrel);
  const CaloSubdetectorGeometry * endcapGeometry = calogeoH->getSubdetectorGeometry(DetId::Ecal, EcalEndcap);

  // GEN INFO
  edm::Handle<GenEventInfoProduct> genevtInfoH;
  edm::Handle<std::vector<PileupSummaryInfo> > pileupInfoH;
  edm::Handle<std::vector<reco::GenParticle> > genparticlesH;

  if (isMC)
  {
    iEvent.getByToken(genevtInfoToken, genevtInfoH);
    iEvent.getByToken(pileupInfoToken, pileupInfoH);
    iEvent.getByToken(genpartsToken,   genparticlesH);
  }

  /////////////////////
  //                 //
  // Object Prepping //
  //                 //
/////////////////////

  // do some prepping of objects
  oot::PrepJets(jetsH,jets,jetpTmin,jetIDmin);
  oot::PrepRecHits(recHitsEB,recHitsEE,recHitMap,rhEmin);
  oot::PrepPhotons(photonsH,photonLooseIdMapH,photonMediumIdMapH,photonTightIdMapH,
		   ootPhotonsH,ootPhotonLooseIdMapH,ootPhotonMediumIdMapH,ootPhotonTightIdMapH,
		   photons,phpTmin);

  /////////////////////////
  //                     //
  // Apply pre-selection //
  //                     //
  /////////////////////////

  // trigger pre-selection
  DisPho::SetTriggerBits(triggerResultsH,iEvent);

  bool triggered = false;
  for (auto triggerBit : triggerBits)
  {
    if (triggerBit) {triggered = true; break;}
  }
  if (!triggered && applyTrigger) return;

  // HT pre-selection
  if (jetsH.isValid()) // check to make sure reco jets exist
  {
    for (const auto& jet : jets)
    {
      jetHT += jet.pt();
    }
  } // end check over reco jets  
  if (jetHT < minHT && applyHT) return;

  ///////////////////////////
  //                       //
  // Event, lumi, run info //
  //                       //
  ///////////////////////////
  run   = iEvent.id().run();
  lumi  = iEvent.luminosityBlock();
  event = iEvent.id().event();

  //////////////////
  //              //
  // Trigger Info //
  //              //
  //////////////////
  hltDisPho = triggerBits[0];

  /////////////////////////
  //                     //   
  // Primary Vertex info //
  //                     //
  /////////////////////////
  DisPho::InitializePVBranches();
  if (verticesH.isValid()) 
  {
    nvtx = verticesH->size();
    const reco::Vertex & primevtx = (*verticesH)[0];
    vtxX = primevtx.position().x();
    vtxY = primevtx.position().y();
    vtxZ = primevtx.position().z();
  }

  ///////////////////
  //               //
  // FixedGrid Rho //
  //               //
  ///////////////////
  const float rho = rhosH.isValid() ? *(rhosH.product()) : 0.f;

  //////////////////
  //              //
  // Type1 PF Met //
  //              //
  //////////////////
  DisPho::InitializeMETBranches();
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
  DisPho::InitializeJetBranches();
  if (jetsH.isValid()) // check to make sure reco jets exist
  {
    njets = jets.size();
    if (njets > 0) DisPho::SetJetBranches(jets[0],jetBranch0);
    if (njets > 1) DisPho::SetJetBranches(jets[1],jetBranch1);
    if (njets > 2) DisPho::SetJetBranches(jets[2],jetBranch2);
    if (njets > 3) DisPho::SetJetBranches(jets[3],jetBranch3);
  }

  //////////////
  //          //
  // Rec Hits //
  //          //
  //////////////
  DisPho::InitializeRecHits();
  if (recHitsEBH.isValid()) DisPho::SetRecHitBranches(recHitsEB,barrelGeometry);
  if (recHitsEEH.isValid()) DisPho::SetRecHitBranches(recHitsEE,endcapGeometry);

  //////////////////
  //              //
  // Reco Photons //
  //              //
  //////////////////
  DisPho::IntializePhotonBranches();
  if (photonsH.isValid() || ootPhotonsH.isValid()) // standard handle check
  {
    nphotons = photons.size();
    if (nphotons > 0) DisPho::SetPhotonBranches(photons[0],phoBranch0,recHitMap,recHitsEB,recHitsEE);
    if (nphotons > 1) DisPho::SetPhotonBranches(photons[1],phoBranch1,recHitMap,recHitsEB,recHitsEE);
    if (nphotons > 2) DisPho::SetPhotonBranches(photons[2],phoBranch2,recHitMap,recHitsEB,recHitsEE);
    if (nphotons > 3) DisPho::SetPhotonBranches(photons[3],phoBranch3,recHitMap,recHitsEB,recHitsEE);
  }

  ///////////////
  //           //
  // Fill Tree //
  //           //
  ///////////////
  tree->Fill();      
}

void DisPho::SetTriggerBits(edm::Handle<edm::TriggerResults> & triggerResultsH, const edm::Event & iEvent)
{
  for (auto triggerBit : triggerBits) triggerBit = false;

  if (triggerResultsH.isValid())
  {
    const edm::TriggerNames &triggerNames = iEvent.triggerNames(*triggerResultsH);
    for (std::size_t itrig = 0; itrig < triggerNames.size(); itrig++)
    {
      TString triggerName = triggerNames.triggerName(itrig);
      for (std::size_t ipath = 0; ipath < pathNames.size(); ipath++)
      {
	if (triggerName.Contains(pathNames[ipath],TString::kExact)) triggerBits[ipath] = triggerResultsH->accept(itrig);
      } // end loop over user path names
    } // end loop over trigger names
  } // end check over valid TriggerResults
}

void DisPho::SetRecHitBranches(const EcalRecHitCollection * recHits, const CaloSubdetectorGeometry * geometry)
{
  for (const auto recHit : *recHits)
  {
    const DetId recHitId(recHit.detid());
    const uint32_t rawId = recHitId.rawId();
    if (recHitMap.count(rawId))
    {
      const int pos = recHitMap[rawId];
      
      const auto recHitPos = geometry->getGeometry(recHitId)->getPosition();
      
      // save position, energy, and time of each rechit to a vector
      phrheta [pos] = recHitPos.eta();
      phrhphi [pos] = recHitPos.phi();
      phrhE   [pos] = recHit->energy();
      phrhtime[pos] = recHit->time();
      phrhOOT [pos] = int(recHit->checkFlag(EcalRecHit::kOutOfTime));   
      phrhID  [pos] = rawId;
    }
  }
}

struct phoBranch
{
  float E_;
  float Pt_;
  float Phi_;
  float Eta_;

  float scE_;
  float scPhi_;
  float scEta_;

  float HoE_;
  float r9_;
  float ChgHadIso_;
  float NeuHadIso_;
  float PhoIso_;

  float Sieie_; 
  float Sipip_;
  float Sieip_;

  float Smaj_;
  float Smin_;
  float alpha_;

  int seedID;
  std::vector<int> recHitIDs;

  bool isOOT_;
  bool isEB_;
  int  ID_;
};

void DisPho::SetPhotonBranches(const oot::Photon& photon, phoStruct & phoBranch, const uiiumap & recHitMap,
			       const EcalRecHitCollection * recHitsEB, const EcalRecHitCollection * recHitsEE)
{
  // basic kinematics
  const pat::Photon & pho = photon.photon(); 
  phoBranch.E_   = pho.energy();
  phoBranch.Pt_  = pho.pt();
  phoBranch.Phi_ = pho.phi();
  phoBranch.Eta_ = pho.eta();

  // super cluster from photon
  const reco::SuperClusterRef& phosc = pho.superCluster().isNonnull() ? pho.superCluster() : pho.parentSuperCluster();
  phoBranch.scE_   = phosc->energy();
  phoBranch.scPhi_ = phosc->phi();
  phoBranch.scEta_ = phosc->eta();

  const float sceta = std::abs(phoBranch.phoscPhi_);

  // ID-like variables
  phoBranch.HoE_       = pho.hadronicOverEm(); // used in ID
  phoBranch.r9_        = pho.r9();
  phoBranch.ChgHadIso_ = std::max(pho.chargedHadronIso() - (rho * oot::GetChargedHadronEA(sceta)),0.f);
  phoBranch.NeuHadIso_ = std::max(pho.neutralHadronIso() - (rho * oot::GetNeutralHadronEA(sceta)),0.f);
  phoBranch.PhoIso_    = std::max(pho.photonIso()        - (rho * oot::GetGammaEA        (sceta)),0.f);

  // Shower Shape Objects
  const reco::Photon::ShowerShape& phoshape = pho.full5x5_showerShapeVariables(); 

  // cluster shape variables
  phoBranch.Sieie_ = phoshape.sigmaIetaIeta;
  phoBranch.Sipip_ = phoshape.sigmaIphiIphi;
  phoBranch.Sieip_ = phoshape.sigmaIetaIphi;

  // use seed to get geometry and recHits
  const DetId & seedDetId = phosc->seed()->seed(); //seed detid
  const bool isEB = (seedDetId.subdetId() == EcalBarrel); //which subdet
  const EcalRecHitCollection * recHits = (isEB ? recHitsEB : recHitsEE); 

  // 2nd moments from official calculation
  if (recHits->size() > 0)
  {
    const Cluster2ndMoments ph2ndMoments = noZS::EcalClusterTools::cluster2ndMoments( *phosc, *recHits);
    // radius of semi-major,minor axis is the inverse square root of the eigenvalues of the covariance matrix
    phoBranch.Smaj_  = ph2ndMoments.sMaj;
    phoBranch.Smin_  = ph2ndMoments.sMin;
    phoBranch.alpha_ = ph2ndMoments.alpha;
  }

  // map of rec hit ids
  uiiumap phrhIDmap;
  const DetIdPairVec & hitsAndFractions = phosc->hitsAndFractions();
  for (DetIdPairVec::const_iterator hafitr = hitsAndFractions.begin(); hafitr != hitsAndFractions.end(); ++hafitr) // loop over all rec hits in SC
  {
    const DetId & recHitId = hafitr->first; // get detid of crystal
    const uint32_t rawId = recHitId.rawId();
    if (recHitMap.count(rawId))
    {
      EcalRecHitCollection::const_iterator recHit = recHits->find(recHitId); // get the underlying rechit
      if (recHit != recHits->end()) // standard check
      { 
	phrhIDmap[recHitMap[rawId]]++;
      } // end standard check recHit
    } // end check over if recHit is in map
  } // end loop over hits and fractions

  // store the position inside the recHits vector 
  for (uiiumap::const_iterator rhiter = phrhIDmap.begin(); rhiter != phrhIDmap.end(); ++rhiter) // loop over only good rec hit ids
  {
    phoBranch.recHits_.emplace_back(rhiter->first);
  }

  // save seed id pos
  if (recHitMap.count(seedDetId.rawId())) 
  {
    phoBranch.seed_ = recHitMap[seedDetId.rawId()];
  }

  // some standard booleans
  phoBranch.isOOT_ = photon.isOOT();
  phoBranch.isEB_  = isEB;

  // 0 --> did not pass anything, 1 --> loose pass, 2 --> medium pass, 3 --> tight pass
  if      (pho.photonID("tight"))  {phoBranch.ID_ = 3;}
  else if (pho.photonID("medium")) {phoBranch.ID_ = 2;}
  else if (pho.photonID("loose"))  {phoBranch.ID_ = 1;}
  else                             {phoBranch.ID_ = 0;}
}

void DisPho::InializeJetBranches()
{
  DisPho::InitializeJetBranches(jetBranch0);
  DisPho::InitializeJetBranches(jetBranch1);
  DisPho::InitializeJetBranches(jetBranch2);
  DisPho::InitializeJetBranches(jetBranch3);
}

struct jetStruct
{
  float E_;
  float Pt_;
  float Phi_;
  float Eta_;
};

void DisPho::SetJetBranches(const pat::Jet & jet, jetStruct & jetBranch)
{
  jetBranch.E_   = jet.energy();
  jetBranch.Pt_  = jet.pt();
  jetBranch.Eta_ = jet.eta();
  jetBranch.Phi_ = jet.phi();
}

struct phoBranch
{
  float E_;
  float Pt_;
  float Phi_;
  float Eta_;

  float scE_;
  float scPhi_;
  float scEta_;

  float HoE_;
  float r9_;
  float ChgHadIso_;
  float NeuHadIso_;
  float PhoIso_;

  float Sieie_; 
  float Sipip_;
  float Sieip_;

  float Smaj_;
  float Smin_;
  float alpha_;

  int seed_;
  std::vector<int> recHits_;

  bool isOOT_;
  bool isEB_;
  int  ID_;
};

void DisPho::InializePhotonBranches()
{
  DisPho::InitializePhotonBranch(phoBranches0);
  DisPho::InitializePhotonBranch(phoBranches1);
  DisPho::InitializePhotonBranch(phoBranches2);
  DisPho::InitializePhotonBranch(phoBranches3);
}

void DisPho::InializePhotonBranch(phoStruct & phoBranch)
{
  phoBranch.E_ = -9999.f;
  phoBranch.Pt_ = -9999.f;
  phoBranch.Eta_ = -9999.f;
  phoBranch.Phi_ = -9999.f;

  phoBranch.scE_ = -9999.f;
  phoBranch.scPhi_ = -9999.f;
  phoBranch.scEta_ = -9999.f;

  phoBranch.HoE_ = -9999.f;
  phoBranch.r9_ = -9999.f;
  phoBranch.ChgHadIso_ = -9999.f;
  phoBranch.NeuHadIso_ = -9999.f;
  phoBranch.PhoIso_ = -9999.f;

  phoBranch.Sieie_ = -9999.f;
  phoBranch.Sipip_ = -9999.f;
  phoBranch.Sieip_ = -9999.f;

  phoBranch.Smaj_ = -9999.f;
  phoBranch.Smin_ = -9999.f;
  phoBranch.alpha_ = -9999.f;

  phoBranch.seed_ = -1;
  phoBranch.recHits_.clear();

  phoBranch.isOOT_ = false;
  phoBranch.isEB_ = false;
  phoBranch.ID_ = -1;
}

void DisPho::InializeJetBranch(jetStruct & jetBranch)
{
  jetBranch.E_   = -9999.f;
  jetBranch.Pt_  = -9999.f;
  jetBranch.Phi_ = -9999.f;
  jetBranch.Eta_ = -9999.f;
}

void DisPho::InitializePVBranches()
{
  nvtx = -9999; 
  vtxX = -9999.f; vtxY = -9999.f; vtxZ = -9999.f;
}

void DisPho::InitializeMETBranches()
{
  t1pfMETpt    = -9999.f; 
  t1pfMETphi   = -9999.f; 
  t1pfMETsumEt = -9999.f;
}

void DisPho::beginJob() 
{
  edm::Service<TFileService> fs;
  tree = fs->make<TTree>("tree","tree");

  // Run, Lumi, Event info
  tree->Branch("event", &event, "event/l");
  tree->Branch("run", &run, "run/i");
  tree->Branch("lumi", &lumi, "lumi/i");
   
  // Trigger Info
  tree->Branch("triggerBits", &triggerBits);

  // Vertex info
  tree->Branch("nvtx", &nvtx, "nvtx/I");
  tree->Branch("vtxX", &vtxX, "vtxX/F");
  tree->Branch("vtxY", &vtxY, "vtxY/F");
  tree->Branch("vtxZ", &vtxZ, "vtxZ/F");

  // MET info
  tree->Branch("t1pfMETpt", &t1pfMETpt, "t1pfMETpt/F");
  tree->Branch("t1pfMETphi", &t1pfMETphi, "t1pfMETphi/F");
  tree->Branch("t1pfMETsumEt", &t1pfMETsumEt, "t1pfMETsumEt/F");

  // Jet Info
  tree->Branch("njets", &njets, "njets/I");
  DisPho::MakeJetBranches(0,jetBranch0);
  DisPho::MakeJetBranches(1,jetBranch1);
  DisPho::MakeJetBranches(2,jetBranch2);
  DisPho::MakeJetBranches(3,jetBranch3);

  // Photon Info
  tree->Branch("nphotons", &nphotons, "nphotons/I");
  DisPho::MakePhoBranches(0,phoBranch0);
  DisPho::MakePhoBranches(1,phoBranch1);
  DisPho::MakePhoBranches(2,phoBranch2);
  DisPho::MakePhoBranches(3,phoBranch3);

}

void DisPho::MakeJetBranches(const int i, const jetStruct& jetBranch)
{
  tree->Branch(Form("jetE%i",i), &jetBranch.E_, Form("jetE%i/F",i));
  tree->Branch(Form("jetPt%i",i), &jetBranch.Pt_, Form("jetPt%i/F",i));
  tree->Branch(Form("jetEta%i",i), &jetBranch.Eta_, Form("jetEta%i/F",i));
  tree->Branch(Form("jetPhi%i",i), &jetBranch.Phi_, Form("jetPhi%i/F",i));
}

void DisPho::MakePhoBranches(const int i, const phoStruct& phoBranch)
{
  tree->Branch(Form("phoE%i",i), &phoBranch.E_, Form("phoE%i/F",i));
  tree->Branch(Form("phoPt%i",i), &phoBranch.Pt_, Form("phoPt%i/F",i));
  tree->Branch(Form("phoEta%i",i), &phoBranch.Eta_, Form("phoEta%i/F",i));
  tree->Branch(Form("phoPhi%i",i), &phoBranch.Phi_, Form("phoPhi%i/F",i));
}



void DisPho::endJob() {}

void DisPho::beginRun(edm::Run const& iRun, edm::EventSetup const& iSetup) {}

void DisPho::endRun(edm::Run const&, edm::EventSetup const&) {}

void DisPho::fillDescriptions(edm::ConfigurationDescriptions& descriptions) 
{
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

DEFINE_FWK_MODULE(DisPho);
