#include "RECORecHits.h"

RECORecHits::RECORecHits(const edm::ParameterSet& iConfig): 
  //recHits
  recHitsFullEBTAG(consumes<EcalRecHitCollection>(iConfig.getParameter<edm::InputTag>( "recHitsFullEB" ))),
  recHitsFullEETAG(consumes<EcalRecHitCollection>(iConfig.getParameter<edm::InputTag>( "recHitsFullEE" )))
{
  usesResource();
  usesResource("TFileService");
}

RECORecHits::~RECORecHits() {}

void RECORecHits::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) 
{
  // ECALELF tools
  EcalClusterLazyTools * clustertools = new EcalClusterLazyTools (iEvent, iSetup, recHitsFullEBTAG, recHitsFullEETAG);
  
  // geometry (from ECAL ELF)
  edm::ESHandle<CaloGeometry> geoHandle;
  iSetup.get<CaloGeometryRecord>().get(geoHandle);
  const CaloSubdetectorGeometry * barrelGeometry = geoHandle->getSubdetectorGeometry(DetId::Ecal, EcalBarrel);
  const CaloSubdetectorGeometry * endcapGeometry = geoHandle->getSubdetectorGeometry(DetId::Ecal, EcalEndcap);

  // Event, lumi, run info
  run   = iEvent.id().run();
  lumi  = iEvent.luminosityBlock();
  event = iEvent.id().event();

  RECORecHits::InitializeBranches();

  /////////////////////
  //                 //
  // Full recHits EB //
  //                 //
  /////////////////////
  const EcalRecHitCollection * recHitsEB = clustertools->getEcalEBRecHitCollection(); 
  nrhEB = recHitsEB->size();
  for (EcalRecHitCollection::const_iterator recHit = recHitsEB->begin(); recHit != recHitsEB->end(); ++recHit)
  {
    const DetId recHitId = recHit->detid();
    const auto recHitPos = barrelGeometry->getGeometry(recHitId)->getPosition();
    rhEs   .push_back(recHit->energy());
    rhphis .push_back(recHitPos.phi());
    rhetas .push_back(recHitPos.eta());
    rhtimes.push_back(recHit->time());
    rhIDs  .push_back(int(recHitId.rawId()));
    rhOOTs .push_back(int(recHit->checkFlag(EcalRecHit::kOutOfTime)));
  }

  /////////////////////
  //                 //
  // Full recHits EE //
  //                 //
  /////////////////////
  const EcalRecHitCollection * recHitsEE = clustertools->getEcalEERecHitCollection(); 
  nrhEE = recHitsEE->size();
  for (EcalRecHitCollection::const_iterator recHit = recHitsEE->begin(); recHit != recHitsEE->end(); ++recHit)
  {
    const DetId recHitId = recHit->detid();
    const auto recHitPos = endcapGeometry->getGeometry(recHitId)->getPosition();
    rhEs   .push_back(recHit->energy());
    rhphis .push_back(recHitPos.phi());
    rhetas .push_back(recHitPos.eta());
    rhtimes.push_back(recHit->time());
    rhIDs  .push_back(int(recHitId.rawId()));
    rhOOTs .push_back(int(recHit->checkFlag(EcalRecHit::kOutOfTime)));
  }

  // fill the tree
  rhtree->Fill();

  // delete cluster tools
  delete clustertools;
}

void RECORecHits::InitializeBranches()
{ 
  rhEs   .clear();
  rhphis .clear();
  rhetas .clear();
  rhtimes.clear();
  rhIDs  .clear();
  rhOOTs .clear();	  
}

void RECORecHits::beginJob() 
{
  edm::Service<TFileService> fs;

  // rh tree
  rhtree = fs->make<TTree>("rhtree", "tree");

  // event level info
  rhtree->Branch("event"             , &event              , "event/I");
  rhtree->Branch("run"               , &run                , "run/I");
  rhtree->Branch("lumi"              , &lumi               , "lumi/I");

  rhtree->Branch("nrhEB"             , &nrhEB              , "nrhEB/I");
  rhtree->Branch("nrhEE"             , &nrhEE              , "nrhEE/I");
  rhtree->Branch("rhEs"              , &rhEs);
  rhtree->Branch("rhphis"            , &rhphis);
  rhtree->Branch("rhetas"            , &rhetas);
  rhtree->Branch("rhtimes"           , &rhtimes);
  rhtree->Branch("rhIDs"             , &rhIDs);
  rhtree->Branch("rhOOTs"            , &rhOOTs);
}

void RECORecHits::endJob() {}

void RECORecHits::beginRun(edm::Run const& iRun, edm::EventSetup const& iSetup) {}

void RECORecHits::endRun(edm::Run const&, edm::EventSetup const&) {}

void RECORecHits::fillDescriptions(edm::ConfigurationDescriptions& descriptions) 
{
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

DEFINE_FWK_MODULE(RECORecHits);
