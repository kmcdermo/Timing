#include "RECORecHits.h"

RECORecHits::RECORecHits(const edm::ParameterSet& iConfig): 
  //recHits
  recHitsEBTag(iConfig.getParameter<edm::InputTag>("recHitsEB")),  
  recHitsEETag(iConfig.getParameter<edm::InputTag>("recHitsEE"))
{
  usesResource();
  usesResource("TFileService");

  // rechits
  recHitsEBToken = consumes<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > (recHitsEBTag);
  recHitsEEToken = consumes<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > (recHitsEETag);
}

RECORecHits::~RECORecHits() {}

void RECORecHits::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) 
{
  // RecHits
  edm::Handle<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > recHitsEBH;
  iEvent.getByToken(recHitsEBToken, recHitsEBH);
  const EcalRecHitCollection * recHitsEB = recHitsEBH.product();

  edm::Handle<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > recHitsEEH;
  iEvent.getByToken(recHitsEEToken, recHitsEEH);
  const EcalRecHitCollection * recHitsEE = recHitsEEH.product();
  
  // geometry (from ECAL ELF)
  edm::ESHandle<CaloGeometry> geoHandle;
  iSetup.get<CaloGeometryRecord>().get(geoHandle);
  const CaloSubdetectorGeometry * barrelGeometry = geoHandle->getSubdetectorGeometry(DetId::Ecal, EcalBarrel);
  const CaloSubdetectorGeometry * endcapGeometry = geoHandle->getSubdetectorGeometry(DetId::Ecal, EcalEndcap);

  // Event, lumi, run info
  event = iEvent.id().event();
  lumi  = iEvent.luminosityBlock();
  run   = iEvent.id().run();

  RECORecHits::ClearBranches();

  /////////////////////
  //                 //
  // Full recHits EB //
  //                 //
  /////////////////////
  nrhEB = recHitsEB->size();
  for (EcalRecHitCollection::const_iterator recHit = recHitsEB->begin(); recHit != recHitsEB->end(); ++recHit)
  {
    const DetId recHitId = recHit->detid();
    const auto recHitPos = barrelGeometry->getGeometry(recHitId)->getPosition();
    rhE   .push_back(recHit->energy());
    rhphi .push_back(recHitPos.phi());
    rheta .push_back(recHitPos.eta());
    rhtime.push_back(recHit->time());
    rhID  .push_back(int(recHitId.rawId()));
    rhOOT .push_back(int(recHit->checkFlag(EcalRecHit::kOutOfTime)));
    rhisEB.push_back(1);
  }

  /////////////////////
  //                 //
  // Full recHits EE //
  //                 //
  /////////////////////
  nrhEE = recHitsEE->size();
  for (EcalRecHitCollection::const_iterator recHit = recHitsEE->begin(); recHit != recHitsEE->end(); ++recHit)
  {
    const DetId recHitId = recHit->detid();
    const auto recHitPos = endcapGeometry->getGeometry(recHitId)->getPosition();
    rhE   .push_back(recHit->energy());
    rhphi .push_back(recHitPos.phi());
    rheta .push_back(recHitPos.eta());
    rhtime.push_back(recHit->time());
    rhID  .push_back(int(recHitId.rawId()));
    rhOOT .push_back(int(recHit->checkFlag(EcalRecHit::kOutOfTime)));
    rhisEB.push_back(0);
  }

  // fill the tree
  tree->Fill();
}

void RECORecHits::ClearBranches()
{ 
  rhE   .clear();
  rhphi .clear();
  rheta .clear();
  rhtime.clear();
  rhID  .clear();
  rhOOT .clear();	  
  rhisEB.clear();	  
}

void RECORecHits::beginJob() 
{
  edm::Service<TFileService> fs;

  // tree
  tree = fs->make<TTree>("tree", "tree");

  // event level info
  tree->Branch("event"             , &event              , "event/l");
  tree->Branch("lumi"              , &lumi               , "lumi/i");
  tree->Branch("run"               , &run                , "run/i");

  tree->Branch("nrhEB"             , &nrhEB              , "nrhEB/I");
  tree->Branch("nrhEE"             , &nrhEE              , "nrhEE/I");
  tree->Branch("rhE"               , &rhE);
  tree->Branch("rhphi"             , &rhphi);
  tree->Branch("rheta"             , &rheta);
  tree->Branch("rhtime"            , &rhtime);
  tree->Branch("rhID"              , &rhID);
  tree->Branch("rhOOT"             , &rhOOT);
  tree->Branch("rhisEB"            , &rhisEB);
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
