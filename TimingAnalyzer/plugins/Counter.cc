#include "Timing/TimingAnalyzer/plugins/Counter.hh"

Counter::Counter(const edm::ParameterSet& iConfig): 
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
  pileupInfoTag(iConfig.getParameter<edm::InputTag>("pileup")),
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
    genevtInfoToken = consumes<GenEventInfoProduct>             (genevtInfoTag);
    pileupInfoToken = consumes<std::vector<PileupSummaryInfo> > (pileupInfoTag);
    genpartsToken   = consumes<std::vector<reco::GenParticle> > (genpartsTag);
  }
}

Counter::~Counter() {}

void Counter::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) 
{
  // PF CANDIDATES
  iEvent.getByToken(candsToken, candsH);

  // MET
  iEvent.getByToken(metsToken, metsH);

  // GEDPHOTONS + IDS
  iEvent.getByToken(gedPhotonsToken, gedPhotonsH);
 
  // OOTPHOTONS + IDS
  iEvent.getByToken(ootPhotonsToken, ootPhotonsH);

  // GEN INFO
  if (isMC)
  {
    iEvent.getByToken(genevtInfoToken, genevtInfoH);
    iEvent.getByToken(pileupInfoToken, pileupInfoH);
    iEvent.getByToken(genpartsToken  , genparticlesH);

    Counter::SetMCInfo();
  }

  // basic counters
  Counter::SetBasicCounters();

  // MET
  Counter::SetMETInfo();

  // fill tree
  tree->Fill();
}

void Counter::SetMCInfo()
{
  genwgt = genevtInfoH->weight();
  for (const auto & puinfo : *pileupInfoH)
  {
    if (puinfo.getBunchCrossing() == 0) 
    {
      genputrue = puinfo.getTrueNumInteractions();
      break;
    } // end check over correct BX
  } // end loop over PU
}

void Counter::SetBasicCounters()
{
  // reset counters
  Counter::ResetCounters();

  // nGED
  for (const auto & gedPhoton : *gedPhotonsH) 
  {
    nGED++;
    if (gedPhoton.photonID(Config::GEDPhotonLooseVID)) nGED_L++;
    if (gedPhoton.photonID(Config::GEDPhotonTightVID)) nGED_T++;
  }

  // nOOT
  for (const auto & ootPhoton : *ootPhotonsH)
  {
    nOOT++;
    if (ootPhoton.photonID(Config::OOTPhotonLooseVID)) nOOT_L++;
    if (ootPhoton.photonID(Config::OOTPhotonTightVID)) nOOT_T++;
  }

  // count em up
  Counter::CountPhotons("NONE","NONE",nOOT_matchedGTGED,nOOT_matchedLTGED,nOOT_unmatchedGED,nOOT_matchedCands);
  Counter::CountPhotons(Config::GEDPhotonLooseVID,Config::OOTPhotonLooseVID,nOOT_L_matchedGTGED,nOOT_L_matchedLTGED,nOOT_L_unmatchedGED,nOOT_L_matchedCands);
  Counter::CountPhotons(Config::GEDPhotonTightVID,Config::OOTPhotonTightVID,nOOT_T_matchedGTGED,nOOT_T_matchedLTGED,nOOT_T_unmatchedGED,nOOT_T_matchedCands);
}

void Counter::CountPhotons(const std::string & gedVID, const std::string & ootVID,
			   int & matchedGTGED, int & matchedLTGED,
			   int & unmatchedGED, int & matchedCands)
{
  // nOOT
  for (const auto & ootPhoton : *ootPhotonsH)
  {
    if (ootVID != "NONE") 
      if (!ootPhoton.photonID(ootVID)) continue;

    auto isMatched = true;

    for (const auto & gedPhoton : *gedPhotonsH)
    {
      if (gedVID != "NONE") 
	if (!gedPhoton.photonID(gedVID)) continue;
      
      if (reco::deltaR(ootPhoton,gedPhoton) < dRmin)
      {
	if   (Counter::isOOT_GT_GED(gedPhoton,ootPhoton)) matchedGTGED++;
	else                                              matchedLTGED++;

	isMatched = true;
	break;
      } // end check over deltaR match
    } // end loop over ged photons

    if (!isMatched)
    {
      unmatchedGED++;

      for (const auto & cand : *candsH)
      {
	if (cand.pt() < pTmin) continue;
	for (const auto & gedPhoton : *gedPhotonsH)
	{
	  if (gedVID != "NONE") 
	    if (!gedPhoton.photonID(gedVID)) continue;
	  if (reco::deltaR(gedPhoton,cand) < dRmin) continue;
	  if (reco::deltaR(ootPhoton,cand) < dRmin)
	  {
	    matchedCands++;
	    break;
	  } // end deltaR match to pfcand
	} // end loop over ged photons
      } // end loop over pf cands
    } // end block over unmatched OOT phos
  } // end loop over ootPhotons
}

void Counter::ResetCounters()
{
  nGED = 0;
  nGED_T = 0;
  nGED_L = 0;

  nOOT = 0;
  nOOT_L = 0;
  nOOT_T = 0;

  nOOT_matchedGTGED = 0;
  nOOT_L_matchedGTGED = 0;
  nOOT_T_matchedGTGED = 0;

  nOOT_matchedLTGED = 0;
  nOOT_L_matchedLTGED = 0;
  nOOT_T_matchedLTGED = 0;

  nOOT_unmatchedGED = 0;
  nOOT_L_unmatchedGED = 0;
  nOOT_T_unmatchedGED = 0;

  nOOT_matchedCands = 0;
  nOOT_L_matchedCands = 0;
  nOOT_T_matchedCands = 0;
}

bool Counter::isOOT_GT_GED(const pat::Photon & gedPhoton, const pat::Photon & ootPhoton)
{
  const auto gedpt = (gedPhoton.userFloat("ecalEnergyPostCorr")/gedPhoton.energy())*gedPhoton.pt();
  const auto ootpt = (ootPhoton.userFloat("ecalEnergyPostCorr")/ootPhoton.energy())*ootPhoton.pt();
  return (ootpt > gedpt);
}

void Counter::SetMETInfo()
{
  const auto & t1pfMET = (*metsH).front();
  const auto & genMET = *(t1pfMET.genMET());
  t1pfMETpt = t1pfMET.pt();
  t1pfMETphi = t1pfMET.phi();
  genMETpt = genMET.pt();
  genMETphi = genMET.phi();
  
  Counter::METCorrection("NONE","NONE",ootMETpt,ootMETphi);
  Counter::METCorrection(Config::GEDPhotonLooseVID,Config::OOTPhotonLooseVID,ootMETpt_L,ootMETphi_L);
  Counter::METCorrection(Config::GEDPhotonTightVID,Config::OOTPhotonTightVID,ootMETpt_L,ootMETphi_L);
}

void Counter::METCorrection(const std::string & gedVID, const std::string & ootVID,
			    float & newMETpt, float & newMETphi)
{
  // initialize
  newMETpt  = t1pfMETpt;
  newMETphi = t1pfMETphi;

  // modify MET
  for (const auto & ootPhoton : *ootPhotonsH)
  {
    if (ootVID != "NONE") 
      if (!ootPhoton.photonID(ootVID)) continue;

    auto isMatched = true;

    for (const auto & gedPhoton : *gedPhotonsH)
    {
      if (gedVID != "NONE") 
	if (!gedPhoton.photonID(gedVID)) continue;
      
      if (reco::deltaR(ootPhoton,gedPhoton) < dRmin)
      {
	if (Counter::isOOT_GT_GED(gedPhoton,ootPhoton))
	{
	  // get compnonents
	  const auto x = newMETpt*std::cos(newMETphi) - ootPhoton.pt()*std::cos(ootPhoton.phi()) + gedPhoton.pt()*std::cos(gedPhoton.phi());
	  const auto y = newMETpt*std::sin(newMETphi) - ootPhoton.pt()*std::sin(ootPhoton.phi()) + gedPhoton.pt()*std::sin(gedPhoton.phi());
	  
	  // make new met
	  newMETpt  = Config::hypo(x,y);
	  newMETphi = Config::phi (x,y);

	  // skip on next found
	  isMatched = true;
	  break;
	} // end check over OOT pt > GED pt
      } // end check over deltaR match
    } // end loop over ged photons

    // check that truly unmatched!
    if (!isMatched)
    {
      // get compnonents
      const auto x = newMETpt*std::cos(newMETphi) - ootPhoton.pt()*std::cos(ootPhoton.phi());
      const auto y = newMETpt*std::sin(newMETphi) - ootPhoton.pt()*std::sin(ootPhoton.phi());
      
      // make new met
      newMETpt  = Config::hypo(x,y);
      newMETphi = Config::phi (x,y);
    } // end block over check if truly unmatched
    
  } // end loop over oot photons
}
			   
void Counter::beginJob() 
{
  edm::Service<TFileService> fs;
  
  // Event tree
  tree = fs->make<TTree>("tree","tree");

  // MC info
  if (isMC)
  {
    tree->Branch("genwgt", &genwgt, "genwgt/F");
    tree->Branch("xsec", &xsec, "xsec/F");
    tree->Branch("BR", &BR, "BR/F");
    tree->Branch("genputrue", &genputrue, "genputrue/I");
  }

  // counter info
  tree->Branch("nGED", &nGED, "nGED/I");
  tree->Branch("nGED_L", &nGED_L, "nGED_L/I");
  tree->Branch("nGED_T", &nGED_T, "nGED_T/I");
  tree->Branch("nOOT", &nOOT, "nOOT/I");
  tree->Branch("nOOT_L", &nOOT_L, "nOOT_L/I");
  tree->Branch("nOOT_T", &nOOT_T, "nOOT_T/I");
  tree->Branch("nOOT_matchedGTGED", &nOOT_matchedGTGED, "nOOT_matchedGTGED/I");
  tree->Branch("nOOT_L_matchedGTGED", &nOOT_L_matchedGTGED, "nOOT_L_matchedGTGED/I");
  tree->Branch("nOOT_T_matchedGTGED", &nOOT_T_matchedGTGED, "nOOT_T_matchedGTGED/I");
  tree->Branch("nOOT_matchedLTGED", &nOOT_matchedLTGED, "nOOT_matchedLTGED/I");
  tree->Branch("nOOT_L_matchedLTGED", &nOOT_L_matchedLTGED, "nOOT_L_matchedLTGED/I");
  tree->Branch("nOOT_T_matchedLTGED", &nOOT_T_matchedLTGED, "nOOT_T_matchedLTGED/I");
  tree->Branch("nOOT_unmatchedGED", &nOOT_unmatchedGED, "nOOT_unmatchedGED/I");
  tree->Branch("nOOT_L_unmatchedGED", &nOOT_L_unmatchedGED, "nOOT_L_unmatchedGED/I");
  tree->Branch("nOOT_T_unmatchedGED", &nOOT_T_unmatchedGED, "nOOT_T_unmatchedGED/I");
  tree->Branch("nOOT_matchedCands", &nOOT_matchedCands, "nOOT_matchedCands/I");
  tree->Branch("nOOT_L_matchedCands", &nOOT_L_matchedCands, "nOOT_L_matchedCands/I");
  tree->Branch("nOOT_T_matchedCands", &nOOT_T_matchedCands, "nOOT_T_matchedCands/I");

  // MET info
  tree->Branch("t1pfMETpt", &t1pfMETpt, "t1pfMETpt/F");
  tree->Branch("t1pfMETphi", &t1pfMETphi, "t1pfMETphi/F");
  tree->Branch("genMETpt", &genMETpt, "genMETpt/F");
  tree->Branch("genMETphi", &genMETphi, "genMETphi/F");
  tree->Branch("ootMETpt", &ootMETpt, "ootMETpt/F");
  tree->Branch("ootMETphi", &ootMETphi, "ootMETphi/F");
  tree->Branch("ootMETpt_L", &ootMETpt_L, "ootMETpt_L/F");
  tree->Branch("ootMETphi_L", &ootMETphi_L, "ootMETphi_L/F");
  tree->Branch("ootMETpt_T", &ootMETpt_T, "ootMETpt_T/F");
  tree->Branch("ootMETphi_T", &ootMETphi_T, "ootMETphi_T/F");
}

void Counter::endJob() {}

void Counter::beginRun(edm::Run const& iRun, edm::EventSetup const& iSetup) {}

void Counter::endRun(edm::Run const&, edm::EventSetup const&) {}

void Counter::fillDescriptions(edm::ConfigurationDescriptions& descriptions) 
{
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

DEFINE_FWK_MODULE(Counter);
