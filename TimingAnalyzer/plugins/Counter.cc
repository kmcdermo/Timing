#include "Timing/TimingAnalyzer/plugins/Counter.hh"

Counter::Counter(const edm::ParameterSet & iConfig): 
  // matching criteria
  dRmin(iConfig.existsAs<double>("dRmin") ? iConfig.getParameter<double>("dRmin") : 0.3),
  pTmin(iConfig.existsAs<double>("pTmin") ? iConfig.getParameter<double>("pTmin") : 20.0),
  pTres(iConfig.existsAs<double>("pTres") ? iConfig.getParameter<double>("pTres") : 1.0),

  // cands
  candsTag(iConfig.getParameter<edm::InputTag>("cands")),

  // mets
  metsTag(iConfig.getParameter<edm::InputTag>("mets")),  

  // photons
  gedPhotonsTag(iConfig.getParameter<edm::InputTag>("gedPhotons")),
  ootPhotonsTag(iConfig.getParameter<edm::InputTag>("ootPhotons")),

  // MC objects
  genevtInfoTag(iConfig.getParameter<edm::InputTag>("genevt")),
  pileupInfosTag(iConfig.getParameter<edm::InputTag>("pileup")),
  genpartsTag(iConfig.getParameter<edm::InputTag>("genparts")),

  // MC config
  isMC(iConfig.existsAs<bool>("isMC") ? iConfig.getParameter<bool>("isMC") : false),
  xsec(iConfig.existsAs<double>("xsec") ? iConfig.getParameter<double>("xsec") : 1.0),
  BR(iConfig.existsAs<double>("BR") ? iConfig.getParameter<double>("BR") : 1.0)
{
  usesResource();
  usesResource("TFileService");

  // cands 
  candsToken = consumes<std::vector<pat::PackedCandidate> > (candsTag);

  // mets
  metsToken = consumes<std::vector<pat::MET> > (metsTag);

  // photons
  gedPhotonsToken = consumes<std::vector<pat::Photon> > (gedPhotonsTag);
  ootPhotonsToken = consumes<std::vector<pat::Photon> > (ootPhotonsTag);

  // MC
  if (isMC)
  {
    genevtInfoToken  = consumes<GenEventInfoProduct>             (genevtInfoTag);
    pileupInfosToken = consumes<std::vector<PileupSummaryInfo> > (pileupInfosTag);
    genpartsToken    = consumes<std::vector<reco::GenParticle> > (genpartsTag);
  }
}

Counter::~Counter() {}

void Counter::analyze(const edm::Event & iEvent, const edm::EventSetup & iSetup) 
{
  ///////////////////////
  // Get Event Objects //
  ///////////////////////

  Counter::GetObjects(iEvent);

  //////////////////
  // Prep Objects //
  //////////////////

  Counter::PrepObjects();

  ////////////////////
  // Set Event Info //
  ////////////////////

  Counter::SetEventInfo();

  ///////////////
  // Fill Tree //
  ///////////////

  tree->Fill();
}

//////////////////////////
// Event Prep Functions //
//////////////////////////

void Counter::GetObjects(const edm::Event & iEvent)
{
  // PF CANDIDATES
  iEvent.getByToken(candsToken, candsH);
  cands = *candsH;

  // MET
  iEvent.getByToken(metsToken, metsH);
  mets = *metsH;

  // GEDPHOTONS + IDS
  iEvent.getByToken(gedPhotonsToken, gedPhotonsH);
  gedPhotons = *gedPhotonsH;

  // OOTPHOTONS + IDS
  iEvent.getByToken(ootPhotonsToken, ootPhotonsH);
  ootPhotons = *ootPhotonsH;

  if (isMC)
  {
    // GEN EVENT RECORD
    iEvent.getByToken(genevtInfoToken, genevtInfoH);
    genevtInfo = *genevtInfoH;

    // PILEUP INFO
    iEvent.getByToken(pileupInfosToken, pileupInfosH);
    pileupInfos = *pileupInfosH;

    // GEN PARTICLES
    iEvent.getByToken(genpartsToken, genparticlesH);
    genparticles = *genparticlesH;
  }
}

void Counter::PrepObjects()
{
  if (isMC)
  {
    // match to gmsb photons
    oot::PrepNeutralinos(genparticlesH,neutralinos);
  }
}

////////////////////
// Main Functions //
////////////////////

void Counter::SetEventInfo()
{
  // reset counters
  Counter::ResetCounters();

  // set basic counters
  Counter::SetBasicCounters();

  // reset photon indices
  Counter::ResetPhotonIndices();

  // set photon indices
  Counter::SetPhotonIndices();

  // set photon counters
  Counter::SetPhotonCounters();

  // reset photon phis
  Counter::ResetPhotonPhis();

  // set photon phis
  Counter::SetPhotonPhis();

  // set met info
  Counter::SetMETInfo();

  if (isMC)
  { 
    // set event record info
    Counter::SetMCInfo();

    // set pt residuals
    //    Counter::SetPtResiduals();
  }
}

void Counter::SetBasicCounters()
{
  // nGED
  nGED_N = gedPhotons.size();
  for (const auto & gedPhoton : gedPhotons)
  {
    if (gedPhoton.photonID(Config::GEDPhotonLooseVID)) nGED_L++;
    if (gedPhoton.photonID(Config::GEDPhotonTightVID)) nGED_T++;
  }

  // nOOT
  nOOT_N = ootPhotons.size();
  for (const auto & ootPhoton : ootPhotons)
  {
    if (ootPhoton.photonID(Config::OOTPhotonLooseVID)) nOOT_L++;
    if (ootPhoton.photonID(Config::OOTPhotonTightVID)) nOOT_T++;
  }
}

void Counter::SetPhotonIndices()
{
  Counter::SetPhotonIndices("NONE","NONE",matchedOOT_N,matchedGTGED_N,matchedLTGED_N,unmatchedGED_N,matchedCands_N);
  Counter::SetPhotonIndices(Config::GEDPhotonLooseVID,Config::OOTPhotonLooseVID,matchedOOT_L,matchedGTGED_L,matchedLTGED_L,unmatchedGED_L,matchedCands_L);
  Counter::SetPhotonIndices(Config::GEDPhotonTightVID,Config::OOTPhotonTightVID,matchedOOT_T,matchedGTGED_T,matchedLTGED_T,unmatchedGED_T,matchedCands_T);
}

void Counter::SetPhotonIndices(const std::string & gedVID, const std::string & ootVID, std::vector<int> & matchedOOT,
			       std::vector<int> & matchedGTGED, std::vector<int> & matchedLTGED,
			       std::vector<int> & unmatchedGED, std::vector<int> & matchedCands)
{
  // save on check size
  const auto nCands = cands.size();

  // loop over OOT photons, setting indices
  for (auto ioot = 0; ioot < nOOT_N; ioot++)
  {
    // get ootPhoton
    const auto & ootPhoton = ootPhotons[ioot];

    // cut on ootVID
    if (ootVID != "NONE") 
      if (!ootPhoton.photonID(ootVID)) continue;

    // temporaries needed for matching to GED
    auto matchedGED = -1;
    auto mindR = dRmin;

    // loop over GED photons, looking for best dR match
    for (auto iged = 0; iged < nGED_N; iged++)
    {
      // skip if already matched
      if (matchedOOT[iged] >= 0) continue;

      // get gedPhoton
      const auto & gedPhoton = gedPhotons[iged];

      // cut on VID
      if (gedVID != "NONE") 
	if (!gedPhoton.photonID(gedVID)) continue;
      
      // cut on lowest deltaR
      const auto phodR = reco::deltaR(ootPhoton,gedPhoton);
      if (phodR < mindR)
      {
	// set tmp index
	matchedGED = iged;

	// set next min threshold
	mindR = phodR;
      } // end check over deltaR match
    } // end loop over ged photons

    // assign pT masking and GED photon masking (for skipping on next OOT)
    if (matchedGED >= 0) 
    {
      const auto & gedPhoton = gedPhotons[matchedGED];
      matchedOOT[matchedGED] = ioot;

      if   (Counter::GetPhotonPt(ootPhoton) > Counter::GetPhotonPt(gedPhoton)) matchedGTGED[ioot] = matchedGED;
      else                                                                     matchedLTGED[ioot] = matchedGED;
    }
    else // OOT photon is truly unmatched to GED
    {
      // mask it for later use
      unmatchedGED[ioot] = 1;

      // loop over pf cands, checking to see if it is picked up by a PF candidate
      for (auto icand = 0U; icand < nCands; icand++)
      {
	// get pf cand
	const auto & cand = cands[icand];

	// forget about the junk
	if (cand.pt() < pTmin) continue;
	
	// just check to see if it is matched just once
	if (reco::deltaR(ootPhoton,cand) < dRmin)
	{
	  matchedCands[ioot] = icand;
	  break;
	} // end deltaR match to pfcand
      } // end loop over pf cands
    } // end block over unmatched OOT phos
  } // end loop over ootPhotons
}

void Counter::SetPhotonCounters()
{
  Counter::SetPhotonCounter(matchedGTGED_N,nOOT_matchedGTGED_N);
  Counter::SetPhotonCounter(matchedLTGED_N,nOOT_matchedLTGED_N);
  Counter::SetPhotonCounter(unmatchedGED_N,nOOT_unmatchedGED_N);
  Counter::SetPhotonCounter(matchedCands_N,nOOT_matchedCands_N);

  Counter::SetPhotonCounter(matchedGTGED_L,nOOT_matchedGTGED_L);
  Counter::SetPhotonCounter(matchedLTGED_L,nOOT_matchedLTGED_L);
  Counter::SetPhotonCounter(unmatchedGED_L,nOOT_unmatchedGED_L);
  Counter::SetPhotonCounter(matchedCands_L,nOOT_matchedCands_L);

  Counter::SetPhotonCounter(matchedGTGED_T,nOOT_matchedGTGED_T);
  Counter::SetPhotonCounter(matchedLTGED_T,nOOT_matchedLTGED_T);
  Counter::SetPhotonCounter(unmatchedGED_T,nOOT_unmatchedGED_T);
  Counter::SetPhotonCounter(matchedCands_T,nOOT_matchedCands_T);
}

void Counter::SetPhotonCounter(const std::vector<int> & indices, int & counter)
{
  for (const auto index : indices)
  {
    if (index > 0) counter++;
  }
}

void Counter::SetPhotonPhis()
{
  Counter::SetPhotonPhis(matchedGTGED_N,matchedLTGED_N,unmatchedGED_N,matchedGTGEDphi_N,unmatchedGEDphi_N);
  Counter::SetPhotonPhis(matchedGTGED_L,matchedLTGED_L,unmatchedGED_L,matchedGTGEDphi_L,unmatchedGEDphi_L);
  Counter::SetPhotonPhis(matchedGTGED_T,matchedLTGED_T,unmatchedGED_T,matchedGTGEDphi_T,unmatchedGEDphi_T);
}

void Counter::SetPhotonPhis(const std::vector<int> & matchedGTGED, const std::vector<int> & matchedLTGED, const std::vector<int> & unmatchedGED, 
			    std::vector<float> & matchedGTGEDphi, std::vector<float> & unmatchedGEDphi)
{
  for (auto ioot = 0; ioot < nOOT_N; ioot++)
  {
    // skip if matched, but lower in pT
    if (matchedLTGED[ioot] < 0) continue;

    // get ootPhoton info --> will use it regardless!
    const auto & ootPhoton = ootPhotons[ioot];
    const auto ootphi = ootPhoton.phi();

    // set phis
    if      (matchedGTGED[ioot] >= 0) matchedGTGEDphi.emplace_back(ootphi);
    else if (unmatchedGED[ioot] >  0) unmatchedGEDphi.emplace_back(ootphi);
  }
}

void Counter::SetMETInfo()
{
  const auto & t1pfMET = mets.front();
  const auto & genMET = *(t1pfMET.genMET());
  t1pfMETpt = t1pfMET.pt();
  t1pfMETphi = t1pfMET.phi();
  genMETpt = genMET.pt();
  genMETphi = genMET.phi();
  
  Counter::SetCorrectedMET(matchedGTGED_N,matchedLTGED_N,unmatchedGED_N,ootMETpt_N,ootMETphi_N);
  Counter::SetCorrectedMET(matchedGTGED_L,matchedLTGED_L,unmatchedGED_L,ootMETpt_L,ootMETphi_L);
  Counter::SetCorrectedMET(matchedGTGED_T,matchedLTGED_T,unmatchedGED_T,ootMETpt_T,ootMETphi_T);
}

void Counter::SetCorrectedMET(const std::vector<int> & matchedGTGED, const std::vector<int> & matchedLTGED,
			      const std::vector<int> & unmatchedGED, float & ootMETpt, float & ootMETphi)
{
  // initialize
  ootMETpt  = t1pfMETpt;
  ootMETphi = t1pfMETphi;

  // change the MET
  for (auto ioot = 0; ioot < nOOT_N; ioot++)
  {
    // skip if matched, but lower in pT
    if (matchedLTGED[ioot] < 0) continue;

    // get ootPhoton info --> will use it regardless!
    const auto & ootPhoton = ootPhotons[ioot];
    const auto ootpt  = Counter::GetPhotonPt(ootPhoton);
    const auto ootphi = ootPhoton.phi();
    
    // first, change MET with overlapping OOT photons; then check if totally unmatched
    if (matchedGTGED[ioot] >= 0)
    {
      // get gedPhoton info
      const auto & gedPhoton = gedPhotons[matchedGTGED[ioot]];
      const auto gedpt  = Counter::GetPhotonPt(gedPhoton);
      const auto gedphi = gedPhoton.phi();

      // get compnonents
      const auto x = ootMETpt*std::cos(ootMETphi) - ootpt*std::cos(ootphi) + gedpt*std::cos(gedphi);
      const auto y = ootMETpt*std::sin(ootMETphi) - ootpt*std::sin(ootphi) + gedpt*std::sin(gedphi);
  
      // make new met
      ootMETpt  = Config::hypo(x,y);
      ootMETphi = Config::phi (x,y);
    } // end check over overlapping OOT photons
    else if (unmatchedGED[ioot] > 0)
    {
      // get compnonents
      const auto x = ootMETpt*std::cos(ootMETphi) - ootpt*std::cos(ootphi);
      const auto y = ootMETpt*std::sin(ootMETphi) - ootpt*std::sin(ootphi);
      
      // make new met
      ootMETpt  = Config::hypo(x,y);
      ootMETphi = Config::phi (x,y);
    } // end check over non-overlapping OOT photons
  } // end loop over OOT photons
}

void Counter::SetMCInfo()
{
  genwgt = genevtInfo.weight();
  for (const auto & pileupInfo : pileupInfos)
  {
    if (pileupInfo.getBunchCrossing() == 0) 
    {
      genputrue = pileupInfo.getTrueNumInteractions();
      break;
    } // end check over correct BX
  } // end loop over PU
}

// void Counter::PrepPhotonCollection()
// {
//   Counter::PrepPhotonCollection(photons_N,matchedOOT_N,matchedGTGED_N)


// }

// void Counter::SetPtResiduals()
// {
//   Counter::SetPtResiduals(afterGEDptres_N,beforeOOTptres_N,afterOOTptres_N);
//   Counter::SetPtResiduals(matchedOOT_L,matchedGTGED_L,beforeGEDptres_L,afterGEDptres_L,beforeOOTptres_L,afterOOTptres_L);
//   Counter::SetPtResiduals(matchedOOT_T,matchedGTGED_T,beforeGEDptres_T,afterGEDptres_T,beforeOOTptres_T,afterOOTptres_T);
// }

// void Counter::SetPtResiduals(const std::vector<int> & matchedOOT, const std::vector<int> & matchedGTGED,
// 			     const std::vector<float> & beforeGEDptres, const std::vector<float> & afterGEDptres,
// 			     const std::vector<float> & beforeOOTptres, const std::vector<float> & afterOOTptres)
// {
//   // loop over neutralinos, find best photon, compute before and after pt residual
//   for (const auto & neutralino : neutralinos)
//   {
//     // get photon daughter stuff
//     const auto phdaughter  =  (neutralino.daughter(0)->pdgId() == 22)?0:1;
//     const auto & genPhoton = *(neutralino.daughter(phdaughter));

//     // check for a reco match!
//     auto mindR = gendRmin;
    
//     // First check 
//     for (auto iged = 0; iged < nGED_N; iged++)
//     {
//       // skip GED photons that are matched OOT, but OOT pt > GED pt
//       const auto ootMatched = matchedOOT[iged];
//       if (ootMatched >= 0)
// 	if (matchedGTGED[ootMatched] >= 0) continue;

//       // get gedPhoton
//       const auto & gedPhoton = gedPhotons[iged];

//       // 
//       const auto phodR = reco::deltaR(genPhoton,gedPhoton);
//       if (phodR < mindR) 
//       {
// 	mindR = phodR;
//       } // end check over deltaR
//     } // end loop over reco photons


//   } // end loop over neutralinos
// }

//////////////////////
// Helper Functions //
//////////////////////

inline float Counter::GetPhotonPt(const pat::Photon & photon)
{
  return (photon.userFloat("ecalEnergyPostCorr")/photon.energy())*photon.pt();
}

void Counter::ResetCounters()
{
  Counter::ResetCounter(nGED_N);
  Counter::ResetCounter(nOOT_N);
  Counter::ResetCounter(nOOT_matchedGTGED_N);
  Counter::ResetCounter(nOOT_matchedLTGED_N);
  Counter::ResetCounter(nOOT_unmatchedGED_N);
  Counter::ResetCounter(nOOT_matchedCands_N);

  Counter::ResetCounter(nGED_L);
  Counter::ResetCounter(nOOT_L);
  Counter::ResetCounter(nOOT_matchedGTGED_L);
  Counter::ResetCounter(nOOT_matchedLTGED_L);
  Counter::ResetCounter(nOOT_unmatchedGED_L);
  Counter::ResetCounter(nOOT_matchedCands_L);
 
  Counter::ResetCounter(nGED_T);
  Counter::ResetCounter(nOOT_T);
  Counter::ResetCounter(nOOT_matchedGTGED_T);
  Counter::ResetCounter(nOOT_matchedLTGED_T);
  Counter::ResetCounter(nOOT_unmatchedGED_T);
  Counter::ResetCounter(nOOT_matchedCands_T);
}

void Counter::ResetCounter(int & counter)
{
  counter = 0;
}

void Counter::ResetPhotonIndices()
{
  Counter::ResetPhotonIndices(matchedOOT_N,nGED_N);
  Counter::ResetPhotonIndices(matchedGTGED_N,nOOT_N);
  Counter::ResetPhotonIndices(matchedLTGED_N,nOOT_N);
  Counter::ResetPhotonIndices(unmatchedGED_N,nOOT_N);
  Counter::ResetPhotonIndices(matchedCands_N,nOOT_N);

  Counter::ResetPhotonIndices(matchedOOT_L,nGED_N);
  Counter::ResetPhotonIndices(matchedGTGED_L,nOOT_N);
  Counter::ResetPhotonIndices(matchedLTGED_L,nOOT_N);
  Counter::ResetPhotonIndices(unmatchedGED_L,nOOT_N);
  Counter::ResetPhotonIndices(matchedCands_L,nOOT_N);

  Counter::ResetPhotonIndices(matchedOOT_T,nGED_N);
  Counter::ResetPhotonIndices(matchedGTGED_T,nOOT_N);
  Counter::ResetPhotonIndices(matchedLTGED_T,nOOT_N);
  Counter::ResetPhotonIndices(unmatchedGED_T,nOOT_N);
  Counter::ResetPhotonIndices(matchedCands_T,nOOT_N);
}

void Counter::ResetPhotonIndices(std::vector<int> & indices, const int size)
{
  indices.resize(size); // loop over photons of one type in the event
  for (auto & index : indices) index = -1;
}

void Counter::ResetPhotonPhis()
{
  Counter::ResetPhotonVars(matchedGTGEDphi_N);
  Counter::ResetPhotonVars(unmatchedGEDphi_N);

  Counter::ResetPhotonVars(matchedGTGEDphi_L);
  Counter::ResetPhotonVars(unmatchedGEDphi_L);

  Counter::ResetPhotonVars(matchedGTGEDphi_T);
  Counter::ResetPhotonVars(unmatchedGEDphi_T);
}

void Counter::ResetPhotonVars(std::vector<float> & vars)
{
  vars.clear();
}

////////////////////////
// Internal Functions //
////////////////////////

void Counter::beginJob()
{
  edm::Service<TFileService> fs;
  
  // Event tree: N = None, L = Loose, T = Tight
  tree = fs->make<TTree>("tree","tree");

  // counter info
  tree->Branch("nGED_N", &nGED_N, "nGED_N/I");
  tree->Branch("nOOT_N", &nOOT_N, "nOOT_N/I");
  tree->Branch("nOOT_matchedGTGED_N", &nOOT_matchedGTGED_N, "nOOT_matchedGTGED_N/I");
  tree->Branch("nOOT_matchedLTGED_N", &nOOT_matchedLTGED_N, "nOOT_matchedLTGED_N/I");
  tree->Branch("nOOT_unmatchedGED_N", &nOOT_unmatchedGED_N, "nOOT_unmatchedGED_N/I");
  tree->Branch("nOOT_matchedCands_N", &nOOT_matchedCands_N, "nOOT_matchedCands_N/I");

  tree->Branch("nGED_L", &nGED_L, "nGED_L/I");
  tree->Branch("nOOT_L", &nOOT_L, "nOOT_L/I");
  tree->Branch("nOOT_matchedGTGED_L", &nOOT_matchedGTGED_L, "nOOT_matchedGTGED_L/I");
  tree->Branch("nOOT_matchedLTGED_L", &nOOT_matchedLTGED_L, "nOOT_matchedLTGED_L/I");
  tree->Branch("nOOT_unmatchedGED_L", &nOOT_unmatchedGED_L, "nOOT_unmatchedGED_L/I");
  tree->Branch("nOOT_matchedCands_L", &nOOT_matchedCands_L, "nOOT_matchedCands_L/I");
 
  tree->Branch("nGED_T", &nGED_T, "nGED_T/I");
  tree->Branch("nOOT_T", &nOOT_T, "nOOT_T/I");
  tree->Branch("nOOT_matchedGTGED_T", &nOOT_matchedGTGED_T, "nOOT_matchedGTGED_T/I");
  tree->Branch("nOOT_matchedLTGED_T", &nOOT_matchedLTGED_T, "nOOT_matchedLTGED_T/I");
  tree->Branch("nOOT_unmatchedGED_T", &nOOT_unmatchedGED_T, "nOOT_unmatchedGED_T/I");
  tree->Branch("nOOT_matchedCands_T", &nOOT_matchedCands_T, "nOOT_matchedCands_T/I");

  // phi info
  tree->Branch("matchedGTGEDphi_N", &matchedGTGEDphi_N);
  tree->Branch("unmatchedGEDphi_N", &unmatchedGEDphi_N);

  tree->Branch("matchedGTGEDphi_L", &matchedGTGEDphi_L);
  tree->Branch("unmatchedGEDphi_L", &unmatchedGEDphi_L);

  tree->Branch("matchedGTGEDphi_T", &matchedGTGEDphi_T);
  tree->Branch("unmatchedGEDphi_T", &unmatchedGEDphi_T);

  // MET info
  tree->Branch("t1pfMETpt", &t1pfMETpt, "t1pfMETpt/F");
  tree->Branch("t1pfMETphi", &t1pfMETphi, "t1pfMETphi/F");
  tree->Branch("genMETpt", &genMETpt, "genMETpt/F");
  tree->Branch("genMETphi", &genMETphi, "genMETphi/F");

  tree->Branch("ootMETpt_N", &ootMETpt_N, "ootMETpt_N/F");
  tree->Branch("ootMETphi_N", &ootMETphi_N, "ootMETphi_N/F");

  tree->Branch("ootMETpt_L", &ootMETpt_L, "ootMETpt_L/F");
  tree->Branch("ootMETphi_L", &ootMETphi_L, "ootMETphi_L/F");

  tree->Branch("ootMETpt_T", &ootMETpt_T, "ootMETpt_T/F");
  tree->Branch("ootMETphi_T", &ootMETphi_T, "ootMETphi_T/F");

  // MC info
  if (isMC)
  {
    tree->Branch("genwgt", &genwgt, "genwgt/F");
    tree->Branch("xsec", &xsec, "xsec/F");
    tree->Branch("BR", &BR, "BR/F");
    tree->Branch("genputrue", &genputrue, "genputrue/I");
  }
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
