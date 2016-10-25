#include "OOTRecHits.h"

OOTRecHits::OOTRecHits(const edm::ParameterSet& iConfig): 
  // rec hit energy cut
  applyrhEcut(iConfig.existsAs<bool>("applyrhEcut") ? iConfig.getParameter<bool>("applyrhEcut") : false),
  rhEcut     (iConfig.existsAs<double>("rhEcut")    ? iConfig.getParameter<double>("rhEcut")    : 1.0),

  photonsTag(iConfig.getParameter<edm::InputTag>("photons")),

  //recHits
  recHitsReducedEBTAG(consumes<EcalRecHitCollection>(iConfig.getParameter<edm::InputTag>( "recHitsReducedEB" ))),
  recHitsReducedEETAG(consumes<EcalRecHitCollection>(iConfig.getParameter<edm::InputTag>( "recHitsReducedEE" ))),
  recHitsFullEBTAG(consumes<EcalRecHitCollection>(iConfig.getParameter<edm::InputTag>( "recHitsFullEB" ))),
  recHitsFullEETAG(consumes<EcalRecHitCollection>(iConfig.getParameter<edm::InputTag>( "recHitsFullEE" )))
{
  usesResource();
  usesResource("TFileService");

  photonsToken = consumes<std::vector<reco::Photon> > (photonsTag);
}

OOTRecHits::~OOTRecHits() {}

void OOTRecHits::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) 
{
  edm::Handle<std::vector<reco::Photon> > photonsH;
  iEvent.getByToken(photonsToken, photonsH);
  std::vector<reco::Photon> photons = *photonsH;
  
  // geometry (from ECAL ELF)
  edm::ESHandle<CaloGeometry> geoHandle;
  iSetup.get<CaloGeometryRecord>().get(geoHandle);
  const CaloSubdetectorGeometry *barrelGeometry = geoHandle->getSubdetectorGeometry(DetId::Ecal, EcalBarrel);
  const CaloSubdetectorGeometry *endcapGeometry = geoHandle->getSubdetectorGeometry(DetId::Ecal, EcalEndcap);

  // Event, lumi, run info
  run   = iEvent.id().run();
  lumi  = iEvent.luminosityBlock();
  event = iEvent.id().event();

  // ECALELF tools
  EcalClusterLazyTools * reducedtools = new EcalClusterLazyTools (iEvent, iSetup, recHitsReducedEBTAG, recHitsReducedEETAG);
  EcalClusterLazyTools * fulltools    = new EcalClusterLazyTools (iEvent, iSetup, recHitsFullEBTAG   , recHitsFullEETAG   );

  // Do all the counting!
  OOTRecHits::ReducedToFullEB(reducedtools,fulltools);
  OOTRecHits::ReducedToFullEE(reducedtools,fulltools);

  OOTRecHits::FullToReducedEB(fulltools,reducedtools);
  OOTRecHits::FullToReducedEE(fulltools,reducedtools);
  // fill the counting tree
  countingtree->Fill();    

  delete reducedtools;
  delete fulltools;
}

void OOTRecHits::ReducedToFullEB(EcalClusterLazyTools *& reducedtools, EcalClusterLazyTools *& fulltools)
{
  nReducedEB = 0; nR2FMatchedEB = 0; nReducedOOTEB = 0; nR2FMatchedOOTEB = 0;

  const EcalRecHitCollection *reducedRecHitsEB = reducedtools->getEcalEBRecHitCollection();
  for (EcalRecHitCollection::const_iterator irecHit = reducedRecHitsEB->begin(); irecHit != reducedRecHitsEB->end(); ++irecHit)
  {
    if (applyrhEcut && irecHit->energy() < rhEcut) continue;

    bool matched = false;

    const EcalRecHitCollection *fullRecHitsEB = fulltools->getEcalEBRecHitCollection();
    for (EcalRecHitCollection::const_iterator jrecHit = fullRecHitsEB->begin(); jrecHit != fullRecHitsEB->end(); ++jrecHit)
    {
      if (irecHit->detid().rawId() == jrecHit->detid().rawId())
      { 
  	matched = true;
  	break;
      }
    }
      
    if (matched) 
    {
      nR2FMatchedEB++;
      if (irecHit->checkFlag(EcalRecHit::kOutOfTime) == 1) 
      {
	nReducedOOTEB++; 
	nR2FMatchedOOTEB++;
      }
    }
    else 
    {
      if (irecHit->checkFlag(EcalRecHit::kOutOfTime) == 1) 
      {
	nReducedOOTEB++;
      }
    }
    nReducedEB++;
  }
}

void OOTRecHits::ReducedToFullEE(EcalClusterLazyTools *& reducedtools, EcalClusterLazyTools *& fulltools)
{
  nReducedEE = 0; nR2FMatchedEE = 0; nReducedOOTEE = 0; nR2FMatchedOOTEE = 0;
  
  const EcalRecHitCollection *reducedRecHitsEE = reducedtools->getEcalEERecHitCollection();
  for (EcalRecHitCollection::const_iterator irecHit = reducedRecHitsEE->begin(); irecHit != reducedRecHitsEE->end(); ++irecHit)
  {
    if (applyrhEcut && irecHit->energy() < rhEcut) continue;

    bool matched = false;

    const EcalRecHitCollection *fullRecHitsEE = fulltools->getEcalEERecHitCollection();
    for (EcalRecHitCollection::const_iterator jrecHit = fullRecHitsEE->begin(); jrecHit != fullRecHitsEE->end(); ++jrecHit)
    {
      if (irecHit->detid().rawId() == jrecHit->detid().rawId())
      { 
  	matched = true;
  	break;
      }
    }
      
    if (matched) 
    {
      nR2FMatchedEE++;
      if (irecHit->checkFlag(EcalRecHit::kOutOfTime) == 1) 
      {
	nReducedOOTEE++; 
	nR2FMatchedOOTEE++;
      }
    }
    else 
    {
      if (irecHit->checkFlag(EcalRecHit::kOutOfTime) == 1) 
      {
	nReducedOOTEE++;
      }
    }
    nReducedEE++;
  }
}

void OOTRecHits::FullToReducedEB(EcalClusterLazyTools *& fulltools, EcalClusterLazyTools *& reducedtools)
{
  nFullEB = 0; nF2RMatchedEB = 0; nFullOOTEB = 0; nF2RMatchedOOTEB = 0;

  const EcalRecHitCollection *fullRecHitsEB = fulltools->getEcalEBRecHitCollection();
  for (EcalRecHitCollection::const_iterator irecHit = fullRecHitsEB->begin(); irecHit != fullRecHitsEB->end(); ++irecHit)
  {
    if (applyrhEcut && irecHit->energy() < rhEcut) continue;

    bool matched = false;

    const EcalRecHitCollection *reducedRecHitsEB = reducedtools->getEcalEBRecHitCollection();
    for (EcalRecHitCollection::const_iterator jrecHit = reducedRecHitsEB->begin(); jrecHit != reducedRecHitsEB->end(); ++jrecHit)
    {
      if (irecHit->detid().rawId() == jrecHit->detid().rawId())
      { 
  	matched = true;
  	break;
      }
    }
      
    if (matched) 
    {
      nF2RMatchedEB++;
      if (irecHit->checkFlag(EcalRecHit::kOutOfTime) == 1) 
      {
	nFullOOTEB++; 
	nF2RMatchedOOTEB++;
      }
    }
    else 
    {
      if (irecHit->checkFlag(EcalRecHit::kOutOfTime) == 1) 
      {
	nFullOOTEB++;
      }
    }
    nFullEB++;
  }
}

void OOTRecHits::FullToReducedEE(EcalClusterLazyTools *& fulltools, EcalClusterLazyTools *& reducedtools)
{
  nFullEE = 0; nF2RMatchedEE = 0; nFullOOTEE = 0; nF2RMatchedOOTEE = 0;

  const EcalRecHitCollection *fullRecHitsEE = fulltools->getEcalEERecHitCollection();
  for (EcalRecHitCollection::const_iterator irecHit = fullRecHitsEE->begin(); irecHit != fullRecHitsEE->end(); ++irecHit)
  {
    if (applyrhEcut && irecHit->energy() < rhEcut) continue;

    bool matched = false;

    const EcalRecHitCollection *reducedRecHitsEE = reducedtools->getEcalEERecHitCollection();
    for (EcalRecHitCollection::const_iterator jrecHit = reducedRecHitsEE->begin(); jrecHit != reducedRecHitsEE->end(); ++jrecHit)
    {
      if (irecHit->detid().rawId() == jrecHit->detid().rawId())
      { 
  	matched = true;
  	break;
      }
    }
      
    if (matched) 
    {
      nF2RMatchedEE++;
      if (irecHit->checkFlag(EcalRecHit::kOutOfTime) == 1) 
      {
	nFullOOTEE++; 
	nF2RMatchedOOTEE++;
      }
    }
    else 
    {
      if (irecHit->checkFlag(EcalRecHit::kOutOfTime) == 1) 
      {
	nFullOOTEE++;
      }
    }
    nFullEE++;
  }
}

void OOTRecHits::beginJob() 
{
  edm::Service<TFileService> fs;
  countingtree = fs->make<TTree>("countingtree", "tree");

  // Run, Lumi, Event info
  countingtree->Branch("event"                , &event                , "event/I");
  countingtree->Branch("run"                  , &run                  , "run/I");
  countingtree->Branch("lumi"                 , &lumi                 , "lumi/I");

  countingtree->Branch("nReducedEB"           , &nReducedEB           , "nReducedEB/I");
  countingtree->Branch("nReducedOOTEB"        , &nReducedOOTEB        , "nReducedOOTEB/I");
  countingtree->Branch("nR2FMatchedEB"        , &nR2FMatchedEB        , "nR2FMatchedEB/I");
  countingtree->Branch("nR2FMatchedOOTEB"     , &nR2FMatchedOOTEB     , "nR2FMatchedOOTEB/I");

  countingtree->Branch("nReducedEE"           , &nReducedEE           , "nReducedEE/I");
  countingtree->Branch("nReducedOOTEE"        , &nReducedOOTEE        , "nReducedOOTEE/I");
  countingtree->Branch("nR2FMatchedEE"        , &nR2FMatchedEE        , "nR2FMatchedEE/I");
  countingtree->Branch("nR2FMatchedOOTEE"     , &nR2FMatchedOOTEE     , "nR2FMatchedOOTEE/I");

  countingtree->Branch("nFullEB"              , &nFullEB              , "nFullEB/I");
  countingtree->Branch("nFullOOTEB"           , &nFullOOTEB           , "nFullOOTEB/I");
  countingtree->Branch("nF2RMatchedEB"        , &nF2RMatchedEB        , "nF2RMatchedEB/I");
  countingtree->Branch("nF2RMatchedOOTEB"     , &nF2RMatchedOOTEB     , "nF2RMatchedOOTEB/I");

  countingtree->Branch("nFullEE"              , &nFullEE              , "nFullEE/I");
  countingtree->Branch("nFullOOTEE"           , &nFullOOTEE           , "nFullOOTEE/I");
  countingtree->Branch("nF2RMatchedEE"        , &nF2RMatchedEE        , "nF2RMatchedEE/I");
  countingtree->Branch("nF2RMatchedOOTEE"     , &nF2RMatchedOOTEE     , "nF2RMatchedOOTEE/I");
}

void OOTRecHits::endJob() {}

void OOTRecHits::beginRun(edm::Run const& iRun, edm::EventSetup const& iSetup) {}

void OOTRecHits::endRun(edm::Run const&, edm::EventSetup const&) {}

void OOTRecHits::fillDescriptions(edm::ConfigurationDescriptions& descriptions) 
{
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

DEFINE_FWK_MODULE(OOTRecHits);
