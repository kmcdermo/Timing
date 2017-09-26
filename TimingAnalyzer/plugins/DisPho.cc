#include "Timing/TimingAnalyzer/plugins/DisPho.hh"

DisPho::DisPho(const edm::ParameterSet& iConfig): 
  // object prep cuts
  jetpTmin(iConfig.existsAs<double>("jetpTmin") ? iConfig.getParameter<double>("jetpTmin") : 15.f),
  jetIDmin(iConfig.existsAs<int>("jetIDmin") ? iConfig.getParameter<int>("jetIDmin") : 1),
  rhEmin(iConfig.existsAs<double>("rhEmin") ? iConfig.getParameter<double>("rhEmin") : 1.f),
  phpTmin(iConfig.existsAs<double>("phpTmin") ? iConfig.getParameter<double>("phpTmin") : 20.f),
  phIDmin(iConfig.existsAs<std::string>("phIDmin") ? iConfig.getParameter<std::string>("phIDmin") : "loose"),

  // pre-selection
  applyTrigger(iConfig.existsAs<bool>("applyTrigger") ? iConfig.getParameter<bool>("applyTrigger") : false),
  minHT(iConfig.existsAs<double>("minHT") ? iConfig.getParameter<double>("minHT") : 400.f),
  applyHT(iConfig.existsAs<bool>("applyHT") ? iConfig.getParameter<bool>("applyHT") : false),
  phgoodpTmin(iConfig.existsAs<double>("phgoodpTmin") ? iConfig.getParameter<double>("phgoodpTmin") : 70.f),
  phgoodIDmin(iConfig.existsAs<std::string>("phgoodIDmin") ? iConfig.getParameter<std::string>("phgoodIDmin") : "medium"),
  applyPhGood(iConfig.existsAs<bool>("applyPhGood") ? iConfig.getParameter<bool>("applyPhGood") : false),

  // matching criteria
  dRmin(iConfig.existsAs<double>("dRmin") ? iConfig.getParameter<double>("dRmin") : 0.4),
  pTres(iConfig.existsAs<double>("pTres") ? iConfig.getParameter<double>("pTres") : 0.5),

  // triggers
  inputPaths       (iConfig.existsAs<std::string>("inputPaths")   ? iConfig.getParameter<std::string>("inputPaths") : ""),
  inputFilters     (iConfig.existsAs<std::string>("inputFilters") ? iConfig.getParameter<std::string>("inputFilters") : ""),
  triggerResultsTag(iConfig.getParameter<edm::InputTag>("triggerResults")),
  triggerObjectsTag(iConfig.getParameter<edm::InputTag>("triggerObjects")),

  // vertices
  verticesTag(iConfig.getParameter<edm::InputTag>("vertices")),

  // rhos
  rhosTag(iConfig.getParameter<edm::InputTag>("rhos")),

  // mets
  metsTag(iConfig.getParameter<edm::InputTag>("mets")),  

  // jets
  jetsTag(iConfig.getParameter<edm::InputTag>("jets")),  

  // photons + ids
  photonsTag          (iConfig.getParameter<edm::InputTag>("photons")),

  // ootPhotons + ids
  ootPhotonsTag          (iConfig.getParameter<edm::InputTag>("ootPhotons")),

  //recHits
  recHitsEBTag(iConfig.getParameter<edm::InputTag>("recHitsEB")),  
  recHitsEETag(iConfig.getParameter<edm::InputTag>("recHitsEE")),

  ///////////// GEN INFO
  // isMC or Data --> default Data
  isGMSB(iConfig.existsAs<bool>("isGMSB") ? iConfig.getParameter<bool>("isGMSB") : false),
  isHVDS(iConfig.existsAs<bool>("isHVDS") ? iConfig.getParameter<bool>("isHVDS") : false),
  isBkg (iConfig.existsAs<bool>("isBkg")  ? iConfig.getParameter<bool>("isBkg")  : false)
{
  usesResource();
  usesResource("TFileService");

  // triggers
  triggerResultsToken = consumes<edm::TriggerResults> (triggerResultsTag);
  triggerObjectsToken = consumes<std::vector<pat::TriggerObjectStandAlone> > (triggerObjectsTag);

  // read in from a stream the trigger paths for saving
  oot::ReadInTriggerNames(inputPaths,pathNames,triggerBitMap);

  // read in from a stream the hlt objects/labels to match to
  oot::ReadInFilterNames(inputFilters,filterNames,triggerObjectsByFilterMap);

  //vertex
  verticesToken = consumes<std::vector<reco::Vertex> > (verticesTag);

  // rhos
  rhosToken = consumes<double> (rhosTag);

  // mets
  metsToken = consumes<std::vector<pat::MET> > (metsTag);

  // jets
  jetsToken = consumes<std::vector<pat::Jet> > (jetsTag);

  // photons + ids
  photonsToken = consumes<std::vector<pat::Photon> > (photonsTag);
  if (not ootPhotonsTag.label().empty())
  {
    ootPhotonsToken = consumes<std::vector<pat::Photon> > (ootPhotonsTag);
  }

  // rechits
  recHitsEBToken = consumes<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > (recHitsEBTag);
  recHitsEEToken = consumes<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > (recHitsEETag);

  // only for simulated samples
  if (isGMSB || isHVDS || isBkg)
  {
    isMC = true;
    genevtInfoToken = consumes<GenEventInfoProduct>             (iConfig.getParameter<edm::InputTag>("genevt"));
    pileupInfoToken = consumes<std::vector<PileupSummaryInfo> > (iConfig.getParameter<edm::InputTag>("pileup"));
    genpartsToken   = consumes<std::vector<reco::GenParticle> > (iConfig.getParameter<edm::InputTag>("genparts"));   
  }
  else 
  {
    isMC = false;
  }
}

DisPho::~DisPho() {}

void DisPho::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) 
{
  ////////////////////////
  //                    //
  // Get Object Handles //
  //                    //
  ////////////////////////

  // TRIGGERS
  edm::Handle<edm::TriggerResults> triggerResultsH;
  iEvent.getByToken(triggerResultsToken, triggerResultsH);

  edm::Handle<std::vector<pat::TriggerObjectStandAlone> > triggerObjectsH;
  iEvent.getByToken(triggerObjectsToken, triggerObjectsH);

  // VERTICES
  edm::Handle<std::vector<reco::Vertex> > verticesH;
  iEvent.getByToken(verticesToken, verticesH);
  
  // RHOS
  edm::Handle<double> rhosH;
  iEvent.getByToken(rhosToken, rhosH);
  rho = rhosH.isValid() ? *(rhosH.product()) : 0.f;

  // MET
  edm::Handle<std::vector<pat::MET> > metsH;
  iEvent.getByToken(metsToken, metsH);

  // JETS
  edm::Handle<std::vector<pat::Jet> > jetsH;
  iEvent.getByToken(jetsToken, jetsH);
  std::vector<pat::Jet> jets; jets.reserve(jetsH->size());

  // PHOTONS + IDS
  edm::Handle<std::vector<pat::Photon> > photonsH;
  iEvent.getByToken(photonsToken, photonsH);
  int phosize = photonsH->size();

  // OOTPHOTONS + IDS
  edm::Handle<std::vector<pat::Photon> > ootPhotonsH;
  if (not ootPhotonsToken.isUninitialized())
  {
    iEvent.getByToken(ootPhotonsToken, ootPhotonsH);
    phosize += ootPhotonsH->size();
  }
  // total photons vector
  std::vector<oot::Photon> photons; photons.reserve(phosize);

  // RecHits
  edm::Handle<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > recHitsEBH;
  iEvent.getByToken(recHitsEBToken, recHitsEBH);
  const EcalRecHitCollection * recHitsEB = recHitsEBH.product();
  edm::Handle<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > recHitsEEH;
  iEvent.getByToken(recHitsEEToken, recHitsEEH);
  const EcalRecHitCollection * recHitsEE = recHitsEBH.product();
  uiiumap recHitMap;

  // geometry (from ECAL ELF)
  edm::ESHandle<CaloGeometry> calogeoH;
  iSetup.get<CaloGeometryRecord>().get(calogeoH);
  const CaloSubdetectorGeometry * barrelGeometry = calogeoH->getSubdetectorGeometry(DetId::Ecal, EcalBarrel);
  const CaloSubdetectorGeometry * endcapGeometry = calogeoH->getSubdetectorGeometry(DetId::Ecal, EcalEndcap);

  // GEN INFO
  edm::Handle<GenEventInfoProduct> genevtInfoH;
  edm::Handle<std::vector<PileupSummaryInfo> > pileupInfoH;
  edm::Handle<std::vector<reco::GenParticle> > genparticlesH;
  genPartVec neutralinos;
  genPartVec vPions;

  if (isMC)
  {
    iEvent.getByToken(genevtInfoToken, genevtInfoH);
    iEvent.getByToken(pileupInfoToken, pileupInfoH);
    iEvent.getByToken(genpartsToken,   genparticlesH);
  }

  /////////////////////
  //                 //
  // Object Prepping //
  //                 //
  /////////////////////
  if (isGMSB) oot::PrepNeutralinos(genparticlesH,neutralinos);
  if (isHVDS) oot::PrepVPions(genparticlesH,vPions);
  oot::PrepTriggerBits(triggerResultsH,iEvent,triggerBitMap);
  oot::PrepTriggerObjects(triggerResultsH,triggerObjectsH,iEvent,triggerObjectsByFilterMap);
  oot::PrepJets(jetsH,jets,jetpTmin,jetIDmin);
  oot::PrepRecHits(recHitsEB,recHitsEE,recHitMap,rhEmin);
  oot::PrepPhotons(photonsH,ootPhotonsH,photons,rho,phpTmin,phIDmin);

  /////////////////////////
  //                     //
  // Apply pre-selection //
  //                     //
  /////////////////////////

  // trigger pre-selection
  bool triggered = false;
  for (const auto & triggerBitPair : triggerBitMap)
  {
    if (triggerBitPair.second) {triggered = true; break;}
  }
  if (!triggered && applyTrigger) return;

  // HT pre-selection
  if (jetsH.isValid()) // check to make sure reco jets exist
  {
    for (const auto& jet : jets)
    {
      jetHT += jet.pt();
    }
  } // end check over reco jets  
  if (jetHT < minHT && applyHT) return;

  // photon pre-selection: at least one good photon in event
  bool isphgood = false;
  int  iph = 0;
  if (photonsH.isValid() || ootPhotonsH.isValid()) 
  {
    for (const auto & photon : photons)
    {
      if (iph >= 4) break;
      iph++;
      
      if (photon.pt() < phgoodpTmin) continue;
      if (phgoodIDmin != "")
      {
	if (!photon.photon().photonID(phgoodIDmin)) continue;
      }
      
      isphgood = true; break;
    } 
  } // end check
  if (!isphgood && applyPhGood) return;

  /////////////
  //         //
  // MC Info //
  //         //
  /////////////

  if (isMC) 
  {
    ///////////////////////
    //                   //
    // Event weight info //
    //                   //
    ///////////////////////
    DisPho::InitializeGenEvtBranches();
    if (genevtInfoH.isValid()) {genwgt = genevtInfoH->weight();}

    /////////////////////
    //                 //
    // Gen pileup info //
    //                 //
    /////////////////////
    DisPho::InitializeGenPUBranches();
    if (pileupInfoH.isValid()) // standard check for pileup
    {
      for (const auto & puinfo : *pileupInfoH)
      {
	if (puinfo.getBunchCrossing() == 0) 
	{
	  genpuobs  = puinfo.getPU_NumInteractions();
	  genputrue = puinfo.getTrueNumInteractions();
	} // end check over correct BX
      } // end loop over PU
    } // end check over pileup

    ///////////////////////
    //                   //
    // Gen particle info //
    //                   //
    ///////////////////////
    
    // GMSB
    if (isGMSB) 
    {
      DisPho::InitializeGMSBBranches();
      if (genparticlesH.isValid()) 
      {
	nNeutoPhGr = neutralinos.size();
	if (nNeutoPhGr > 0) DisPho::SetGMSBBranch(neutralinos[0],gmsbBranch0,photons);
	if (nNeutoPhGr > 1) DisPho::SetGMSBBranch(neutralinos[1],gmsbBranch1,photons);
      } // check genparticles are okay
    } // isGMSB

    // HVDS
    if (isHVDS) 
    {
      DisPho::InitializeHVDSBranches();
      if (genparticlesH.isValid()) 
      {
	nvPions = vPions.size();
	if (nvPions > 0) DisPho::SetHVDSBranch(vPions[0],hvdsBranch0,photons);
	if (nvPions > 1) DisPho::SetHVDSBranch(vPions[1],hvdsBranch1,photons);
	if (nvPions > 2) DisPho::SetHVDSBranch(vPions[2],hvdsBranch2,photons);
	if (nvPions > 3) DisPho::SetHVDSBranch(vPions[3],hvdsBranch3,photons);
      } // check genparticles are okay
    } // isHVDS
  } // isMC

  ///////////////////////////
  //                       //
  // Event, lumi, run info //
  //                       //
  ///////////////////////////
  run   = iEvent.id().run();
  lumi  = iEvent.luminosityBlock();
  event = iEvent.id().event();

  //////////////////
  //              //
  // Trigger Info //
  //              //
  //////////////////
  hltDisPho = triggerBitMap["HLT_Photon60_R9Id90_CaloIdL_IsoL_DisplacedIdL_PFHT350MinPFJet15_v"];

  /////////////////////////
  //                     //   
  // Primary Vertex info //
  //                     //
  /////////////////////////
  DisPho::InitializePVBranches();
  if (verticesH.isValid()) 
  {
    nvtx = verticesH->size();
    const reco::Vertex & primevtx = (*verticesH)[0];
    vtxX = primevtx.position().x();
    vtxY = primevtx.position().y();
    vtxZ = primevtx.position().z();
  }

  //////////////////
  //              //
  // Type1 PF Met //
  //              //
  //////////////////
  DisPho::InitializeMETBranches();
  if (metsH.isValid())
  {
    const pat::MET & t1pfMET = (*metsH)[0];

    // Type1 PF MET (corrected)
    t1pfMETpt    = t1pfMET.pt();
    t1pfMETphi   = t1pfMET.phi();
    t1pfMETsumEt = t1pfMET.sumEt();
  }

  /////////////////////////
  //                     //
  // Jets (AK4 standard) //
  //                     //
  /////////////////////////
  DisPho::InitializeJetBranches();
  if (jetsH.isValid()) // check to make sure reco jets exist
  {
    njets = jets.size();
    if (njets > 0) DisPho::SetJetBranch(jets[0],jetBranch0);
    if (njets > 1) DisPho::SetJetBranch(jets[1],jetBranch1);
    if (njets > 2) DisPho::SetJetBranch(jets[2],jetBranch2);
    if (njets > 3) DisPho::SetJetBranch(jets[3],jetBranch3);
  }

  //////////////
  //          //
  // Rec Hits //
  //          //
  //////////////
  nrechits = recHitMap.size();
  DisPho::InitializeRecHitBranches();
  if (recHitsEBH.isValid() && recHitsEEH.isValid())
  {
    DisPho::SetRecHitBranches(recHitsEB,barrelGeometry,recHitMap);
    DisPho::SetRecHitBranches(recHitsEE,endcapGeometry,recHitMap);
  }

  //////////////////
  //              //
  // Reco Photons //
  //              //
  //////////////////
  DisPho::InitializePhoBranches();
  if (isMC) DisPho::InitializePhoBranchesMC();
  if (photonsH.isValid() || ootPhotonsH.isValid()) // standard handle check
  {
    nphotons = photons.size();
    if (nphotons > 0) DisPho::SetPhoBranch(photons[0],phoBranch0,recHitMap,recHitsEB,recHitsEE);
    if (nphotons > 1) DisPho::SetPhoBranch(photons[1],phoBranch1,recHitMap,recHitsEB,recHitsEE);
    if (nphotons > 2) DisPho::SetPhoBranch(photons[2],phoBranch2,recHitMap,recHitsEB,recHitsEE);
    if (nphotons > 3) DisPho::SetPhoBranch(photons[3],phoBranch3,recHitMap,recHitsEB,recHitsEE);

    if (isMC)
    {
      if (nphotons > 0) DisPho::SetPhoBranchMC(0,photons[0],phoBranch0,genparticlesH);
      if (nphotons > 1) DisPho::SetPhoBranchMC(1,photons[1],phoBranch1,genparticlesH);
      if (nphotons > 2) DisPho::SetPhoBranchMC(2,photons[2],phoBranch2,genparticlesH);
      if (nphotons > 3) DisPho::SetPhoBranchMC(3,photons[3],phoBranch3,genparticlesH);
    }
  }

  ///////////////
  //           //
  // Fill Tree //
  //           //
  ///////////////
  tree->Fill();
}

void DisPho::InitializeGenEvtBranches()
{
  genwgt = -9999.f;
}

void DisPho::InitializeGenPUBranches()
{
  genpuobs = -9999; genputrue = -9999;
}

void DisPho::InitializeGMSBBranches()
{
  nNeutoPhGr = -9999;
  DisPho::InitializeGMSBBranch(gmsbBranch0);
  DisPho::InitializeGMSBBranch(gmsbBranch1);
}

void DisPho::InitializeGMSBBranch(gmsbStruct& gmsbBranch)
{
  gmsbBranch.genNmass_ = -9999.f; gmsbBranch.genNE_ = -9999.f; gmsbBranch.genNpt_ = -9999.f; gmsbBranch.genNphi_ = -9999.f; gmsbBranch.genNeta_ = -9999.f;
  gmsbBranch.genNprodvx_ = -9999.f; gmsbBranch.genNprodvy_ = -9999.f; gmsbBranch.genNprodvz_ = -9999.f;
  gmsbBranch.genNdecayvx_ = -9999.f; gmsbBranch.genNdecayvy_ = -9999.f; gmsbBranch.genNdecayvz_ = -9999.f;
  
  gmsbBranch.genphE_ = -9999.f; gmsbBranch.genphpt_ = -9999.f; gmsbBranch.genphphi_ = -9999.f; gmsbBranch.genpheta_ = -9999.f; gmsbBranch.genphmatch_ = -9999;
  gmsbBranch.gengrmass_ = -9999.f; gmsbBranch.gengrE_ = -9999.f; gmsbBranch.gengrpt_ = -9999.f; gmsbBranch.gengrphi_ = -9999.f; gmsbBranch.gengreta_ = -9999.f;
}

void DisPho::SetGMSBBranch(const reco::GenParticle & neutralino, gmsbStruct & gmsbBranch, const std::vector<oot::Photon> & photons)
{
  // neutralino 4-vector
  gmsbBranch.genNmass_ = neutralino.mass();
  gmsbBranch.genNE_    = neutralino.energy();
  gmsbBranch.genNpt_   = neutralino.pt();
  gmsbBranch.genNphi_  = neutralino.phi();
  gmsbBranch.genNeta_  = neutralino.eta();

  // neutralino production vertex
  gmsbBranch.genNprodvx_ = neutralino.vx();
  gmsbBranch.genNprodvy_ = neutralino.vy();
  gmsbBranch.genNprodvz_ = neutralino.vz();
  
  // neutralino decay vertex (same for both daughters unless really screwed up)
  gmsbBranch.genNdecayvx_ = neutralino.daughter(0)->vx();
  gmsbBranch.genNdecayvy_ = neutralino.daughter(0)->vy();
  gmsbBranch.genNdecayvz_ = neutralino.daughter(0)->vz();
  
  // set photon daughter stuff
  const int phdaughter = (neutralino.daughter(0)->pdgId() == 22)?0:1;
	    
  gmsbBranch.genphE_   = neutralino.daughter(phdaughter)->energy();
  gmsbBranch.genphpt_  = neutralino.daughter(phdaughter)->pt();
  gmsbBranch.genphphi_ = neutralino.daughter(phdaughter)->phi();
  gmsbBranch.genpheta_ = neutralino.daughter(phdaughter)->eta();
  
  // check for a reco match!
  int   iph   = 0;
  float mindR = dRmin;
  for (const auto & photon : photons)
  {
    if (iph > 4) break;
    if (std::abs(photon.pt()-gmsbBranch.genphpt_)/gmsbBranch.genphpt_ < pTres)
    {
      const float delR = deltaR(gmsbBranch.genphphi_,gmsbBranch.genpheta_,photon.phi(),photon.eta());
      if (delR < mindR) 
      {
	mindR = delR;
	gmsbBranch.genphmatch_ = iph; 
      } // end check over deltaR
    } // end check over pt resolution
    iph++;
  } // end loop over reco photons
  
  // set gravitino daughter stuff
  const int grdaughter = (neutralino.daughter(0)->pdgId() == 1000039)?0:1;

  gmsbBranch.gengrmass_ = neutralino.daughter(grdaughter)->mass();
  gmsbBranch.gengrE_    = neutralino.daughter(grdaughter)->energy();
  gmsbBranch.gengrpt_   = neutralino.daughter(grdaughter)->pt();
  gmsbBranch.gengrphi_  = neutralino.daughter(grdaughter)->phi();
  gmsbBranch.gengreta_  = neutralino.daughter(grdaughter)->eta();
} 

void DisPho::InitializeHVDSBranches()
{
  nvPions = -9999;
  DisPho::InitializeHVDSBranch(hvdsBranch0);
  DisPho::InitializeHVDSBranch(hvdsBranch1);
  DisPho::InitializeHVDSBranch(hvdsBranch2);
  DisPho::InitializeHVDSBranch(hvdsBranch3);
}

void DisPho::InitializeHVDSBranch(hvdsStruct& hvdsBranch)
{
  hvdsBranch.genvPionmass_ = -9999.f; hvdsBranch.genvPionE_ = -9999.f; hvdsBranch.genvPionpt_ = -9999.f; hvdsBranch.genvPionphi_ = -9999.f; hvdsBranch.genvPioneta_ = -9999.f;
  hvdsBranch.genvPionprodvx_ = -9999.f; hvdsBranch.genvPionprodvy_ = -9999.f; hvdsBranch.genvPionprodvz_ = -9999.f;
  hvdsBranch.genvPiondecayvx_ = -9999.f; hvdsBranch.genvPiondecayvy_ = -9999.f; hvdsBranch.genvPiondecayvz_ = -9999.f;

  hvdsBranch.genHVph0E_ = -9999.f; hvdsBranch.genHVph0pt_ = -9999.f; hvdsBranch.genHVph0phi_ = -9999.f; hvdsBranch.genHVph0eta_ = -9999.f; hvdsBranch.genHVph0match_ = -9999;
  hvdsBranch.genHVph1E_ = -9999.f; hvdsBranch.genHVph1pt_ = -9999.f; hvdsBranch.genHVph1phi_ = -9999.f; hvdsBranch.genHVph1eta_ = -9999.f; hvdsBranch.genHVph1match_ = -9999;
}

void DisPho::SetHVDSBranch(const reco::GenParticle & vPion, hvdsStruct & hvdsBranch, const std::vector<oot::Photon> & photons)
{
  // set neutralino parameters
  hvdsBranch.genvPionmass_ = vPion.mass();
  hvdsBranch.genvPionE_    = vPion.energy();
  hvdsBranch.genvPionpt_   = vPion.pt();
  hvdsBranch.genvPionphi_  = vPion.phi();
  hvdsBranch.genvPioneta_  = vPion.eta();
	  
  // vPion production vertex
  hvdsBranch.genvPionprodvx_ = vPion.vx();
  hvdsBranch.genvPionprodvy_ = vPion.vy();
  hvdsBranch.genvPionprodvz_ = vPion.vz();
  
  // vPion decay vertex (same for both daughters unless really screwed up)
  hvdsBranch.genvPiondecayvx_ = vPion.daughter(0)->vx();
  hvdsBranch.genvPiondecayvy_ = vPion.daughter(0)->vy();
  hvdsBranch.genvPiondecayvz_ = vPion.daughter(0)->vz();

  const int leading    = (vPion.daughter(0)->pt()>vPion.daughter(1)->pt())?0:1;
  const int subleading = (vPion.daughter(0)->pt()>vPion.daughter(1)->pt())?1:0;
  
  hvdsBranch.genHVph0E_   = vPion.daughter(leading)->energy();
  hvdsBranch.genHVph0pt_  = vPion.daughter(leading)->pt();
  hvdsBranch.genHVph0phi_ = vPion.daughter(leading)->phi();
  hvdsBranch.genHVph0eta_ = vPion.daughter(leading)->eta();
  
  hvdsBranch.genHVph1E_   = vPion.daughter(subleading)->energy();
  hvdsBranch.genHVph1pt_  = vPion.daughter(subleading)->pt();
  hvdsBranch.genHVph1phi_ = vPion.daughter(subleading)->phi();
  hvdsBranch.genHVph1eta_ = vPion.daughter(subleading)->eta();
  
  // check for a reco match!
  int   tmpph0 = -9999, tmpph1 = -9999;
  int   iph = 0;
  float mindR0 = dRmin, mindR1 = dRmin; // at least this much
  for (const auto & photon : photons)
  {
    if (iph > 4) break;
    const float tmppt  = photon.pt();
    const float tmpphi = photon.phi();
    const float tmpeta = photon.eta();
    
    // check photon 1
    if (std::abs(tmppt-hvdsBranch.genHVph0pt_)/hvdsBranch.genHVph0pt_ < pTres)
    {
      const float delR = deltaR(hvdsBranch.genHVph0phi_,hvdsBranch.genHVph0eta_,tmpphi,tmpeta);
      if (delR < mindR0) 
      {
	mindR0 = delR;
	tmpph0 = iph;
      } // end check over deltaR
    } // end check over pt resolution
	      
    // check photon 2
    if (std::abs(tmppt-hvdsBranch.genHVph1pt_)/hvdsBranch.genHVph1pt_ < pTres)
    {
      const float delR = deltaR(hvdsBranch.genHVph1phi_,hvdsBranch.genHVph1eta_,tmpphi,tmpeta);
      if (delR < mindR1) 
      {
	mindR1 = delR;
	tmpph1 = iph;
      } // end check over deltaR
    } // end check over pt resolution
    
    // now update iph
    iph++;
  } // end loop over reco photons
  
  // now save tmp photon iphs
  hvdsBranch.genHVph0match_ = tmpph0;
  hvdsBranch.genHVph1match_ = tmpph1;
} 

void DisPho::InitializePVBranches()
{
  nvtx = -9999; 
  vtxX = -9999.f; vtxY = -9999.f; vtxZ = -9999.f;
}

void DisPho::InitializeMETBranches()
{
  t1pfMETpt    = -9999.f; 
  t1pfMETphi   = -9999.f; 
  t1pfMETsumEt = -9999.f;
}

void DisPho::InitializeJetBranches()
{
  DisPho::InitializeJetBranch(jetBranch0);
  DisPho::InitializeJetBranch(jetBranch1);
  DisPho::InitializeJetBranch(jetBranch2);
  DisPho::InitializeJetBranch(jetBranch3);
}

void DisPho::InitializeJetBranch(jetStruct & jetBranch)
{
  jetBranch.E_   = -9999.f;
  jetBranch.Pt_  = -9999.f;
  jetBranch.Phi_ = -9999.f;
  jetBranch.Eta_ = -9999.f;
}

void DisPho::SetJetBranch(const pat::Jet & jet, jetStruct & jetBranch)
{
  jetBranch.E_   = jet.energy();
  jetBranch.Pt_  = jet.pt();
  jetBranch.Eta_ = jet.eta();
  jetBranch.Phi_ = jet.phi();
}

void DisPho::InitializeRecHitBranches()
{
  rheta.clear();
  rhphi.clear();
  rhE.clear();
  rhtime.clear();
  rhOOT.clear();
  rhID.clear();

  rheta.resize(nrechits);
  rhphi.resize(nrechits);
  rhE.resize(nrechits);
  rhtime.resize(nrechits);
  rhOOT.resize(nrechits);
  rhID.resize(nrechits);
}

void DisPho::SetRecHitBranches(const EcalRecHitCollection * recHits, const CaloSubdetectorGeometry * geometry, 
			       const uiiumap & recHitMap)
{
  for (const auto recHit : *recHits)
  {
    const DetId recHitId(recHit.detid());
    const uint32_t rawId = recHitId.rawId();
    if (recHitMap.count(rawId))
    {
      const int pos = recHitMap.at(rawId);
      
      const auto recHitPos = geometry->getGeometry(recHitId)->getPosition();
      
      // save position, energy, and time of each rechit to a vector
      rheta [pos] = recHitPos.eta();
      rhphi [pos] = recHitPos.phi();
      rhE   [pos] = recHit.energy();
      rhtime[pos] = recHit.time();
      rhOOT [pos] = int(recHit.checkFlag(EcalRecHit::kOutOfTime));   
      rhID  [pos] = rawId;
    }
  }
}

void DisPho::InitializePhoBranches()
{
  DisPho::InitializePhoBranch(phoBranch0);
  DisPho::InitializePhoBranch(phoBranch1);
  DisPho::InitializePhoBranch(phoBranch2);
  DisPho::InitializePhoBranch(phoBranch3);
}

void DisPho::InitializePhoBranch(phoStruct & phoBranch)
{
  phoBranch.E_ = -9999.f;
  phoBranch.Pt_ = -9999.f;
  phoBranch.Eta_ = -9999.f;
  phoBranch.Phi_ = -9999.f;

  phoBranch.scE_ = -9999.f;
  phoBranch.scPhi_ = -9999.f;
  phoBranch.scEta_ = -9999.f;

  phoBranch.HadTowOE_ = -9999.f;
  phoBranch.HadronOE_ = -9999.f;
  phoBranch.r9_ = -9999.f;
  phoBranch.fullr9_ = -9999.f;
  phoBranch.ChgHadIso_ = -9999.f;
  phoBranch.NeuHadIso_ = -9999.f;
  phoBranch.PhoIso_ = -9999.f;

  phoBranch.EcalPFClIso_ = -9999.f;
  phoBranch.HcalPFClIso_ = -9999.f;
  phoBranch.TrkIso_ = -9999.f;

  phoBranch.Sieie_ = -9999.f;
  phoBranch.Sipip_ = -9999.f;
  phoBranch.Sieip_ = -9999.f;

  phoBranch.Smaj_ = -9999.f;
  phoBranch.Smin_ = -9999.f;
  phoBranch.alpha_ = -9999.f;

  phoBranch.seed_ = -1;
  phoBranch.recHits_.clear();

  phoBranch.isOOT_ = false;
  phoBranch.isEB_ = false;
  phoBranch.isHLT_ = false;
  phoBranch.ID_ = -1;
}

void DisPho::SetPhoBranch(const oot::Photon& photon, phoStruct & phoBranch, const uiiumap & recHitMap,
			  const EcalRecHitCollection * recHitsEB, const EcalRecHitCollection * recHitsEE)
{
  // basic kinematics
  const pat::Photon & pho = photon.photon(); 
  phoBranch.E_   = pho.energy();
  phoBranch.Pt_  = pho.pt();
  phoBranch.Phi_ = pho.phi();
  phoBranch.Eta_ = pho.eta();

  // super cluster from photon
  const reco::SuperClusterRef& phosc = pho.superCluster().isNonnull() ? pho.superCluster() : pho.parentSuperCluster();
  phoBranch.scE_   = phosc->energy();
  phoBranch.scPhi_ = phosc->phi();
  phoBranch.scEta_ = phosc->eta();

  const float sceta = std::abs(phoBranch.scEta_);

  // ID-like variables
  phoBranch.HadTowOE_  = pho.hadTowOverEm(); // used in ID + trigger
  phoBranch.HadronOE_  = pho.hadronicOverEm(); 
  phoBranch.r9_        = pho.r9(); // used in slimming in PAT + trigger
  phoBranch.fullr9_    = pho.full5x5_r9();
  phoBranch.ChgHadIso_ = std::max(pho.chargedHadronIso() - (rho * oot::GetChargedHadronEA(sceta)),0.f);
  phoBranch.NeuHadIso_ = std::max(pho.neutralHadronIso() - (rho * oot::GetNeutralHadronEA(sceta)),0.f);
  phoBranch.PhoIso_    = std::max(pho.photonIso()        - (rho * oot::GetGammaEA        (sceta)),0.f);

  // More ID variables
  phoBranch.EcalPFClIso_ = pho.ecalPFClusterIso();
  phoBranch.HcalPFClIso_ = pho.hcalPFClusterIso();
  phoBranch.TrkIso_      = pho.trackIso();

  // Shower Shape Objects
  const reco::Photon::ShowerShape& phoshape = pho.full5x5_showerShapeVariables(); 

  // cluster shape variables
  phoBranch.Sieie_ = phoshape.sigmaIetaIeta;
  phoBranch.Sipip_ = phoshape.sigmaIphiIphi;
  phoBranch.Sieip_ = phoshape.sigmaIetaIphi;

  // use seed to get geometry and recHits
  const DetId & seedDetId = phosc->seed()->seed(); //seed detid
  const bool isEB = (seedDetId.subdetId() == EcalBarrel); //which subdet
  const EcalRecHitCollection * recHits = (isEB ? recHitsEB : recHitsEE); 

  // 2nd moments from official calculation
  if (recHits->size() > 0)
  {
    const Cluster2ndMoments ph2ndMoments = noZS::EcalClusterTools::cluster2ndMoments( *phosc, *recHits);
    // radius of semi-major,minor axis is the inverse square root of the eigenvalues of the covariance matrix
    phoBranch.Smaj_  = ph2ndMoments.sMaj;
    phoBranch.Smin_  = ph2ndMoments.sMin;
    phoBranch.alpha_ = ph2ndMoments.alpha;
  }

  // map of rec hit ids
  uiiumap phrhIDmap;
  const DetIdPairVec & hitsAndFractions = phosc->hitsAndFractions();
  for (DetIdPairVec::const_iterator hafitr = hitsAndFractions.begin(); hafitr != hitsAndFractions.end(); ++hafitr) // loop over all rec hits in SC
  {
    const DetId & recHitId = hafitr->first; // get detid of crystal
    const uint32_t rawId = recHitId.rawId();
    if (recHitMap.count(rawId))
    {
      EcalRecHitCollection::const_iterator recHit = recHits->find(recHitId); // get the underlying rechit
      if (recHit != recHits->end()) // standard check
      { 
	phrhIDmap[recHitMap.at(rawId)]++;
      } // end standard check recHit
    } // end check over if recHit is in map
  } // end loop over hits and fractions

  // store the position inside the recHits vector 
  for (uiiumap::const_iterator rhiter = phrhIDmap.begin(); rhiter != phrhIDmap.end(); ++rhiter) // loop over only good rec hit ids
  {
    phoBranch.recHits_.emplace_back(rhiter->first);
  }

  // save seed id pos
  if (recHitMap.count(seedDetId.rawId())) 
  {
    phoBranch.seed_ = recHitMap.at(seedDetId.rawId());
  }

  // some standard booleans
  phoBranch.isOOT_ = photon.isOOT();
  phoBranch.isEB_  = isEB;

  // HLT Matching!
  strBitMap isHLTMatched;
  for (const auto & filter : filterNames) isHLTMatched[filter] = false;
  oot::HLTToObjectMatching(triggerObjectsByFilterMap,isHLTMatched,photon,pTres,dRmin);
  phoBranch.isHLT_ = isHLTMatched["hltEG60R9Id90CaloIdLIsoLDisplacedIdFilter"];

  // 0 --> did not pass anything, 1 --> loose pass, 2 --> medium pass, 3 --> tight pass
  if      (pho.photonID("tight"))  {phoBranch.ID_ = 3;}
  else if (pho.photonID("medium")) {phoBranch.ID_ = 2;}
  else if (pho.photonID("loose"))  {phoBranch.ID_ = 1;}
  else                             {phoBranch.ID_ = 0;}
}

void DisPho::InitializePhoBranchesMC()
{
  DisPho::InitializePhoBranchMC(phoBranch0);
  DisPho::InitializePhoBranchMC(phoBranch1);
  DisPho::InitializePhoBranchMC(phoBranch2);
  DisPho::InitializePhoBranchMC(phoBranch3);
}

void DisPho::InitializePhoBranchMC(phoStruct & phoBranch)
{
  if (isGMSB || isHVDS) phoBranch.isSignal_ = -9999;
  phoBranch.isGen_ = false;
}

void DisPho::SetPhoBranchMC(const int iph, const oot::Photon& photon, phoStruct& phoBranch, 
			    const edm::Handle<std::vector<reco::GenParticle> > & genparticlesH)
{
  if (isGMSB)
  {
    // matched to photon from leading neutralino == 1
    // matched to photon from subleading neutralino == 2
    // matched to both photons from both neutralinos (probably will never happen) == 3
    // no corresponding match == 0

    if      (iph == gmsbBranch0.genphmatch_ && iph != gmsbBranch1.genphmatch_) phoBranch.isSignal_ = 1;
    else if (iph != gmsbBranch0.genphmatch_ && iph == gmsbBranch1.genphmatch_) phoBranch.isSignal_ = 2;
    else if (iph == gmsbBranch0.genphmatch_ && iph == gmsbBranch1.genphmatch_) phoBranch.isSignal_ = 3; 
    else                                                                       phoBranch.isSignal_ = 0;
  } 

  if (isHVDS) 
  {
    // since we have four vPions to explore, store the result as an int mask
    // ones digit is leading vPion, tens as subleading, hundreds as subsub, etc.
    // matched to leading photon and not to subleading photon == 1
    // matched to subleading photon and not to leading photon == 2
    // matched to both photons inside vPion == 3
    // no corresponding match == 0

    const int match0 = DisPho::CheckMatchHVDS(iph,hvdsBranch0);
    const int match1 = DisPho::CheckMatchHVDS(iph,hvdsBranch1);
    const int match2 = DisPho::CheckMatchHVDS(iph,hvdsBranch2);
    const int match3 = DisPho::CheckMatchHVDS(iph,hvdsBranch3);

    phoBranch.isSignal_ = match0 + 10*match1 + 100*match2 + 1000*match3;
  }

  // standard dR matching
  phoBranch.isGen_ = oot::GenToObjectMatching(photon,genparticlesH,pTres,dRmin);
}

int DisPho::CheckMatchHVDS(const int iph, const hvdsStruct& hvdsBranch)
{
  if (iph == hvdsBranch.genHVph0match_ && iph != hvdsBranch.genHVph1match_) return 1;
  if (iph != hvdsBranch.genHVph0match_ && iph == hvdsBranch.genHVph1match_) return 2;
  if (iph == hvdsBranch.genHVph0match_ && iph == hvdsBranch.genHVph1match_) return 3;
  else                                                                      return 0;
}

void DisPho::beginJob() 
{
  edm::Service<TFileService> fs;
  tree = fs->make<TTree>("tree","tree");

  // Generic MC Info
  if (isMC)
  {
    tree->Branch("genwgt", &genwgt, "genwgt/F");
    tree->Branch("genpuobs", &genpuobs, "genpuobs/I");
    tree->Branch("genputrue", &genputrue, "genputrue/I");
  }

  // GMSB Info
  if (isGMSB)
  {
    tree->Branch("nNeutoPhGr", &nNeutoPhGr, "nNeutoPhGr/I");
    DisPho::MakeGMSBBranch(0,gmsbBranch0);
    DisPho::MakeGMSBBranch(1,gmsbBranch1);
  }

  // HVDS Info
  if (isHVDS)
  {
    tree->Branch("nvPions", &nvPions, "nvPions/I");
    DisPho::MakeHVDSBranch(0,hvdsBranch0);
    DisPho::MakeHVDSBranch(1,hvdsBranch1);
    DisPho::MakeHVDSBranch(2,hvdsBranch2);
    DisPho::MakeHVDSBranch(3,hvdsBranch3);
  }

  // Run, Lumi, Event info
  tree->Branch("run", &run, "run/i");
  tree->Branch("lumi", &lumi, "lumi/i");
  tree->Branch("event", &event, "event/l");
   
  // Trigger Info
  tree->Branch("hltDisPho", &hltDisPho, "hltDisPho/O");

  // Vertex info
  tree->Branch("nvtx", &nvtx, "nvtx/I");
  tree->Branch("vtxX", &vtxX, "vtxX/F");
  tree->Branch("vtxY", &vtxY, "vtxY/F");
  tree->Branch("vtxZ", &vtxZ, "vtxZ/F");

  // rho info
  tree->Branch("rho", &rho, "rho/F");

  // MET info
  tree->Branch("t1pfMETpt", &t1pfMETpt, "t1pfMETpt/F");
  tree->Branch("t1pfMETphi", &t1pfMETphi, "t1pfMETphi/F");
  tree->Branch("t1pfMETsumEt", &t1pfMETsumEt, "t1pfMETsumEt/F");

  // HT Info
  tree->Branch("jetHT", &jetHT, "jetHT/F");  

  // Jet Info
  tree->Branch("njets", &njets, "njets/I");
  DisPho::MakeJetBranch(0,jetBranch0);
  DisPho::MakeJetBranch(1,jetBranch1);
  DisPho::MakeJetBranch(2,jetBranch2);
  DisPho::MakeJetBranch(3,jetBranch3);

  // RecHit Info
  tree->Branch("nrechits", &nrechits, "nrechits/I");
  tree->Branch("rheta", &rheta);
  tree->Branch("rhphi", &rhphi);
  tree->Branch("rhE", &rhE);
  tree->Branch("rhtime", &rhtime);
  tree->Branch("rhOOT", &rhOOT);
  tree->Branch("rhID", &rhID);

  // Photon Info
  tree->Branch("nphotons", &nphotons, "nphotons/I");
  DisPho::MakePhoBranch(0,phoBranch0);
  DisPho::MakePhoBranch(1,phoBranch1);
  DisPho::MakePhoBranch(2,phoBranch2);
  DisPho::MakePhoBranch(3,phoBranch3);

  // PhotonMC info
  if (isMC)
  {
    DisPho::MakePhoBranchMC(0,phoBranch0);
    DisPho::MakePhoBranchMC(1,phoBranch1);
    DisPho::MakePhoBranchMC(2,phoBranch2);
    DisPho::MakePhoBranchMC(3,phoBranch3);
  }
}

void DisPho::MakeGMSBBranch(const int i, gmsbStruct& gmsbBranch)
{
  tree->Branch(Form("genNmass%i",i), &gmsbBranch.genNmass_, Form("genNmass%i/F",i));
  tree->Branch(Form("genNE%i",i), &gmsbBranch.genNE_, Form("genNE%i/F",i));
  tree->Branch(Form("genNpt%i",i), &gmsbBranch.genNpt_, Form("genNpt%i/F",i));
  tree->Branch(Form("genNphi%i",i), &gmsbBranch.genNphi_, Form("genNphi%i/F",i));
  tree->Branch(Form("genNeta%i",i), &gmsbBranch.genNeta_, Form("genNeta%i/F",i));

  tree->Branch(Form("genNprodvx%i",i), &gmsbBranch.genNprodvx_, Form("genNprodvx%i/F",i));
  tree->Branch(Form("genNprodvy%i",i), &gmsbBranch.genNprodvy_, Form("genNprodvy%i/F",i));
  tree->Branch(Form("genNprodvz%i",i), &gmsbBranch.genNprodvz_, Form("genNprodvz%i/F",i));

  tree->Branch(Form("genNdecayvx%i",i), &gmsbBranch.genNdecayvx_, Form("genNdecayvx%i/F",i));
  tree->Branch(Form("genNdecayvy%i",i), &gmsbBranch.genNdecayvy_, Form("genNdecayvy%i/F",i));
  tree->Branch(Form("genNdecayvz%i",i), &gmsbBranch.genNdecayvz_, Form("genNdecayvz%i/F",i));

  tree->Branch(Form("genphE%i",i), &gmsbBranch.genphE_, Form("genphE%i/F",i));
  tree->Branch(Form("genphpt%i",i), &gmsbBranch.genphpt_, Form("genphpt%i/F",i));
  tree->Branch(Form("genphphi%i",i), &gmsbBranch.genphphi_, Form("genphphi%i/F",i));
  tree->Branch(Form("genpheta%i",i), &gmsbBranch.genpheta_, Form("genpheta%i/F",i));
  tree->Branch(Form("genphmatch%i",i), &gmsbBranch.genphmatch_, Form("genphmatch%i/I",i));

  tree->Branch(Form("gengrmass%i",i), &gmsbBranch.gengrmass_, Form("gengrmass%i/F",i));
  tree->Branch(Form("gengrE%i",i), &gmsbBranch.gengrE_, Form("gengrE%i/F",i));
  tree->Branch(Form("gengrpt%i",i), &gmsbBranch.gengrpt_, Form("gengrpt%i/F",i));
  tree->Branch(Form("gengrphi%i",i), &gmsbBranch.gengrphi_, Form("gengrphi%i/F",i));
  tree->Branch(Form("gengreta%i",i), &gmsbBranch.gengreta_, Form("gengreta%i/F",i));
}

void DisPho::MakeHVDSBranch(const int i, hvdsStruct& hvdsBranch)
{
  tree->Branch(Form("genvPionmass%i",i), &hvdsBranch.genvPionmass_, Form("genvPionmass%i/F",i));
  tree->Branch(Form("genvPionE%i",i), &hvdsBranch.genvPionE_, Form("genvPionE%i/F",i));
  tree->Branch(Form("genvPionpt%i",i), &hvdsBranch.genvPionpt_, Form("genvPionpt%i/F",i));
  tree->Branch(Form("genvPionphi%i",i), &hvdsBranch.genvPionphi_, Form("genvPionphi%i/F",i));
  tree->Branch(Form("genvPioneta%i",i), &hvdsBranch.genvPioneta_, Form("genvPioneta%i/F",i));

  tree->Branch(Form("genvPionprodvx%i",i), &hvdsBranch.genvPionprodvx_, Form("genvPionprodvx%i/F",i));
  tree->Branch(Form("genvPionprodvy%i",i), &hvdsBranch.genvPionprodvy_, Form("genvPionprodvy%i/F",i));
  tree->Branch(Form("genvPionprodvz%i",i), &hvdsBranch.genvPionprodvz_, Form("genvPionprodvz%i/F",i));

  tree->Branch(Form("genvPiondecayvx%i",i), &hvdsBranch.genvPiondecayvx_, Form("genvPiondecayvx%i/F",i));
  tree->Branch(Form("genvPiondecayvy%i",i), &hvdsBranch.genvPiondecayvy_, Form("genvPiondecayvy%i/F",i));
  tree->Branch(Form("genvPiondecayvz%i",i), &hvdsBranch.genvPiondecayvz_, Form("genvPiondecayvz%i/F",i));

  tree->Branch(Form("genHVph0E%i",i), &hvdsBranch.genHVph0E_, Form("genHVph0E%i/F",i));
  tree->Branch(Form("genHVph0pt%i",i), &hvdsBranch.genHVph0pt_, Form("genHVph0pt%i/F",i));
  tree->Branch(Form("genHVph0phi%i",i), &hvdsBranch.genHVph0phi_, Form("genHVph0phi%i/F",i));
  tree->Branch(Form("genHVph0eta%i",i), &hvdsBranch.genHVph0eta_, Form("genHVph0eta%i/F",i));
  tree->Branch(Form("genHVph0match%i",i), &hvdsBranch.genHVph0match_, Form("genHVph0match%i/I",i));

  tree->Branch(Form("genHVph1E%i",i), &hvdsBranch.genHVph1E_, Form("genHVph1E%i/F",i));
  tree->Branch(Form("genHVph1pt%i",i), &hvdsBranch.genHVph1pt_, Form("genHVph1pt%i/F",i));
  tree->Branch(Form("genHVph1phi%i",i), &hvdsBranch.genHVph1phi_, Form("genHVph1phi%i/F",i));
  tree->Branch(Form("genHVph1eta%i",i), &hvdsBranch.genHVph1eta_, Form("genHVph1eta%i/F",i));
  tree->Branch(Form("genHVph1match%i",i), &hvdsBranch.genHVph1match_, Form("genHVph1match%i/I",i));
}

void DisPho::MakeJetBranch(const int i, jetStruct& jetBranch)
{
  tree->Branch(Form("jetE%i",i), &jetBranch.E_, Form("jetE%i/F",i));
  tree->Branch(Form("jetpt%i",i), &jetBranch.Pt_, Form("jetpt%i/F",i));
  tree->Branch(Form("jeteta%i",i), &jetBranch.Eta_, Form("jeteta%i/F",i));
  tree->Branch(Form("jetphi%i",i), &jetBranch.Phi_, Form("jetphi%i/F",i));
}

void DisPho::MakePhoBranch(const int i, phoStruct& phoBranch)
{
  tree->Branch(Form("phoE%i",i), &phoBranch.E_, Form("phoE%i/F",i));
  tree->Branch(Form("phopt%i",i), &phoBranch.Pt_, Form("phopt%i/F",i));
  tree->Branch(Form("phoeta%i",i), &phoBranch.Eta_, Form("phoeta%i/F",i));
  tree->Branch(Form("phophi%i",i), &phoBranch.Phi_, Form("phophi%i/F",i));

  tree->Branch(Form("phoscE%i",i), &phoBranch.scE_, Form("phoscE%i/F",i));
  tree->Branch(Form("phosceta%i",i), &phoBranch.scEta_, Form("phosceta%i/F",i));
  tree->Branch(Form("phoscphi%i",i), &phoBranch.scPhi_, Form("phoscphi%i/F",i));

  tree->Branch(Form("phoHadTowOE%i",i), &phoBranch.HadTowOE_, Form("phoHadTowOE%i/F",i));
  tree->Branch(Form("phoHadronOE%i",i), &phoBranch.HadronOE_, Form("phoHadronOE%i/F",i));
  tree->Branch(Form("phor9%i",i), &phoBranch.r9_, Form("phor9%i/F",i));
  tree->Branch(Form("phofullr9%i",i), &phoBranch.fullr9_, Form("phofullr9%i/F",i));
  tree->Branch(Form("phoChgHadIso%i",i), &phoBranch.ChgHadIso_, Form("phoChgHadIso%i/F",i));
  tree->Branch(Form("phoNeuHadIso%i",i), &phoBranch.NeuHadIso_, Form("phoNeuHadIso%i/F",i));
  tree->Branch(Form("phoPhoIso%i",i), &phoBranch.PhoIso_, Form("phoPhoIso%i/F",i));

  tree->Branch(Form("phoEcalPFClIso%i",i), &phoBranch.EcalPFClIso_, Form("phoEcalPFClIso%i/F",i));
  tree->Branch(Form("phoHcalPFClIso%i",i), &phoBranch.HcalPFClIso_, Form("phoHcalPFClIso%i/F",i));
  tree->Branch(Form("phoTrkIso%i",i), &phoBranch.TrkIso_, Form("phoTrkIso%i/F",i));

  tree->Branch(Form("phosmaj%i",i), &phoBranch.Smaj_, Form("phosmaj%i/F",i));
  tree->Branch(Form("phosmin%i",i), &phoBranch.Smin_, Form("phosmin%i/F",i));
  tree->Branch(Form("phoalpha%i",i), &phoBranch.alpha_, Form("phoalpha%i/F",i));

  tree->Branch(Form("phoseed%i",i), &phoBranch.seed_, Form("phoseed%i/I",i));
  tree->Branch(Form("phorecHis%i",i), &phoBranch.recHits_);

  tree->Branch(Form("phoisOOT%i",i), &phoBranch.isOOT_, Form("phoisOOT%i/O",i));
  tree->Branch(Form("phoisEB%i",i), &phoBranch.isEB_, Form("phoisEB%i/O",i));
  tree->Branch(Form("phoisHLT%i",i), &phoBranch.isHLT_, Form("phoisHLT%i/O",i));
  tree->Branch(Form("phoID%i",i), &phoBranch.ID_, Form("phoID%i/I",i));
}

void DisPho::MakePhoBranchMC(const int i, phoStruct& phoBranch)
{
  if (isGMSB || isHVDS) tree->Branch(Form("isSignal%i",i), &phoBranch.isSignal_, Form("isSignal%i/I",i));
  tree->Branch(Form("isGen%i",i), &phoBranch.isGen_, Form("isGen%i/O",i));
}

void DisPho::endJob() {}

void DisPho::beginRun(edm::Run const& iRun, edm::EventSetup const& iSetup) {}

void DisPho::endRun(edm::Run const&, edm::EventSetup const&) {}

void DisPho::fillDescriptions(edm::ConfigurationDescriptions& descriptions) 
{
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

DEFINE_FWK_MODULE(DisPho);
