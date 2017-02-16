#include "AODRecHitsCheck.h"

AODRecHitsCheck::AODRecHitsCheck(const edm::ParameterSet& iConfig): 
  //photons
  photonsTag   (iConfig.getParameter<edm::InputTag>("photons")),
  gedPhotonsTag(iConfig.getParameter<edm::InputTag>("gedPhotons")),

  //clusters?
  clustersEBTag(iConfig.getParameter<edm::InputTag>("clustersEB")),
  clustersEETag(iConfig.getParameter<edm::InputTag>("clustersEE")),

  //recHits
  recHitsEBTag(iConfig.getParameter<edm::InputTag>("recHitsEB")),  
  recHitsEETag(iConfig.getParameter<edm::InputTag>("recHitsEE"))
{
  photonsToken    = consumes<std::vector<reco::Photon> > (photonsTag);
  gedPhotonsToken = consumes<std::vector<reco::Photon> > (gedPhotonsTag);

  clustersEBToken = consumes<std::vector<reco::SuperCluster> > (clustersEBTag);
  clustersEEToken = consumes<std::vector<reco::CaloCluster > > (clustersEETag);

  recHitsEBToken = consumes<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > (recHitsEBTag);
  recHitsEEToken = consumes<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > (recHitsEETag);

  usesResource();
  usesResource("TFileService");
}

AODRecHitsCheck::~AODRecHitsCheck() {}

void AODRecHitsCheck::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) 
{
  // Photons
  edm::Handle<std::vector<reco::Photon> > photonsH;
  iEvent.getByToken(photonsToken, photonsH);

  edm::Handle<std::vector<reco::Photon> > gedPhotonsH;
  iEvent.getByToken(gedPhotonsToken, gedPhotonsH);

  // Clusters
  edm::Handle<std::vector<reco::SuperCluster> > clustersEBH;
  iEvent.getByToken(clustersEBToken, clustersEBH);

  edm::Handle<std::vector<reco::CaloCluster > > clustersEEH;
  iEvent.getByToken(clustersEEToken, clustersEEH);

  // RecHits
  edm::Handle<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > recHitsEBH;
  iEvent.getByToken(recHitsEBToken, recHitsEBH);

  edm::Handle<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > recHitsEEH;
  iEvent.getByToken(recHitsEEToken, recHitsEEH);

  AODRecHitsCheck::InitializeBranches();

  // Event number
  event = iEvent.id().event();

  if (evseedmap.count(event)) 
  {
    isEvent = true;

    // get rechits / isEB
    const DetId seedId(evseedmap[event].rawid_);
    isEB = (seedId.subdetId() == EcalBarrel);
    const EcalRecHitCollection * recHits = (isEB?recHitsEBH:recHitsEEH).product();

    // get the (potential) photons
    if (photonsH.isValid())
    {
      for (std::vector<reco::Photon>::const_iterator phiter = photonsH->begin(); phiter != photonsH->end(); ++phiter)
      {
       const DetId phSeedId = phiter->superCluster()->seed()->seed();
       if (phSeedId.rawId() == seedId.rawId()) {isPhoton = true; break;}
      } 
    }

    if (gedPhotonsH.isValid())
    {
      for (std::vector<reco::Photon>::const_iterator phiter = gedPhotonsH->begin(); phiter != gedPhotonsH->end(); ++phiter)
      {
       const DetId phSeedId = phiter->superCluster()->seed()->seed();
       if (phSeedId.rawId() == seedId.rawId()) {isGedPhoton = true; break;}
      } 
    }

    // get the (potential) clusters???
    if (isEB)
    {
      if (clustersEBH.isValid())
      {
	for (std::vector<reco::SuperCluster>::const_iterator sciter = clustersEBH->begin(); sciter != clustersEBH->end(); ++sciter)
	{
	  const DetId scSeedId = sciter->seed()->seed();
	  if (scSeedId.rawId() == seedId.rawId()) {isCluster = true; break;}
	}
      }
    }
    else
    {
      if (clustersEEH.isValid())
      {
	for (std::vector<reco::CaloCluster>::const_iterator cciter = clustersEEH->begin(); cciter != clustersEEH->end(); ++cciter)
	{
	  const DetId ccSeedId = cciter->seed(); // Photon->SuperCluster->CaloCluster->DetId
	  if (ccSeedId.rawId() == seedId.rawId()) {isCluster = true; break;}
	}
      }
    }

    // displaced photon seed rechit info
    seedID   = evseedmap[event].rawid_;
    seedE    = evseedmap[event].energy_;
    seedtime = evseedmap[event].time_;
    seedOOT  = evseedmap[event].oot_;
    
    // does it exist in AOD??
    isSeed   = (recHits->find(seedId) != recHits->end());

    nRHs = evrhvecmap[event].size();
    for (const auto& rhinfo : evrhvecmap[event])
    {
      rhIDs.push_back(rhinfo.rawid_);
      rhEs.push_back(rhinfo.energy_);
      rhtimes.push_back(rhinfo.time_);
      rhOOTs.push_back(rhinfo.oot_);

      const DetId rhId(rhinfo.rawid_);
      isRHs.push_back((recHits->find(rhId) != recHits->end()));
    }
  }
  
  // fill the tree
  tree->Fill();
}

void AODRecHitsCheck::InitializeBranches()
{
  event    = -9999;
  isEvent  = false;
  seedID   = -9999;
  isEB     = false;
  isPhoton = false;
  isGedPhoton = false;
  isCluster =  false;
  seedE    = -9999.f;
  seedtime = -9999.f;
  seedOOT  = false; 
  isSeed   = false;

  nRHs     = -9999;
  rhIDs.clear();
  rhEs.clear();
  rhtimes.clear();
  rhOOTs.clear();
  isRHs.clear();
}

void AODRecHitsCheck::beginJob() 
{
  ////////////////////////
  //                    //
  // read in text files //
  //                    //
  ////////////////////////
  std::ifstream seedreader;
  seedreader.open("/afs/cern.ch/user/k/kmcdermo/private/dispho/CMSSW_8_0_21/src/Timing/TimingAnalyzer/tmp/seeddump.txt",std::ios::in);
  int   ev  = -9999, iph = -9999, irh = -9999, rawid = -9999;
  bool  oot = false;
  float energy = -9999.f, time = -9999.f;
  while(seedreader >> ev >> iph >> irh >> rawid >> oot >> energy >> time)
  {
    evseedmap[ev] = RHInfo(rawid,oot,energy,time);
  }
  seedreader.close();

  std::ifstream rhreader;
  rhreader.open("/afs/cern.ch/user/k/kmcdermo/private/dispho/CMSSW_8_0_21/src/Timing/TimingAnalyzer/tmp/allrhdump.txt",std::ios::in);
  ev  = -9999, iph = -9999, irh = -9999, rawid = -9999;
  oot = false;
  energy = -9999.f, time = -9999.f;
  while(rhreader >> ev >> iph >> irh >> rawid >> oot >> energy >> time)
  {
    evrhvecmap[ev].push_back(RHInfo(rawid,oot,energy,time));
  }
  rhreader.close();

  ///////////////////
  //               //
  // set up output //
  //               //
  ///////////////////
  edm::Service<TFileService> fs;

  // rh tree
  tree = fs->make<TTree>("tree", "tree");

  // event level info
  tree->Branch("event"             , &event              , "event/I");
  tree->Branch("isEvent"           , &isEvent            , "isEvent/O");

  tree->Branch("seedID"            , &seedID             , "seedID/I");
  tree->Branch("isEB"              , &isEB               , "isEB/O");
  tree->Branch("isPhoton"          , &isPhoton           , "isPhoton/O");
  tree->Branch("isGedPhoton"       , &isGedPhoton        , "isGedPhoton/O");
  tree->Branch("isCluster"         , &isCluster          , "isCluster/O");
  tree->Branch("seedE"             , &seedE              , "seedE/F");
  tree->Branch("seedtime"          , &seedtime           , "seedtime/F");
  tree->Branch("seedOOT"           , &seedOOT            , "seedOOT/O");
  tree->Branch("isSeed"            , &isSeed             , "isSeed/O");

  tree->Branch("nRHs"              , &nRHs               , "nRHs/I");
  tree->Branch("rhIDs"             , &rhIDs);
  tree->Branch("rhEs"              , &rhEs);
  tree->Branch("rhtimes"           , &rhtimes);
  tree->Branch("rhOOTs"            , &rhOOTs);
  tree->Branch("isRHs"             , &isRHs);
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
