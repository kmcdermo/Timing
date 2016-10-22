#include "PhotonDump.h"

PhotonDump::PhotonDump(const edm::ParameterSet& iConfig): 
  // vertexes
  verticesTag(iConfig.getParameter<edm::InputTag>("vertices")),

  // mets
  metsTag(iConfig.getParameter<edm::InputTag>("mets")),  

  // jets
  jetsTag(iConfig.getParameter<edm::InputTag>("jets")),  

  // photons + ids
  photonLooseIdMapTag (iConfig.getParameter<edm::InputTag>("loosePhotonID")),  
  photonMediumIdMapTag(iConfig.getParameter<edm::InputTag>("mediumPhotonID")),  
  photonTightIdMapTag (iConfig.getParameter<edm::InputTag>("tightPhotonID")),  
  photonsTag(iConfig.getParameter<edm::InputTag>("photons")),  

  //recHits
  recHitCollectionEBTAG(consumes<EcalRecHitCollection>(iConfig.getParameter<edm::InputTag>( "recHitCollectionEB" ))),
  recHitCollectionEETAG(consumes<EcalRecHitCollection>(iConfig.getParameter<edm::InputTag>( "recHitCollectionEE" ))),

  ///////////// GEN INFO
  // isMC or Data --> default Data
  isMC(iConfig.existsAs<bool>("isMC") ? iConfig.getParameter<bool>("isMC") : false)
{
  usesResource();
  usesResource("TFileService");

  //vertex
  verticesToken = consumes<std::vector<reco::Vertex> > (verticesTag);

  // mets
  metsToken = consumes<std::vector<pat::MET> > (metsTag);

  // jets
  jetsToken = consumes<std::vector<pat::Jet> > (jetsTag);

  // photons + ids
  photonLooseIdMapToken  = consumes<edm::ValueMap<bool> > (photonLooseIdMapTag);
  photonMediumIdMapToken = consumes<edm::ValueMap<bool> > (photonMediumIdMapTag);
  photonTightIdMapToken  = consumes<edm::ValueMap<bool> > (photonTightIdMapTag);
  photonsToken           = consumes<std::vector<pat::Photon> > (photonsTag);

  // only for simulated samples
  if (isMC)
  {
    genevtInfoToken = consumes<GenEventInfoProduct>             (iConfig.getParameter<edm::InputTag>("genevt"));
    pileupInfoToken = consumes<std::vector<PileupSummaryInfo> > (iConfig.getParameter<edm::InputTag>("pileup"));
    genpartsToken   = consumes<std::vector<reco::GenParticle> > (iConfig.getParameter<edm::InputTag>("genparts"));   
    genjetsToken    = consumes<std::vector<reco::GenJet> >      (iConfig.getParameter<edm::InputTag>("genjets"));   
  }
}

PhotonDump::~PhotonDump() {}

void PhotonDump::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) 
{
  // VERTEX
  edm::Handle<std::vector<reco::Vertex> > verticesH;
  iEvent.getByToken(verticesToken, verticesH);
  
  // MET
  edm::Handle<std::vector<pat::MET> > metsH;
  iEvent.getByToken(metsToken, metsH);

  // JETS
  edm::Handle<std::vector<pat::Jet> > jetsH;
  iEvent.getByToken(jetsToken, jetsH);

  // PHOTONS + IDS
  edm::Handle<edm::ValueMap<bool> > photonLooseIdMapH;
  iEvent.getByToken(photonLooseIdMapToken, photonLooseIdMapH);
  edm::ValueMap<bool> photonLooseIdMap = *photonLooseIdMapH;

  edm::Handle<edm::ValueMap<bool> > photonMediumIdMapH;
  iEvent.getByToken(photonMediumIdMapToken, photonMediumIdMapH);
  edm::ValueMap<bool> photonMediumIdMap = *photonMediumIdMapH;

  edm::Handle<edm::ValueMap<bool> > photonTightIdMapH;
  iEvent.getByToken(photonTightIdMapToken, photonTightIdMapH);
  edm::ValueMap<bool> photonTightIdMap = *photonTightIdMapH;

  edm::Handle<std::vector<pat::Photon> > photonsH;
  iEvent.getByToken(photonsToken, photonsH);

  // geometry (from ECAL ELF)
  edm::ESHandle<CaloGeometry> calogeoH;
  iSetup.get<CaloGeometryRecord>().get(calogeoH);
  const CaloSubdetectorGeometry * barrelGeometry = calogeoH->getSubdetectorGeometry(DetId::Ecal, EcalBarrel);
  const CaloSubdetectorGeometry * endcapGeometry = calogeoH->getSubdetectorGeometry(DetId::Ecal, EcalEndcap);

  ///////////////////////////
  //                       //
  // Event, lumi, run info //
  //                       //
  ///////////////////////////
  event = iEvent.id().event();
  run   = iEvent.id().run();
  lumi  = iEvent.luminosityBlock();

  /////////////
  //         //
  // MC INFO //
  //         //
  /////////////
  if (isMC) 
  {
    edm::Handle<GenEventInfoProduct> genevtInfoH;
    iEvent.getByToken(genevtInfoToken, genevtInfoH);

    edm::Handle<std::vector<PileupSummaryInfo> > pileupInfoH;
    iEvent.getByToken(pileupInfoToken, pileupInfoH);

    edm::Handle<std::vector<reco::GenParticle> > genparticlesH;
    iEvent.getByToken(genpartsToken, genparticlesH);

    edm::Handle<std::vector<reco::GenJet> > genjetsH;
    iEvent.getByToken(genjetsToken, genjetsH);
    
    ///////////////////////
    //                   //
    // Event weight info //
    //                   //
    ///////////////////////
    genwgt = -9999.f; 
    if (genevtInfoH.isValid()) {genwgt = genevtInfoH->weight();}

    /////////////////////
    //                 //
    // Gen pileup info //
    //                 //
    /////////////////////
    genpuobs  = -9999;
    genputrue = -9999;
    if (pileupInfoH.isValid()) // standard check for pileup
    {
      for (std::vector<PileupSummaryInfo>::const_iterator puiter = pileupInfoH->begin(); puiter != pileupInfoH->end(); ++puiter) 
      {
	if (puiter->getBunchCrossing() == 0) 
	{
	  genpuobs  = puiter->getPU_NumInteractions();
	  genputrue = puiter->getTrueNumInteractions();
	} // end check over correct BX
      } // end loop over PU
    } // end check over pileup
  } // end block over isMC

  /////////////////
  //             //   
  // Vertex info //
  //             //
  /////////////////
  nvtx = -9999; 
  vtxX = -9999.f; vtxY = -9999.f; vtxZ = -9999.f;
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
  t1pfmet      = -9999.f;
  t1pfmetphi   = -9999.f;
  t1pfmeteta   = -9999.f;
  t1pfmetsumEt = -9999.f;
  if (metsH.isValid())
  {
    const pat::MET & t1pfMET = (*metsH)[0];
    t1pfmet      = t1pfMET.pt();
    t1pfmetphi   = t1pfMET.phi();
    t1pfmeteta   = t1pfMET.eta();
    t1pfmetsumEt = t1pfMET.sumEt();
  }

  /////////////////////////
  //                     //
  // Jets (AK4 standard) //
  //                     //
  /////////////////////////
  njets = -9999;
  PhotonDump::ClearJetBranches();
  if (jetsH.isValid()) // check to make sure it is valid
  {
    njets = jetsH->size();
    if (njets > 0) PhotonDump::InitializeJetBranches();
    int ijet = 0;
    for (std::vector<pat::Jet>::const_iterator jetiter = jetsH->begin(); jetiter != jetsH->end(); ++jetiter)
    {
      jetE  [ijet] = jetiter->energy();
      jetpt [ijet] = jetiter->pt();
      jetphi[ijet] = jetiter->phi();
      jeteta[ijet] = jetiter->eta();

      ijet++;
    }      
  }
  
  //////////////////
  //              //
  // Reco Photons //
  //              //
  //////////////////
  nphotons = -9999;
  PhotonDump::ClearRecoPhotonBranches();
  if (photonsH.isValid()) // standard handle check
  {
    // ECALELF tools
    EcalClusterLazyTools * clustertools = new EcalClusterLazyTools (iEvent, iSetup, recHitCollectionEBTAG, recHitCollectionEETAG);

    nphotons = photonsH->size();
    if (nphotons > 0) PhotonDump::InitializeRecoPhotonBranches();

    int iph = 0;
    for (std::vector<pat::Photon>::const_iterator phiter = photonsH->begin(); phiter != photonsH->end(); ++phiter) // loop over photon vector
    {
      // Get the id of the photon
      const edm::Ptr<pat::Photon> photonPtr(photonsH, phiter - photonsH->begin());

      // loose > medium > tight
      if (photonLooseIdMap [photonPtr]) {phVID[iph] = 1;}
      if (photonMediumIdMap[photonPtr]) {phVID[iph] = 2;}
      if (photonTightIdMap [photonPtr]) {phVID[iph] = 3;}

      // standard photon branches
      phE  [iph] = phiter->energy();
      phpt [iph] = phiter->pt();
      phphi[iph] = phiter->phi();
      pheta[iph] = phiter->eta();

      // super cluster from photon
      const reco::SuperClusterRef& phsc = phiter->superCluster().isNonnull() ? phiter->superCluster() : phiter->parentSuperCluster();
      if (phsc.isNonnull()) // check to make sure supercluster is good
      {
	phscX[iph] = phsc->position().x();
	phscY[iph] = phsc->position().y();
	phscZ[iph] = phsc->position().z();
	phscE[iph] = phsc->energy();

	// use seed to get geometry and recHits
	const DetId seedDetId = phsc->seed()->seed(); //seed detid
	const bool isEB = (seedDetId.subdetId() == EcalBarrel); //which subdet
	const EcalRecHitCollection * recHits = isEB ? clustertools->getEcalEBRecHitCollection() : clustertools->getEcalEERecHitCollection();

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
      
	phnrhs[iph] = phrhIDmap.size();
	if (phnrhs[iph] > 0) PhotonDump::InitializeRecoRecHitBranches(iph);

	int irh = 0;
	for (uiiumap::const_iterator rhiter = phrhIDmap.begin(); rhiter != phrhIDmap.end(); ++rhiter) // loop over only good rec hit ids
        {
	  const uint32_t rhID = rhiter->first;

	  const DetId recHitId(rhID);
	  EcalRecHitCollection::const_iterator recHit = recHits->find(recHitId); // get the underlying rechit
	  
	  const auto recHitPos = isEB ? barrelGeometry->getGeometry(recHitId)->getPosition() : endcapGeometry->getGeometry(recHitId)->getPosition();

	  // save position, energy, and time of each rechit to a vector
	  phrhXs[iph][irh]    = recHitPos.x();
	  phrhYs[iph][irh]    = recHitPos.y();
	  phrhZs[iph][irh]    = recHitPos.z();
	  phrhEs[iph][irh]    = recHit->energy();
	  phrhtimes[iph][irh] = recHit->time();
	  phrhIDs[iph][irh]   = int(rhID);
	  phrhOOTs[iph][irh]  = int(recHit->checkFlag(EcalRecHit::kOutOfTime));
	  
	  // save the position in the vector of the seed 
	  if (seedDetId.rawId() == recHitId) { phseedpos[iph] = iph; }

	  irh++; // increment rechit counter
	} // end loop over rec hit id map
      } // end check over super cluster
      iph++; // increment photon counter
    } // end loop over photon vector
    delete clustertools; // delete cluster tools once done with loop over photons
  } // end check over photon handle valid

  ///////////////
  //           //
  // Fill Tree //
  //           //
  ///////////////
  tree->Fill();      
}    

void PhotonDump::ClearJetBranches()
{
  jetE.clear();
  jetpt.clear();
  jetphi.clear();
  jeteta.clear();
}

void PhotonDump::InitializeJetBranches()
{
  jetE.resize(njets);
  jetpt.resize(njets);
  jetphi.resize(njets);
  jeteta.resize(njets);

  for (int ijet = 0; ijet < njets; ijet++)
  {
    jetE  [ijet] = -9999.f;
    jetpt [ijet] = -9999.f;
    jetphi[ijet] = -9999.f;
    jeteta[ijet] = -9999.f;
  }
}

void PhotonDump::ClearRecoPhotonBranches()
{
  phE.clear();
  phpt.clear();
  phphi.clear(); 
  pheta.clear(); 
  
  phscX.clear(); 
  phscY.clear(); 
  phscZ.clear(); 
  phscE.clear(); 

  phnrhs.clear();

  phrhXs.clear(); 
  phrhYs.clear(); 
  phrhZs.clear(); 
  phrhEs.clear(); 
  phrhtimes.clear();
  phrhIDs.clear();
  phrhOOTs.clear();

  phseedpos.clear();
}

void PhotonDump::InitializeRecoPhotonBranches()
{
  phVID.resize(nphotons);

  phE.resize(nphotons);
  phpt.resize(nphotons);
  phphi.resize(nphotons);
  pheta.resize(nphotons);

  phscX.resize(nphotons);
  phscY.resize(nphotons);
  phscZ.resize(nphotons);
  phscE.resize(nphotons);

  phnrhs.resize(nphotons);

  phrhXs.resize(nphotons);
  phrhYs.resize(nphotons);
  phrhZs.resize(nphotons);
  phrhEs.resize(nphotons);
  phrhtimes.resize(nphotons);
  phrhIDs.resize(nphotons);
  phrhOOTs.resize(nphotons);

  phseedpos.resize(nphotons);

  for (int iph = 0; iph < nphotons; iph++)
  {
    phVID[iph] = 0;

    phE  [iph] = -9999.f; 
    phpt [iph] = -9999.f; 
    phphi[iph] = -9999.f; 
    pheta[iph] = -9999.f; 

    phscX[iph] = -9999.f; 
    phscY[iph] = -9999.f; 
    phscZ[iph] = -9999.f; 
    phscE[iph] = -9999.f; 

    phnrhs[iph] = -9999;

    phseedpos[iph] = -9999;
  }
}

void PhotonDump::InitializeRecoRecHitBranches(int iph)
{
  phrhXs[iph].resize(phnrhs[iph]);
  phrhYs[iph].resize(phnrhs[iph]);
  phrhZs[iph].resize(phnrhs[iph]);
  phrhEs[iph].resize(phnrhs[iph]);
  phrhtimes[iph].resize(phnrhs[iph]);
  phrhIDs[iph].resize(phnrhs[iph]);
  phrhOOTs[iph].resize(phnrhs[iph]);

  for (int irh = 0; irh < phnrhs[iph]; irh++)
  {
    phrhXs   [iph][irh] = -9999.f;
    phrhYs   [iph][irh] = -9999.f;
    phrhZs   [iph][irh] = -9999.f;
    phrhEs   [iph][irh] = -9999.f;
    phrhtimes[iph][irh] = -9999.f;
    phrhIDs  [iph][irh] = -9999;
    phrhOOTs [iph][irh] = -9999;
  }
}

void PhotonDump::beginJob() 
{
  edm::Service<TFileService> fs;
  tree = fs->make<TTree>("tree"     , "tree");

  // Run, Lumi, Event info
  tree->Branch("event"                , &event                , "event/I");
  tree->Branch("run"                  , &run                  , "run/I");
  tree->Branch("lumi"                 , &lumi                 , "lumi/I");
  
  // MC info
  tree->Branch("genwgt"               , &genwgt               , "genwgt/F");
  tree->Branch("genpuobs"             , &genpuobs             , "genpuobs/I");
  tree->Branch("genputrue"            , &genputrue            , "genputrue/I");

  // Vertex info
  tree->Branch("nvtx"                 , &nvtx                 , "nvtx/I");
  tree->Branch("vtxX"                 , &vtxX                 , "vtxX/F");
  tree->Branch("vtxY"                 , &vtxY                 , "vtxY/F");
  tree->Branch("vtxZ"                 , &vtxZ                 , "vtxZ/F");

  // MET info
  tree->Branch("t1pfmet"              , &t1pfmet              , "t1pfmet/F");
  tree->Branch("t1pfmetphi"           , &t1pfmetphi           , "t1pfmetphi/F");
  tree->Branch("t1pfmeteta"           , &t1pfmeteta           , "t1pfmeteta/F");
  tree->Branch("t1pfmetsumEt"         , &t1pfmetsumEt         , "t1pfmetsumEt/F");
  
  // Jet Info
  tree->Branch("njets"                , &njets                , "njets/I");
  tree->Branch("jetE"                 , &jetE);
  tree->Branch("jetpt"                , &jetpt);
  tree->Branch("jetphi"               , &jetphi);
  tree->Branch("jeteta"               , &jeteta);
  
  // Photon Info
  tree->Branch("nphotons"             , &nphotons             , "nhotons/I");
  tree->Branch("phVID"                , &phVID);
  tree->Branch("phE"                  , &phE);
  tree->Branch("phpt"                 , &phpt);
  tree->Branch("phphi"                , &phphi);
  tree->Branch("pheta"                , &pheta);

  tree->Branch("phscX"                , &phscX);
  tree->Branch("phscY"                , &phscY);
  tree->Branch("phscZ"                , &phscZ);
  tree->Branch("phscE"                , &phscE);
  tree->Branch("phnrhs"               , &phnrhs);

  tree->Branch("phseedpos"            , &phseedpos);

  tree->Branch("phrhXs"               , &phrhXs);
  tree->Branch("phrhYs"               , &phrhYs);
  tree->Branch("phrhZs"               , &phrhZs);
  tree->Branch("phrhEs"               , &phrhEs);
  tree->Branch("phrhtimes"            , &phrhtimes);
  tree->Branch("phrhIDs"              , &phrhIDs);
  tree->Branch("phrhOOTs"             , &phrhOOTs);
}

void PhotonDump::endJob() {}

void PhotonDump::beginRun(edm::Run const& iRun, edm::EventSetup const& iSetup) {}

void PhotonDump::endRun(edm::Run const&, edm::EventSetup const&) {}

void PhotonDump::fillDescriptions(edm::ConfigurationDescriptions& descriptions) 
{
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

DEFINE_FWK_MODULE(PhotonDump);
