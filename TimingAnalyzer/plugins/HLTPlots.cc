#include "Timing/TimingAnalyzer/plugins/HLTPlots.hh"

/////////////////
// Class Setup //
/////////////////

HLTPlots::HLTPlots(const edm::ParameterSet & iConfig) :
  // pre-selection
  applyTriggerPS(iConfig.existsAs<bool>("applyTriggerPS") ? iConfig.getParameter<bool>("applyTriggerPS") : false),
  psPath        (iConfig.existsAs<std::string>("psPath")  ? iConfig.getParameter<std::string>("psPath") : "HLT_IsoMu27_v"),

  // cuts
  jetpTmin(iConfig.existsAs<double>("jetpTmin") ? iConfig.getParameter<double>("jetpTmin") : 30.0),
  jetIDmin(iConfig.existsAs<int>("jetIDmin") ? iConfig.getParameter<int>("jetIDmin") : 1),
  jetEtamax(iConfig.existsAs<int>("jetEtamax") ? iConfig.getParameter<double>("jetEtamax") : 3.0),
  phpTmin(iConfig.existsAs<double>("phpTmin") ? iConfig.getParameter<double>("phpTmin") : 20.0),
  phIDmin(iConfig.existsAs<std::string>("phIDmin") ? iConfig.getParameter<std::string>("phIDmin") : "loose"),
  
  // object extra pruning cuts
  seedTimemin(iConfig.existsAs<double>("seedTimemin") ? iConfig.getParameter<double>("seedTimemin") : -25.f),
  nPhosmax(iConfig.existsAs<int>("nPhosmax") ? iConfig.getParameter<int>("nPhosmax") : 2),
  
  // matching cuts
  dRmin(iConfig.existsAs<double>("dRmin") ? iConfig.getParameter<double>("dRmin") : 0.3),
  pTres(iConfig.existsAs<double>("pTres") ? iConfig.getParameter<double>("pTres") : 0.5),
  trackpTmin(iConfig.existsAs<double>("trackpTmin") ? iConfig.getParameter<double>("trackpTmin") : 5.0),
  trackdRmin(iConfig.existsAs<double>("trackdRmin") ? iConfig.getParameter<double>("trackdRmin") : 0.2),
  
  // triggers
  inputPaths       (iConfig.existsAs<std::string>("inputPaths")   ? iConfig.getParameter<std::string>("inputPaths") : ""),
  inputFilters     (iConfig.existsAs<std::string>("inputFilters") ? iConfig.getParameter<std::string>("inputFilters") : ""),
  triggerResultsTag(iConfig.getParameter<edm::InputTag>("triggerResults")),
  triggerObjectsTag(iConfig.getParameter<edm::InputTag>("triggerObjects")),
  
  // tracks
  tracksTag(iConfig.getParameter<edm::InputTag>("tracks")),
  
  // rho
  rhoTag(iConfig.getParameter<edm::InputTag>("rho")),

  // mets
  metsTag(iConfig.getParameter<edm::InputTag>("mets")),

  // jets
  jetsTag(iConfig.getParameter<edm::InputTag>("jets")),

  // recHits
  recHitsEBTag(iConfig.getParameter<edm::InputTag>("recHitsEB")),  
  recHitsEETag(iConfig.getParameter<edm::InputTag>("recHitsEE")),
  
  // photons
  gedPhotonsTag(iConfig.getParameter<edm::InputTag>("gedPhotons")),
  ootPhotonsTag(iConfig.getParameter<edm::InputTag>("ootPhotons"))  
{
  // internal setup
  usesResource();
  usesResource("TFileService");
  
  // setup tokens
  HLTPlots::ConsumeTokens();
  
  // store pre-selection path if it exists
  if (applyTriggerPS && psPath != "") triggerBitMap[psPath] = false;
  
  // read in from a stream the trigger paths for saving
  oot::ReadInTriggerNames(inputPaths,pathNames,triggerBitMap);
  
  // read in from a stream the hlt objects/labels to match to
  oot::ReadInFilterNames(inputFilters,filterNames,triggerObjectsByFilterMap);

  // setup effTestMap
  HLTPlots::SetupEffTestMap();
}

void HLTPlots::ConsumeTokens()
{
  // triggers
  triggerResultsToken = consumes<edm::TriggerResults> (triggerResultsTag);
  triggerObjectsToken = consumes<std::vector<pat::TriggerObjectStandAlone> > (triggerObjectsTag);

  // tracks 
  tracksToken = consumes<std::vector<reco::Track> > (tracksTag);

  // rho
  rhoToken = consumes<double> (rhoTag);

  // mets
  metsToken = consumes<std::vector<pat::MET> > (metsTag);

  // jets
  jetsToken = consumes<std::vector<pat::Jet> > (jetsTag);

  // rechits
  recHitsEBToken = consumes<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > (recHitsEBTag);
  recHitsEEToken = consumes<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > (recHitsEETag);

  // photons
  gedPhotonsToken = consumes<std::vector<pat::Photon> > (gedPhotonsTag);
  ootPhotonsToken = consumes<std::vector<pat::Photon> > (ootPhotonsTag);
}

void HLTPlots::SetupEffTestMap()
{
  // set test options
  const TestResults initResults = {false,-1};
  effTestMap["L1"]      = {{""                     ,Config::L1Trigger      ,-1,-1},initResults};
  effTestMap["L1toHLT"] = {{Config::L1Trigger      ,Config::L1toHLTFilter  ,-1,-1},initResults};
  effTestMap["ET"]      = {{Config::L1toHLTFilter  ,Config::ETFilter       ,-1,-1},initResults};
  effTestMap["PhoID"]   = {{Config::ETFilter       ,Config::PhoIDLastFilter,-1,-1},initResults};
  effTestMap["DispID"]  = {{Config::PhoIDLastFilter,Config::DispIDFilter   ,-1,-1},initResults};
  effTestMap["HT"]      = {{Config::DispIDFilter   ,Config::SignalPath     ,-1,-1},initResults};
  for (auto & effTestPair : effTestMap)
  {
    // init options
    auto & options = effTestPair.second.options;
    for (auto ifilter = 0U; ifilter < filterNames.size(); ifilter++)
    {
      const auto & filterName = filterNames[ifilter];
      if (filterName == options.denomName) options.idenom = ifilter;
      if (filterName == options.numerName) options.inumer = ifilter;
    }
  }
}

HLTPlots::~HLTPlots() {}

////////////////////
// Analyze Events //
////////////////////

void HLTPlots::analyze(const edm::Event & iEvent, const edm::EventSetup & iSetup)
{
  ///////////////////////
  // Get Event Objects //
  ///////////////////////

  if (!HLTPlots::GetObjects(iEvent,iSetup)) return;

  ////////////////////////
  // Initialize Objects //
  ////////////////////////

  HLTPlots::InitializeObjects();

  //////////////////
  // Prep Objects //
  //////////////////

  HLTPlots::PrepObjects(iEvent);

  ///////////////////
  // Pre-Selection //
  ///////////////////

  // if event does not pass pre-selection, skip it!
  if (applyTriggerPS && HLTPlots::ApplyPreSelection()) return;

  //////////////////
  // Set jet info //
  //////////////////

  HLTPlots::SetJetInfo();

  /////////////////////
  // Set photon info //
  /////////////////////

  HLTPlots::SetPhotonInfo();

  //////////////////////
  // Set Good Photons //
  //////////////////////

  HLTPlots::SetGoodPhotons();

  // if no good photons, skip the event!
  if (goodphs.size() == 0) return; 

  ///////////////////
  // Perform Tests //
  ///////////////////

  HLTPlots::PerformTests();
}

//////////////////
// Prep Objects //
//////////////////

bool HLTPlots::GetObjects(const edm::Event & iEvent, const edm::EventSetup & iSetup)
{
  // RHO
  iEvent.getByToken(rhoToken,rhoH);
  if (oot::BadHandle(rhoH,"rho")) return false;

  // TRIGGER RESULTS
  iEvent.getByToken(triggerResultsToken,triggerResultsH);
  if (oot::BadHandle(triggerResultsH,"triggerResults")) return false;

  // TRIGGER OBJECTS
  iEvent.getByToken(triggerObjectsToken,triggerObjectsH);
  if (oot::BadHandle(triggerObjectsH,"triggerObjects")) return false;

  // Tracks
  iEvent.getByToken(tracksToken,tracksH);
  if (oot::BadHandle(tracksH,"tracks")) return false;

  // METS
  iEvent.getByToken(metsToken,metsH);
  if (oot::BadHandle(metsH,"mets")) return false;

  // JETS
  iEvent.getByToken(jetsToken,jetsH);
  if (oot::BadHandle(jetsH,"jets")) return false;

  // RecHits
  iEvent.getByToken(recHitsEBToken,recHitsEBH);
  if (oot::BadHandle(recHitsEBH,"recHitsEB")) return false;

  iEvent.getByToken(recHitsEEToken,recHitsEEH);
  if (oot::BadHandle(recHitsEEH,"recHitsEE")) return false;

  // GED PHOTONS
  iEvent.getByToken(gedPhotonsToken,gedPhotonsH);
  if (oot::BadHandle(gedPhotonsH,"gedPhotons")) return false;

  // OOT PHOTONS
  iEvent.getByToken(ootPhotonsToken,ootPhotonsH);
  if (oot::BadHandle(ootPhotonsH,"ootPhotons")) return false;

  // geometry (from ECAL ELF)
  iSetup.get<CaloGeometryRecord>().get(caloGeoH);
  if (oot::BadHandle(caloGeoH,"caloGeo")) return false;

  // if no bad handles, return true
  return true;
}

void HLTPlots::InitializeObjects()
{
  // INPUT RHO
  rho = *(rhoH.product());

  // INPUT RECHITS
  recHitsEB = recHitsEBH.product();
  recHitsEE = recHitsEEH.product();

  // INPUT GEOMETRY
  barrelGeometry = caloGeoH->getSubdetectorGeometry(DetId::Ecal,EcalSubdetector::EcalBarrel);
  endcapGeometry = caloGeoH->getSubdetectorGeometry(DetId::Ecal,EcalSubdetector::EcalEndcap);

  // OUTPUT MET
  t1pfMET = pat::MET(metsH->front());

  // OUTPUT JETS
  jets.clear(); 
  jets.reserve(jetsH->size());

  // OUTPUT PHOTONS
  photons.clear();
  photons.reserve(gedPhotonsH->size()+ootPhotonsH->size());
  goodphs.clear();
}

void HLTPlots::PrepObjects(const edm::Event & iEvent)
{
  oot::PrepTriggerBits(triggerResultsH,iEvent,triggerBitMap);
  oot::PrepTriggerObjects(triggerResultsH,triggerObjectsH,iEvent,triggerObjectsByFilterMap);
  oot::PrepJets(jetsH,jets,jetpTmin,jetEtamax,jetIDmin);
  oot::PrepPhotonsCorrectMET(gedPhotonsH,ootPhotonsH,photons,t1pfMET,rho,dRmin,phpTmin,phIDmin);

  oot::PrunePhotons(photons,recHitsEB,recHitsEE,seedTimemin);
  oot::PruneJets(jets,photons,nPhosmax,dRmin);
}

inline bool HLTPlots::ApplyPreSelection()
{
  if (triggerBitMap.count(psPath))
  {
    return !triggerBitMap.at(psPath);
  }
  else return false;
}

//////////////////
// Set Jet Info //
//////////////////

void HLTPlots::ClearJetBranches()
{
  njets = -9999;

  jetpt.clear();
  jeteta.clear();

  pfJetHT = 0.f;
}

void HLTPlots::InitializeJetBranches()
{
  jetpt.resize(njets,-9999.f);
  jeteta.resize(njets,-9999.f);
}

void HLTPlots::SetJetInfo()
{
  HLTPlots::ClearJetBranches();
  njets = jets.size();
  if (njets > 0) HLTPlots::InitializeJetBranches();

  auto ijet = 0;
  for (const auto & jet : jets)
  {
    jetpt [ijet] = jet.pt();
    jeteta[ijet] = jet.eta();

    pfJetHT += jetpt[ijet];
    
    ijet++;
  } // end loop over reco jets
}

/////////////////////
// Set Photon Info //
/////////////////////

void HLTPlots::ClearRecoPhotonBranches()
{
  nphotons = -9999;

  phpt.clear();
  phphi.clear();
  pheta.clear();

  phHoE.clear();
  phr9.clear();

  phChgIso.clear();
  phNeuIso.clear();
  phPhoIso.clear();
  phPFClEcalIso.clear();
  phPFClHcalIso.clear();
  phHollowTkIso.clear();

  phsieie.clear();
  phsmaj.clear();
  phsmin.clear();

  phIsHLTMatched.clear();
  phIsOOT.clear();
  phIsEB.clear();
  phIsTrack.clear();

  phseedtime.clear();
}

void HLTPlots::InitializeRecoPhotonBranches()
{
  phpt.resize(nphotons,-9999.f);
  phphi.resize(nphotons,-9999.f);
  pheta.resize(nphotons,-9999.f);

  phHoE.resize(nphotons,-9999.f);
  phr9.resize(nphotons,-9999.f);

  phChgIso.resize(nphotons,-9999.f);
  phNeuIso.resize(nphotons,-9999.f);
  phPhoIso.resize(nphotons,-9999.f);
  phPFClEcalIso.resize(nphotons,-9999.f);
  phPFClHcalIso.resize(nphotons,-9999.f);
  phHollowTkIso.resize(nphotons,-9999.f);

  phsieie.resize(nphotons,-9999.f);
  phsmaj .resize(nphotons,-9999.f);
  phsmin .resize(nphotons,-9999.f);

  phIsHLTMatched.resize(nphotons);
  for (auto iph = 0; iph < nphotons; iph++) phIsHLTMatched[iph].resize(filterNames.size(),-1);
  phIsOOT.resize(nphotons,-1);
  phIsEB.resize(nphotons,-1);
  phIsTrack.resize(nphotons,-1);

  phseedtime.resize(nphotons,-9999.f);
}

void HLTPlots::SetPhotonInfo()
{
  HLTPlots::ClearRecoPhotonBranches();
  nphotons = photons.size();
  if (nphotons > 0) HLTPlots::InitializeRecoPhotonBranches();
  
  auto iph = 0;
  for (const auto & photon : photons)
  {
    // standard photon branches
    phpt [iph] = oot::GetPhotonPt(photon);
    phphi[iph] = photon.phi();
    pheta[iph] = photon.eta();
    
    // super cluster from photon
    const auto & phsc = photon.superCluster().isNonnull() ? photon.superCluster() : photon.parentSuperCluster();
    
    // ID-like variables
    phHoE[iph] = photon.hadTowOverEm(); // ID + trigger == single tower
    phr9 [iph] = photon.r9();
    
    // cluster shape variables
    const auto & phshape = photon.full5x5_showerShapeVariables();
    phsieie[iph] = phshape.sigmaIetaIeta;
 
    // tmp vars needed for correcting isos
    const auto uncpt = photon.pt();
    const auto sceta = std::abs(phsc->eta());
 
    // PF Isolations
    phChgIso[iph] = std::max(photon.chargedHadronIso() - (rho * oot::GetChargedHadronEA(sceta))                                              ,0.f);
    phNeuIso[iph] = std::max(photon.neutralHadronIso() - (rho * oot::GetNeutralHadronEA(sceta)) - (oot::GetNeutralHadronPtScale(sceta,uncpt)),0.f);
    phPhoIso[iph] = std::max(photon.photonIso()        - (rho * oot::GetGammaEA        (sceta)) - (oot::GetGammaPtScale        (sceta,uncpt)),0.f);
    
    // PF Cluster Isolations
    phPFClEcalIso[iph] = std::max(photon.ecalPFClusterIso()       - (rho * oot::GetEcalPFClEA(sceta)) - (oot::GetEcalPFClPtScale(sceta,uncpt)),0.f);
    phPFClHcalIso[iph] = std::max(photon.hcalPFClusterIso()       - (rho * oot::GetHcalPFClEA(sceta)) - (oot::GetHcalPFClPtScale(sceta,uncpt)),0.f);
    phHollowTkIso[iph] = std::max(photon.trkSumPtHollowConeDR03() - (rho * oot::GetTrackEA   (sceta))                                         ,0.f);

    // check for HLT filter matches!
    strBitMap isHLTMatched;
    oot::InitializeBitMap(filterNames,isHLTMatched);
    oot::HLTToObjectMatching(triggerObjectsByFilterMap,isHLTMatched,photon,pTres,dRmin);
    for (auto ifilter = 0U; ifilter < filterNames.size(); ifilter++)
    {
      phIsHLTMatched[iph][ifilter] = isHLTMatched[filterNames[ifilter]];
    }
    
    // from ootPhoton collection
    phIsOOT[iph] = *(photon.userData<bool>(Config::IsOOT));
    
    // check for simple track veto
    phIsTrack[iph] = oot::TrackToObjectMatching(tracksH,photon,trackpTmin,trackdRmin);
    
    // use seed to get geometry and recHits
    const auto seedDetId = phsc->seed()->seed(); //seed detid
    phIsEB[iph] = (seedDetId.subdetId() == EcalSubdetector::EcalBarrel); //which subdet
    const auto recHits = (phIsEB[iph] ? recHitsEB : recHitsEE);
    const auto seedRecHit = recHits->find(seedDetId);
    
    if (seedRecHit != recHits->end())
    {
      phseedtime[iph] = seedRecHit->time();
      
      if (recHits->size() > 0)
      {
	// radius of semi-major,minor axis is the inverse square root of the eigenvalues of the covariance matrix
	const auto ph2ndMoments = noZS::EcalClusterTools::cluster2ndMoments( *phsc, *recHits);
	phsmaj [iph] = ph2ndMoments.sMaj;
	phsmin [iph] = ph2ndMoments.sMin;
      } // end check over recHits size
    } // end check over seed recHit exists
    
    iph++; // increment photon counter
  } // end loop over photon vector
}

////////////////////////
// Trigger Test Setup //
////////////////////////

void HLTPlots::SetGoodPhotons()
{
  for (auto iph = 0; iph < nphotons; iph++)
  {
    // common offline selection for GED and VID photons
    if (!phIsEB[iph])           continue;
    if (phIsTrack[iph])         continue;
    if (phr9   [iph] < 0.90   ) continue;
    if (phHoE  [iph] > 0.02148) continue;
    if (phsmaj [iph] > 1.3    ) continue;
    if (phsmin [iph] > 0.4    ) continue;
    if (phsieie[iph] > 0.014  ) continue; // N.B. This is the hand-made "loose" sieie to keep time flat!
    
    // apply vid cuts
    if (phIsOOT[iph]) // apply OOT VID cuts
    {
      if (phPFClEcalIso[iph] > 5.f) continue;
      if (phPFClHcalIso[iph] > 4.f) continue;
      if (phHollowTkIso[iph] > 4.f) continue;
    }
    else // apply GED VID cuts
    {
      if (phChgIso[iph] > 0.65 ) continue;
      if (phNeuIso[iph] > 0.317) continue;
      if (phPhoIso[iph] > 2.044) continue;
    }
    
    // save index for now
    goodphs.emplace_back(iph);
  } // end loop over photons
}

void HLTPlots::ResetTestResults()
{
  for (auto & effTestPair : effTestMap)
  {
    auto & results =  effTestPair.second.results;
    
    results.passed = false;
    results.goodph = -1;
  }
}

////////////////////////
// Main Trigger Tests //
////////////////////////

void HLTPlots::PerformTests()
{
  // Reset test results
  HLTPlots::ResetTestResults();

  // Get Results
  HLTPlots::GetFirstLegResult(goodphs,"L1");
  HLTPlots::GetStandardLegResult(goodphs,"L1toHLT",false);
  HLTPlots::GetStandardLegResult(goodphs,"ET",false);

  // filter photons below pt < threshold
  const auto ptcut = 70.f;
  goodphs.erase(std::remove_if(goodphs.begin(),goodphs.end(),[&](const auto iph){return phpt[iph] < ptcut;}),goodphs.end());

  HLTPlots::GetStandardLegResult(goodphs,"PhoID",false);
  HLTPlots::GetStandardLegResult(goodphs,"DispID",true);
  HLTPlots::GetLastLegResult(goodphs,"HT");
  
  // fill in the plots!
  for (const auto & effTestPair : effTestMap)
  {
    const auto iph      = effTestPair.second.results.goodph;

    if (iph < 0) continue;

    const auto & label  = effTestPair.first;
    const auto   passed = effTestPair.second.results.passed;
    
    effETs[label]->Fill(passed,phpt[iph]);

    // want to see flat in phi/eta, so factor out offline ET turn-on
    if (phpt[iph] > ptcut)
    {
      effetas [label]->Fill(passed,pheta[iph]);
      effphis [label]->Fill(passed,phphi[iph]);
      efftimes[label]->Fill(passed,phseedtime[iph]);
      effHTs  [label]->Fill(passed,pfJetHT);
    }
  }
}

//////////////////
// Test Methods //
//////////////////

void HLTPlots::GetDenomPhs(const std::vector<int> & goodphs, const int idenom, std::vector<int> & denomphs)
{
  for (const auto iph : goodphs)
  {
    auto isGoodDenom = true;

    for (auto ifilter = 0; ifilter <= idenom; ifilter++)
    { 
      if (!phIsHLTMatched[iph][ifilter]) 
      {
	isGoodDenom = false;
	break;
      }
    }

    if (!isGoodDenom) continue;
    denomphs.emplace_back(iph);
  }
}

void HLTPlots::GetFirstLegResult(const std::vector<int> & goodphs, const std::string & label)
{
  const auto & options = effTestMap[label].options;
  auto       & results = effTestMap[label].results;

  // get numer
  for (const auto iph : goodphs)
  {
    if (phIsHLTMatched[iph][options.inumer])
    {
      results.passed = true;
      results.goodph = iph;
      break;
    }
  }

  // set to leading photon if no good one
  if (results.goodph < 0) results.goodph = goodphs[0];
}

void HLTPlots::GetStandardLegResult(const std::vector<int> & goodphs, const std::string & label, const bool sortByTime)
{
  const auto & options = effTestMap[label].options;
  auto       & results = effTestMap[label].results;
  
  // get potential denom
  std::vector<int> denomphs;
  HLTPlots::GetDenomPhs(goodphs,options.idenom,denomphs);

  // need at least one match!
  if (denomphs.size() == 0) return;

  // sort by most delayed photon
  if (sortByTime) std::sort(denomphs.begin(),denomphs.end(),
			    [&](const auto iph1, const auto iph2)
			    {
			      return phseedtime[iph1]>phseedtime[iph2];
			    });

  // get numer
  for (const auto iph : denomphs)
  {
    if (phIsHLTMatched[iph][options.inumer])
    {
      results.passed = true;
      results.goodph = iph;
      break;
    }
  }

  // set to leading photon if no good one
  if (results.goodph < 0) results.goodph = denomphs[0];
}

void HLTPlots::GetLastLegResult(const std::vector<int> & goodphs, const std::string & label)
{
  const auto & options = effTestMap[label].options;
  auto       & results = effTestMap[label].results;
  
  // get potential denom
  std::vector<int> denomphs;
  HLTPlots::GetDenomPhs(goodphs,options.idenom,denomphs);
  
  // need at least one match!
  if (denomphs.size() == 0) return;
  
  // get numer
  if (triggerBitMap.count(options.numerName))
  {
    if (triggerBitMap.at(options.numerName))
    {
      results.passed = true;
    }
  }

  // set to leading photon
  results.goodph = denomphs[0];
}

//////////////////////
// Internal Methods //
//////////////////////

void HLTPlots::beginJob() 
{
  edm::Service<TFileService> fs;

  // ET bins
  const std::vector<double> xbinsET {20,30,40,45,50,52,54,56,58,60,62,64,66,68,70,75,80,100,200,500,1000};
  const auto  binsETX = &xbinsET[0];
  const auto nbinsETX = xbinsET.size()-1;
  
  // Time bins
  const std::vector<double> xbinsTime = {-5,-3,-2.5,-2,-1.5,-1,-0.5,0,0.5,1,1.5,2,2.5,3,5,10,15,25};
  const auto  binsTimeX = &xbinsTime[0];
  const auto nbinsTimeX = xbinsTime.size()-1;

  // HT bins
  const std::vector<double> xbinsHT = {100,150,200,250,300,325,350,375,400,425,450,475,500,525,550,575,600,625,650,700,750,1000,1500,2000};
  const auto  binsHTX = &xbinsHT[0];
  const auto nbinsHTX = xbinsHT.size()-1;

  for (const auto & effTestPair : effTestMap)
  {
    const auto & label = effTestPair.first;

    effETs  [label] = fs->make<TEfficiency>(Form("effET_%s",label.c_str()),"HLT Efficiency vs Leading Photon E_{T} [EB];Photon Offline E_{T};Efficiency",nbinsETX,binsETX);
    effetas [label] = fs->make<TEfficiency>(Form("effeta_%s",label.c_str()),"HLT Efficiency vs Leading Photon #eta;Photon Offline #eta;Efficiency",30,-1.5f,1.5f);
    effphis [label] = fs->make<TEfficiency>(Form("effphi_%s",label.c_str()),"HLT Efficiency vs Leading Photon #phi;Photon Offline #phi;Efficiency",32,-3.2f,3.2f);
    efftimes[label] = fs->make<TEfficiency>(Form("efftime_%s",label.c_str()),"HLT Efficiency vs Leading Photon Seed Time [ns];Photon Offline Seed Time [ns];Efficiency",nbinsTimeX,binsTimeX);
    effHTs  [label] = fs->make<TEfficiency>(Form("effHT_%s",label.c_str()),"HLT Efficiency vs PF H_{T};Offline PF H_{T} (p_{T}^{Jet}>30, |eta^{Jet}|<3.0, Tight PFJetID);Efficiency",nbinsHTX,binsHTX);
  }
}

void HLTPlots::endJob() {}

void HLTPlots::beginRun(edm::Run const & iRun, edm::EventSetup const & iSetup) {}

void HLTPlots::endRun(edm::Run const & iRun, edm::EventSetup const & iSetup) {}

void HLTPlots::fillDescriptions(edm::ConfigurationDescriptions & descriptions)
{
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

DEFINE_FWK_MODULE(HLTPlots);
