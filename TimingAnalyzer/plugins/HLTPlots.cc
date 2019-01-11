#include "Timing/TimingAnalyzer/plugins/HLTPlots.hh"

HLTPlots::HLTPlots(const edm::ParameterSet & iConfig) :
  // pre-selection
  applyTriggerPS(iConfig.existsAs<bool>("applyTriggerPS") ? iConfig.getParameter<bool>("applyTriggerPS") : false),
  psPath        (iConfig.existsAs<std::string>("psPath")  ? iConfig.getParameter<std::string>("psPath") : "HLT_IsoMu27_v"),

  // cuts
  jetpTmin(iConfig.existsAs<double>("jetpTmin") ? iConfig.getParameter<double>("jetpTmin") : 15.0),
  jetIDmin(iConfig.existsAs<int>("jetIDmin") ? iConfig.getParameter<int>("jetIDmin") : 1),
  jetEtamax(iConfig.existsAs<int>("jetEtamax") ? iConfig.getParameter<double>("jetEtamax") : 3.0),
  phpTmin(iConfig.existsAs<double>("phpTmin") ? iConfig.getParameter<double>("phpTmin") : 20.0),
  phIDmin(iConfig.existsAs<std::string>("phIDmin") ? iConfig.getParameter<std::string>("phIDmin") : "loose"),
  
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
  
  // rho
  rhoTag(iConfig.getParameter<edm::InputTag>("rho")),
  
  // mets
  metsTag(iConfig.getParameter<edm::InputTag>("mets")),  

  // jets
  jetsTag(iConfig.getParameter<edm::InputTag>("jets")),  

  // photons
  gedPhotonsTag(iConfig.getParameter<edm::InputTag>("gedPhotons")),  
  
  // ootPhotons
  ootPhotonsTag(iConfig.getParameter<edm::InputTag>("ootPhotons")),  
  
  // recHits
  recHitsEBTag(iConfig.getParameter<edm::InputTag>("recHitsEB")),  
  recHitsEETag(iConfig.getParameter<edm::InputTag>("recHitsEE")),
  
  // tracks
  tracksTag(iConfig.getParameter<edm::InputTag>("tracks"))  
{
  // internal setup
  usesResource();
  usesResource("TFileService");
  
  // triggers
  triggerResultsToken = consumes<edm::TriggerResults> (triggerResultsTag);
  triggerObjectsToken = consumes<std::vector<pat::TriggerObjectStandAlone> > (triggerObjectsTag);
    
  // store pre-selection path if it exists
  if (applyTriggerPS && psPath != "") triggerBitMap[psPath] = false;
  
  // read in from a stream the trigger paths for saving
  oot::ReadInTriggerNames(inputPaths,pathNames,triggerBitMap);
  triggerBits.resize(pathNames.size());
  
  // read in from a stream the hlt objects/labels to match to
  oot::ReadInFilterNames(inputFilters,filterNames,triggerObjectsByFilterMap);
  
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

  // rho
  rhoToken = consumes<double> (rhoTag);

  // mets
  metsToken = consumes<std::vector<pat::MET> > (metsTag);

  // jets
  jetsToken = consumes<std::vector<pat::Jet> > (jetsTag);

  // photons
  gedPhotonsToken = consumes<std::vector<pat::Photon> > (gedPhotonsTag);
  ootPhotonsToken = consumes<std::vector<pat::Photon> > (ootPhotonsTag);

  // rechits
  recHitsEBToken = consumes<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > (recHitsEBTag);
  recHitsEEToken = consumes<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > (recHitsEETag);

  // tracks 
  tracksToken = consumes<std::vector<reco::Track> > (tracksTag);
}

HLTPlots::~HLTPlots() {}

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

  //////////////////////
  // Set trigger info //
  //////////////////////

  HLTPlots::SetTriggerInfo();

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

  // METS
  iEvent.getByToken(metsToken,metsH);
  if (oot::BadHandle(metsH,"mets")) return false;

  // JETS
  iEvent.getByToken(jetsToken,jetsH);
  if (oot::BadHandle(jetsH,"jets")) return false;

  // GED PHOTONS
  iEvent.getByToken(gedPhotonsToken,gedPhotonsH);
  if (oot::BadHandle(gedPhotonsH,"gedPhotons")) return false;

  // OOT PHOTONS
  iEvent.getByToken(ootPhotonsToken,ootPhotonsH);
  if (oot::BadHandle(ootPhotonsH,"ootPhotons")) return false;

  // RecHits
  iEvent.getByToken(recHitsEBToken,recHitsEBH);
  if (oot::BadHandle(recHitsEBH,"recHitsEB")) return false;

  iEvent.getByToken(recHitsEEToken,recHitsEEH);
  if (oot::BadHandle(recHitsEEH,"recHitsEE")) return false;

  // Tracks
  iEvent.getByToken(tracksToken,tracksH);
  if (oot::BadHandle(tracksH,"tracks")) return false;

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
}

inline bool HLTPlots::ApplyPreSelection()
{
  if (triggerBitMap.count(psPath))
  {
    return !triggerBitMap[psPath];
  }
  else return false;
}

void HLTPlots::SetTriggerInfo()
{
  HLTPlots::InitializeTriggerBranches();
  for (auto ipath = 0U; ipath < pathNames.size(); ipath++)
  {
    triggerBits[ipath] = triggerBitMap[pathNames[ipath]];
  }
}

void HLTPlots::SetJetInfo()
{
  HLTPlots::ClearJetBranches();
  njets = jets.size();
  if (njets > 0) HLTPlots::InitializeJetBranches();

  auto ijet = 0;
  for (const auto & jet : jets)
  {
    jetE  [ijet] = jet.energy();
    jetpt [ijet] = jet.pt();
    jetphi[ijet] = jet.phi();
    jeteta[ijet] = jet.eta();

    pfJetHT += jetpt[ijet];
    
    ijet++;
  } // end loop over reco jets
}
 
void HLTPlots::SetPhotonInfo()
{
  HLTPlots::ClearRecoPhotonBranches();
  nphotons = photons.size();
  if (nphotons > 0) HLTPlots::InitializeRecoPhotonBranches();
  
  auto iph = 0;
  for (const auto & photon : photons)
  {
    // from ootPhoton collection
    phisOOT[iph] = *(photon.userData<bool>("isOOT"));
    
    // standard photon branches
    phE  [iph] = photon.energy();
    phpt [iph] = oot::GetPhotonPt(photon);
    phphi[iph] = photon.phi();
    pheta[iph] = photon.eta();
    
    // check for HLT filter matches!
    strBitMap isHLTMatched;
    oot::InitializeBitMap(filterNames,isHLTMatched);
    oot::HLTToObjectMatching(triggerObjectsByFilterMap,isHLTMatched,photon,pTres,dRmin);
    for (auto ifilter = 0U; ifilter < filterNames.size(); ifilter++)
    {
      phIsHLTMatched[iph][ifilter] = isHLTMatched[filterNames[ifilter]];
    }
    
    // check for simple track veto
    phIsTrack[iph] = oot::TrackToObjectMatching(tracksH,photon,trackpTmin,trackdRmin);
    
    // super cluster from photon
    const auto & phsc = photon.superCluster().isNonnull() ? photon.superCluster() : photon.parentSuperCluster();
    phscE  [iph] = phsc->energy();
    phsceta[iph] = phsc->eta();
    phscphi[iph] = phsc->phi();
    
    // Shower Shape Objects
    const auto & phshape = photon.full5x5_showerShapeVariables(); // photon.showerShapeVariables();
    
    // ID-like variables
    phHoE[iph] = photon.hadTowOverEm(); // ID + trigger == single tower
    phr9 [iph] = photon.r9();
    
    // pseudo-track veto
    phPixSeed[iph] = photon.passElectronVeto();
    phEleVeto[iph] = photon.hasPixelSeed();
    
    // cluster shape variables
    phsieie[iph] = phshape.sigmaIetaIeta;
    phsipip[iph] = phshape.sigmaIphiIphi;
    phsieip[iph] = phshape.sigmaIetaIphi;
    
    // PF Isolations
    const auto sceta = std::abs(phsceta[iph]);
    phChgIso[iph] = std::max(photon.chargedHadronIso() - (rho * oot::GetChargedHadronEA(sceta)),0.f);
    phNeuIso[iph] = std::max(photon.neutralHadronIso() - (rho * oot::GetNeutralHadronEA(sceta)),0.f);
    phIso   [iph] = std::max(photon.photonIso()        - (rho * oot::GetGammaEA        (sceta)),0.f);
    
    // PF Cluster Isolations
    phPFClEcalIso[iph] = photon.ecalPFClusterIso();
    phPFClHcalIso[iph] = photon.hcalPFClusterIso();
    
    // Track Isolation (dR of outer cone < 0.3 as matching in trigger)
    phHollowTkIso[iph] = photon.trkSumPtHollowConeDR03();
     
    // use seed to get geometry and recHits
    const auto seedDetId = phsc->seed()->seed(); //seed detid
    const auto isEB = (seedDetId.subdetId() == EcalSubdetector::EcalBarrel); //which subdet
    const auto recHits = (isEB ? recHitsEB : recHitsEE);
    const auto seedRecHit = recHits->find(seedDetId);
    
    if (seedRecHit != recHits->end())
    {
      // save position, energy, and time of each rechit to a vector
      const auto seedPos = isEB ? barrelGeometry->getGeometry(seedDetId)->getPosition() : endcapGeometry->getGeometry(seedDetId)->getPosition();
      phseedeta [iph] = seedPos.eta();
      phseedphi [iph] = seedPos.phi();
      
      phseedE   [iph] = seedRecHit->energy();
      phseedtime[iph] = seedRecHit->time();
      phseedID  [iph] = int(seedDetId.rawId());
      phseedOOT [iph] = int(seedRecHit->checkFlag(EcalRecHit::kOutOfTime));
      
      if (recHits->size() > 0)
      {
	// radius of semi-major,minor axis is the inverse square root of the eigenvalues of the covariance matrix
	const auto ph2ndMoments = noZS::EcalClusterTools::cluster2ndMoments( *phsc, *recHits);
	phsmaj [iph] = ph2ndMoments.sMaj;
	phsmin [iph] = ph2ndMoments.sMin;
	phalpha[iph] = ph2ndMoments.alpha;
	
	// map of rec hit ids
	uiiumap phrhIDmap;
	
	// all rechits in superclusters
	const auto hitsAndFractions = phsc->hitsAndFractions();
	
	for (const auto & hitAndFraction : hitsAndFractions) // loop over all rec hits in SC
        {
	  const auto recHitId = hitAndFraction.first; // get detid of crystal
	  const auto recHit = recHits->find(recHitId); // get the underlying rechit
	  if (recHit != recHits->end()) // standard check
	  { 
	    phrhIDmap[recHitId.rawId()]++;
	  } // end standard check recHit
	} // end loop over hits and fractions
	
	phnrh[iph] = phrhIDmap.size();
      } // end check over recHits size
    } // end check over seed recHit exists
    
    iph++; // increment photon counter
  } // end loop over photon vector
}

void HLTPlots::SetGoodPhotons()
{
  for (auto iph = 0; iph < nphotons; iph++)
  {
    const auto pt = phpt[iph];
    const auto eta = std::abs(phsceta[iph]);

    if (phr9[iph] < 0.95) continue;
    if (phsmaj[iph] > 1.f) continue;
    if (phsmin[iph] > 0.3) continue;
    if (phHollowTkIso[iph] > (3.f + 0.002*pt)) continue;
    
    if (phIsTrack[iph]) continue;
    if (eta < Config::etaEBmax)
    {
      if (phHoE[iph] > 0.0396) continue;
      if (phsieie[iph] > 0.01022) continue;
      if (phChgIso[iph] > 0.441) continue;
      if (phNeuIso[iph] > (2.725+0.0148*pt+0.000017*pt*pt)) continue;
      if (phIso[iph] > (2.571+0.0047*pt)) continue;
      if (phPFClEcalIso[iph] > (2.5 + 0.01*pt)) continue;
      if (phPFClHcalIso[iph] > (6.f + 0.03*pt + 0.00003*pt*pt)) continue;

      // save index for now
      goodphs.emplace_back(iph);
    } // end check over EB
    else if ((eta > Config::etaEEmin) && (eta < Config::etaEEmax))
    {
      if (phHoE[iph] > 0.0219) continue;
      if (phsieie[iph] > 0.03001) continue;
      if (phChgIso[iph] > 0.442) continue;
      if (phNeuIso[iph] > (1.715+0.0163*pt+0.000014*pt*pt)) continue;
      if (phIso[iph] > (3.863+0.0034*pt)) continue;
      if (phPFClEcalIso[iph] > (4.f + 0.01*pt)) continue;
      if (phPFClHcalIso[iph] > (3.5 + 0.03*pt + 0.00003*pt*pt)) continue;
      
      // save index for now
      goodphs.emplace_back(iph);
    } // end check over EE
  } // end loop over photons
}

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
    
    const auto eta = std::abs(phsceta[iph]);
    if (eta < Config::etaEBmax)
    {
      effETEBs[label]->Fill(passed,phpt[iph]);
    }
    else if ((eta > Config::etaEEmin) && (eta < Config::etaEEmax))
    {
      effETEEs[label]->Fill(passed,phpt[iph]);
    }

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

void HLTPlots::ResetTestResults()
{
  for (auto & effTestPair : effTestMap)
  {
    auto & results =  effTestPair.second.results;
    
    results.passed = false;
    results.goodph = -1;
  }
}

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
    if (triggerBitMap[options.numerName])
    {
      results.passed = true;
    }
  }

  // set to leading photon
  results.goodph = denomphs[0];
}    

void HLTPlots::InitializeTriggerBranches()
{
  for (auto ipath = 0U; ipath < pathNames.size(); ipath++)
  { 
    triggerBits[ipath] = false;
  }
}

void HLTPlots::ClearJetBranches()
{
  njets = -9999;

  jetE.clear();
  jetpt.clear();
  jetphi.clear();
  jeteta.clear();

  pfJetHT = 0.f;
}

void HLTPlots::InitializeJetBranches()
{
  jetE.resize(njets);
  jetpt.resize(njets);
  jetphi.resize(njets);
  jeteta.resize(njets);

  for (auto ijet = 0; ijet < njets; ijet++)
  {
    jetE  [ijet] = -9999.f;
    jetpt [ijet] = -9999.f;
    jetphi[ijet] = -9999.f;
    jeteta[ijet] = -9999.f;
  }
}

void HLTPlots::ClearRecoPhotonBranches()
{
  nphotons = -9999;

  phisOOT.clear();

  phE.clear();
  phpt.clear();
  phphi.clear(); 
  pheta.clear(); 

  phscE.clear(); 
  phsceta.clear(); 
  phscphi.clear(); 

  phHoE.clear();
  phr9.clear();

  phPixSeed.clear();
  phEleVeto.clear();

  phChgIso.clear();
  phNeuIso.clear();
  phIso.clear();
  phPFClEcalIso.clear();
  phPFClHcalIso.clear();
  phHollowTkIso.clear();

  phsieie.clear();
  phsipip.clear();
  phsieip.clear();
  phsmaj.clear();
  phsmin.clear();
  phalpha.clear();

  phIsHLTMatched.clear();

  phIsTrack.clear();

  phnrh.clear();

  phseedeta.clear(); 
  phseedphi.clear(); 
  phseedE.clear(); 
  phseedtime.clear();
  phseedID.clear();
  phseedOOT.clear();
}

void HLTPlots::InitializeRecoPhotonBranches()
{
  phisOOT.resize(nphotons);

  phE.resize(nphotons);
  phpt.resize(nphotons);
  phphi.resize(nphotons);
  pheta.resize(nphotons);

  phscE.resize(nphotons);
  phsceta.resize(nphotons);
  phscphi.resize(nphotons);

  phHoE.resize(nphotons);
  phr9.resize(nphotons);

  phPixSeed.resize(nphotons);
  phEleVeto.resize(nphotons);

  phChgIso.resize(nphotons);
  phNeuIso.resize(nphotons);
  phIso.resize(nphotons);
  phPFClEcalIso.resize(nphotons);
  phPFClHcalIso.resize(nphotons);
  phHollowTkIso.resize(nphotons);

  phsieie.resize(nphotons);
  phsipip.resize(nphotons);
  phsieip.resize(nphotons);
  phsmaj.resize(nphotons);
  phsmin.resize(nphotons);
  phalpha.resize(nphotons);

  phIsHLTMatched.resize(nphotons);

  phIsTrack.resize(nphotons);

  phnrh.resize(nphotons);

  phseedeta.resize(nphotons);
  phseedphi.resize(nphotons);
  phseedE.resize(nphotons);
  phseedtime.resize(nphotons);
  phseedID.resize(nphotons);
  phseedOOT.resize(nphotons);

  for (auto iph = 0; iph < nphotons; iph++)
  {
    phisOOT[iph] = -1;

    phE  [iph] = -9999.f; 
    phpt [iph] = -9999.f; 
    phphi[iph] = -9999.f; 
    pheta[iph] = -9999.f; 

    phscE  [iph] = -9999.f; 
    phsceta[iph] = -9999.f; 
    phscphi[iph] = -9999.f; 

    phHoE[iph] = -9999.f;
    phr9 [iph] = -9999.f;

    phPixSeed[iph] = false;
    phEleVeto[iph] = false;

    phChgIso     [iph] = -9999.f;
    phNeuIso     [iph] = -9999.f;
    phIso        [iph] = -9999.f;
    phPFClEcalIso[iph] = -9999.f;
    phPFClHcalIso[iph] = -9999.f;
    phHollowTkIso[iph] = -9999.f;

    phsieie[iph] = -9999.f;
    phsipip[iph] = -9999.f;
    phsieip[iph] = -9999.f;
    phsmaj [iph] = -9999.f;
    phsmin [iph] = -9999.f;
    phalpha[iph] = -9999.f;

    phnrh    [iph] = -9999;

    phIsHLTMatched[iph].resize(filterNames.size());
    for (auto ifilter = 0U; ifilter < filterNames.size(); ifilter++)
    {
      phIsHLTMatched[iph][ifilter] = -1; // false
    }

    phIsTrack [iph] = -1;

    phseedeta [iph] = -9999.f;
    phseedphi [iph] = -9999.f;
    phseedE   [iph] = -9999.f;
    phseedtime[iph] = -9999.f;
    phseedID  [iph] = -9999.f;
    phseedOOT [iph] = -9999.f;
  }
}

void HLTPlots::beginJob() 
{
  edm::Service<TFileService> fs;

  const auto nbinsxET = 20;
  double xbinsET[nbinsxET+1] = {20,30,40,45,50,52,54,56,58,60,62,64,66,68,70,75,80,100,200,500,1000};

  const auto nbinsxTime = 18;
  double xbinsTime[nbinsxTime+1] = {-10,-5,-3,-2.5,-2,-1.5,-1,-0.5,0,0.5,1,1.5,2,2.5,3,5,10,15,25};

  const auto nbinsxHT = 23;
  double xbinsHT[nbinsxHT+1] = {100,150,200,250,300,325,350,375,400,425,450,475,500,525,550,575,600,625,650,700,750,1000,1500,2000};
  
  for (const auto & effTestPair : effTestMap)
  {
    const auto & label = effTestPair.first;

    effETEBs[label] = fs->make<TEfficiency>(Form("effETEB_%s",label.c_str()),"HLT Efficiency vs Leading Photon E_{T} [EB];Photon Offline E_{T};Efficiency",nbinsxET,xbinsET);
    effETEEs[label] = fs->make<TEfficiency>(Form("effETEE_%s",label.c_str()),"HLT Efficiency vs Leading Photon E_{T} [EE];Photon Offline E_{T};Efficiency",nbinsxET,xbinsET);
    effetas [label] = fs->make<TEfficiency>(Form("effeta_%s",label.c_str()),"HLT Efficiency vs Leading Photon #eta;Photon Offline #eta;Efficiency",30,-3.f,3.f);
    effphis [label] = fs->make<TEfficiency>(Form("effphi_%s",label.c_str()),"HLT Efficiency vs Leading Photon #phi;Photon Offline #phi;Efficiency",32,-3.2f,3.2f);
    efftimes[label] = fs->make<TEfficiency>(Form("efftime_%s",label.c_str()),"HLT Efficiency vs Leading Photon Seed Time [ns];Photon Offline Seed Time [ns];Efficiency",nbinsxTime,xbinsTime);
    effHTs  [label] = fs->make<TEfficiency>(Form("effHT_%s",label.c_str()),"HLT Efficiency vs PF H_{T};Offline PF H_{T} (Min PFJet p_{T} > 15);Efficiency",nbinsxHT,xbinsHT);
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
