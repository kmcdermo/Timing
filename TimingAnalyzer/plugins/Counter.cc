#include "Timing/TimingAnalyzer/plugins/Counter.hh"

Counter::Counter(const edm::ParameterSet & iConfig): 
  // matching criteria
  dRmin(iConfig.existsAs<double>("dRmin") ? iConfig.getParameter<double>("dRmin") : 0.3),
  pTmin(iConfig.existsAs<double>("pTmin") ? iConfig.getParameter<double>("pTmin") : 20.0),
  pTres(iConfig.existsAs<double>("pTres") ? iConfig.getParameter<double>("pTres") : 0.5),

  // use GED VID
  useGEDVID(iConfig.existsAs<bool>("useGEDVID") ? iConfig.getParameter<bool>("useGEDVID") : false),

  // debug it all
  debug(iConfig.existsAs<bool>("debug") ? iConfig.getParameter<bool>("debug") : false),

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

void Counter::GetObjects(const edm::Event & iEvent)
{
  // PF CANDIDATES
  iEvent.getByToken(candsToken, candsH);

  // MET
  iEvent.getByToken(metsToken, metsH);

  // GEDPHOTONS + IDS
  iEvent.getByToken(gedPhotonsToken, gedPhotonsH);

  // OOTPHOTONS + IDS
  iEvent.getByToken(ootPhotonsToken, ootPhotonsH);

  if (isMC)
  {
    // GEN EVENT RECORD
    iEvent.getByToken(genevtInfoToken, genevtInfoH);

    // PILEUP INFO
    iEvent.getByToken(pileupInfosToken, pileupInfosH);

    // GEN PARTICLES
    iEvent.getByToken(genpartsToken, genparticlesH);
  }
}

void Counter::PrepObjects()
{
  Counter::SortPhotonsByPt(gedPhotons);
  Counter::SortPhotonsByPt(ootPhotons);

  if (isMC)
  {
    // clear the neutralinos first
    neutralinos.clear();

    // match to gmsb photons
    oot::PrepNeutralinos(genparticlesH,neutralinos);
  }
}

void Counter::SortPhotonsByPt(std::vector<pat::Photon> & photons)
{
  std::sort(photons.begin(),photons.end(),
	    [](const auto & photon1, const auto & photon2)
	    {
	      return GetPhotonPt(photon1) > GetPhotonPt(photon2);
	    });
}

void Counter::PrepPhotonCollections()
{
  Counter::PrepPhotonCollection(reducedPhotons_N,matchedOOT_N,matchedLTGED_N);
  Counter::PrepPhotonCollection(reducedPhotons_L,matchedOOT_L,matchedLTGED_L);
  Counter::PrepPhotonCollection(reducedPhotons_T,matchedOOT_T,matchedLTGED_T);
}

void Counter::PrepPhotonCollection(std::vector<ReducedPhoton> & reducedPhotons, const std::vector<int> & matchedOOT, const std::vector<int> & matchedLTGED)
{
  // only take unmatched GED photons, or those that are higher pt than OOT
  for (auto iged = 0; iged < nGED_N; iged++)
  {
    const auto ioot = matchedOOT[iged];
    if (ioot >= 0)
      if (matchedLTGED[ioot] != iged) continue;

    reducedPhotons.emplace_back(iged,true);
  }

  // only take unmatched OOT photons, or those that are higher pt than GED
  for (auto ioot = 0; ioot < nOOT_N; ioot++)
  {
    if (matchedLTGED[ioot] >= 0) continue;

    reducedPhotons.emplace_back(ioot,false);
  }

  // sort them in pt
  std::sort(reducedPhotons.begin(),reducedPhotons.end(),
	    [&](const auto & reducedPhoton1, const auto & reducedPhoton2)
	    {
	      const auto & photon1 = (reducedPhoton1.isGED?(*gedPhotonsH)[reducedPhoton1.idx]:(*ootPhotons)[reducedPhoton1.idx]);
	      const auto & photon2 = (reducedPhoton2.isGED?(*gedPhotonsH)[reducedPhoton2.idx]:(*ootPhotons)[reducedPhoton2.idx]);
	      return (GetPhotonPt(photon1) > GetPhotonPt(photon2));
	    });
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

    // reset E residuals
    Counter::ResetEResiduals();

    // reset photon collections
    Counter::ResetPhotonCollections();

    // prep photons
    Counter::PrepPhotonCollections();
    
    // set E residuals
    Counter::SetEResiduals();
  }
}

void Counter::SetBasicCounters()
{
  // nGED
  nGED_N = gedPhotons.size();
  for (const auto & gedPhoton : *gedPhotonsH)
  {
    if (gedPhoton.photonID(Config::GEDPhotonLooseVID)) nGED_L++;
    if (gedPhoton.photonID(Config::GEDPhotonTightVID)) nGED_T++;
  }

  // nOOT
  nOOT_N = ootPhotons.size();
  for (const auto & ootPhoton : *ootPhotonsH)
  {
    if (ootPhoton.photonID(Config::OOTPhotonLooseVID)) nOOT_L++;
    if (ootPhoton.photonID(Config::OOTPhotonTightVID)) nOOT_T++;
  }
}

void Counter::SetPhotonIndices()
{
  Counter::SetPhotonIndices("NONE","NONE",matchedOOT_N,matchedGTGED_N,matchedLTGED_N,unmatchedGED_N,matchedCands_N);
  Counter::SetPhotonIndices((useGEDVID?Config::GEDPhotonLooseVID:"NONE"),Config::OOTPhotonLooseVID,matchedOOT_L,matchedGTGED_L,matchedLTGED_L,unmatchedGED_L,matchedCands_L);
  Counter::SetPhotonIndices((useGEDVID?Config::GEDPhotonTightVID:"NONE"),Config::OOTPhotonTightVID,matchedOOT_T,matchedGTGED_T,matchedLTGED_T,unmatchedGED_T,matchedCands_T);
}

void Counter::SetPhotonIndices(const std::string & gedVID, const std::string & ootVID, std::vector<int> & matchedOOT,
			       std::vector<int> & matchedGTGED, std::vector<int> & matchedLTGED,
			       std::vector<int> & unmatchedGED, std::vector<int> & matchedCands)
{
  // save on check size
  const auto nCands = candsH->size();

  // loop over OOT photons, setting indices
  for (auto ioot = 0; ioot < nOOT_N; ioot++)
  {
    // get ootPhoton
    const auto & ootPhoton = (*ootPhotonsH)[ioot];

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
      const auto & gedPhoton = (*gedPhotonsH)[iged];

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
      const auto & gedPhoton = (*gedPhotonsH)[matchedGED];
      matchedOOT[matchedGED] = ioot;

      if   (GetPhotonPt(ootPhoton) > GetPhotonPt(gedPhoton)) matchedGTGED[ioot] = matchedGED;
      else                                                   matchedLTGED[ioot] = matchedGED;
    }
    else // OOT photon is truly unmatched to GED
    {
      // mask it for later use
      unmatchedGED[ioot] = 1;
      
      // get oot photon pt
      const auto ootPt = GetPhotonPt(ootPhoton);

      // loop over pf cands, checking to see if it is picked up by a PF candidate
      for (auto icand = 0U; icand < nCands; icand++)
      {
	// get pf cand + pt
	const auto & cand = cands[icand];
	const auto candpt = cand.pt();

	// forget about the junk
	if (cand.pt() < pTmin) continue;
	
	// pt resolution
	if (ootPt < ((1.f-pTres) * candpt)) continue;
	if (ootPt > ((1.f+pTres) * candpt)) continue;
	
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
    if (index >= 0) counter++;
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
    if (matchedLTGED[ioot] >= 0) continue;

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
  t1pfMETpt = t1pfMET.pt();
  t1pfMETphi = t1pfMET.phi();

  if (isMC)
  {
    const auto & genMET = *(t1pfMET.genMET());
    genMETpt = genMET.pt();
    genMETphi = genMET.phi();
  }
  
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
    if (matchedLTGED[ioot] >= 0) continue;

    // get ootPhoton info --> will use it regardless!
    const auto & ootPhoton = ootPhotons[ioot];
    const auto ootpt  = GetPhotonPt(ootPhoton);
    const auto ootphi = ootPhoton.phi();
    
    // first, change MET with overlapping OOT photons; then check if totally unmatched
    if (matchedGTGED[ioot] >= 0)
    {
      // get gedPhoton info
      const auto & gedPhoton = gedPhotons[matchedGTGED[ioot]];
      const auto gedpt  = GetPhotonPt(gedPhoton);
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

void Counter::SetEResiduals()
{
  Counter::SetEResiduals(reducedPhotons_N,beforeGEDEres_N,afterGEDEres_N,beforeOOTEres_N,afterOOTEres_N);
  Counter::SetEResiduals(reducedPhotons_L,beforeGEDEres_L,afterGEDEres_L,beforeOOTEres_L,afterOOTEres_L);
  Counter::SetEResiduals(reducedPhotons_T,beforeGEDEres_T,afterGEDEres_T,beforeOOTEres_T,afterOOTEres_T);
}

void Counter::SetEResiduals(const std::vector<ReducedPhoton> & reducedPhotons,
			    std::vector<float> & beforeGEDEres, std::vector<float> & afterGEDEres,
			    std::vector<float> & beforeOOTEres, std::vector<float> & afterOOTEres)
{
  // save on size check
  const auto nPhos = reducedPhotons.size();

  // loop over neutralinos, find best photon, compute before and after E residual
  for (const auto & neutralino : neutralinos)
  {
    // get photon daughter stuff
    const auto phdaughter  =  (neutralino.daughter(0)->pdgId() == 22)?0:1;
    const auto & genPhoton = *(neutralino.daughter(phdaughter));

    // temp variables for checking
    auto mindR = dRmin;
    auto matchedIdx = -1;

    // loop over all photons saved
    for (auto ipho = 0U; ipho < nPhos; ipho++)
    {
      // get photon
      const auto & reducedPhoton = reducedPhotons[ipho];
      const auto & photon = (reducedPhoton.isGED?gedPhotons[reducedPhoton.idx]:ootPhotons[reducedPhoton.idx]);
      
      // test for dR matching
      const auto phodR = reco::deltaR(genPhoton,photon);
      if (phodR < mindR) 
      {
	matchedIdx = ipho;
	mindR = phodR;
      } // end check over deltaR
    } // end loop over reco photons

    // check for match
    if (matchedIdx < 0) continue;

    // get photon
    const auto & reducedPhoton = reducedPhotons[matchedIdx];
    const auto & photon = (reducedPhoton.isGED?gedPhotons[reducedPhoton.idx]:ootPhotons[reducedPhoton.idx]);

    // split saving of vars by OOT
    if (reducedPhoton.isGED)
    {
      beforeGEDEres.emplace_back(genPhoton.energy() - photon.energy());
      afterGEDEres .emplace_back(genPhoton.energy() - photon.userFloat("ecalEnergyPostCorr"));
    }
    else
    {
      beforeOOTEres.emplace_back(genPhoton.energy() - photon.energy());
      afterOOTEres .emplace_back(genPhoton.energy() - photon.userFloat("ecalEnergyPostCorr"));
    }
  } // end loop over neutralinos
}

//////////////////////
// Helper Functions //
//////////////////////

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

void Counter::ResetEResiduals()
{
  Counter::ResetPhotonVars(beforeGEDEres_N);
  Counter::ResetPhotonVars(afterGEDEres_N);
  Counter::ResetPhotonVars(beforeOOTEres_N);
  Counter::ResetPhotonVars(afterOOTEres_N);

  Counter::ResetPhotonVars(beforeGEDEres_L);
  Counter::ResetPhotonVars(afterGEDEres_L);
  Counter::ResetPhotonVars(beforeOOTEres_L);
  Counter::ResetPhotonVars(afterOOTEres_L);

  Counter::ResetPhotonVars(beforeGEDEres_T);
  Counter::ResetPhotonVars(afterGEDEres_T);
  Counter::ResetPhotonVars(beforeOOTEres_T);
  Counter::ResetPhotonVars(afterOOTEres_T);
}

void Counter::ResetPhotonVars(std::vector<float> & vars)
{
  vars.clear();
}

void Counter::ResetPhotonCollections()
{
  Counter::ResetPhotonCollection(reducedPhotons_N);
  Counter::ResetPhotonCollection(reducedPhotons_L);
  Counter::ResetPhotonCollection(reducedPhotons_T);
}

void Counter::ResetPhotonCollection(std::vector<ReducedPhoton> & reducedPhotons)
{
  reducedPhotons.clear();
}

/////////////////////
// DEBUG FUNCTIONS //
/////////////////////

void Counter::DumpPhotons(const edm::Event & iEvent)
{
  std::cout << "------------------- EvtID: " << iEvent.id().event() << " -------------------" << std::endl;
  for (auto iged = 0; iged < nGED_N; iged++)
  {
    const auto & gedPhoton = gedPhotons[iged];
    const auto loose = static_cast<int>(gedPhoton.photonID(Config::GEDPhotonLooseVID));
    const auto tight = static_cast<int>(gedPhoton.photonID(Config::GEDPhotonTightVID));

    Counter::DumpPhoton(iged,gedPhoton,"iged: "," Loose: "+std::to_string(loose)+" Tight: "+std::to_string(tight));
  }

  std::cout << std::endl;

  for (auto ioot = 0; ioot < nOOT_N; ioot++)
  {
    const auto & ootPhoton = ootPhotons[ioot];
    const auto loose = static_cast<int>(ootPhoton.photonID(Config::OOTPhotonLooseVID));
    const auto tight = static_cast<int>(ootPhoton.photonID(Config::OOTPhotonTightVID));

    Counter::DumpPhoton(ioot,ootPhoton,"ioot: "," Loose: "+std::to_string(loose)+" Tight: "+std::to_string(tight));
  }

  std::cout << std::endl;

  Counter::DumpPhotons("NONE",matchedOOT_N,matchedGTGED_N,matchedLTGED_N,unmatchedGED_N);
  Counter::DumpPhotons("LOOSE",matchedOOT_L,matchedGTGED_L,matchedLTGED_L,unmatchedGED_L);
  Counter::DumpPhotons("TIGHT",matchedOOT_T,matchedGTGED_T,matchedLTGED_T,unmatchedGED_T);

  std::cout << "-----------------------------------------------" << std::endl;
}

void Counter::DumpPhotons(const std::string & group, const std::vector<int> & matchedOOT,
			  const std::vector<int> & matchedGTGED, const std::vector<int> & matchedLTGED,
			  const std::vector<int> & unmatchedGED)
{
  std::cout << "GROUP: " << group.c_str() << std::endl;
  Counter::DumpPhotons(gedPhotons,nGED_N,matchedOOT,true,"matchedOOT",ootPhotons);
  Counter::DumpPhotons(ootPhotons,nOOT_N,matchedGTGED,true,"matchedGTGED",gedPhotons);
  Counter::DumpPhotons(ootPhotons,nOOT_N,matchedLTGED,true,"matchedLTGED",gedPhotons);
  Counter::DumpPhotons(ootPhotons,nOOT_N,unmatchedGED,false,"unmatchedGED",gedPhotons);
  std::cout << std::endl;
}

void Counter::DumpPhotons(const std::vector<pat::Photon> & photons, const int size, 
			  const std::vector<int> & indices, const bool check, const std::string & label,
			  const std::vector<pat::Photon> & refPhotons)
{
  std::cout << "========= " << label.c_str() << " =========" << std::endl;
  for (auto ipho = 0; ipho < size; ipho++)
  {
    const auto & photon = photons[ipho];
    const auto index    = indices[ipho];

    if (index >= 0)
    {
      Counter::DumpPhoton(ipho,photon,"ipho: ","");
      if (check)
      {
	const auto & refPhoton = refPhotons[index];
	Counter::DumpPhoton(index,refPhoton,"  MATCHED --> ipho: "," delR: "+std::to_string(reco::deltaR(photon,refPhoton)));
      }
    }
  }
  std::cout << "================================" << std::endl;
}

void Counter::DumpPhoton(const int i, const pat::Photon & photon, const std::string & prefix, const std::string & suffix)
{
  std::cout << prefix.c_str() << "i: " << i 
	    << " phi: " << photon.phi() << " eta: " << photon.eta() << " pt: " << photon.pt()
	    << suffix.c_str() << std::endl;
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

  if (isMC)
  {
    tree->Branch("genMETpt", &genMETpt, "genMETpt/F");
    tree->Branch("genMETphi", &genMETphi, "genMETphi/F");
  }

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

    tree->Branch("beforeGEDEres_N", &beforeGEDEres_N);
    tree->Branch("afterGEDEres_N", &afterGEDEres_N);
    tree->Branch("beforeOOTEres_N", &beforeOOTEres_N);
    tree->Branch("afterOOTEres_N", &afterOOTEres_N);

    tree->Branch("beforeGEDEres_L", &beforeGEDEres_L);
    tree->Branch("afterGEDEres_L", &afterGEDEres_L);
    tree->Branch("beforeOOTEres_L", &beforeOOTEres_L);
    tree->Branch("afterOOTEres_L", &afterOOTEres_L);

    tree->Branch("beforeGEDEres_T", &beforeGEDEres_T);
    tree->Branch("afterGEDEres_T", &afterGEDEres_T);
    tree->Branch("beforeOOTEres_T", &beforeOOTEres_T);
    tree->Branch("afterOOTEres_T", &afterOOTEres_T);
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
