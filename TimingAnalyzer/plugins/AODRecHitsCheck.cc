#include "AODRecHitsCheck.h"

AODRecHitsCheck::AODRecHitsCheck(const edm::ParameterSet& iConfig): 
  //recHits
  recHitsEBTag(iConfig.getParameter<edm::InputTag>("recHitsEB")),  
  recHitsEETag(iConfig.getParameter<edm::InputTag>("recHitsEE"))
{
  recHitsEBToken = consumes<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > (recHitsEBTag);
  recHitsEEToken = consumes<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > (recHitsEETag);

  usesResource();
  usesResource("TFileService");
}

AODRecHitsCheck::~AODRecHitsCheck() {}

void AODRecHitsCheck::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) 
{
  // RecHits
  edm::Handle<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > recHitsEBH;
  iEvent.getByToken(recHitsEBToken, recHitsEBH);
  edm::Handle<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > recHitsEEH;
  iEvent.getByToken(recHitsEEToken, recHitsEEH);

  AODRecHitsCheck::InitializeBranches();

  // Event number
  event = iEvent.id().event();

  if (evmap.count(event)) 
  {
    isEvent = true;

    const DetId seedId(evmap[event].rawid_);
    const bool  isEB = (seedId.subdetId() == EcalBarrel);
    
    isSeed = AODRecHitsCheck::SeedCheck(isEB?recHitsEBH:recHitsEEH);
    
    seedID   = evmap[event].rawid_;
    seedDet  = isEB;
    seedE    = evmap[event].energy_;
    seedtime = evmap[event].time_;
    seedOOT  = evmap[event].oot_;
  }
  
  // fill the tree
  tree->Fill();
}

void AODRecHitsCheck::InitializeBranches()
{
  event    = -9999;
  isEvent  = false;
  seedID   = -9999;
  seedDet  = false;
  seedE    = -9999.f;
  seedtime = -9999.f;
  seedOOT  = false; 
  isSeed   = false;
}

bool AODRecHitsCheck::SeedCheck(const edm::Handle<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > & recHits)
{
  bool found = false;
  
  for (auto recHitIter = recHits->begin(); recHitIter != recHits->end(); ++recHitIter)
  {
    int recHitId = recHitIter->detid().rawId();
    if (recHitId == evmap[event].rawid_) {found = true; break;}
  }
  
  return found;
}

void AODRecHitsCheck::beginJob() 
{
  // read in text file
  std::ifstream seedreader;
  seedreader.open("/afs/cern.ch/user/k/kmcdermo/private/dispho/CMSSW_8_0_21/src/Timing/TimingAnalyzer/tmp/seeddump.txt",std::ios::in);
  int   ev  = -1, iph = -1, irh = -1, rawid = -1;
  bool  oot = false;
  float energy, time;

  while(seedreader >> ev >> iph >> irh >> rawid >> oot >> energy >> time)
  {
    evmap[ev] = SeedInfo(rawid,oot,energy,time);
  }

  // set up output
  edm::Service<TFileService> fs;

  // rh tree
  tree = fs->make<TTree>("tree", "tree");

  // event level info
  tree->Branch("event"             , &event              , "event/I");
  tree->Branch("isEvent"           , &isEvent            , "isEvent/O");
  tree->Branch("seedID"            , &seedID             , "seedID/I");
  tree->Branch("seedDet"           , &seedDet            , "seedDet/O");
  tree->Branch("seedE"             , &seedE              , "seedE/F");
  tree->Branch("seedtime"          , &seedtime           , "seedtime/F");
  tree->Branch("seedOOT"           , &seedOOT            , "seedOOT/O");
  tree->Branch("isSeed"            , &isSeed             , "isSeed/O");
}

void AODRecHitsCheck::endJob() {}

void AODRecHitsCheck::beginRun(edm::Run const& iRun, edm::EventSetup const& iSetup) {}

void AODRecHitsCheck::endRun(edm::Run const&, edm::EventSetup const&) {}

void AODRecHitsCheck::fillDescriptions(edm::ConfigurationDescriptions& descriptions) 
{
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

DEFINE_FWK_MODULE(AODRecHitsCheck);
