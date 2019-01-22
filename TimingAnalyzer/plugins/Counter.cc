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

  matchingInfoMap[N].npho = photons.size();
  for (const auto & photon : photons)
  {
    if (photon.photonID(matchingInfoMap[L].baseVID)) matchingInfoMap[L].npho++;
    if (photon.photonID(matchingInfoMap[T].baseVID)) matchingInfoMap[T].npho++;
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
  const auto nbase = baseMatchingInfoMap.at(N).npho;
  const auto ntest = testMatchingInfoMap.at(N).npho;

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
  const auto nbase = baseMatchingInfoMap.at(N).npho;

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

//////////////////
// Set MET Info //
//////////////////

void Counter::SetMETInfo()
{
  const auto & t1pfMET = (*metsH).front();
  t1pfMETpt = t1pfMET.pt();
  t1pfMETphi = t1pfMET.phi();

  if (isMC)
  {
    const auto & genMET = *(t1pfMET.genMET());
    genMETpt = genMET.pt();
    genMETphi = genMET.phi();
  }
  
//   Counter::SetCorrectedMET(matchedGTGED_N,matchedLTGED_N,unmatchedGED_N,ootMETpt_N,ootMETphi_N);
//   Counter::SetCorrectedMET(matchedGTGED_L,matchedLTGED_L,unmatchedGED_L,ootMETpt_L,ootMETphi_L);
//   Counter::SetCorrectedMET(matchedGTGED_T,matchedLTGED_T,unmatchedGED_T,ootMETpt_T,ootMETphi_T);
}

// void Counter::SetCorrectedMET(const std::vector<int> & matchedGTGED, const std::vector<int> & matchedLTGED,
// 			      const std::vector<int> & unmatchedGED, float & ootMETpt, float & ootMETphi)
// {
//   // initialize
//   ootMETpt  = t1pfMETpt;
//   ootMETphi = t1pfMETphi;

//   // change the MET
//   for (auto ioot = 0; ioot < nOOT_N; ioot++)
//   {
//     // skip if matched, but lower in pT
//     if (matchedLTGED[ioot] >= 0) continue;

//     // get ootPhoton info --> will use it regardless!
//     const auto & ootPhoton = ootPhotons[ioot];
//     const auto ootpt  = oot::GetPhotonPt(ootPhoton);
//     const auto ootphi = ootPhoton.phi();
    
//     // first, change MET with overlapping OOT photons; then check if totally unmatched
//     if (matchedGTGED[ioot] >= 0)
//     {
//       // get gedPhoton info
//       const auto & gedPhoton = gedPhotons[matchedGTGED[ioot]];
//       const auto gedpt  = oot::GetPhotonPt(gedPhoton);
//       const auto gedphi = gedPhoton.phi();

//       // get compnonents
//       const auto x = ootMETpt*std::cos(ootMETphi) - ootpt*std::cos(ootphi) + gedpt*std::cos(gedphi);
//       const auto y = ootMETpt*std::sin(ootMETphi) - ootpt*std::sin(ootphi) + gedpt*std::sin(gedphi);
  
//       // make new met
//       ootMETpt  = Config::hypo(x,y);
//       ootMETphi = Config::phi (x,y);
//     } // end check over overlapping OOT photons
//     else if (unmatchedGED[ioot] > 0)
//     {
//       // get compnonents
//       const auto x = ootMETpt*std::cos(ootMETphi) - ootpt*std::cos(ootphi);
//       const auto y = ootMETpt*std::sin(ootMETphi) - ootpt*std::sin(ootphi);
      
//       // make new met
//       ootMETpt  = Config::hypo(x,y);
//       ootMETphi = Config::phi (x,y);
//     } // end check over non-overlapping OOT photons
//   } // end loop over OOT photons
// }

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
  const auto npho = matchingInfoMap.at(N).npho;

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
  std::cout << "========== " << basePhotonInfo.label.c_str() << " Photons ==========" << std::endl;

  // get photon refs
  const auto & basePhotons = basePhotonInfo.photons;

  // get matching info map refs
  const auto & baseMatchingInfoMap = basePhotonInfo.matchingInfoMap;

  // get photon counts
  const auto nbase = baseMatchingInfoMap.at(N).npho;

  // loop over base photons, setting indices
  for (auto ibase = 0; ibase < nbase; ibase++)
  {
    // get base photon
    const auto & basePhoton = basePhotons[ibase];
    const auto loose = basePhoton.photonID(baseMatchingInfoMap.at(L).baseVID);
    const auto tight = basePhoton.photonID(baseMatchingInfoMap.at(T).baseVID);

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
  
  // dump photons from indices : overlaps
  Counter::DumpPhotons(basePhoton,testPhotons,base_to_tests_GT,"GT Test ["+label+"]");
  Counter::DumpPhotons(basePhoton,testPhotons,base_to_tests_LT,"LT Test ["+label+"]");

  // dump if unmatched
  if (base_unmatched) std::cout << "  --> Not matched to any test photon ["+label+"]." << std::endl;

  // dump self overlaps
  Counter::DumpPhotons(basePhoton,basePhotons,base_to_bases,"Self Test ["+label+"]");
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
    Counter::DumpPhoton(itest,testPhoton,"  --> MATCH ","("+text+", delR: "+stream.str()+")");
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
  gedInfo.matchingInfoMap[N].baseVID = "NONE";
  gedInfo.matchingInfoMap[L].baseVID = Config::GEDPhotonLooseVID;
  gedInfo.matchingInfoMap[T].baseVID = Config::GEDPhotonTightVID;

  gedInfo.matchingInfoMap[N].testVID = "NONE";
  gedInfo.matchingInfoMap[L].testVID = (useOOTVID?Config::OOTPhotonLooseVID:"NONE");
  gedInfo.matchingInfoMap[T].testVID = (useOOTVID?Config::OOTPhotonTightVID:"NONE");

  // OOT
  ootInfo.matchingInfoMap[N].baseVID = "NONE";
  ootInfo.matchingInfoMap[L].baseVID = Config::OOTPhotonLooseVID;
  ootInfo.matchingInfoMap[T].baseVID = Config::OOTPhotonTightVID;

  ootInfo.matchingInfoMap[N].testVID = "NONE";
  ootInfo.matchingInfoMap[L].testVID = (useGEDVID?Config::GEDPhotonLooseVID:"NONE");
  ootInfo.matchingInfoMap[T].testVID = (useGEDVID?Config::GEDPhotonTightVID:"NONE");

  // set labels
  Counter::SetLabels(gedInfo,"GED");
  Counter::SetLabels(ootInfo,"OOT");
}

void Counter::SetLabels(PhotonInfo & photonInfo, const std::string & label)
{
  photonInfo.label = label;
  photonInfo.matchingInfoMap[N].label = "N";
  photonInfo.matchingInfoMap[L].label = "L";
  photonInfo.matchingInfoMap[T].label = "T";
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
