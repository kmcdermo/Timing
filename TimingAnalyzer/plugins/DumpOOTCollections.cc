#include "DumpOOTCollections.h"

DumpOOTCollections::DumpOOTCollections(const edm::ParameterSet& iConfig): 
  // photons
  photonsTag(iConfig.getParameter<edm::InputTag>("photons")),  

  // photoncores
  photonCoresTag(iConfig.getParameter<edm::InputTag>("photonCores")),  

  // SCs
  superClustersEBTag(iConfig.getParameter<edm::InputTag>("superClustersEB")),  
  superClustersEETag(iConfig.getParameter<edm::InputTag>("superClustersEE")),  

  // Basic Clusters
  clustersEBTag(iConfig.getParameter<edm::InputTag>("clustersEB")),    
  clustersEETag(iConfig.getParameter<edm::InputTag>("clustersEE")),    
  clustersESTag(iConfig.getParameter<edm::InputTag>("clustersES")),    
  
  // recHits
  recHitsEBTag(iConfig.getParameter<edm::InputTag>("recHitsEB")),  
  recHitsEETag(iConfig.getParameter<edm::InputTag>("recHitsEE"))
{
  usesResource();
  usesResource("TFileService");

  // photons
  photonsToken = consumes<std::vector<reco::Photon> > (photonsTag);

  // photoncores
  photonCoresToken = consumes<std::vector<reco::PhotonCore> > (photonCoresTag);

  // SCs
  superClustersEBToken = consumes<std::vector<reco::SuperCluster> > (superClustersEBTag);
  superClustersEEToken = consumes<std::vector<reco::SuperCluster> > (superClustersEETag);

  // clusters
  clustersEBToken = consumes<std::vector<reco::CaloCluster> > (clustersEBTag);
  clustersEEToken = consumes<std::vector<reco::CaloCluster> > (clustersEETag);
  clustersESToken = consumes<std::vector<reco::CaloCluster> > (clustersESTag);

  // rechits
  recHitsEBToken = consumes<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > (recHitsEBTag);
  recHitsEEToken = consumes<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > (recHitsEETag);
}

DumpOOTCollections::~DumpOOTCollections() {}

void DumpOOTCollections::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) 
{
  // Photons
  edm::Handle<std::vector<reco::Photon> > photonsH;
  iEvent.getByToken(photonsToken, photonsH);

  // PhotonCores
  edm::Handle<std::vector<reco::PhotonCore> > photonCoresH;
  iEvent.getByToken(photonCoresToken, photonCoresH);

  // SuperClusters
  edm::Handle<std::vector<reco::SuperCluster> > superClustersEBH;
  iEvent.getByToken(superClustersEBToken, superClustersEBH);
  edm::Handle<std::vector<reco::SuperCluster> > superClustersEEH;
  iEvent.getByToken(superClustersEEToken, superClustersEEH);

  // CaloClusters
  edm::Handle<std::vector<reco::CaloCluster> > clustersEBH;
  iEvent.getByToken(clustersEBToken, clustersEBH);
  edm::Handle<std::vector<reco::CaloCluster> > clustersEEH;
  iEvent.getByToken(clustersEEToken, clustersEEH);
  edm::Handle<std::vector<reco::CaloCluster> > clustersESH;
  iEvent.getByToken(clustersESToken, clustersESH);

  // RecHits
  edm::Handle<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > recHitsEBH;
  iEvent.getByToken(recHitsEBToken, recHitsEBH);
  edm::Handle<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > recHitsEEH;
  iEvent.getByToken(recHitsEEToken, recHitsEEH);

  // Event, lumi, run info 
  run   = iEvent.id().run();
  lumi  = iEvent.luminosityBlock();
  event = iEvent.id().event();

  // nobj variables
  nPhotons = 0;
  nPhotonCores = 0;
  nSuperClustersEB = 0;
  nSuperClustersEE = 0;
  nClustersEB = 0;
  nClustersEE = 0;
  nClustersES = 0;

  // Reco Photons 
  nPhClusters.clear();
  nPhRecHits.clear();
  if (photonsH.isValid()) 
  {
    for (std::vector<reco::Photon>::const_iterator phiter = photonsH->begin(); phiter != photonsH->end(); ++phiter) // loop over photon vector
    {
      // super cluster from photon
      const reco::SuperClusterRef& phsc = phiter->superCluster();
      nPhClusters.push_back(phsc->clustersSize());
      
      // use seed to get geometry and recHits
      const DetId seedDetId = phsc->seed()->seed(); //seed detid
      const bool isEB = (seedDetId.subdetId() == EcalBarrel); //which subdet
      const EcalRecHitCollection * recHits = (isEB ? recHitsEBH : recHitsEEH).product();
      
      // all rechits in superclusters
      const DetIdPairVec hitsAndFractions = phsc->hitsAndFractions();
      nPhRecHits.push_back(DumpOOTCollections::GetNRecHits(hitsAndFractions,recHits));
    } 
  }

  if (photonsH.isValid()) nPhotons = photonsH->size();
  if (photonCoresH.isValid()) nPhotonCores = photonCoresH->size();
  if (superClustersEBH.isValid()) nSuperClustersEB = superClustersEBH->size();
  if (superClustersEEH.isValid()) nSuperClustersEE = superClustersEEH->size();
  if (clustersEBH.isValid()) nClustersEB = clustersEBH->size();
  if (clustersEEH.isValid()) nClustersEE = clustersEEH->size();
  if (clustersESH.isValid()) nClustersES = clustersESH->size();

  tree->Fill();
}    

int DumpOOTCollections::GetNRecHits(const DetIdPairVec & hitsAndFractions, const EcalRecHitCollection *& recHits)
{
  uiiumap rhIDmap;
  for (DetIdPairVec::const_iterator hafitr = hitsAndFractions.begin(); hafitr != hitsAndFractions.end(); ++hafitr) // loop over all rec hits in SC
  {
    const DetId recHitId = hafitr->first; // get detid of crystal
    EcalRecHitCollection::const_iterator recHit = recHits->find(recHitId); // get the underlying rechit
    if (recHit != recHits->end()) // standard check
    { 
      rhIDmap[recHitId.rawId()]++;
    } // end standard check recHit
  } // end loop over hits and fractions

  return rhIDmap.size();
}

void DumpOOTCollections::beginJob() 
{
  edm::Service<TFileService> fs;
  tree = fs->make<TTree>("tree"     , "tree");

  // Run, Lumi, Event info
  tree->Branch("event"                  , &event                , "event/l");
  tree->Branch("run"                    , &run                  , "run/i");
  tree->Branch("lumi"                   , &lumi                 , "lumi/i");
   
  // Event-level object counts
  tree->Branch("nPhotons"             , &nPhotons             , "nPhotons/I");
  tree->Branch("nPhotonCores"         , &nPhotonCores         , "nPhotonCores/I");
  tree->Branch("nSuperClustersEB"     , &nSuperClustersEB     , "nSuperClustersEB/I");
  tree->Branch("nSuperClustersEE"     , &nSuperClustersEE     , "nSuperClustersEE/I");
  tree->Branch("nClustersEB"          , &nClustersEB          , "nClustersEB/I");
  tree->Branch("nClustersEE"          , &nClustersEE          , "nClustersEE/I");
  tree->Branch("nClustersES"          , &nClustersES          , "nClustersES/I");

  tree->Branch("nPhClusters"          , &nPhClusters);
  tree->Branch("nPhRecHits"           , &nPhRecHits);
}

void DumpOOTCollections::endJob() {}

void DumpOOTCollections::beginRun(edm::Run const& iRun, edm::EventSetup const& iSetup) {}

void DumpOOTCollections::endRun(edm::Run const&, edm::EventSetup const&) {}

void DumpOOTCollections::fillDescriptions(edm::ConfigurationDescriptions& descriptions) 
{
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

DEFINE_FWK_MODULE(DumpOOTCollections);
