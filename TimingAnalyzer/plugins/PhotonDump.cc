#include "PhotonDump.h"

PhotonDump::PhotonDump(const edm::ParameterSet& iConfig): 
  // vertexes
  verticesTag(iConfig.getParameter<edm::InputTag>("vertices")),

  // jets
  jetsTag(iConfig.getParameter<edm::InputTag>("jets")),  

  // photons
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

  // jets
  jetsToken = consumes<std::vector<pat::Jet> > (jetsTag);

  // photons
  photonsToken = consumes<std::vector<pat::Photon> > (photonsTag);

  // only for simulated samples
  if (isMC)
  {
    pileupInfoToken = consumes<std::vector<PileupSummaryInfo> > (iConfig.getParameter<edm::InputTag>("pileup"));
    genevtInfoToken = consumes<GenEventInfoProduct> (iConfig.getParameter<edm::InputTag>("genevt"));
    gensToken       = consumes<edm::View<reco::GenParticle> > (iConfig.getParameter<edm::InputTag>("gens"));   
  }
}

PhotonDump::~PhotonDump() {}

void PhotonDump::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) 
{
  // VERTEX
  edm::Handle<std::vector<reco::Vertex> > verticesH;
  iEvent.getByToken(verticesToken, verticesH);
  std::vector<reco::Vertex> vertices = *verticesH;

  // JETS
  edm::Handle<std::vector<pat::Jet> > jetsH;
  iEvent.getByToken(jetsToken, jetsH);
  std::vector<pat::Jet> jets = *jetsH;

  // PHOTONS
  edm::Handle<std::vector<pat::Photon> > photonsH;
  iEvent.getByToken(photonsToken, photonsH);
  std::vector<pat::Photon> photons = *photonsH;

  // geometry (from ECAL ELF)
  edm::ESHandle<CaloGeometry> geoHandle;
  iSetup.get<CaloGeometryRecord>().get(geoHandle);
  const CaloSubdetectorGeometry *barrelGeometry = geoHandle->getSubdetectorGeometry(DetId::Ecal, EcalBarrel);
  const CaloSubdetectorGeometry *endcapGeometry = geoHandle->getSubdetectorGeometry(DetId::Ecal, EcalEndcap);

  // Event, lumi, run info
  event = iEvent.id().event();
  run   = iEvent.id().run();
  lumi  = iEvent.luminosityBlock();

  // Vertex info
  nvtx = -9999; 
  vtxX = -9999.f; vtxY = -9999.f; vtxZ = -9999.f;
  if (verticesH.isValid()) 
  {
    nvtx = vertices.size();
    vtxX = vertices[0].position().x();
    vtxY = vertices[0].position().y();
    vtxZ = vertices[0].position().z();
  }

  // Jets
  njets = -9999;
  PhotonDump::ClearJetBranches();
  if (jetsH.isValid()) // check to make sure it is valid
  {
    std::sort(jets.begin(),jets.end(),sortJetsByPt); // sort jets
    njets = jets.size();
    if (njets > 0) PhotonDump::InitializeJetBranches();
    int ijet = 0;
    for (std::vector<pat::Jet>::const_iterator jetiter = jets.begin(); jetiter != jets.end(); ++jetiter)
    {
      jetE  [ijet] = jetiter->energy();
      jetpt [ijet] = jetiter->pt();
      jetphi[ijet] = jetiter->phi();
      jeteta[ijet] = jetiter->eta();

      ijet++;
    }      
  }
  
  // photons
  nphotons = -9999;
  PhotonDump::ClearRecoPhotonBranches();
  if (photonsH.isValid()) // standard handle check
  {
    // ECALELF tools
    EcalClusterLazyTools * clustertools = new EcalClusterLazyTools (iEvent, iSetup, recHitCollectionEBTAG, recHitCollectionEETAG);

    std::sort(photons.begin(),photons.end(),sortPhotonsByPt);
    nphotons = photons.size();
    if (nphotons > 0) PhotonDump::InitializeRecoPhotonBranches();

    int iph = 0;
    for (std::vector<pat::Photon>::const_iterator phiter = photons.begin(); phiter != photons.end(); ++phiter) // loop over photon vector
    {
      // initialize photon branches!
      PhotonDump::InitializeRecoPhotonBranches();

      // photon branches
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
	  if (seedDetId.rawId() == recHitId) { phseedpos[iph] = phrhIDs.size() - 1; }

	  irh++; // increment rechit counter
	} // end loop over rec hit id map
      } // end check over super cluster
      iph++; // increment photon counter
    } // end loop over photon vector
    delete clustertools; // delete cluster tools once done with loop over photons
  } // end check over photon handle valid

  evtree->Fill();      
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
  evtree = fs->make<TTree>("evtree"     , "tree");

  // Run, Lumi, Event info
  evtree->Branch("event"                , &event                , "event/I");
  evtree->Branch("run"                  , &run                  , "run/I");
  evtree->Branch("lumi"                 , &lumi                 , "lumi/I");
  
  // Vertex info
  evtree->Branch("nvtx"                 , &nvtx                 , "nvtx/I");
  evtree->Branch("vtxX"                 , &vtxX                 , "vtxX/F");
  evtree->Branch("vtxY"                 , &vtxY                 , "vtxY/F");
  evtree->Branch("vtxZ"                 , &vtxZ                 , "vtxZ/F");
  
  // Jet Info
  evtree->Branch("njets"                , &njets                , "njets/I");
  evtree->Branch("jetE"                 , &jetE);
  evtree->Branch("jetpt"                , &jetpt);
  evtree->Branch("jetphi"               , &jetphi);
  evtree->Branch("jeteta"               , &jeteta);
  
  // Photon Info
  evtree->Branch("nphotons"             , &nphotons             , "nhotons/I");
  evtree->Branch("phE"                  , &phE);
  evtree->Branch("phpt"                 , &phpt);
  evtree->Branch("phphi"                , &phphi);
  evtree->Branch("pheta"                , &pheta);
  evtree->Branch("phscX"                , &phscX);
  evtree->Branch("phscY"                , &phscY);
  evtree->Branch("phscZ"                , &phscZ);
  evtree->Branch("phscE"                , &phscE);
  evtree->Branch("phnrhs"               , &phnrhs);

  evtree->Branch("phrhXs"               , &phrhXs);
  evtree->Branch("phrhYs"               , &phrhYs);
  evtree->Branch("phrhZs"               , &phrhZs);
  evtree->Branch("phrhEs"               , &phrhEs);

  evtree->Branch("phrhtimes"            , &phrhtimes);
  evtree->Branch("phrhIDs"              , &phrhIDs);
  evtree->Branch("phrhOOTs"             , &phrhOOTs);
  evtree->Branch("phseedpos"            , &phseedpos);
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
