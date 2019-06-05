#include "Timing/TimingAnalyzer/plugins/RecHitCounter.hh"

////////////////////////
// Internal Functions //
////////////////////////

RecHitCounter::RecHitCounter(const edm::ParameterSet & iConfig): 
  // matching criteria
  dRmin(iConfig.existsAs<double>("dRmin") ? iConfig.getParameter<double>("dRmin") : 0.3),
  phpTmin(iConfig.existsAs<double>("phpTmin") ? iConfig.getParameter<double>("phpTmin") : 20.0),
  phIDmin(iConfig.existsAs<std::string>("phIDmin") ? iConfig.getParameter<std::string>("phIDmin") : "loose"),

  // rho
  rhoTag(iConfig.getParameter<edm::InputTag>("rho")),

  // mets
  metsTag(iConfig.getParameter<edm::InputTag>("mets")),  

  // photons
  gedPhotonsTag(iConfig.getParameter<edm::InputTag>("gedPhotons")),
  ootPhotonsTag(iConfig.getParameter<edm::InputTag>("ootPhotons"))
{
  // internal setup
  usesResource();
  usesResource("TFileService");

  // rho
  rhoToken = consumes<double> (rhoTag);

  // mets
  metsToken = consumes<std::vector<pat::MET> > (metsTag);

  // photons
  gedPhotonsToken = consumes<std::vector<pat::Photon> > (gedPhotonsTag);
  ootPhotonsToken = consumes<std::vector<pat::Photon> > (ootPhotonsTag);
}

RecHitCounter::~RecHitCounter() {}

void RecHitCounter::analyze(const edm::Event & iEvent, const edm::EventSetup & iSetup) 
{
  ///////////////////////
  // Get Event Objects //
  ///////////////////////

  if (!RecHitCounter::GetObjects(iEvent)) return;

  ////////////////////////
  // Initialize Objects //
  ////////////////////////

  RecHitCounter::InitializeObjects();

  //////////////////
  // Prep Objects //
  //////////////////

  RecHitCounter::PrepObjects();

  ////////////////////
  // Set Event Info //
  ////////////////////

  RecHitCounter::SetEventInfo();

  ///////////////
  // Fill Tree //
  ///////////////

  tree->Fill();
}

//////////////////////////
// Event Prep Functions //
//////////////////////////

bool RecHitCounter::GetObjects(const edm::Event & iEvent)
{
  // RHO
  iEvent.getByToken(rhoToken,rhoH);
  if (oot::BadHandle(rhoH,"rho")) return false;

  // MET
  iEvent.getByToken(metsToken,metsH);
  if (oot::BadHandle(metsH,"mets")) return false;

  // GEDPHOTONS
  iEvent.getByToken(gedPhotonsToken,gedPhotonsH);
  if (oot::BadHandle(gedPhotonsH,"gedPhotons")) return false;
  
  // OOTPHOTONS
  iEvent.getByToken(ootPhotonsToken,ootPhotonsH);
  if (oot::BadHandle(ootPhotonsH,"ootPhotons")) return false;

  // if no bad handles, return true
  return true;
}

void RecHitCounter::InitializeObjects()
{
  // INPUT + OUTPUT RHO
  rho = *(rhoH.product());

  // OUTPUT MET
  t1pfMET = pat::MET(metsH->front());

  photons.clear();
  photons.reserve(gedPhotonsH->size()+ootPhotonsH->size());
}

void RecHitCounter::PrepObjects()
{
  // prep merged photons 
  oot::PrepPhotonsCorrectMET(gedPhotonsH,ootPhotonsH,photons,t1pfMET,rho,dRmin,phpTmin,phIDmin);
}

void RecHitCounter::ResetRecHitMaps()
{
  gedRecHitMap.clear();
  ootRecHitMap.clear();
}

///////////////////
// Main Function //
///////////////////

void RecHitCounter::SetEventInfo()
{
  // reset maps
  RecHitCounter::ResetRecHitMaps();

  // count rechits (pre)
  RecHitCounter::CountRecHitsPre();

  // reset maps
  RecHitCounter::ResetRecHitMaps();

  // count rechits (post)
  RecHitCounter::CountRecHitsPost();
}

void RecHitCounter::CountRecHitsPre()
{
  // get GED rechits map
  RecHitCounter::SetRecHitMap(gedPhotonsH,gedRecHitMap,nGEDRH_Pre);

  // get OOT rechits map
  RecHitCounter::SetRecHitMap(ootPhotonsH,ootRecHitMap,nOOTRH_Pre);

  // count shared hits
  RecHitCounter::SetSharedHits(nSharedRH_Pre);
}

void RecHitCounter::CountRecHitsPost()
{
  // set rechits maps
  RecHitCounter::SetRecHitMaps();

  // count shared hits
  RecHitCounter::SetSharedHits(nSharedRH_Post);
}

void RecHitCounter::SetRecHitMap(const edm::Handle<std::vector<pat::Photon> > & photonsH, uiiumap & recHitMap, int & nRH)
{
  for (const auto & photon : *photonsH)
  {
    RecHitCounter::AppendPhotonRecHits(photon,recHitMap);
  } // end loop over photons

  // count nRH --> could consider tallying entries, versus just size()
  nRH = recHitMap.size();
}

void RecHitCounter::SetRecHitMaps()
{
  for (const auto & photon : photons)
  {
    // get rechit map for GED/OOT
    const auto isOOT = *(photon.userData<bool>(Config::IsOOT));
    auto & recHitMap = (isOOT ? ootRecHitMap : gedRecHitMap);

    RecHitCounter::AppendPhotonRecHits(photon,recHitMap);
  } // end loop over photons

  // count nRH --> could consider tallying entries, versus just size()
  nGEDRH_Post = gedRecHitMap.size();
  nOOTRH_Post = ootRecHitMap.size();
}

void RecHitCounter::AppendPhotonRecHits(const pat::Photon & photon, uiiumap & recHitMap)
{
  // local set of raw ids
  std::unordered_set<uint32_t> idxs;
  
  // super cluster info from photon + hits and fractions
  const auto & phosc = photon.superCluster().isNonnull() ? photon.superCluster() : photon.parentSuperCluster();
  const auto & hitsAndFractions = phosc->hitsAndFractions();
  
  // loop hits and fractions, store det ids
  for (auto hafitr = hitsAndFractions.begin(); hafitr != hitsAndFractions.end(); ++hafitr) // loop over all rec hits in SC
  {
    idxs.emplace(hafitr->first.rawId()); // get detid of crystal
  } // end loop over hits and fractions
  
  // append uiiumap with set
  for (const auto idx : idxs)
  {
    recHitMap[idx]++;
  }
}

void RecHitCounter::SetSharedHits(int & nRH)
{
  uiiumap intersection;
  std::set_intersection(gedRecHitMap.begin(), gedRecHitMap.end(),
			ootRecHitMap.begin(), ootRecHitMap.end(),
			std::inserter(intersection, intersection.begin()),
			[](const auto & pair1, const auto & pair2)
			{
			  return pair1.first < pair2.first;
			});

  // set n hits from intersection routine
  nRH = intersection.size();
}

//////////////////////////
// Setup Tree Functions //
//////////////////////////

void RecHitCounter::MakeTree()
{
  edm::Service<TFileService> fs;

  // Event tree
  tree = fs->make<TTree>("tree","tree");

  // nRecHits info: Pre
  tree->Branch("nGEDRH_Pre"     , &nGEDRH_Pre);
  tree->Branch("nOOTRH_Pre"     , &nOOTRH_Pre);
  tree->Branch("nSharedRH_Pre"  , &nSharedRH_Pre);

  // nRecHits info: Post
  tree->Branch("nGEDRH_Post"    , &nGEDRH_Post);
  tree->Branch("nOOTRH_Post"    , &nOOTRH_Post);
  tree->Branch("nSharedRH_Post" , &nSharedRH_Post);
}

////////////////////////
// Internal Functions //
////////////////////////

void RecHitCounter::beginJob()
{
  RecHitCounter::MakeTree();
}

void RecHitCounter::endJob() {}

void RecHitCounter::beginRun(const edm::Run & iRun, const edm::EventSetup & iSetup) {}

void RecHitCounter::endRun(const edm::Run & iRun, const edm::EventSetup & iSetup) {}

void RecHitCounter::fillDescriptions(edm::ConfigurationDescriptions & descriptions) 
{
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

DEFINE_FWK_MODULE(RecHitCounter);
