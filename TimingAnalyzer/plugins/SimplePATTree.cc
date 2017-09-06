#include "Timing/TimingAnalyzer/plugins/SimplePATTree.hh"
#include "FWCore/Utilities/interface/isFinite.h"

SimplePATTree::SimplePATTree(const edm::ParameterSet& iConfig): 
  // isMC
  isMC(iConfig.existsAs<bool>("isMC") ? iConfig.getParameter<bool>("isMC") : false),

  // rhos
  rhosTag(iConfig.getParameter<edm::InputTag>("rhos")),

   // vertexes
  verticesTag(iConfig.getParameter<edm::InputTag>("vertices")),

  // mets
  metsTag(iConfig.getParameter<edm::InputTag>("mets")),  

  // jets
  jetsTag(iConfig.getParameter<edm::InputTag>("jets")),  

  //photons
  photonsTag   (iConfig.getParameter<edm::InputTag>("photons")),  
  ootPhotonsTag(iConfig.getParameter<edm::InputTag>("ootphotons")),  
  
  //recHits
  recHitsEBTag(iConfig.getParameter<edm::InputTag>("recHitsEB")),  
  recHitsEETag(iConfig.getParameter<edm::InputTag>("recHitsEE"))
{
  usesResource();
  usesResource("TFileService");

  if (isMC)
  {
    pileupToken = consumes<std::vector<PileupSummaryInfo> > (iConfig.getParameter<edm::InputTag>("pileup"));
  }

  // rhos
  rhosToken = consumes<double> (rhosTag);

  //vertex
  verticesToken = consumes<std::vector<reco::Vertex> > (verticesTag);

  // mets
  metsToken = consumes<std::vector<pat::MET> > (metsTag);

  // jets
  jetsToken = consumes<std::vector<pat::Jet> > (jetsTag);

  // photons
  photonsToken    = consumes<std::vector<pat::Photon> > (photonsTag);
  if (not ootPhotonsTag.label().empty())
  {
    ootPhotonsToken = consumes<std::vector<pat::Photon> > (ootPhotonsTag);
  }

  // rechits
  recHitsEBToken = consumes<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > (recHitsEBTag);
  recHitsEEToken = consumes<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > (recHitsEETag);
}

SimplePATTree::~SimplePATTree() {}

void SimplePATTree::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) 
{
  // MC Info
  edm::Handle<std::vector<PileupSummaryInfo> > pileupH;
  if (isMC)
  {
    iEvent.getByToken(pileupToken, pileupH);
  }
  
  // RHOS
  edm::Handle<double> rhosH;
  iEvent.getByToken(rhosToken, rhosH);

  // VERTICES
  edm::Handle<std::vector<reco::Vertex> > verticesH;
  iEvent.getByToken(verticesToken, verticesH);
  
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

  // do some prepping of objects
  oot::PrepJets(jetsH,jets);
  oot::PrepPhotons(photonsH,ootPhotonsH,photons);

  ///////////////////////////
  //                       //
  // Event, lumi, run info //
  //                       //
  ///////////////////////////
  run   = iEvent.id().run();
  lumi  = iEvent.luminosityBlock();
  event = iEvent.id().event();

  /////////////
  //         //
  // MC Info //
  //         //
  /////////////
  if (isMC)
  {
    npuobs  = -1;
    nputrue = -1;
    if (pileupH.isValid())
    {
      for (std::vector<PileupSummaryInfo>::const_iterator puiter = pileupH->begin(); puiter != pileupH->end(); ++puiter) 
      {
	if (puiter->getBunchCrossing() == 0) 
	{
	  npuobs  = puiter->getPU_NumInteractions();
	  nputrue = puiter->getTrueNumInteractions();
	} // end check over correct BX
      } // end loop over PU
    } // end check over pileup info valid
  } // end block over MC only code
  
  ///////////////////
  //               //
  // FixedGrid Rho //
  //               //
  ///////////////////

  rho = -9999.f;
  if (rhosH.isValid())
  {
    rho = *(rhosH.product());
  }

  /////////////////////////
  //                     //   
  // Primary Vertex info //
  //                     //
  /////////////////////////
  SimplePATTree::InitializePVBranches();
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
  // Type1 PF Met //
  //              //
  //////////////////
  SimplePATTree::InitializeMETBranches();
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
  SimplePATTree::InitializeJetBranches();
  if (jetsH.isValid()) // check to make sure reco jets exist
  {
    njets = jets.size();
    
    njets15 = 0; jetHT15 = 0.f;
    njets30 = 0; jetHT30 = 0.f;
    for (std::vector<pat::Jet>::const_iterator jetiter = jets.begin(); jetiter != jets.end(); ++jetiter)
    {
      if (jetiter->pt() > 15.) {njets15++; jetHT15 += jetiter->pt();}
      if (jetiter->pt() > 30.) {njets30++; jetHT30 += jetiter->pt();}
    }
  }

  //////////////////
  //              //
  // Reco Photons //
  //              //
  //////////////////
  SimplePATTree::ClearRecoPhotonBranches();
  if (photonsH.isValid()) // standard handle check
  {
    nphotons = photons.size();
    if (nphotons > 0) SimplePATTree::InitializeRecoPhotonBranches();

    int iph = 0;
    for (std::vector<oot::Photon>::const_iterator phiter = photons.begin(); phiter != photons.end(); ++phiter) // loop over photon vector
    {
      const pat::Photon& photon = phiter->photon();

      // which photon collection
      phIsOOT[iph] = phiter->isOOT();

      // standard photon branches
      phE  [iph] = photon.energy();
      phpt [iph] = photon.pt();
      phphi[iph] = photon.phi();
      pheta[iph] = photon.eta();

      // ID-like variables
      phHoE  [iph] = photon.hadTowOverEm();
      phr9   [iph] = photon.r9();
      phsieie[iph] = photon.full5x5_sigmaIetaIeta();

      // super cluster from photon
      const reco::SuperClusterRef& phsc = photon.superCluster().isNonnull() ? photon.superCluster() : photon.parentSuperCluster();
      const reco::CaloClusterPtr& phbc = phsc->seed();
      const DetId seedDetId = phbc->seed(); //seed detid

      const EcalRecHitCollection * recHits = nullptr;
      if      (seedDetId.subdetId() == EcalBarrel) 
      {
	phIsEB[iph] = 1;
	recHits = recHitsEBH.product();
      }
      else if (seedDetId.subdetId() == EcalEndcap) 
      {
	phIsEB[iph] = 0;
	recHits = recHitsEEH.product();
      }

      // need recHits to calculate shape variables
      if (recHits->size() > 0)
      {
	// 2nd moments from official calculation
	const Cluster2ndMoments ph2ndMoments = noZS::EcalClusterTools::cluster2ndMoments( *phsc, *recHits);
	// radius of semi-major,minor axis is the inverse square root of the eigenvalues of the covariance matrix
	phsmaj [iph] = ph2ndMoments.sMaj;
	phsmin [iph] = ph2ndMoments.sMin;
	phalpha[iph] = ph2ndMoments.alpha;
      }

      // PF Cluster Isolations
      phPFClEcalIso[iph] = photon.ecalPFClusterIso();
      phPFClHcalIso[iph] = photon.hcalPFClusterIso();

      // Track Isolation (dR of outer cone < 0.3 as matching in trigger)
      phHollowTkIso[iph] = photon.trkSumPtHollowConeDR03();

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
      int irh = 0;
      for (uiiumap::const_iterator rhiter = phrhIDmap.begin(); rhiter != phrhIDmap.end(); ++rhiter) // loop over only good rec hit ids
      {
	const uint32_t rhID = rhiter->first;
	
	const DetId recHitId(rhID);
	EcalRecHitCollection::const_iterator recHit = recHits->find(recHitId); // get the underlying rechit
	
	// extra info from the SEED
	if (seedDetId.rawId() == recHitId) 
	{ 
	  phseedE   [iph] = recHit->energy();
	  phseedtime[iph] = recHit->time();
	  phseedOOT [iph] = int(recHit->checkFlag(EcalRecHit::kOutOfTime));
	}
	
	irh++; // increment rechit counter
      } // end loop over rec hit id map
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

void SimplePATTree::InitializePVBranches()
{
  nvtx = -1;
  vtxX = -9999.f;
  vtxY = -9999.f;
  vtxZ = -9999.f;
}

void SimplePATTree::InitializeMETBranches()
{
  t1pfMETpt    = -9999.f;
  t1pfMETphi   = -9999.f;
  t1pfMETsumEt = -9999.f;
}

void SimplePATTree::InitializeJetBranches()
{
  njets = -1;
  njets15 = -1; 
  njets30 = -1; 
  
  jetHT15 = -9999.f;
  jetHT30 = -9999.f;
}

void SimplePATTree::ClearRecoPhotonBranches()
{
  nphotons = -1;

  phIsOOT.clear();
  phIsEB.clear();

  phE.clear();
  phpt.clear();
  phphi.clear(); 
  pheta.clear(); 

  phHoE.clear();
  phr9.clear();
  phsieie.clear();

  phsmaj.clear();
  phsmin.clear();
  phalpha.clear();

  phPFClEcalIso.clear();
  phPFClHcalIso.clear();
  phHollowTkIso.clear();

  phnrh.clear();

  phseedE.clear(); 
  phseedtime.clear();
  phseedOOT.clear();
}

void SimplePATTree::InitializeRecoPhotonBranches()
{
  phIsOOT.resize(nphotons);
  phIsEB.resize(nphotons);
  
  phE.resize(nphotons);
  phpt.resize(nphotons);
  phphi.resize(nphotons);
  pheta.resize(nphotons);
  
  phHoE.resize(nphotons);
  phr9.resize(nphotons);
  phsieie.resize(nphotons);

  phsmaj.resize(nphotons);
  phsmin.resize(nphotons);
  phalpha.resize(nphotons);

  phPFClEcalIso.resize(nphotons);
  phPFClHcalIso.resize(nphotons);
  phHollowTkIso.resize(nphotons);

  phnrh.resize(nphotons);

  phseedE.resize(nphotons);
  phseedtime.resize(nphotons);
  phseedOOT.resize(nphotons);

  for (int iph = 0; iph < nphotons; iph++)
  {
    phIsOOT[iph] = -1;
    phIsEB [iph] = -1;

    phE  [iph] = -9999.f; 
    phpt [iph] = -9999.f; 
    phphi[iph] = -9999.f; 
    pheta[iph] = -9999.f; 

    phHoE  [iph] = -9999.f;
    phr9   [iph] = -9999.f;
    phsieie[iph] = -9999.f;

    phsmaj [iph] = -9999.f;
    phsmin [iph] = -9999.f;
    phalpha[iph] = -9999.f;

    phPFClEcalIso[iph] = -9999.f;
    phPFClHcalIso[iph] = -9999.f;
    phHollowTkIso[iph] = -9999.f;

    phnrh[iph] = -1;

    phseedE   [iph] = -9999.f;
    phseedtime[iph] = -9999.f;
    phseedOOT [iph] = -1;
  }
}

void SimplePATTree::beginJob() 
{
  edm::Service<TFileService> fs;
  tree = fs->make<TTree>("tree"     , "tree");

  // Run, Lumi, Event info
  tree->Branch("event"                , &event                , "event/l");
  tree->Branch("run"                  , &run                  , "run/i");
  tree->Branch("lumi"                 , &lumi                 , "lumi/i");
  
  // Pileup info
  tree->Branch("npuobs"               , &npuobs               , "npuobs/I");
  tree->Branch("nputrue"              , &nputrue              , "nputrue/I");

  // rho
  tree->Branch("rho"                  , &rho                  , "rho/F");

  // Reco Vertex info
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
  tree->Branch("njets15"              , &njets15              , "njets15/I");
  tree->Branch("njets30"              , &njets30              , "njets30/I");
  tree->Branch("jetHT15"              , &jetHT15              , "jetHT15/F");
  tree->Branch("jetHT30"              , &jetHT30              , "jetHT30/F");
 
  // Photon Info
  tree->Branch("nphotons"             , &nphotons             , "nphotons/I");

  tree->Branch("phIsOOT"              , &phIsOOT);
  tree->Branch("phIsEB"               , &phIsEB);

  tree->Branch("phE"                  , &phE);
  tree->Branch("phpt"                 , &phpt);
  tree->Branch("phphi"                , &phphi);
  tree->Branch("pheta"                , &pheta);

  tree->Branch("phHoE"                , &phHoE);
  tree->Branch("phr9"                 , &phr9);
  tree->Branch("phsieie"              , &phsieie);

  tree->Branch("phsmaj"               , &phsmaj);
  tree->Branch("phsmin"               , &phsmin);
  tree->Branch("phalpha"              , &phalpha);

  tree->Branch("phPFClEcalIso"        , &phPFClEcalIso);
  tree->Branch("phPFClHcalIso"        , &phPFClHcalIso);
  tree->Branch("phHollowTkIso"        , &phHollowTkIso);

  tree->Branch("phnrh"                , &phnrh);

  tree->Branch("phseedE"              , &phseedE);
  tree->Branch("phseedtime"           , &phseedtime);
  tree->Branch("phseedOOT"            , &phseedOOT);
}

void SimplePATTree::endJob() {}

void SimplePATTree::beginRun(edm::Run const& iRun, edm::EventSetup const& iSetup) {}

void SimplePATTree::endRun(edm::Run const&, edm::EventSetup const&) {}

void SimplePATTree::fillDescriptions(edm::ConfigurationDescriptions& descriptions) 
{
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

DEFINE_FWK_MODULE(SimplePATTree);
