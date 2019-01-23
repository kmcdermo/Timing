#include "Timing/TimingAnalyzer/plugins/Counter.hh"

////////////////////////
// Internal Functions //
////////////////////////

Counter::Counter(const edm::ParameterSet & iConfig): 
  // matching criteria
  dRmin(iConfig.existsAs<double>("dRmin") ? iConfig.getParameter<double>("dRmin") : 0.3),
  pTmin(iConfig.existsAs<double>("pTmin") ? iConfig.getParameter<double>("pTmin") : 20.0),

  // use VID
  useGEDVID(iConfig.existsAs<bool>("useGEDVID") ? iConfig.getParameter<bool>("useGEDVID") : false),
  useOOTVID(iConfig.existsAs<bool>("useOOTVID") ? iConfig.getParameter<bool>("useOOTVID") : false),

  // debug it all
  debug(iConfig.existsAs<bool>("debug") ? iConfig.getParameter<bool>("debug") : false),

  // MC config
  isMC(iConfig.existsAs<bool>("isMC") ? iConfig.getParameter<bool>("isMC") : false),

  // mets
  metsTag(iConfig.getParameter<edm::InputTag>("mets")),  

  // photons
  gedPhotonsTag(iConfig.getParameter<edm::InputTag>("gedPhotons")),
  ootPhotonsTag(iConfig.getParameter<edm::InputTag>("ootPhotons"))
{
  // internal setup
  usesResource();
  usesResource("TFileService");

  // mets
  metsToken = consumes<std::vector<pat::MET> > (metsTag);

  // photons
  gedPhotonsToken = consumes<std::vector<pat::Photon> > (gedPhotonsTag);
  ootPhotonsToken = consumes<std::vector<pat::Photon> > (ootPhotonsTag);

  // setup photon infos
  Counter::SetInternalInfo();
}

Counter::~Counter() {}

void Counter::analyze(const edm::Event & iEvent, const edm::EventSetup & iSetup) 
{
  ///////////////////////
  // Get Event Objects //
  ///////////////////////

  if (!Counter::GetObjects(iEvent)) return;

  ////////////////////////
  // Initialize Objects //
  ////////////////////////

  Counter::InitializeObjects();

  //////////////////
  // Prep Objects //
  //////////////////

  Counter::PrepObjects();

  ////////////////////
  // Set Event Info //
  ////////////////////

  Counter::SetEventInfo();

  ///////////
  // Debug //
  ///////////

  if (debug) Counter::DumpPhotons(iEvent);

  ///////////////
  // Fill Tree //
  ///////////////

  tree->Fill();
}

//////////////////////////
// Event Prep Functions //
//////////////////////////

bool Counter::GetObjects(const edm::Event & iEvent)
{
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

void Counter::InitializeObjects()
{
  // GED PHOTONS
  gedInfo.photons.clear();
  gedInfo.photons = *gedPhotonsH;

  // OOT PHOTONS
  ootInfo.photons.clear();
  ootInfo.photons = *ootPhotonsH;
}

void Counter::PrepObjects()
{
  // Ensure photons sorted by modified pt
  oot::SortPhotonsByPt(gedInfo.photons);
  oot::SortPhotonsByPt(ootInfo.photons);
}

///////////////////
// Main Function //
///////////////////

void Counter::SetEventInfo()
{
  // reset counters
  Counter::ResetCounters();

  // set basic counters
  Counter::SetBasicCounters();

  // reset photon indices
  Counter::ResetOverlapIndices();

  // set photon indices
  Counter::SetOverlapIndices();

  // set photon counters
  Counter::SetOverlapCounters();

  // reset photon sets
  Counter::ResetPhotonSets();

  // set the photon keep/drop sets
  Counter::SetPhotonSets();

  // set photon set counters
  Counter::SetPhotonSetCounters();

  // set met info
  Counter::SetMETInfo();
}

////////////////////////
// Set Basic Counters //
////////////////////////

void Counter::SetBasicCounters()
{
  Counter::SetBasicCounters(gedInfo);
  Counter::SetBasicCounters(ootInfo);
}

void Counter::SetBasicCounters(PhotonInfo & photonInfo)
{
  // get photons + matching info refs
  const auto & photons   = photonInfo.photons;
  auto & matchingInfoMap = photonInfo.matchingInfoMap;

  // get size of photons
  matchingInfoMap[IDType::N].npho = photons.size();

  // loop over photons to check for VID
  for (const auto & photon : photons)
  {
    if (photon.photonID(matchingInfoMap[IDType::L].baseVID)) matchingInfoMap[IDType::L].npho++;
    if (photon.photonID(matchingInfoMap[IDType::T].baseVID)) matchingInfoMap[IDType::T].npho++;
  }
}

/////////////////////////
// Set Overlap Indices //
/////////////////////////

void Counter::SetOverlapIndices()
{
  Counter::SetBaseToTestIndices(gedInfo,ootInfo); // GED Overlaps With OOT
  Counter::SetBaseToTestIndices(ootInfo,gedInfo); // OOT Overlaps With GED

  Counter::SetBaseToBaseIndices(gedInfo); // Self overlaps : GED
  Counter::SetBaseToBaseIndices(ootInfo); // Self overlaps : OOT
}

// count overlaps with another collection
void Counter::SetBaseToTestIndices(PhotonInfo & basePhotonInfo, const PhotonInfo & testPhotonInfo)
{
  // get photon refs
  const auto & basePhotons = basePhotonInfo.photons;
  const auto & testPhotons = testPhotonInfo.photons;

  // get matching info refs
  auto & baseMatchingInfoMap = basePhotonInfo.matchingInfoMap;
  const auto & testMatchingInfoMap = testPhotonInfo.matchingInfoMap;

  // get photon counts
  const auto nbase = baseMatchingInfoMap.at(IDType::N).npho;
  const auto ntest = testMatchingInfoMap.at(IDType::N).npho;

  // loop over matching map (VIDs)
  for (auto & baseMatchingInfoPair : baseMatchingInfoMap)
  {
    // get value
    auto & baseMatchingInfo = baseMatchingInfoPair.second;

    // get IDs
    const auto & baseVID = baseMatchingInfo.baseVID;
    const auto & testVID = baseMatchingInfo.testVID;

    // get indices
    auto & bases_to_tests_GT = baseMatchingInfo.bases_to_tests_GT;
    auto & bases_to_tests_LT = baseMatchingInfo.bases_to_tests_LT;
    auto & bases_unmatched   = baseMatchingInfo.bases_unmatched;

    // loop over base photons, setting indices
    for (auto ibase = 0; ibase < nbase; ibase++)
    {
      // get base photon
      const auto & basePhoton = basePhotons[ibase];

      // cut on baseVID
      if (baseVID != "NONE") 
	if (!basePhoton.photonID(baseVID)) continue;

      // tmp variables for matching to tests
      std::vector<int> base_to_tests;

      // loop over test photons, looking for all within dR
      for (auto itest = 0; itest < ntest; itest++)
      {
	// get test photon
	const auto & testPhoton = testPhotons[itest];

	// cut on VID
	if (testVID != "NONE") 
	  if (!testPhoton.photonID(testVID)) continue;
      
	// check for delR match!
	const auto phodR = reco::deltaR(basePhoton,testPhoton);
	if (phodR < dRmin)
	{
	  // save it!
	  base_to_tests.emplace_back(itest);
	} // end check over deltaR match
      } // end loop over test photons

      // assign pT masking
      if (base_to_tests.size() > 0)
      {
	for (const auto itest : base_to_tests)
	{
	  const auto & testPhoton = testPhotons[itest];
	
	  if   (oot::GetPhotonPt(basePhoton) > oot::GetPhotonPt(testPhoton)) bases_to_tests_GT[ibase].emplace_back(itest);
	  else                                                               bases_to_tests_LT[ibase].emplace_back(itest);
	}
      }
      else // base photon is truly unmatched to test photons
      {
	// mask it for later use
	bases_unmatched[ibase] = 1;
      } // end block over unmatched phos
    } // end loop over base photons
  } // end loop over VIDs
}

// count self-overlaps
void Counter::SetBaseToBaseIndices(PhotonInfo & basePhotonInfo)
{
  // get photon refs
  const auto & basePhotons = basePhotonInfo.photons;

  // get matching info ref
  auto & baseMatchingInfoMap = basePhotonInfo.matchingInfoMap;
 
  // get photon counts
  const auto nbase = baseMatchingInfoMap.at(IDType::N).npho;

  // loop over matching map (VIDs)
  for (auto & baseMatchingInfoPair : baseMatchingInfoMap)
  {
    // get value
    auto & baseMatchingInfo = baseMatchingInfoPair.second;

    // get ID
    const auto & baseVID = baseMatchingInfo.baseVID;

    // get indices
    auto & bases_to_bases = baseMatchingInfo.bases_to_bases;

    // loop over base photons, setting indices
    for (auto ibase = 0; ibase < nbase; ibase++)
    {
      // get base photon
      const auto & basePhoton = basePhotons[ibase];
      
      // cut on ootVID
      if (baseVID != "NONE")
	if (!basePhoton.photonID(baseVID)) continue;

      // loop over base photons again, only looking for UNIQUE matches (no need to double count)
      for (auto jbase = ibase+1; jbase < nbase; jbase++)
      {
	// get test photon
	const auto & testPhoton = basePhotons[jbase];
	
	// cut on VID
	if (baseVID != "NONE")
	  if (!testPhoton.photonID(baseVID)) continue;
      
	// check for delR match!
	const auto phodR = reco::deltaR(basePhoton,testPhoton);
	if (phodR < dRmin)
	{
	  // save it!
	  bases_to_bases[ibase].emplace_back(jbase);
	} // end check over deltaR match
      } // end loop over test photons
    } // end loop over base photons
  } // end loop over VIDs
}

//////////////////////////
// Set Overlap Counters //
//////////////////////////

void Counter::SetOverlapCounters()
{
  Counter::SetOverlapCounters(gedInfo);
  Counter::SetOverlapCounters(ootInfo);
}

void Counter::SetOverlapCounters(PhotonInfo & photonInfo)
{
  // loop over matching map (VIDs)
  for (auto & matchingInfoPair : photonInfo.matchingInfoMap)
  {
    // get value
    auto & matchingInfo = matchingInfoPair.second;

    // set counters
    Counter::SetOverlapCounter(matchingInfo.bases_to_tests_GT,matchingInfo.nbase_to_tests_GT);
    Counter::SetOverlapCounter(matchingInfo.bases_to_tests_LT,matchingInfo.nbase_to_tests_LT);
    Counter::SetOverlapCounter(matchingInfo.bases_unmatched  ,matchingInfo.nbase_unmatched);
    Counter::SetOverlapCounter(matchingInfo.bases_to_bases   ,matchingInfo.nbase_to_bases);
  }
}

void Counter::SetOverlapCounter(const std::vector<std::vector<int> > & indicesvec, std::vector<int> & counter)
{
  for (const auto & indices : indicesvec)
  {
    counter.emplace_back(indices.size());
  }
}

void Counter::SetOverlapCounter(const std::vector<int> & indices, int & counter)
{
  for (const auto & index : indices)
  {
    if (index > 0) counter++;
  }
}

/////////////////////
// Set Photon Sets //
/////////////////////

void Counter::SetPhotonSets()
{
  Counter::SetPhotonSets(gedInfo,ootInfo);
  Counter::SetPhotonSets(ootInfo,gedInfo);
}

void Counter::SetPhotonSets(PhotonInfo & basePhotonInfo, const PhotonInfo & testPhotonInfo)
{
  // get size
  const auto nbase = basePhotonInfo.matchingInfoMap.at(N).npho;

  // loop over matching map (VIDs) from base
  for (auto & baseMatchingInfoPair : basePhotonInfo.matchingInfoMap)
  {
    // get key + value
    const auto & ID         = baseMatchingInfoPair.first;
    auto & baseMatchingInfo = baseMatchingInfoPair.second;

    // get indices (base)
    const auto & bases_to_tests_GT = baseMatchingInfo.bases_to_tests_GT;
    const auto & bases_to_tests_LT = baseMatchingInfo.bases_to_tests_LT;
    const auto & bases_unmatched   = baseMatchingInfo.bases_unmatched;

    // get sets
    auto & bases_to_keep = baseMatchingInfo.bases_to_keep;
    auto & bases_to_drop = baseMatchingInfo.bases_to_drop;

    // loop over base indices first
    for (auto ibase = 0; ibase < nbase; ibase++)
    {
      // to keep first
      if (bases_to_tests_GT[ibase].size() > 0) bases_to_keep.emplace(ibase);
      if (bases_unmatched[ibase] > 0)          bases_to_keep.emplace(ibase);   

      // to drop second
      if (bases_to_tests_LT[ibase].size() > 0) bases_to_drop.emplace(ibase);
    } // end loop over N base photons

    // get corresponding test info --> cross-check: should not be different than base
    const auto & testMatchingInfo = testPhotonInfo.matchingInfoMap.at(ID);

    // get indices (test)
    const auto & tests_to_bases_GT = testMatchingInfo.bases_to_tests_GT;
    const auto & tests_to_bases_LT = testMatchingInfo.bases_to_tests_LT;
    
    // to keep first
    for (const auto & test_to_bases_LT : tests_to_bases_LT)
    {
      for (const auto ibase : test_to_bases_LT)
      {
	bases_to_keep.emplace(ibase);
      } // end loop over base indices in test vectors
    } // end loop over test indices

    // to drop second
    for (const auto & test_to_bases_GT : tests_to_bases_GT)
    {
      for (const auto ibase : test_to_bases_GT)
      {
	bases_to_drop.emplace(ibase);
      } // end loop over base indices in test vectors
    } // end loop over test indices

  } // end loop over VID map
}

/////////////////////////////
// Set Photon Set Counters //
/////////////////////////////

void Counter::SetPhotonSetCounters()
{
  Counter::SetPhotonSetCounters(gedInfo);
  Counter::SetPhotonSetCounters(ootInfo);
}

void Counter::SetPhotonSetCounters(PhotonInfo & photonInfo)
{
  // get photons + matching info refs
  auto & matchingInfoMap = photonInfo.matchingInfoMap;

  // get size of photons
  const auto npho = matchingInfoMap[IDType::N].npho;

  // loop over matching map (VIDs)
  for (auto & matchingInfoPair : matchingInfoMap)
  {
    // get value
    auto & matchingInfo = matchingInfoPair.second;

    // get sets
    const auto & bases_to_keep = matchingInfo.bases_to_keep;
    const auto & bases_to_drop = matchingInfo.bases_to_drop;

    // set simple counters first
    matchingInfo.nbase_to_keep = bases_to_keep.size();
    matchingInfo.nbase_to_drop = bases_to_drop.size();

    // cross-check for no overlaps
    for (auto ipho = 0; ipho < npho; ipho++)
    {
      if (bases_to_keep.count(ipho) && bases_to_drop.count(ipho)) matchingInfo.nbase_to_both++; // SHOULD NEVER HAPPEN
    } // end loop over nphos
  } // end loop over VID map
}

//////////////////
// Set MET Info //
//////////////////

void Counter::SetMETInfo()
{
  // reco met
  const auto & t1pfMET = (*metsH).front();
  t1pfMETpt = t1pfMET.pt();
  t1pfMETphi = t1pfMET.phi();

  // gen met
  if (isMC)
  {
    const auto & genMET = *(t1pfMET.genMET());
    genMETpt = genMET.pt();
    genMETphi = genMET.phi();
  }

  // compute corrected MET
  Counter::SetCorrectedMET(N,ootMETpt_N,ootMETphi_N);
  Counter::SetCorrectedMET(L,ootMETpt_L,ootMETphi_L);
  Counter::SetCorrectedMET(T,ootMETpt_T,ootMETphi_T);
}

void Counter::SetCorrectedMET(const IDType ID, float & ootMETpt, float & ootMETphi)
{
  // initialize px, py
  auto ootMETpx = t1pfMETpt * std::cos(t1pfMETphi);
  auto ootMETpy = t1pfMETpt * std::sin(t1pfMETphi);

  // get input photons
  const auto & gedPhotons = gedInfo.photons;
  const auto & ootPhotons = ootInfo.photons;

  // get matching info maps
  const auto & gedMatchingInfo = gedInfo.matchingInfoMap.at(ID);
  const auto & ootMatchingInfo = ootInfo.matchingInfoMap.at(ID);

  // get photon sets
  const auto & geds_to_drop = gedMatchingInfo.bases_to_drop;
  const auto & oots_to_keep = ootMatchingInfo.bases_to_keep;

  // change the MET --> drop GED first
  for (const auto iged : geds_to_drop)
  {
    const auto & gedPhoton = gedPhotons[iged];
    const auto gedpt  = oot::GetPhotonPt(gedPhoton);
    const auto gedphi = gedPhoton.phi();
    
    ootMETpx += (gedpt * std::cos(gedphi));
    ootMETpy += (gedpt * std::sin(gedphi));
  }

  // change the MET --> add OOT second
  for (const auto ioot : oots_to_keep)
  {
    const auto & ootPhoton = ootPhotons[ioot];
    const auto ootpt  = oot::GetPhotonPt(ootPhoton);
    const auto ootphi = ootPhoton.phi();
    
    ootMETpx -= (ootpt * std::cos(ootphi));
    ootMETpy -= (ootpt * std::sin(ootphi));
  }
      
  // set the new met!
  ootMETpt  = Config::hypo(ootMETpx,ootMETpy);
  ootMETphi = Config::phi (ootMETpx,ootMETpy);
}

////////////////////
// Reset Counters //
////////////////////

void Counter::ResetCounters()
{
  Counter::ResetCounters(gedInfo);
  Counter::ResetCounters(ootInfo);
}

void Counter::ResetCounters(PhotonInfo & photonInfo)
{
  // loop over matching map (VIDs)
  for (auto & matchingInfoPair : photonInfo.matchingInfoMap)
  {
    // get value
    auto & matchingInfo = matchingInfoPair.second;

    // set counters
    Counter::ResetCounter(matchingInfo.npho);
    Counter::ResetCounter(matchingInfo.nbase_to_tests_GT);
    Counter::ResetCounter(matchingInfo.nbase_to_tests_LT);
    Counter::ResetCounter(matchingInfo.nbase_unmatched);
    Counter::ResetCounter(matchingInfo.nbase_to_bases);
    Counter::ResetCounter(matchingInfo.nbase_to_keep);
    Counter::ResetCounter(matchingInfo.nbase_to_drop);
    Counter::ResetCounter(matchingInfo.nbase_to_both);
  }
}

void Counter::ResetCounter(std::vector<int> & counter)
{
  counter.clear();
}

void Counter::ResetCounter(int & counter)
{
  counter = 0;
}

///////////////////
// Reset Indices //
///////////////////

void Counter::ResetOverlapIndices()
{
  Counter::ResetOverlapIndices(gedInfo);
  Counter::ResetOverlapIndices(ootInfo);
}

void Counter::ResetOverlapIndices(PhotonInfo & photonInfo)
{
  // get matching map ref
  auto & matchingInfoMap = photonInfo.matchingInfoMap;

  // get photon counts
  const auto npho = matchingInfoMap.at(IDType::N).npho;

  // loop over matching map (VIDs)
  for (auto & matchingInfoPair : matchingInfoMap)
  {
    // get value
    auto & matchingInfo = matchingInfoPair.second;

    // set counters
    Counter::ResetOverlapIndices(matchingInfo.bases_to_tests_GT,npho);
    Counter::ResetOverlapIndices(matchingInfo.bases_to_tests_LT,npho);
    Counter::ResetOverlapIndices(matchingInfo.bases_unmatched  ,npho);
    Counter::ResetOverlapIndices(matchingInfo.bases_to_bases   ,npho);
  }
}

void Counter::ResetOverlapIndices(std::vector<std::vector<int> > & indices, const int size)
{
  // clear and resize
  indices.clear();
  indices.resize(size);
}

void Counter::ResetOverlapIndices(std::vector<int> & indices, const int size)
{
  // resize and initialize
  indices.resize(size);
  for (auto & index : indices) index = -1;
}

///////////////////////
// Reset Photon Sets //
///////////////////////

void Counter::ResetPhotonSets()
{
  Counter::ResetPhotonSets(gedInfo);
  Counter::ResetPhotonSets(ootInfo);
}

void Counter::ResetPhotonSets(PhotonInfo & photonInfo)
{
  // get matching map ref
  auto & matchingInfoMap = photonInfo.matchingInfoMap;

  // loop over matching map (VIDs)
  for (auto & matchingInfoPair : matchingInfoMap)
  {
    // get value
    auto & matchingInfo = matchingInfoPair.second;

    Counter::ResetPhotonSet(matchingInfo.bases_to_keep);
    Counter::ResetPhotonSet(matchingInfo.bases_to_drop);
  }
}

void Counter::ResetPhotonSet(std::unordered_set<int> & indices)
{
  indices.clear();
}

/////////////////////
// DEBUG FUNCTIONS //
/////////////////////

void Counter::DumpPhotons(const edm::Event & iEvent)
{
  std::cout << "------------------- EvtID: " << iEvent.id().event() << " -------------------" << std::endl;

  Counter::DumpPhotons(gedInfo,ootInfo.photons);
  Counter::DumpPhotons(ootInfo,gedInfo.photons);

  std::cout << "-----------------------------------------------" << std::endl;
}

void Counter::DumpPhotons(const PhotonInfo & basePhotonInfo, const std::vector<pat::Photon> & testPhotons)
{
  // get photon refs
  const auto & basePhotons = basePhotonInfo.photons;

  // get matching info map refs
  const auto & baseMatchingInfoMap = basePhotonInfo.matchingInfoMap;

  // get photon counts
  const auto nbase = baseMatchingInfoMap.at(IDType::N).npho;

  // skip if no photons found
  if (nbase == 0) return;

  // print out header
  std::cout << "========== " << basePhotonInfo.label.c_str() << " Photons ==========" << std::endl;

  // loop over base photons, setting indices
  for (auto ibase = 0; ibase < nbase; ibase++)
  {
    // get base photon
    const auto & basePhoton = basePhotons[ibase];
    const auto loose = basePhoton.photonID(baseMatchingInfoMap.at(IDType::L).baseVID);
    const auto tight = basePhoton.photonID(baseMatchingInfoMap.at(IDType::T).baseVID);

    // dump basic info
    Counter::DumpPhoton(ibase,basePhoton," "," Loose: "+std::to_string(loose)+" Tight: "+std::to_string(tight));
    
    // dump matching info
    for (const auto & baseMatchingInfoPair : baseMatchingInfoMap)
    {
      // get matching info
      const auto & baseMatchingInfo = baseMatchingInfoPair.second;

      // dump it
      Counter::DumpPhotons(ibase,basePhoton,basePhotons,testPhotons,baseMatchingInfo);
    }
  }
}

void Counter::DumpPhotons(const int ibase, const pat::Photon & basePhoton,
			  const std::vector<pat::Photon> & basePhotons,
			  const std::vector<pat::Photon> & testPhotons,
			  const MatchingInfo & baseMatchingInfo)
{
  // get label
  const auto & label = baseMatchingInfo.label;

  // get indices
  const auto & base_to_tests_GT = baseMatchingInfo.bases_to_tests_GT[ibase];
  const auto & base_to_tests_LT = baseMatchingInfo.bases_to_tests_LT[ibase];
  const auto & base_unmatched   = baseMatchingInfo.bases_unmatched  [ibase];
  const auto & base_to_bases    = baseMatchingInfo.bases_to_bases   [ibase];
  
  // get sets
  const auto & bases_to_keep = baseMatchingInfo.bases_to_keep;
  const auto & bases_to_drop = baseMatchingInfo.bases_to_drop;

  // dump photons from indices : overlaps
  Counter::DumpPhotons(basePhoton,testPhotons,base_to_tests_GT,"GT ["+label+"]");
  Counter::DumpPhotons(basePhoton,testPhotons,base_to_tests_LT,"LT ["+label+"]");

  // dump weird situation
  if (bases_to_keep.count(ibase) && bases_to_drop.count(ibase))
    std::cout << " ==> ERROR: Photon has matched both greater than pT and lower pT than in test photons!" << std::endl;

  // dump if unmatched
  if (base_unmatched > 0) std::cout << "  --> Not matched to any test photon ["+label+"]." << std::endl;

  // dump self overlaps
  Counter::DumpPhotons(basePhoton,basePhotons,base_to_bases,"Self ["+label+"]");
}

void Counter::DumpPhotons(const pat::Photon & basePhoton, const std::vector<pat::Photon> & testPhotons, 
			  const std::vector<int> & indices, const std::string & text)
{
  // loop indices
  for (const auto itest : indices)
  {
    // get "test" photon
    const auto & testPhoton = testPhotons[itest];

    // delR
    std::stringstream stream;
    stream << std::setprecision(3) << reco::deltaR(basePhoton,testPhoton);

    // dump it
    Counter::DumpPhoton(itest,testPhoton,"  --> MATCH with "+text+" "," (delR: "+stream.str()+")");
  }
}

void Counter::DumpPhoton(const int i, const pat::Photon & photon, const std::string & prefix, const std::string & suffix)
{
  std::cout << prefix.c_str() << "i: " << i 
	    << " phi: " << std::setprecision(3) << photon.phi()
	    << " eta: " << std::setprecision(3) << photon.eta()
	    << " pt: "  << std::setprecision(3) << photon.pt()
	    << suffix.c_str() << std::endl;
}

//////////////////////////////
// Internal Setup Functions //
//////////////////////////////

void Counter::SetInternalInfo()
{
  // GED
  gedInfo.matchingInfoMap[IDType::N].baseVID = "NONE";
  gedInfo.matchingInfoMap[IDType::L].baseVID = Config::GEDPhotonLooseVID;
  gedInfo.matchingInfoMap[IDType::T].baseVID = Config::GEDPhotonTightVID;

  gedInfo.matchingInfoMap[IDType::N].testVID = "NONE";
  gedInfo.matchingInfoMap[IDType::L].testVID = (useOOTVID?Config::OOTPhotonLooseVID:"NONE");
  gedInfo.matchingInfoMap[IDType::T].testVID = (useOOTVID?Config::OOTPhotonTightVID:"NONE");

  // OOT
  ootInfo.matchingInfoMap[IDType::N].baseVID = "NONE";
  ootInfo.matchingInfoMap[IDType::L].baseVID = Config::OOTPhotonLooseVID;
  ootInfo.matchingInfoMap[IDType::T].baseVID = Config::OOTPhotonTightVID;

  ootInfo.matchingInfoMap[IDType::N].testVID = "NONE";
  ootInfo.matchingInfoMap[IDType::L].testVID = (useGEDVID?Config::GEDPhotonLooseVID:"NONE");
  ootInfo.matchingInfoMap[IDType::T].testVID = (useGEDVID?Config::GEDPhotonTightVID:"NONE");

  // set labels
  Counter::SetLabels(gedInfo,"GED");
  Counter::SetLabels(ootInfo,"OOT");
}

void Counter::SetLabels(PhotonInfo & photonInfo, const std::string & label)
{
  photonInfo.label = label;
  photonInfo.matchingInfoMap[IDType::N].label = "N";
  photonInfo.matchingInfoMap[IDType::L].label = "L";
  photonInfo.matchingInfoMap[IDType::T].label = "T";
}

//////////////////////////
// Setup Tree Functions //
//////////////////////////

void Counter::MakeTree()
{
  edm::Service<TFileService> fs;

  // Event tree: N = None, L = Loose, T = Tight
  tree = fs->make<TTree>("tree","tree");

  // counter info
  Counter::InitBranches();

  // MET info
  tree->Branch("t1pfMETpt", &t1pfMETpt);
  tree->Branch("t1pfMETphi", &t1pfMETphi);

  if (isMC)
  {
    tree->Branch("genMETpt", &genMETpt);
    tree->Branch("genMETphi", &genMETphi);
  }

  tree->Branch("ootMETpt_N", &ootMETpt_N);
  tree->Branch("ootMETphi_N", &ootMETphi_N);

  tree->Branch("ootMETpt_L", &ootMETpt_L);
  tree->Branch("ootMETphi_L", &ootMETphi_L);

  tree->Branch("ootMETpt_T", &ootMETpt_T);
  tree->Branch("ootMETphi_T", &ootMETphi_T);
}

void Counter::InitBranches()
{
  Counter::InitBranches(gedInfo,"ged","oot");
  Counter::InitBranches(ootInfo,"oot","ged");
}

void Counter::InitBranches(PhotonInfo & photonInfo, const std::string & base, const std::string & test)
{
  // loop matching map and set branches
  for (auto & matchingInfoPair : photonInfo.matchingInfoMap)
  {
    // get matching info
    auto & matchingInfo = matchingInfoPair.second;

    // get label
    const auto & label = matchingInfo.label;

    // set branches
    const auto npho = "n"+base+"_"+label;
    tree->Branch(npho.c_str(),&matchingInfo.npho);

    const auto nbase_to_tests_GT = "n"+base+"_to_"+test+"s_GT_"+label;
    tree->Branch(nbase_to_tests_GT.c_str(),&matchingInfo.nbase_to_tests_GT);
    
    const auto nbase_to_tests_LT = "n"+base+"_to_"+test+"s_LT_"+label; 
    tree->Branch(nbase_to_tests_LT.c_str(),&matchingInfo.nbase_to_tests_LT);

    const auto nbase_unmatched = "n"+base+"_unmatched_"+label;
    tree->Branch(nbase_unmatched.c_str(),&matchingInfo.nbase_unmatched);

    const auto nbase_to_bases = "n"+base+"_to_"+base+"s_"+label;
    tree->Branch(nbase_to_bases.c_str(),&matchingInfo.nbase_to_bases);

    const auto nbase_to_keep = "n"+base+"_to_keep_"+label;
    tree->Branch(nbase_to_keep.c_str(),&matchingInfo.nbase_to_keep);

    const auto nbase_to_drop = "n"+base+"_to_drop_"+label;
    tree->Branch(nbase_to_drop.c_str(),&matchingInfo.nbase_to_drop);

    const auto nbase_to_both = "n"+base+"_to_both_"+label;
    tree->Branch(nbase_to_both.c_str(),&matchingInfo.nbase_to_both);
  }
}

////////////////////////
// Internal Functions //
////////////////////////

void Counter::beginJob()
{
  Counter::MakeTree();
}

void Counter::endJob() {}

void Counter::beginRun(const edm::Run & iRun, const edm::EventSetup & iSetup) {}

void Counter::endRun(const edm::Run & iRun, const edm::EventSetup & iSetup) {}

void Counter::fillDescriptions(edm::ConfigurationDescriptions & descriptions) 
{
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

DEFINE_FWK_MODULE(Counter);
