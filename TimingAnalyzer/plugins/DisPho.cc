#include "Timing/TimingAnalyzer/plugins/DisPho.hh"

DisPho::DisPho(const edm::ParameterSet & iConfig) :
  // blinding cuts
  blindSF(iConfig.existsAs<int>("blindSF") ? iConfig.getParameter<int>("blindSF") : 1000),
  applyBlindSF(iConfig.existsAs<bool>("applyBlindSF") ? iConfig.getParameter<bool>("applyBlindSF") : false),
  blindMET(iConfig.existsAs<double>("blindMET") ? iConfig.getParameter<double>("blindMET") : 100.f),
  applyBlindMET(iConfig.existsAs<bool>("applyBlindMET") ? iConfig.getParameter<bool>("applyBlindMET") : false),

  // object prep cuts
  jetpTmin(iConfig.existsAs<double>("jetpTmin") ? iConfig.getParameter<double>("jetpTmin") : 15.f),
  jetEtamax(iConfig.existsAs<double>("jetEtamax") ? iConfig.getParameter<double>("jetEtamax") : 3.f),
  jetIDmin(iConfig.existsAs<int>("jetIDmin") ? iConfig.getParameter<int>("jetIDmin") : 1),
  rhEmin(iConfig.existsAs<double>("rhEmin") ? iConfig.getParameter<double>("rhEmin") : 1.f),
  phpTmin(iConfig.existsAs<double>("phpTmin") ? iConfig.getParameter<double>("phpTmin") : 20.f),
  phIDmin(iConfig.existsAs<std::string>("phIDmin") ? iConfig.getParameter<std::string>("phIDmin") : "loose"),

  // object extra pruning cuts
  seedTimemin(iConfig.existsAs<double>("seedTimemin") ? iConfig.getParameter<double>("seedTimemin") : -25.f),
  nPhosmax(iConfig.existsAs<int>("nPhosmax") ? iConfig.getParameter<int>("nPhosmax") : 2),

  // photon storing
  splitPho(iConfig.existsAs<bool>("splitPho") ? iConfig.getParameter<bool>("splitPho") : false),
  onlyGED (iConfig.existsAs<bool>("onlyGED")  ? iConfig.getParameter<bool>("onlyGED")  : false),
  onlyOOT (iConfig.existsAs<bool>("onlyOOT")  ? iConfig.getParameter<bool>("onlyOOT")  : false),

  // lepton prep cuts
  ellowpTmin(iConfig.existsAs<double>("ellowpTmin") ? iConfig.getParameter<double>("ellowpTmin") : 20.f),
  elhighpTmin(iConfig.existsAs<double>("elhighpTmin") ? iConfig.getParameter<double>("elhighpTmin") : 50.f),
  mulowpTmin(iConfig.existsAs<double>("mulowpTmin") ? iConfig.getParameter<double>("mulowpTmin") : 20.f),
  muhighpTmin(iConfig.existsAs<double>("muhighpTmin") ? iConfig.getParameter<double>("muhighpTmin") : 50.f),

  // recHits storing
  storeRecHits(iConfig.existsAs<bool>("storeRecHits") ? iConfig.getParameter<bool>("storeRecHits") : true),

  // pre-selection
  applyTrigger(iConfig.existsAs<bool>("applyTrigger") ? iConfig.getParameter<bool>("applyTrigger") : false),
  minHT(iConfig.existsAs<double>("minHT") ? iConfig.getParameter<double>("minHT") : 400.f),
  applyHT(iConfig.existsAs<bool>("applyHT") ? iConfig.getParameter<bool>("applyHT") : false),
  phgoodpTmin(iConfig.existsAs<double>("phgoodpTmin") ? iConfig.getParameter<double>("phgoodpTmin") : 70.f),
  phgoodIDmin(iConfig.existsAs<std::string>("phgoodIDmin") ? iConfig.getParameter<std::string>("phgoodIDmin") : "loose"),
  applyPhGood(iConfig.existsAs<bool>("applyPhGood") ? iConfig.getParameter<bool>("applyPhGood") : false),

  // matching criteria
  dRmin(iConfig.existsAs<double>("dRmin") ? iConfig.getParameter<double>("dRmin") : 0.3),
  pTres(iConfig.existsAs<double>("pTres") ? iConfig.getParameter<double>("pTres") : 100.0),
  gendRmin(iConfig.existsAs<double>("gendRmin") ? iConfig.getParameter<double>("gendRmin") : 0.1),
  genpTres(iConfig.existsAs<double>("genpTres") ? iConfig.getParameter<double>("genpTres") : 0.5),
  trackdRmin(iConfig.existsAs<double>("trackdRmin") ? iConfig.getParameter<double>("trackdRmin") : 0.2),
  trackpTmin(iConfig.existsAs<double>("trackpTmin") ? iConfig.getParameter<double>("trackpTmin") : 5.f),
  genjetdRmin(iConfig.existsAs<double>("genjetdRmin") ? iConfig.getParameter<double>("genjetdRmin") : 0.2), // 0.4 from AK4PF / 2
  genjetpTfactor(iConfig.existsAs<double>("genjetpTfactor") ? iConfig.getParameter<double>("genjetpTfactor") : 3.f),
  leptondRmin(iConfig.existsAs<double>("leptondRmin") ? iConfig.getParameter<double>("leptondRmin") : 0.3),

  // JER extra info
  smearjetEmin(iConfig.existsAs<double>("smearjetEmin") ? iConfig.getParameter<double>("smearjetEmin") : 0.01),

  // triggers
  inputPaths       (iConfig.existsAs<std::string>("inputPaths")   ? iConfig.getParameter<std::string>("inputPaths") : ""),
  inputFilters     (iConfig.existsAs<std::string>("inputFilters") ? iConfig.getParameter<std::string>("inputFilters") : ""),
  triggerResultsTag(iConfig.getParameter<edm::InputTag>("triggerResults")),
  triggerObjectsTag(iConfig.getParameter<edm::InputTag>("triggerObjects")),
  
  // MET flags
  inputFlags         (iConfig.existsAs<std::string>("inputFlags") ? iConfig.getParameter<std::string>("inputFlags") : ""),
  triggerFlagsTag    (iConfig.getParameter<edm::InputTag>("triggerFlags")),
  ecalBadCalibFlagTag(iConfig.getParameter<edm::InputTag>("ecalBadCalibFlag")),

  // tracks
  tracksTag(iConfig.getParameter<edm::InputTag>("tracks")),

  // vertices
  verticesTag(iConfig.getParameter<edm::InputTag>("vertices")),

  // rho
  rhoTag(iConfig.getParameter<edm::InputTag>("rho")),

  // mets
  metsTag(iConfig.getParameter<edm::InputTag>("mets")),  

  // jets
  jetsTag(iConfig.getParameter<edm::InputTag>("jets")),  

  // electrons
  electronsTag(iConfig.getParameter<edm::InputTag>("electrons")),  

  // muons
  muonsTag(iConfig.getParameter<edm::InputTag>("muons")),  

  // recHits
  recHitsEBTag(iConfig.getParameter<edm::InputTag>("recHitsEB")),  
  recHitsEETag(iConfig.getParameter<edm::InputTag>("recHitsEE")),

  // photons
  gedPhotonsTag(iConfig.getParameter<edm::InputTag>("gedPhotons")),
  ootPhotonsTag(iConfig.getParameter<edm::InputTag>("ootPhotons")),

  // isMC
  isGMSB(iConfig.existsAs<bool>("isGMSB") ? iConfig.getParameter<bool>("isGMSB") : false),
  isHVDS(iConfig.existsAs<bool>("isHVDS") ? iConfig.getParameter<bool>("isHVDS") : false),
  isBkgd(iConfig.existsAs<bool>("isBkgd") ? iConfig.getParameter<bool>("isBkgd") : false),
  isToy (iConfig.existsAs<bool>("isToy")  ? iConfig.getParameter<bool>("isToy")  : false),
  isADD (iConfig.existsAs<bool>("isADD")  ? iConfig.getParameter<bool>("isADD")  : false),
  
  // MC config
  xsec(iConfig.existsAs<double>("xsec") ? iConfig.getParameter<double>("xsec") : 1.0),
  filterEff(iConfig.existsAs<double>("filterEff") ? iConfig.getParameter<double>("filterEff") : 1.0),
  BR(iConfig.existsAs<double>("BR") ? iConfig.getParameter<double>("BR") : 1.0),

  // gen objects
  genEvtInfoTag(iConfig.getParameter<edm::InputTag>("genEvt")),
  gent0Tag(iConfig.getParameter<edm::InputTag>("gent0")),
  genxyz0Tag(iConfig.getParameter<edm::InputTag>("genxyz0")),
  pileupInfosTag(iConfig.getParameter<edm::InputTag>("pileups")),
  genParticlesTag(iConfig.getParameter<edm::InputTag>("genParticles")),
  genJetsTag(iConfig.getParameter<edm::InputTag>("genJets"))
{
  // internal setup
  usesResource();
  usesResource("TFileService");

  // set isMC
  if (isGMSB || isHVDS || isBkgd || isToy || isADD) isMC = true;
  else                                              isMC = false;             

  // setup tokens
  DisPho::ConsumeTokens();

  // setup labels for cut flow histogram
  DisPho::SetupCutFlowLabels();

  // read in from a stream the trigger paths for saving
  oot::ReadInTriggerNames(inputPaths,pathNames,triggerBitMap);

  // read in from a stream the hlt objects/labels to match to
  oot::ReadInFilterNames(inputFilters,filterNames,triggerObjectsByFilterMap);

  // read in from a stream the trigger paths for saving
  oot::ReadInTriggerNames(inputFlags,flagNames,triggerFlagMap);
}

void DisPho::ConsumeTokens()
{
  // triggers
  triggerResultsToken = consumes<edm::TriggerResults> (triggerResultsTag);
  triggerObjectsToken = consumes<std::vector<pat::TriggerObjectStandAlone> > (triggerObjectsTag);

  // MET flags
  triggerFlagsToken     = consumes<edm::TriggerResults> (triggerFlagsTag);
  ecalBadCalibFlagToken = consumes<bool> (ecalBadCalibFlagTag);

  // tracks 
  tracksToken = consumes<std::vector<reco::Track> > (tracksTag);

  // vertices
  verticesToken = consumes<std::vector<reco::Vertex> > (verticesTag);

  // rho
  rhoToken = consumes<double> (rhoTag);

  // mets
  metsToken = consumes<std::vector<pat::MET> > (metsTag);

  // jets
  jetsToken = consumes<std::vector<pat::Jet> > (jetsTag);

  // leptons
  electronsToken = consumes<std::vector<pat::Electron> > (electronsTag);
  muonsToken = consumes<std::vector<pat::Muon> > (muonsTag);

  // rechits
  recHitsEBToken = consumes<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > (recHitsEBTag);
  recHitsEEToken = consumes<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > (recHitsEETag);

  // photons
  gedPhotonsToken = consumes<std::vector<pat::Photon> > (gedPhotonsTag);
  ootPhotonsToken = consumes<std::vector<pat::Photon> > (ootPhotonsTag);

  // only for simulated samples
  if (isMC)
  {
    genEvtInfoToken   = consumes<GenEventInfoProduct>             (genEvtInfoTag);
    gent0Token        = consumes<float>                           (gent0Tag);
    genxyz0Token      = consumes<Point3D>                         (genxyz0Tag);
    pileupInfosToken  = consumes<std::vector<PileupSummaryInfo> > (pileupInfosTag);
    genParticlesToken = consumes<std::vector<reco::GenParticle> > (genParticlesTag);
    genJetsToken      = consumes<std::vector<reco::GenJet> >      (genJetsTag);
  }
}

void DisPho::SetupCutFlowLabels()
{
  std::vector<std::string> cutflowLabelVec = {"All","nEvBlinding","METBlinding","Trigger","H_{T}","Good Photon"};
  auto ibin = 0;
  for (const auto & cutflowLabel : cutflowLabelVec) cutflowLabelMap[cutflowLabel] = ibin++;
}

DisPho::~DisPho() {}

void DisPho::analyze(const edm::Event & iEvent, const edm::EventSetup & iSetup)
{
  /////////////////
  // Get Objects //
  /////////////////

  if (!DisPho::GetObjects(iEvent,iSetup)) return;

  ////////////////////////
  // Initialize Objects //
  ////////////////////////  

  DisPho::InitializeObjects(iEvent);

  /////////////////
  // Get Weights //
  /////////////////

  DisPho::GetWeights();

  ///////////////////////
  // Always fill hists //
  ///////////////////////

  DisPho::AlwaysFillHists();

  ////////////////////
  // Apply Blinding //
  ////////////////////

  if (applyBlindSF && DisPho::ApplyBlindSF()) return;
  DisPho::FillBlindSF();

  // Get MET corrected in time for apply blinding MET --> early object prep
  oot::PrepPhotonsCorrectMET(gedPhotonsH,ootPhotonsH,photons,t1pfMET,rho,dRmin,phpTmin,phIDmin);

  if (applyBlindMET && DisPho::ApplyBlindMET()) return;
  DisPho::FillBlindMET();
  
  //////////////////
  // Prep Objects //
  //////////////////

  DisPho::PrepObjects(iEvent);

  /////////////////////////
  // Apply pre-selection //
  /////////////////////////

  if (applyTrigger && DisPho::ApplyPreSelectionTrigger()) return;
  DisPho::FillPreSelectionTrigger();

  if (applyHT && DisPho::ApplyPreSelectionHT()) return;
  DisPho::FillPreSelectionHT();

  if (applyPhGood && DisPho::ApplyPreSelectionGoodPhoton()) return;
  DisPho::FillPreSelectionGoodPhoton();

  ////////////////////////////
  // Fill Tree From Objects //
  ////////////////////////////

  DisPho::FillTreeFromObjects(iEvent);
}

///////////////////////////
// Object Prep Functions //
///////////////////////////

// get handles from tokens --> ensure they exist!
bool DisPho::GetObjects(const edm::Event & iEvent, const edm::EventSetup & iSetup)
{
  //////////////////////
  // Standard Objects //
  //////////////////////

  if (!DisPho::GetStandardObjects(iEvent)) return false;

  ///////////////////////////
  // Calibration Constants //
  ///////////////////////////
  
  if (!DisPho::GetCalibrationConstants(iSetup)) return false;

  ////////////////
  // MC Objects //
  ////////////////

  if (isMC)
  {
    if (!DisPho::GetMCObjects(iEvent)) return false;
  }

  // if no bad handles, return true
  return true;
}

bool DisPho::GetStandardObjects(const edm::Event & iEvent)
{
  // TRIGGER RESULTS
  iEvent.getByToken(triggerResultsToken,triggerResultsH);
  if (oot::BadHandle(triggerResultsH,"triggerResults")) return false;
      
  // TRIGGER OBJECTS
  iEvent.getByToken(triggerObjectsToken,triggerObjectsH);
  if (oot::BadHandle(triggerObjectsH,"triggerObjects")) return false;

  // MET FLAGS
  iEvent.getByToken(triggerFlagsToken,triggerFlagsH);
  if (oot::BadHandle(triggerFlagsH,"triggerFlags")) return false;

  iEvent.getByToken(ecalBadCalibFlagToken,ecalBadCalibFlagH);
  if (oot::BadHandle(ecalBadCalibFlagH,"ecalBadCalibFlag")) return false;

  // TRACKS
  iEvent.getByToken(tracksToken,tracksH);
  if (oot::BadHandle(tracksH,"tracks")) return false;

  // VERTICES
  iEvent.getByToken(verticesToken,verticesH);
  if (oot::BadHandle(verticesH,"vertices")) return false;

  // RHO
  iEvent.getByToken(rhoToken,rhoH);
  if (oot::BadHandle(rhoH,"rho")) return false;

  // METS
  iEvent.getByToken(metsToken,metsH);
  if (oot::BadHandle(metsH,"mets")) return false;

  // JETS
  iEvent.getByToken(jetsToken,jetsH);
  if (oot::BadHandle(jetsH,"jets")) return false;

  // LEPTONS
  iEvent.getByToken(electronsToken,electronsH);
  if (oot::BadHandle(electronsH,"electrons")) return false;

  iEvent.getByToken(muonsToken,muonsH);
  if (oot::BadHandle(muonsH,"muons")) return false;

  // ECAL RECHITS
  iEvent.getByToken(recHitsEBToken,recHitsEBH);
  if (oot::BadHandle(recHitsEBH,"recHitsEB")) return false;

  iEvent.getByToken(recHitsEEToken,recHitsEEH);
  if (oot::BadHandle(recHitsEEH,"recHitsEE")) return false;

  // PHOTONS
  iEvent.getByToken(gedPhotonsToken,gedPhotonsH);
  if (oot::BadHandle(gedPhotonsH,"gedPhotons")) return false;

  iEvent.getByToken(ootPhotonsToken,ootPhotonsH);
  if (oot::BadHandle(ootPhotonsH,"ootPhotons")) return false;

  // if no bad handles, return true
  return true;
}

bool DisPho::GetCalibrationConstants(const edm::EventSetup & iSetup)
{
  // GEOMETRY : https://gitlab.cern.ch/shervin/ECALELF
  iSetup.get<CaloGeometryRecord>().get(caloGeoH);
  if (oot::BadHandle(caloGeoH,"caloGeo")) return false;

  //////////////////
  // ECAL RECORDS // 
  //////////////////

  // Laser constants : http://cmslxr.fnal.gov/source/RecoEcal/EgammaCoreTools/interface/EcalClusterLazyTools.h
  iSetup.get<EcalLaserDbRecord>().get(laserH);
  if (oot::BadHandle(laserH,"laser")) return false;

  // Intercalibration constants : http://cmslxr.fnal.gov/source/RecoEcal/EgammaCoreTools/interface/EcalClusterLazyTools.h
  iSetup.get<EcalIntercalibConstantsRcd>().get(interCalibH);
  if (oot::BadHandle(interCalibH,"interCalib")) return false;
  
  // ADCToGeV : http://cmslxr.fnal.gov/source/RecoEcal/EgammaCoreTools/interface/EcalClusterLazyTools.h
  iSetup.get<EcalADCToGeVConstantRcd>().get(adcToGeVH);
  if (oot::BadHandle(adcToGeVH,"adcToGeV")) return false;

  // Pedestals : https://github.com/ferriff/usercode/blob/master/DBDump/plugins/DBDump.cc
  iSetup.get<EcalPedestalsRcd>().get(pedestalsH);
  if (oot::BadHandle(pedestalsH,"pedestals")) return false;

  /////////////////
  // JET DB INFO //
  /////////////////

  // JECs : https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookJetEnergyCorrections#CorrPatJets  
  iSetup.get<JetCorrectionsRecord>().get("AK4PFchs",jetCorrH); 
  if (oot::BadHandle(jetCorrH,"jetCorr")) return false;

  // JERs : https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookJetEnergyResolution#Accessing_factors_from_Global_Ta
  jetRes = JME::JetResolution::get(iSetup,"AK4PFchs_pt");
  jetRes_sf = JME::JetResolutionScaleFactor::get(iSetup,"AK4PFchs");

  // if no bad handles, return true
  return true;
}

bool DisPho::GetMCObjects(const edm::Event & iEvent)
{
  //////////////
  // GEN INFO //
  //////////////

  iEvent.getByToken(genEvtInfoToken,genEvtInfoH);
  if (oot::BadHandle(genEvtInfoH,"genEvtInfo")) return false;
  
  iEvent.getByToken(gent0Token,gent0H);
  if (oot::BadHandle(gent0H,"gent0")) return false;
  
  iEvent.getByToken(genxyz0Token,genxyz0H);
  if (oot::BadHandle(genxyz0H,"genxyz0")) return false;
  
  iEvent.getByToken(pileupInfosToken,pileupInfosH);
  if (oot::BadHandle(pileupInfosH,"pileupInfos")) return false;
  
  iEvent.getByToken(genParticlesToken,genParticlesH);
  if (oot::BadHandle(genParticlesH,"genParticles")) return false;
  
  iEvent.getByToken(genJetsToken,genJetsH);
  if (oot::BadHandle(genJetsH,"genJets")) return false;

  // no bad handles, so return that everything is ok!
  return true;
}

void DisPho::InitializeObjects(const edm::Event & iEvent)
{
  // INPUT + OUTPUT RHO
  rho = *(rhoH.product());

  // OUTPUT MET
  t1pfMET = pat::MET(metsH->front());

  // OUPUT JETS
  jets.clear(); 
  jets.reserve(jetsH->size());

  // OUTPUT ELECTRONS
  electrons.clear(); 
  electrons.reserve(electronsH->size());

  // OUTPUT MUONS
  muons.clear(); 
  muons.reserve(muonsH->size());

  // INPUT ECAL RECHITS
  recHitsEB = recHitsEBH.product();
  recHitsEE = recHitsEEH.product();

  // OUTPUT RECHIT MAP
  recHitMap.clear();

  // OUTPUT PHOTONS
  photons.clear();
  photons.reserve(gedPhotonsH->size()+ootPhotonsH->size());

  // GEOMETRY
  barrelGeometry = caloGeoH->getSubdetectorGeometry(DetId::Ecal, EcalSubdetector::EcalBarrel);
  endcapGeometry = caloGeoH->getSubdetectorGeometry(DetId::Ecal, EcalSubdetector::EcalEndcap);

  // LASER
  evTime = iEvent.time();

  // INTERCALIB
  interCalibMap = &interCalibH->getMap();
  
  // ADCTOGEV
  adcToGeVEB = adcToGeVH->getEBValue();
  adcToGeVEE = adcToGeVH->getEEValue();
  
  // GEN INFO
  if (isMC)
  {
    if (isGMSB) neutralinos.clear();
    if (isHVDS) vPions.clear();
    if (isToy) toys.clear();
  }
}
 
void DisPho::GetWeights()
{
  /////////////////
  // gen weights //
  /////////////////

  if (isMC)
  {  
    ///////////////////////
    // Event weight info //
    ///////////////////////

    genwgt = genEvtInfoH->weight();
    
    /////////////////////
    // Gen pileup info //
    /////////////////////

    DisPho::InitializeGenPUBranches();
    DisPho::SetGenPUBranches();
  } // end check on isMC

  //////////////////////
  // Set Event Weight //
  //////////////////////

  wgt = (isMC ? genwgt : 1.f);
}

void DisPho::InitializeGenPUBranches()
{
  genpuobs = -9999; 
  genputrue = -9999;
}

void DisPho::SetGenPUBranches()
{
  for (const auto & pileupInfo : *pileupInfosH)
  {
    if (pileupInfo.getBunchCrossing() == 0) 
    {
      genpuobs  = pileupInfo.getPU_NumInteractions();
      genputrue = pileupInfo.getTrueNumInteractions();
      
      break;
    } // end check over correct BX
  } // end loop over PU
}

void DisPho::AlwaysFillHists()
{
  // Fill total cutflow regardless of cuts
  h_cutflow    ->Fill((cutflowLabelMap["All"]*1.f));
  h_cutflow_wgt->Fill((cutflowLabelMap["All"]*1.f),wgt);

  // Fill PU hists regardless of cuts
  if (isMC)
  {
    h_genpuobs     ->Fill(genpuobs);
    h_genpuobs_wgt ->Fill(genpuobs,wgt);
    h_genputrue    ->Fill(genputrue);
    h_genputrue_wgt->Fill(genputrue,wgt);
  }
}

void DisPho::PrepObjects(const edm::Event & iEvent)
{
  /////////////////////
  // Object Prepping //
  /////////////////////

  if (isGMSB) oot::PrepNeutralinos(genParticlesH,neutralinos);
  if (isHVDS) oot::PrepVPions(genParticlesH,vPions);
  if (isToy)  oot::PrepToys(genParticlesH,toys);
  oot::PrepTriggerBits(triggerResultsH,iEvent,triggerBitMap);
  oot::PrepTriggerBits(triggerFlagsH,iEvent,triggerFlagMap);
  oot::PrepTriggerObjects(triggerResultsH,triggerObjectsH,iEvent,triggerObjectsByFilterMap);
  oot::PrepJets(jetsH,jets,jetpTmin,jetEtamax,jetIDmin);
  oot::PrepRecHits(recHitsEB,recHitsEE,recHitMap,rhEmin);

  ///////////////////
  // Extra Pruning //
  ///////////////////

  oot::PrunePhotons(photons,recHitsEB,recHitsEE,seedTimemin);
  oot::PruneJets(jets,photons,nPhosmax,dRmin);

  /////////////////////////////
  // Photon Storing Options  //
  /////////////////////////////

  if (splitPho) oot::SplitPhotons(photons,Config::nPhotons/2);     // split photons by OOT and GED (store at most nPhotons/2 of each)
  if (onlyGED)  oot::StoreOnlyPho(photons,Config::nPhotons,false); // store only GED photons, top nPhotons only
  if (onlyOOT)  oot::StoreOnlyPho(photons,Config::nPhotons,true);  // store only OOT photons, top nPhotons only

  /////////////////////
  // Lepton Prepping //
  /////////////////////

  oot::PrepLeptons(electronsH,electrons,photons,ellowpTmin,leptondRmin); // consider only electrons NOT matched to our photons
  oot::PrepLeptons(muonsH,muons,photons,mulowpTmin,leptondRmin); // consider only muons NOT matched to our photons
		
  ///////////////////////////////
  // Object Counts for Storing //
  ///////////////////////////////

  nJets    = std::min(int(jets.size()),Config::nJets);
  nRecHits = recHitMap.size();
  nPhotons = std::min(int(photons.size()),Config::nPhotons);
}

////////////////////////////////
// Blinding and Pre-Selection //
////////////////////////////////

inline bool DisPho::ApplyBlindSF()
{
  return (event%blindSF!=0);
}

void DisPho::FillBlindSF()
{
  h_cutflow    ->Fill((cutflowLabelMap["nEvBlinding"]*1.f));
  h_cutflow_wgt->Fill((cutflowLabelMap["nEvBlinding"]*1.f),wgt);
}

inline bool DisPho::ApplyBlindMET()
{
  return (t1pfMET.userFloat(Config::OOTMETPt) > blindMET);
}

void DisPho::FillBlindMET()
{
  h_cutflow    ->Fill((cutflowLabelMap["METBlinding"]*1.f));
  h_cutflow_wgt->Fill((cutflowLabelMap["METBlinding"]*1.f),wgt);
}

bool DisPho::ApplyPreSelectionTrigger()
{
  auto triggered = false;
  
  for (const auto & triggerBitPair : triggerBitMap)
  {
    if (triggerBitPair.second) {triggered = true; break;}
  }
  
  return !triggered;
}

void DisPho::FillPreSelectionTrigger()
{
  h_cutflow    ->Fill((cutflowLabelMap["Trigger"]*1.f));
  h_cutflow_wgt->Fill((cutflowLabelMap["Trigger"]*1.f),wgt);
}

bool DisPho::ApplyPreSelectionHT()
{
  auto jetHT = 0.f;
  for (auto ijet = 0; ijet < nJets; ijet++) jetHT += jets[ijet].pt();
  return (jetHT < minHT);
}

void DisPho::FillPreSelectionHT()
{
  h_cutflow    ->Fill((cutflowLabelMap["H_{T}"]*1.f));
  h_cutflow_wgt->Fill((cutflowLabelMap["H_{T}"]*1.f),wgt);
}

bool DisPho::ApplyPreSelectionGoodPhoton()
{
  // photon pre-selection: at least one good photon in event
  auto isphgood = false;
  
  for (auto iphoton = 0; iphoton < nPhotons; iphoton++)
  {
    const auto & photon = photons[iphoton];
    
    //      if (iphoton > 0) break; // only consider first photon
    
    const auto pt = oot::GetPhotonPt(photon);
    if (pt < phgoodpTmin) continue;
    
    // const float sceta = std::abs(photon.superCluster()->eta());
    // if (sceta > Config::etaEBcutoff) continue;
    
    if (phgoodIDmin != Config::EmptyVID)
    {
      if (!*(photon.userData<bool>(Config::IsOOT)) && !photon.photonID(phgoodIDmin+"-ged")) continue;
      if ( *(photon.userData<bool>(Config::IsOOT)) && !photon.photonID(phgoodIDmin+"-oot")) continue;
    }
    
    isphgood = true; break;
  } // end loop over photons
  
  return !isphgood;
}

void DisPho::FillPreSelectionGoodPhoton()
{
  h_cutflow    ->Fill((cutflowLabelMap["Good Photon"]*1.f));
  h_cutflow_wgt->Fill((cutflowLabelMap["Good Photon"]*1.f),wgt);
}

//////////////////////////////////////
// Fill Tree from Objects Functions //
//////////////////////////////////////

void DisPho::FillTreeFromObjects(const edm::Event & iEvent)
{
  /////////////
  // MC Info //
  /////////////

  if (isMC) DisPho::SetMCInfo();

  ///////////////////////////
  // Event, lumi, run info //
  ///////////////////////////

  DisPho::SetRecordInfo(iEvent);

  //////////////////
  // Trigger Info //
  //////////////////

  DisPho::SetTriggerBranches();

  /////////////////////
  // MET Filter Info //
  /////////////////////

  DisPho::SetMETFilterBranches();

  /////////////////////////
  // Primary Vertex info //
  /////////////////////////

  DisPho::InitializePVBranches();
  DisPho::SetPVBranches();

  //////////////////
  // Type1 PF Met //
  //////////////////

  DisPho::InitializeMETBranches();
  DisPho::SetMETBranches();

  if (isMC) 
  {
    DisPho::InitializeMETBranchesMC();
    DisPho::SetMETBranchesMC();
  }

  /////////////////////////
  // Jets (AK4 standard) //
  /////////////////////////

  DisPho::InitializeJetBranches();
  DisPho::SetJetBranches();

  if (isMC)
  {
    DisPho::InitializeJetBranchesMC();
    DisPho::SetJetBranchesMC();
  }

  ///////////////
  // Electrons //
  ///////////////

  DisPho::InitializeElectronBranches();
  DisPho::SetElectronBranches();

  ///////////
  // Muons //
  ///////////

  DisPho::InitializeMuonBranches();
  DisPho::SetMuonBranches();

  //////////////
  // Rec Hits //
  //////////////

  DisPho::InitializeRecHitBranches();
  DisPho::SetRecHitBranches();

  //////////////////
  // Reco Photons //
  //////////////////

  DisPho::InitializePhoBranches();
  DisPho::SetPhoBranches();
  if (isMC) 
  {
    DisPho::InitializePhoBranchesMC();
    DisPho::SetPhoBranchesMC();
  }

  ///////////////
  // Fill Tree //
  ///////////////

  disphotree->Fill();
}

void DisPho::SetMCInfo()
{
  ////////////////
  // xyzt0 info //
  ////////////////
  
  DisPho::InitializeGenPointBranches();
  DisPho::SetGenT0Branches();
  DisPho::SetGenXYZ0Branches();
  
  ///////////////////////
  // Gen particle info //
  ///////////////////////

  if (isGMSB) 
  {
    DisPho::InitializeGMSBBranches();
    DisPho::SetGMSBBranches();
  } // isGMSB

  if (isHVDS) 
  {
    DisPho::InitializeHVDSBranches();
    DisPho::SetHVDSBranches();
  } // isHVDS

  if (isToy) 
  {
    DisPho::InitializeToyBranches();
    DisPho::SetToyBranches();
  } // isHVDS
}

void DisPho::InitializeGenPointBranches()
{
  gent0 = -9999.f;
  genx0 = -9999.f;
  geny0 = -9999.f;
  genz0 = -9999.f;
}

void DisPho::SetGenT0Branches()
{
  gent0 = *(gent0H.product());
}

void DisPho::SetGenXYZ0Branches()
{
  const auto & genxyz0 = *(genxyz0H.product());

  genx0 = genxyz0.X();
  geny0 = genxyz0.Y();
  genz0 = genxyz0.Z();
}

void DisPho::InitializeGMSBBranches()
{
  nNeutoPhGr = -9999;

  for (auto igmsb = 0; igmsb < Config::nGMSBs; igmsb++)
  {
    auto & gmsbBranch = gmsbBranches[igmsb];
 
    gmsbBranch.genNmass_ = -9999.f; gmsbBranch.genNE_ = -9999.f; gmsbBranch.genNpt_ = -9999.f; gmsbBranch.genNphi_ = -9999.f; gmsbBranch.genNeta_ = -9999.f;
    gmsbBranch.genNprodvx_ = -9999.f; gmsbBranch.genNprodvy_ = -9999.f; gmsbBranch.genNprodvz_ = -9999.f;
    gmsbBranch.genNdecayvx_ = -9999.f; gmsbBranch.genNdecayvy_ = -9999.f; gmsbBranch.genNdecayvz_ = -9999.f;
    
    gmsbBranch.genphE_ = -9999.f; gmsbBranch.genphpt_ = -9999.f; gmsbBranch.genphphi_ = -9999.f; gmsbBranch.genpheta_ = -9999.f; gmsbBranch.genphmatch_ = -9999;
    gmsbBranch.gengrmass_ = -9999.f; gmsbBranch.gengrE_ = -9999.f; gmsbBranch.gengrpt_ = -9999.f; gmsbBranch.gengrphi_ = -9999.f; gmsbBranch.gengreta_ = -9999.f;
  }
} 

void DisPho::SetGMSBBranches()
{
  nNeutoPhGr = neutralinos.size();
  
  const auto nGmsbs = std::min(nNeutoPhGr,Config::nGMSBs);
  for (auto igmsb = 0; igmsb < nGmsbs; igmsb++)
  {
    const auto & neutralino = neutralinos[igmsb];
    auto & gmsbBranch = gmsbBranches[igmsb];

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

    // set photon daughter stuff
    const auto phdaughter  =  (neutralino.daughter(0)->pdgId() == 22)?0:1;
    const auto & genphoton = *(neutralino.daughter(phdaughter));

    // neutralino decay vertex (same for both daughters unless really screwed up)
    gmsbBranch.genNdecayvx_ = genphoton.vx();
    gmsbBranch.genNdecayvy_ = genphoton.vy();
    gmsbBranch.genNdecayvz_ = genphoton.vz();
        
    gmsbBranch.genphE_   = genphoton.energy();
    gmsbBranch.genphpt_  = genphoton.pt();
    gmsbBranch.genphphi_ = genphoton.phi();
    gmsbBranch.genpheta_ = genphoton.eta();
    
    // check for a reco match!
    auto mindR = gendRmin;
    
    for (auto iphoton = 0; iphoton < nPhotons; iphoton++)
    {
      const auto & photon = photons[iphoton];
      
      if (oot::GetPhotonPt(photon) < ((1.f-genpTres) * gmsbBranch.genphpt_)) continue;
      if (oot::GetPhotonPt(photon) > ((1.f+genpTres) * gmsbBranch.genphpt_)) continue;
      
      const auto delR = reco::deltaR(genphoton,photon);
      if (delR < mindR)
      {
	mindR = delR;
	gmsbBranch.genphmatch_ = iphoton;
      } // end check over deltaR
    } // end loop over reco photons
  
    // set gravitino daughter stuff
    const auto grdaughter     =  (neutralino.daughter(0)->pdgId() == 1000039)?0:1;
    const auto & gengravitino = *(neutralino.daughter(grdaughter));
    
    gmsbBranch.gengrmass_ = gengravitino.mass();
    gmsbBranch.gengrE_    = gengravitino.energy();
    gmsbBranch.gengrpt_   = gengravitino.pt();
    gmsbBranch.gengrphi_  = gengravitino.phi();
    gmsbBranch.gengreta_  = gengravitino.eta();
  } // end loop over GMSBs 
}

void DisPho::InitializeHVDSBranches()
{
  nvPions = -9999;

  for (auto ihvds = 0; ihvds < Config::nHVDSs; ihvds++)
  {
    auto & hvdsBranch = hvdsBranches[ihvds];

    hvdsBranch.genvPionmass_ = -9999.f; hvdsBranch.genvPionE_ = -9999.f; hvdsBranch.genvPionpt_ = -9999.f; hvdsBranch.genvPionphi_ = -9999.f; hvdsBranch.genvPioneta_ = -9999.f;
    hvdsBranch.genvPionprodvx_ = -9999.f; hvdsBranch.genvPionprodvy_ = -9999.f; hvdsBranch.genvPionprodvz_ = -9999.f;
    hvdsBranch.genvPiondecayvx_ = -9999.f; hvdsBranch.genvPiondecayvy_ = -9999.f; hvdsBranch.genvPiondecayvz_ = -9999.f;
    
    hvdsBranch.genHVph0E_ = -9999.f; hvdsBranch.genHVph0pt_ = -9999.f; hvdsBranch.genHVph0phi_ = -9999.f; hvdsBranch.genHVph0eta_ = -9999.f; hvdsBranch.genHVph0match_ = -9999;
    hvdsBranch.genHVph1E_ = -9999.f; hvdsBranch.genHVph1pt_ = -9999.f; hvdsBranch.genHVph1phi_ = -9999.f; hvdsBranch.genHVph1eta_ = -9999.f; hvdsBranch.genHVph1match_ = -9999;
  }
}

void DisPho::SetHVDSBranches()
{
  nvPions = vPions.size();
  
  const auto nHvdss = std::min(nvPions,Config::nHVDSs); 
  for (auto ihvds = 0; ihvds < nHvdss; ihvds++)
  {
    const auto & vPion = vPions[ihvds];
    auto & hvdsBranch = hvdsBranches[ihvds];

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
    
    // get daughters
    const auto leading      = (vPion.daughter(0)->pt()>vPion.daughter(1)->pt())?0:1;
    const auto subleading   = (vPion.daughter(0)->pt()>vPion.daughter(1)->pt())?1:0;
    const auto & genphoton0 = *(vPion.daughter(leading));
    const auto & genphoton1 = *(vPion.daughter(subleading));

    // vPion decay vertex (same for both daughters unless really screwed up)
    hvdsBranch.genvPiondecayvx_ = genphoton0.vx();
    hvdsBranch.genvPiondecayvy_ = genphoton0.vy();
    hvdsBranch.genvPiondecayvz_ = genphoton0.vz();

    // save daughter info
    hvdsBranch.genHVph0E_   = genphoton0.energy();
    hvdsBranch.genHVph0pt_  = genphoton0.pt();
    hvdsBranch.genHVph0phi_ = genphoton0.phi();
    hvdsBranch.genHVph0eta_ = genphoton0.eta();
    
    hvdsBranch.genHVph1E_   = genphoton1.energy();
    hvdsBranch.genHVph1pt_  = genphoton1.pt();
    hvdsBranch.genHVph1phi_ = genphoton1.phi();
    hvdsBranch.genHVph1eta_ = genphoton1.eta();
  
    // check for a reco match!
    auto tmpph0 = -9999, tmpph1 = -9999;
    auto mindR0 = gendRmin, mindR1 = gendRmin; // at least this much
    
    for (auto iphoton = 0; iphoton < nPhotons; iphoton++)
    {
      const auto & photon = photons[iphoton];
      const auto tmppt = oot::GetPhotonPt(photon);

      // check gen photon 0
      if (tmppt < ((1.f-genpTres) * hvdsBranch.genHVph0pt_)) continue;
      if (tmppt > ((1.f+genpTres) * hvdsBranch.genHVph0pt_)) continue;

      const auto delR0 = reco::deltaR(genphoton0,photon);
      if (delR0 < mindR0) 
      {
	mindR0 = delR0;
	tmpph0 = iphoton;
      } // end check over deltaR
      
      // check gen photon 1
      if (tmppt < ((1.f-genpTres) * hvdsBranch.genHVph1pt_)) continue;
      if (tmppt > ((1.f+genpTres) * hvdsBranch.genHVph1pt_)) continue;
      
      const auto delR1 = reco::deltaR(genphoton1,photon);
      if (delR1 < mindR1) 
      {
	mindR1 = delR1;
	tmpph1 = iphoton;
      } // end check over deltaR
    } // end loop over reco photons
    
    // now save tmp photon iphs
    hvdsBranch.genHVph0match_ = tmpph0;
    hvdsBranch.genHVph1match_ = tmpph1;
  } // end loop over HVDSs
}

void DisPho::InitializeToyBranches()
{
  nToyPhs = -9999;

  for (auto itoy = 0; itoy < Config::nToys; itoy++)
  {
    auto & toyBranch = toyBranches[itoy];

    toyBranch.genphE_   = -9999.f;
    toyBranch.genphpt_  = -9999.f;
    toyBranch.genphphi_ = -9999.f;
    toyBranch.genpheta_ = -9999.f;

    toyBranch.genphmatch_ = -9999;
    toyBranch.genphmatch_ptres_ = -9999;
    toyBranch.genphmatch_status_ = -9999;
  }
}

void DisPho::SetToyBranches()
{
  nToyPhs = toys.size();
  
  const auto nToys = std::min(nToyPhs,Config::nToys);
  for (auto itoy = 0; itoy < nToys; itoy++)
  {
    const auto & toy = toys[itoy];
    auto & toyBranch = toyBranches[itoy];

    toyBranch.genphE_   = toy.energy();
    toyBranch.genphpt_  = toy.pt();
    toyBranch.genphphi_ = toy.phi();
    toyBranch.genpheta_ = toy.eta();
    
    auto mindR = gendRmin, mindR_ptres = gendRmin, mindR_status = gendRmin;
    for (auto iphoton = 0; iphoton < nPhotons; iphoton++)
    {
      const auto & photon = photons[iphoton];

      const auto delR = reco::deltaR(toy,photon);
      if (delR < mindR) 
      {
	mindR = delR;
	toyBranch.genphmatch_ = iphoton; 
      } // end check over deltaR
      
      if ( (oot::GetPhotonPt(photon) >= ((1.f-genpTres) * toyBranch.genphpt_)) && (oot::GetPhotonPt(photon) <= ((1.f+genpTres) * toyBranch.genphpt_)) )
      {
	const auto delR_ptres = reco::deltaR(toy,photon);
	if (delR_ptres < mindR_ptres) 
	{
	  mindR_ptres = delR_ptres;
	  toyBranch.genphmatch_ptres_ = iphoton; 
	} // end check over deltaR
	
	if (toy.isPromptFinalState())
	{
	  const auto delR_status = reco::deltaR(toy,photon);
	  if (delR_status < mindR_status) 
	  {
	    mindR_status = delR_status;
	    toyBranch.genphmatch_status_ = iphoton; 
	  } // end check over deltaR
	} // end check over final state
      } // end check over pt res
    } // end loop over reco photons
  } // end loop over nToys
}

void DisPho::SetRecordInfo(const edm::Event & iEvent)
{
  run   = iEvent.id().run();
  lumi  = iEvent.luminosityBlock();
  event = iEvent.id().event();
}

void DisPho::SetTriggerBranches()
{
  hltSignal = (triggerBitMap.count(Config::SignalPath) ? triggerBitMap[Config::SignalPath] : false);
  hltRefPhoID = (triggerBitMap.count(Config::RefPhoIDPath) ? triggerBitMap[Config::RefPhoIDPath] : false);
  hltRefDispID = (triggerBitMap.count(Config::RefDispIDPath) ? triggerBitMap[Config::RefDispIDPath] : false);
  hltRefHT = (triggerBitMap.count(Config::RefHTPath) ? triggerBitMap[Config::RefHTPath] : false);
  hltPho50 = (triggerBitMap.count(Config::Pho50Path) ? triggerBitMap[Config::Pho50Path] : false);
  hltPho200 = (triggerBitMap.count(Config::Pho200Path) ? triggerBitMap[Config::Pho200Path] : false);
  hltDiPho70 = (triggerBitMap.count(Config::DiPho70Path) ? triggerBitMap[Config::DiPho70Path] : false);
  hltDiPho3022M90 = (triggerBitMap.count(Config::DiPho3022M90Path) ? triggerBitMap[Config::DiPho3022M90Path] : false);
  hltDiPho30PV18PV = (triggerBitMap.count(Config::DiPho30PV18PVPath) ? triggerBitMap[Config::DiPho30PV18PVPath] : false);
  hltEle32WPT = (triggerBitMap.count(Config::Ele32WPTPath) ? triggerBitMap[Config::Ele32WPTPath] : false);
  hltDiEle33MW = (triggerBitMap.count(Config::DiEle33MWPath) ? triggerBitMap[Config::DiEle33MWPath] : false);
  hltJet500 = (triggerBitMap.count(Config::Jet500Path) ? triggerBitMap[Config::Jet500Path] : false);
}

void DisPho::SetMETFilterBranches()
{
  metPV = (triggerFlagMap.count(Config::PVFlag) ? triggerFlagMap[Config::PVFlag] : false);
  metBeamHalo = (triggerFlagMap.count(Config::BeamHaloFlag) ? triggerFlagMap[Config::BeamHaloFlag] : false);
  metHBHENoise = (triggerFlagMap.count(Config::HBHENoiseFlag) ? triggerFlagMap[Config::HBHENoiseFlag] : false);
  metHBHEisoNoise = (triggerFlagMap.count(Config::HBHEisoNoiseFlag) ? triggerFlagMap[Config::HBHEisoNoiseFlag] : false);
  metECALTP = (triggerFlagMap.count(Config::ECALTPFlag) ? triggerFlagMap[Config::ECALTPFlag] : false);
  metPFMuon = (triggerFlagMap.count(Config::PFMuonFlag) ? triggerFlagMap[Config::PFMuonFlag] : false);
  metPFChgHad = (triggerFlagMap.count(Config::PFChgHadFlag) ? triggerFlagMap[Config::PFChgHadFlag] : false);
  metEESC = (triggerFlagMap.count(Config::EESCFlag) ? triggerFlagMap[Config::EESCFlag] : false);
  metECALCalib = (triggerFlagMap.count(Config::ECALCalibFlag) ? triggerFlagMap[Config::ECALCalibFlag] : false);

  // special remade calib flag
  metECALBadCalib = *(ecalBadCalibFlagH.product());
}

void DisPho::InitializePVBranches()
{
  nvtx = -9999; 
  vtxX = -9999.f;
  vtxY = -9999.f;
  vtxZ = -9999.f;
}

void DisPho::SetPVBranches()
{
  nvtx = verticesH->size();
  
  const auto & primevtx = verticesH->front();

  vtxX = primevtx.position().x();
  vtxY = primevtx.position().y();
  vtxZ = primevtx.position().z();
}

void DisPho::InitializeMETBranches()
{
  t1pfMETpt    = -9999.f;
  t1pfMETphi   = -9999.f;
  t1pfMETsumEt = -9999.f;

  t1pfMETptUncorr    = -9999.f;
  t1pfMETphiUncorr   = -9999.f;
  t1pfMETsumEtUncorr = -9999.f;
}

void DisPho::SetMETBranches()
{
  t1pfMETpt    = t1pfMET.userFloat(Config::OOTMETPt);
  t1pfMETphi   = t1pfMET.phi(Config::OOTMETPhi);
  t1pfMETsumEt = t1pfMET.sumEt(Config::OOTMETSumEt);

  t1pfMETptUncorr    = t1pfMET.pt();
  t1pfMETphiUncorr   = t1pfMET.phi();
  t1pfMETsumEtUncorr = t1pfMET.sumEt();
}

void DisPho::InitializeMETBranchesMC()
{
  genMETpt    = -9999.f;
  genMETphi   = -9999.f;
}

void DisPho::SetMETBranchesMC()
{
  const auto & genMET = *(t1pfMET.genMET());

  genMETpt  = genMET.pt();
  genMETphi = genMET.phi();
}
			    
void DisPho::InitializeJetBranches()
{
  jetE.clear();
  jetpt.clear();
  jetphi.clear();
  jeteta.clear();
  jetID.clear();
  jetNHF.clear();
  jetNEMF.clear();
  jetCHF.clear();
  jetCEMF.clear();
  jetMUF.clear();
  jetNHM.clear();
  jetCHM.clear();
 
  jetE.resize(nJets);
  jetpt.resize(nJets);
  jetphi.resize(nJets);
  jeteta.resize(nJets);
  jetID.resize(nJets);
  jetNHF.resize(nJets);
  jetNEMF.resize(nJets);
  jetCHF.resize(nJets);
  jetCEMF.resize(nJets);
  jetMUF.resize(nJets);
  jetNHM.resize(nJets);
  jetCHM.resize(nJets);

  for (auto i = 0; i < nJets; i++)
  {
    jetE  [i] = -9999.f;
    jetpt [i] = -9999.f;
    jetphi[i] = -9999.f;
    jeteta[i] = -9999.f;

    jetID [i] = -9999;

    jetNHF [i] = -9999.f;
    jetNEMF[i] = -9999.f;
    jetCHF [i] = -9999.f;
    jetCEMF[i] = -9999.f;
    jetMUF [i] = -9999.f;
    jetNHM [i] = -9999.f;
    jetCHM [i] = -9999.f;
  }
}

void DisPho::SetJetBranches()
{
  njets = jets.size();

  for (auto ijet = 0; ijet < nJets; ijet++)
  {
    const auto & jet = jets[ijet];
    
    jetE  [ijet] = jet.energy();
    jetpt [ijet] = jet.pt();
    jetphi[ijet] = jet.phi();
    jeteta[ijet] = jet.eta();

    jetID [ijet] = jet.userInt(Config::JetID);

    jetNHF [ijet] = jet.neutralHadronEnergyFraction();
    jetNEMF[ijet] = jet.neutralEmEnergyFraction();
    jetCHF [ijet] = jet.chargedHadronEnergyFraction();
    jetCEMF[ijet] = jet.chargedEmEnergyFraction();
    jetMUF [ijet] = jet.muonEnergyFraction();
    jetNHM [ijet] = jet.neutralMultiplicity();
    jetCHM [ijet] = jet.chargedMultiplicity();
  }
}

void DisPho::InitializeJetBranchesMC()
{
  jetscaleRel.clear();
  jetsmearSF.clear();
  jetsmearDownSF.clear();
  jetsmearUpSF.clear();
  jetisGen.clear();
  
  jetscaleRel.resize(nJets);
  jetsmearSF.resize(nJets);
  jetsmearDownSF.resize(nJets);
  jetsmearUpSF.resize(nJets);
  jetisGen.resize(nJets);

  for (auto ijet = 0; ijet < nJets; ijet++)
  {
    jetscaleRel   [ijet] = -9999.f;

    jetsmearSF    [ijet] = -9999.f;
    jetsmearDownSF[ijet] = -9999.f;
    jetsmearUpSF  [ijet] = -9999.f;

    jetisGen[ijet] = -1;
  }
}

void DisPho::SetJetBranchesMC()
{
  // JEC : ugh to do this here
  JetCorrectionUncertainty jetCorrUnc((*jetCorrH)["Uncertainty"]);

  // JER procedure explanation from https://twiki.cern.ch/twiki/bin/viewauth/CMS/JetResolution#Smearing_procedures
  // JER implementation copied from https://github.com/cms-sw/cmssw/blob/master/PhysicsTools/PatUtils/interface/SmearedJetProducerT.h#L208-L215
  const auto runNum_uint = static_cast <unsigned int> (run);
  const auto lumiNum_uint = static_cast <unsigned int> (lumi);
  const auto evNum_uint = static_cast <unsigned int> (event);
  const auto jet0eta = uint32_t(jets.empty() ? 0 : jets.front().eta()/0.01);
  std::mt19937 mt_rand(1 + jet0eta + (lumiNum_uint<<10) + (runNum_uint<<20) + evNum_uint);

  // loop over jets to get scale and smearings
  for (auto ijet = 0; ijet < nJets; ijet++)
  {
    const auto & jet = jets[ijet];
    const auto pt  = jetpt[ijet];
    const auto eta = jeteta[ijet];

    // set JEC uncs
    jetCorrUnc.setJetPt (pt);
    jetCorrUnc.setJetEta(eta);
    
    jetscaleRel[ijet] = jetCorrUnc.getUncertainty(true);

    // jet JER uncs
    const auto jer         = jetRes   .getResolution ({{JME::Binning::JetPt, pt}, {JME::Binning::JetEta, eta}, {JME::Binning::Rho, rho}});
    const auto jer_sf      = jetRes_sf.getScaleFactor({{JME::Binning::JetEta, eta}}, Variation::NOMINAL);
    const auto jer_sf_down = jetRes_sf.getScaleFactor({{JME::Binning::JetEta, eta}}, Variation::DOWN);
    const auto jer_sf_up   = jetRes_sf.getScaleFactor({{JME::Binning::JetEta, eta}}, Variation::UP);

    // get genjet
    auto igenjet = DisPho::GenJetMatcher(jet,genJetsH,jer);

    if (igenjet > 0) // if matched, use scaling method
    {
      const auto & genJet = (*genJetsH)[igenjet];
      const auto ptres = 1.f - (genJet.pt() / pt);

      jetsmearSF    [ijet] = 1.f + ((jer_sf      - 1.f) * ptres);
      jetsmearDownSF[ijet] = 1.f + ((jer_sf_down - 1.f) * ptres);
      jetsmearUpSF  [ijet] = 1.f + ((jer_sf_up   - 1.f) * ptres);

      jetisGen[ijet] = 1;
    }
    else // if not matched, try to do stochastic smear, otherwise return 1.f
    {
      DisPho::GetStochasticSmear(mt_rand,jer,jer_sf     ,jetsmearSF    [ijet]); // nominal
      DisPho::GetStochasticSmear(mt_rand,jer,jer_sf_down,jetsmearDownSF[ijet]); // down
      DisPho::GetStochasticSmear(mt_rand,jer,jer_sf_up  ,jetsmearUpSF  [ijet]); // up

      jetisGen[ijet] = 0;
    }

    // final checks
    const auto energy = jetE[ijet];
    DisPho::CheckJetSmear(energy,jetsmearSF    [ijet]);
    DisPho::CheckJetSmear(energy,jetsmearDownSF[ijet]);
    DisPho::CheckJetSmear(energy,jetsmearUpSF  [ijet]);
  }
}

int DisPho::GenJetMatcher(const pat::Jet & jet, const edm::Handle<std::vector<reco::GenJet> > & genJetsH, const float jer)
{
  auto mindR = genjetdRmin;

  auto igenjet = 0; // counter
  auto matchedjet = -1; // return value
  for (const auto & genJet : *genJetsH)
  {
    const auto delR = reco::deltaR(jet, genJet);
    
    if (delR < mindR)
    {
      if (std::abs(genJet.pt() - jet.pt()) < (genjetpTfactor * jer * jet.pt()))
      {
	mindR = delR;
	matchedjet = igenjet;
      }
    }

    // increment jet counter
    igenjet++;
  }
  
  return matchedjet;
}

void DisPho::GetStochasticSmear(std::mt19937 & mt_rand, const float jer, const float jer_sf, float & jet_smear)
{
  if (jer_sf > 1.f)
  {
    std::normal_distribution<float> norm_dist(0.f, jer * std::sqrt(jer_sf * jer_sf - 1.f));
    jet_smear = 1.f + norm_dist(mt_rand);
  }
  else
  {
    jet_smear = 1.f;
  }
}

void DisPho::CheckJetSmear(const float energy, float & jet_smear)
{    
  if (energy * jet_smear < smearjetEmin) jet_smear = smearjetEmin / energy;
}

void DisPho::InitializeElectronBranches()
{
  nelLowL  = 0;
  nelLowM  = 0;
  nelLowT  = 0;
  nelHighL = 0;
  nelHighM = 0;
  nelHighT = 0;
}

void DisPho::SetElectronBranches()
{
  // loop over prepped electrons: know that loose is subset of medium which is a subset of tight
  for (const auto & electron : electrons)
  {
    if (electron.electronID(Config::ElectronTightVID))
    {
      nelLowL++;
      nelLowM++;
      nelLowT++;

      if (electron.pt() < elhighpTmin) continue;

      nelHighL++;
      nelHighM++;
      nelHighT++;
    }
    else if (electron.electronID(Config::ElectronMediumVID))
    {
      nelLowL++;
      nelLowM++;

      if (electron.pt() < elhighpTmin) continue;

      nelHighL++;
      nelHighM++;
    }
    else if (electron.electronID(Config::ElectronLooseVID))
    {
      nelLowL++;

      if (electron.pt() < elhighpTmin) continue;
      nelHighL++;
    }
  }
}

void DisPho::InitializeMuonBranches()
{
  nmuLowL  = 0;
  nmuLowM  = 0;
  nmuLowT  = 0;
  nmuHighL = 0;
  nmuHighM = 0;
  nmuHighT = 0;
}

void DisPho::SetMuonBranches()
{
  // loop over muons, check if passed ID && loose PF iso, from MEZ
  //  --> unclear if medium is subset of tight... so have to do this inefficiently
  // https://twiki.cern.ch/twiki/bin/view/CMS/SWGuideMuonIdRun2#Muon_selectors_Since_9_4_X
  for (const auto & muon : muons)
  {
    if (muon.pt() < muhighpTmin)
    {
      if (muon.passed(reco::Muon::CutBasedIdLoose &reco::Muon::PFIsoLoose)) nmuLowL++;
      if (muon.passed(reco::Muon::CutBasedIdMedium&reco::Muon::PFIsoLoose)) nmuLowM++;
      if (muon.passed(reco::Muon::CutBasedIdTight &reco::Muon::PFIsoLoose)) nmuLowT++;
    }
    else
    {
      if (muon.passed(reco::Muon::CutBasedIdLoose &reco::Muon::PFIsoLoose))
      {
	nmuLowL++;
	nmuHighL++;
      }
      if (muon.passed(reco::Muon::CutBasedIdMedium&reco::Muon::PFIsoLoose))
      { 
	nmuLowM++;
	nmuHighM++;
      }
      if (muon.passed(reco::Muon::CutBasedIdTight &reco::Muon::PFIsoLoose))
      { 
	nmuLowT++;
	nmuHighT++;
      }
    }
  }
}

void DisPho::InitializeRecHitBranches()
{
  rhX.clear();
  rhY.clear();
  rhZ.clear();
  rhE.clear();
  rhtime.clear();
  rhtimeErr.clear();
  rhTOF.clear();
  rhID.clear();
  rhisOOT.clear();
  rhisGS6.clear();
  rhisGS1.clear();
  rhadcToGeV.clear();
  rhped12.clear();
  rhped6.clear();
  rhped1.clear();
  rhpedrms12.clear();
  rhpedrms6.clear();
  rhpedrms1.clear();

  rhX.resize(nRecHits);
  rhY.resize(nRecHits);
  rhZ.resize(nRecHits);
  rhE.resize(nRecHits);
  rhtime.resize(nRecHits);
  rhtimeErr.resize(nRecHits);
  rhTOF.resize(nRecHits);
  rhID.resize(nRecHits);
  rhisOOT.resize(nRecHits);
  rhisGS6.resize(nRecHits);
  rhisGS1.resize(nRecHits);
  rhadcToGeV.resize(nRecHits);
  rhped12.resize(nRecHits);
  rhped6.resize(nRecHits);
  rhped1.resize(nRecHits);
  rhpedrms12.resize(nRecHits);
  rhpedrms6.resize(nRecHits);
  rhpedrms1.resize(nRecHits);

  for (auto i = 0; i < nRecHits; i++)
  {
    rhX[i] = -9999.f;
    rhY[i] = -9999.f;
    rhZ[i] = -9999.f;
    rhE[i] = -9999.f;

    rhtime   [i] = -9999.f;
    rhtimeErr[i] = -9999.f;
    rhTOF    [i] = -9999.f;
    
    rhID[i] = 0; // non-ideal

    rhisOOT[i] = false;
    rhisGS6[i] = false;
    rhisGS1[i] = false;

    rhadcToGeV[i] = -9999.f;

    rhped12[i] = -9999.f;
    rhped6 [i] = -9999.f;
    rhped1 [i] = -9999.f;

    rhpedrms12[i] = -9999.f;
    rhpedrms6 [i] = -9999.f;
    rhpedrms1 [i] = -9999.f;
  }
}

void DisPho::SetRecHitBranches()
{
  nrechits = recHitMap.size();
  
  DisPho::SetRecHitBranches(recHitsEB,barrelGeometry,adcToGeVEB);
  DisPho::SetRecHitBranches(recHitsEE,endcapGeometry,adcToGeVEE);
}

void DisPho::SetRecHitBranches(const EcalRecHitCollection * recHits, const CaloSubdetectorGeometry * geometry, const float adcToGeV)
{
  for (const auto recHit : *recHits)
  {
    const auto recHitId(recHit.detid());
    const auto rawId = recHitId.rawId();
    if (recHitMap.count(rawId))
    {
      const auto pos = recHitMap.at(rawId);
      const auto recHitPos = geometry->getGeometry(recHitId)->getPosition();
      
      // save position, energy, and time of each rechit to a vector
      rhX[pos] = recHitPos.x();
      rhY[pos] = recHitPos.y();
      rhZ[pos] = recHitPos.z();
      rhE[pos] = recHit.energy();

      // time info: compute TOF
      const auto d_orig = Config::hypo(rhX[pos],rhY[pos],rhZ[pos]);
      const auto d_pv   = Config::hypo(rhX[pos]-vtxX,rhY[pos]-vtxY,rhZ[pos]-vtxZ);
      rhtime   [pos] = recHit.time();
      rhtimeErr[pos] = recHit.timeError();
      rhTOF    [pos] = (d_orig-d_pv) / Config::sol;
      
      // detid
      rhID[pos] = rawId;

      // flags: isOOT, isGainSwitch6/1
      rhisOOT[pos] = recHit.checkFlag(EcalRecHit::kOutOfTime);
      rhisGS6[pos] = recHit.checkFlag(EcalRecHit::kHasSwitchToGain6);
      rhisGS1[pos] = recHit.checkFlag(EcalRecHit::kHasSwitchToGain1);

      // adcToGeVInfo : http://cmslxr.fnal.gov/source/RecoEcal/EgammaCoreTools/src/EcalClusterLazyTools.cc#0204
      const auto laser = laserH->getLaserCorrection(recHitId,evTime);
      const auto interCalibIter = interCalibMap->find(recHitId);
      const auto interCalib = ((interCalibIter != interCalibMap->end()) ? (*interCalibIter) : - 1.f);
      if ((laser > 0.f) && (interCalib > 0.f) && (adcToGeV > 0.f)) rhadcToGeV[pos] = (laser*interCalib*adcToGeV);

      // pedestal info
      const auto & pediter = pedestalsH->find(recHitId);
      if (pediter != pedestalsH->end())
      {
	const auto & ped = (*pediter);

	rhped12[pos] = ped.mean(1);
	rhped6 [pos] = ped.mean(2);
	rhped1 [pos] = ped.mean(3);

	rhpedrms12[pos] = ped.rms(1);
	rhpedrms6 [pos] = ped.rms(2);
	rhpedrms1 [pos] = ped.rms(3);
      }
    }
  }
}

void DisPho::InitializePhoBranches()
{
  for (auto iphoton = 0; iphoton < Config::nPhotons; iphoton++)
  {
    auto & phoBranch = phoBranches[iphoton];

    phoBranch.E_ = -9999.f;
    phoBranch.pt_ = -9999.f;
    phoBranch.eta_ = -9999.f;
    phoBranch.phi_ = -9999.f;

    phoBranch.scE_ = -9999.f;
    phoBranch.scphi_ = -9999.f;
    phoBranch.sceta_ = -9999.f;

    phoBranch.HoE_ = -9999.f;
    phoBranch.r9_ = -9999.f;

    phoBranch.ChgHadIso_ = -9999.f;
    phoBranch.NeuHadIso_ = -9999.f;
    phoBranch.PhoIso_ = -9999.f;

    phoBranch.EcalPFClIso_ = -9999.f;
    phoBranch.HcalPFClIso_ = -9999.f;
    phoBranch.TrkIso_ = -9999.f;

    phoBranch.ChgHadIsoC_ = -9999.f;
    phoBranch.NeuHadIsoC_ = -9999.f;
    phoBranch.PhoIsoC_ = -9999.f;

    phoBranch.EcalPFClIsoC_ = -9999.f;
    phoBranch.HcalPFClIsoC_ = -9999.f;
    phoBranch.TrkIsoC_ = -9999.f;

    phoBranch.e2x2_ = -9999.f;
    phoBranch.e3x3_ = -9999.f;
    phoBranch.e5x5_ = -9999.f;

    phoBranch.sieie_ = -9999.f;
    phoBranch.sipip_ = -9999.f;
    phoBranch.sieip_ = -9999.f;

    phoBranch.smaj_ = -9999.f;
    phoBranch.smin_ = -9999.f;
    phoBranch.alpha_ = -9999.f;

    if (storeRecHits)
    {
      phoBranch.seed_ = -1;
      phoBranch.recHits_.clear();
    }
    else
    {
      phoBranch.seedX_        = -9999.f;
      phoBranch.seedY_        = -9999.f;
      phoBranch.seedZ_        = -9999.f;
      phoBranch.seedE_        = -9999.f;
      phoBranch.seedtime_     = -9999.f;
      phoBranch.seedtimeErr_  = -9999.f;
      phoBranch.seedTOF_      = -9999.f;
      phoBranch.seedID_       = 0; // non-ideal
      phoBranch.seedisOOT_    = -1;
      phoBranch.seedisGS6_    = -1;
      phoBranch.seedisGS1_    = -1;
      phoBranch.seedadcToGeV_ = -9999.f;
      phoBranch.seedped12_    = -9999.f;
      phoBranch.seedped6_     = -9999.f;
      phoBranch.seedped1_     = -9999.f;
      phoBranch.seedpedrms12_ = -9999.f;
      phoBranch.seedpedrms6_  = -9999.f;
      phoBranch.seedpedrms1_  = -9999.f;
    }
  
    phoBranch.suisseX_ = -9999.f;

    phoBranch.isOOT_ = false;
    phoBranch.isEB_  = false;
    phoBranch.isHLT_ = false;
    phoBranch.isTrk_ = false;
    phoBranch.passEleVeto_ = false;
    phoBranch.hasPixSeed_  = false;

    phoBranch.gedID_ = -1;
    phoBranch.ootID_ = -1;
  }
}

void DisPho::SetPhoBranches()
{
  nphotons = photons.size();
  
  for (auto iphoton = 0; iphoton < nPhotons; iphoton++)
  {
    // get objects
    const auto & photon = photons[iphoton];
    auto & phoBranch = phoBranches[iphoton];
    
    // basic kinematic with v2: https://twiki.cern.ch/twiki/bin/viewauth/CMS/EgammaMiniAODV2#Applying_the_Energy_Scale_and_sm
    const auto phop4 = photon.p4() * oot::GetCorrFactor(photon);
    phoBranch.E_   = phop4.energy();
    phoBranch.pt_  = phop4.pt();
    phoBranch.phi_ = phop4.phi();
    phoBranch.eta_ = phop4.eta();

    // super cluster info from photon
    const auto & phosc = photon.superCluster().isNonnull() ? photon.superCluster() : photon.parentSuperCluster();

    phoBranch.scE_   = phosc->energy();
    phoBranch.scphi_ = phosc->phi();
    phoBranch.sceta_ = phosc->eta();

    // save pt, SC eta for later use
    const auto phoPt = photon.pt(); // use uncorrected pT for isolations
    const auto scEta = std::abs(phoBranch.sceta_);

    // ID-like variables
    phoBranch.HoE_ = photon.hadTowOverEm(); // used in ID + trigger (single tower HoverE)
    phoBranch.r9_  = photon.r9(); // used in slimming in PAT + trigger

    // PF Isolations : GED ID
    phoBranch.ChgHadIso_ = photon.chargedHadronIso();
    phoBranch.NeuHadIso_ = photon.neutralHadronIso();
    phoBranch.PhoIso_    = photon.photonIso();

    // PF Cluster Isos : OOT ID
    phoBranch.EcalPFClIso_ = photon.ecalPFClusterIso();
    phoBranch.HcalPFClIso_ = photon.hcalPFClusterIso();
    phoBranch.TrkIso_      = photon.trkSumPtHollowConeDR03();

    // corrected values for isolations
    phoBranch.ChgHadIsoC_ = std::max(phoBranch.ChgHadIso_ - (rho * oot::GetChargedHadronEA(scEta))                                              ,0.f);
    phoBranch.NeuHadIsoC_ = std::max(phoBranch.NeuHadIso_ - (rho * oot::GetNeutralHadronEA(scEta)) - (oot::GetNeutralHadronPtScale(scEta,phoPt)),0.f);
    phoBranch.PhoIsoC_    = std::max(phoBranch.PhoIso_    - (rho * oot::GetGammaEA        (scEta)) - (oot::GetGammaPtScale        (scEta,phoPt)),0.f);

    phoBranch.EcalPFClIsoC_ = std::max(phoBranch.EcalPFClIso_ - (rho * oot::GetEcalPFClEA(scEta)) - (oot::GetEcalPFClPtScale(scEta,phoPt)),0.f);
    phoBranch.HcalPFClIsoC_ = std::max(phoBranch.HcalPFClIso_ - (rho * oot::GetHcalPFClEA(scEta)) - (oot::GetHcalPFClPtScale(scEta,phoPt)),0.f);
    phoBranch.TrkIsoC_      = std::max(phoBranch.TrkIso_      - (rho * oot::GetTrackEA   (scEta)) - (oot::GetTrackPtScale   (scEta,phoPt)),0.f);

    // Shower Shape Objects
    const auto & phoshape = photon.full5x5_showerShapeVariables(); 

    // cluster shape variables
    phoBranch.sieie_ = phoshape.sigmaIetaIeta;
    phoBranch.sipip_ = phoshape.sigmaIphiIphi;
    phoBranch.sieip_ = phoshape.sigmaIetaIphi;

    // other cluster shape variables
    phoBranch.e2x2_ = phoshape.e2x2;
    phoBranch.e3x3_ = phoshape.e3x3;
    phoBranch.e5x5_ = phoshape.e5x5;

    // use seed to get geometry and recHits
    const auto & seedDetId = phosc->seed()->seed(); // seed detid
    const auto seedRawId = seedDetId.rawId(); // crystal number
    const auto isEB = (seedDetId.subdetId() == EcalBarrel); // which subdet
    const auto recHits = (isEB ? recHitsEB : recHitsEE); 

    // 2nd moments from official calculation
    if (recHits->size() > 0)
    {
      const auto & ph2ndMoments = noZS::EcalClusterTools::cluster2ndMoments( *phosc, *recHits);
      // radius of semi-major,minor axis is the inverse square root of the eigenvalues of the covariance matrix
      phoBranch.smaj_  = ph2ndMoments.sMaj;
      phoBranch.smin_  = ph2ndMoments.sMin;
      phoBranch.alpha_ = ph2ndMoments.alpha;
    } // end check over non-zero recHits

    // map of rec hit ids
    uiiumap phrhIDmap;
    const auto & hitsAndFractions = phosc->hitsAndFractions();
    for (auto hafitr = hitsAndFractions.begin(); hafitr != hitsAndFractions.end(); ++hafitr) // loop over all rec hits in SC
    {
      const auto & recHitId = hafitr->first; // get detid of crystal
      const auto rawId = recHitId.rawId();
      if (recHitMap.count(rawId))
      {
	auto recHit = recHits->find(recHitId); // get the underlying rechit
	if (recHit != recHits->end()) // standard check
        { 
	  phrhIDmap[recHitMap.at(rawId)]++;
	} // end standard check recHit
      } // end check over if recHit is in map
    } // end loop over hits and fractions
    
    // store the position inside the recHits vector 
    if (storeRecHits)
    {
      for (auto rhiter = phrhIDmap.begin(); rhiter != phrhIDmap.end(); ++rhiter) // loop over only good rec hit ids
      {
	phoBranch.recHits_.emplace_back(rhiter->first);
      }

      //  sort rec hit list in photon by rechitE which is already stored for this event
      std::sort(phoBranch.recHits_.begin(),phoBranch.recHits_.end(),
		[&](const auto rh1, const auto rh2)
		{
		  return (rhE[rh1] > rhE[rh2]);
		});
    } // end check over storing recHits
  
    // save seed info + swiss cross
    if (recHitMap.count(seedRawId)) 
    {
      const auto seedpos = recHitMap.at(seedRawId); 
      if (storeRecHits) 
      {
	// store just the seed for accessing through recHit branches
	phoBranch.seed_ = seedpos;
      }
      else
      {
	phoBranch.seedX_ = rhX[seedpos];
	phoBranch.seedY_ = rhY[seedpos];
	phoBranch.seedZ_ = rhZ[seedpos];
	phoBranch.seedE_ = rhE[seedpos];
	phoBranch.seedtime_ = rhtime[seedpos];
	phoBranch.seedtimeErr_ = rhtimeErr[seedpos];
	phoBranch.seedTOF_ = rhTOF[seedpos];
	phoBranch.seedID_ = rhID[seedpos];
	phoBranch.seedisOOT_ = rhisOOT[seedpos];
	phoBranch.seedisGS6_ = rhisGS6[seedpos];
	phoBranch.seedisGS1_ = rhisGS1[seedpos];
	phoBranch.seedadcToGeV_ = rhadcToGeV[seedpos];
	phoBranch.seedped12_ = rhped12[seedpos];
	phoBranch.seedped6_ = rhped6[seedpos];
	phoBranch.seedped1_ = rhped1[seedpos];
	phoBranch.seedpedrms12_ = rhpedrms12[seedpos];
	phoBranch.seedpedrms6_ = rhpedrms6[seedpos];
	phoBranch.seedpedrms1_ = rhpedrms1[seedpos];
      } // end check over storing rechits 
    
      // swiss cross
      if (recHits->size() > 0) phoBranch.suisseX_ = EcalTools::swissCross(seedDetId, *recHits, rhEmin);
    } // end check over if seed exists
    
    // some standard booleans
    phoBranch.isOOT_ = *(photon.userData<bool>(Config::IsOOT));
    phoBranch.isEB_  = isEB;

    // HLT Matching!
    strBitMap isHLTMatched;
    oot::InitializeBitMap(filterNames,isHLTMatched);
    oot::HLTToObjectMatching(triggerObjectsByFilterMap,isHLTMatched,photon,pTres,dRmin);
    const auto filter = Config::DispIDFilter;
    phoBranch.isHLT_ = (isHLTMatched.count(filter) ? isHLTMatched[filter] : false);

    // check for simple track veto
    phoBranch.isTrk_ = oot::TrackToObjectMatching(tracksH,photon,trackpTmin,trackdRmin);

    // other track vetoes
    phoBranch.passEleVeto_ = photon.passElectronVeto();
    phoBranch.hasPixSeed_  = photon.hasPixelSeed();
  
    // 0 --> did not pass anything, 1 --> loose pass, 2 --> medium pass, 3 --> tight pass
    // GED first
    if      (photon.photonID(Config::TightGED))  {phoBranch.gedID_ = 3;}
    else if (photon.photonID(Config::MediumGED)) {phoBranch.gedID_ = 2;}
    else if (photon.photonID(Config::LooseGED))  {phoBranch.gedID_ = 1;}
    else                                         {phoBranch.gedID_ = 0;}
    // OOT second
    if      (photon.photonID(Config::TightOOT))  {phoBranch.ootID_ = 3;}
    else if (photon.photonID(Config::LooseOOT))  {phoBranch.ootID_ = 1;}
    else                                         {phoBranch.ootID_ = 0;}
  } // end loop over nPhotons
}

void DisPho::InitializePhoBranchesMC()
{
  for (auto iphoton = 0; iphoton < Config::nPhotons; iphoton++)
  {
    auto & phoBranch = phoBranches[iphoton];

    if (isGMSB || isHVDS) phoBranch.isSignal_ = -9999;
    phoBranch.isGen_ = false;
    
    phoBranch.scaleAbs_ = -9999.f;
    phoBranch.smearAbs_ = -9999.f;
  }
}

void DisPho::SetPhoBranchesMC()
{
  for (auto iphoton = 0; iphoton < nPhotons; iphoton++)
  {
    // get objects
    const auto & photon = photons[iphoton];
    auto & phoBranch = phoBranches[iphoton];
    
    // extra info for gen matching
    if (isGMSB)
    {
      // matched to photon from leading neutralino == 1
      // matched to photon from subleading neutralino == 2
      // matched to both photons from both neutralinos (probably will never happen) == 3
      // no corresponding match == 0
      
      const auto & gmsbBranch0 = gmsbBranches[0];
      const auto & gmsbBranch1 = gmsbBranches[1];
      
      if      (iphoton == gmsbBranch0.genphmatch_ && iphoton != gmsbBranch1.genphmatch_) phoBranch.isSignal_ = 1;
      else if (iphoton != gmsbBranch0.genphmatch_ && iphoton == gmsbBranch1.genphmatch_) phoBranch.isSignal_ = 2;
      else if (iphoton == gmsbBranch0.genphmatch_ && iphoton == gmsbBranch1.genphmatch_) phoBranch.isSignal_ = 3; 
      else                                                                               phoBranch.isSignal_ = 0;
    } // end block over isGMSB
      
    if (isHVDS) 
    {
      // since we have N vPions to explore, store the result as an int mask
      // ones digit is leading vPion, tens as subleading, hundreds as subsub, etc.
      // matched to leading photon and not to subleading photon == 1
      // matched to subleading photon and not to leading photon == 2
      // matched to both photons inside vPion == 3
      // no corresponding match == 0
      
      for (auto ihvds = 0; ihvds < Config::nHVDSs; ihvds++)
      {
	phoBranch.isSignal_ += (DisPho::CheckMatchHVDS(iphoton,hvdsBranches[ihvds]) * std::pow(10,ihvds));
      } // end loop over all possible HVDSs
    } // end block over is HVDS
  
    // standard dR matching
    phoBranch.isGen_ = oot::GenToObjectMatching(genParticlesH,photon,genpTres,gendRmin);
    
    // scale and smearing uncs
    const auto phoE = phoBranch.E_; // assumed this already set!!!
      
    // eval up/down, then save bigger one (diff is order of one of MeV's...
    const auto down_scale = std::abs(phoE-photon.userFloat(Config::EnergyScaleDown));
    const auto up_scale   = std::abs(photon.userFloat(Config::EnergyScaleUp)-phoE);
    phoBranch.scaleAbs_   = (up_scale > down_scale ? up_scale : down_scale);

    // eval up/down, then save bigger one (diff is order of one of MeV's...
    const auto down_smear = std::abs(phoE-photon.userFloat(Config::EnergySigmaDown));
    const auto up_smear   = std::abs(photon.userFloat(Config::EnergySigmaUp)-phoE);
    phoBranch.smearAbs_   = (up_smear > down_smear ? up_smear : down_smear);
  }
}

int DisPho::CheckMatchHVDS(const int iphoton, const hvdsStruct & hvdsBranch)
{
  if      (iphoton == hvdsBranch.genHVph0match_ && iphoton != hvdsBranch.genHVph1match_) return 1;
  else if (iphoton != hvdsBranch.genHVph0match_ && iphoton == hvdsBranch.genHVph1match_) return 2;
  else if (iphoton == hvdsBranch.genHVph0match_ && iphoton == hvdsBranch.genHVph1match_) return 3;
  else                                                                                   return 0;
}

//////////////////////////////////////////
// Internal and Analyzer Prep Functions //
//////////////////////////////////////////

void DisPho::beginJob() 
{
  edm::Service<TFileService> fs;
  
  // histograms needed
  h_cutflow     = fs->make<TH1F>("h_cutflow"    , "Cut Flow"           , cutflowLabelMap.size(), 0, cutflowLabelMap.size());
  h_cutflow_wgt = fs->make<TH1F>("h_cutflow_wgt", "Cut Flow (Weighted)", cutflowLabelMap.size(), 0, cutflowLabelMap.size());
  
  if (isMC)
  {
    h_genpuobs      = fs->make<TH1F>("h_genpuobs"     , "Gen PU Observed"           , 150, 0, 150);
    h_genpuobs_wgt  = fs->make<TH1F>("h_genpuobs_wgt" , "Gen PU Observed (Weighted)", 150, 0, 150);
    h_genputrue     = fs->make<TH1F>("h_genputrue"    , "Gen PU True"               , 150, 0, 150);
    h_genputrue_wgt = fs->make<TH1F>("h_genputrue_wgt", "Gen PU True (Weighted)"    , 150, 0, 150);
  }
  DisPho::MakeHists();

  // Config tree, filled once
  configtree = fs->make<TTree>("configtree","configtree");
  DisPho::MakeAndFillConfigTree();

  // Event tree
  disphotree = fs->make<TTree>("disphotree","disphotree");
  DisPho::MakeEventTree();
}

void DisPho::MakeHists()
{
  // cut flow settings
  for (const auto & cutflowLabelPair : cutflowLabelMap)
  {
    h_cutflow    ->GetXaxis()->SetBinLabel(cutflowLabelPair.second+1,cutflowLabelPair.first.c_str()); // +1 to account for bins in ROOT from [1,nBins]
    h_cutflow_wgt->GetXaxis()->SetBinLabel(cutflowLabelPair.second+1,cutflowLabelPair.first.c_str()); // +1 to account for bins in ROOT from [1,nBins]
  }
  h_cutflow    ->GetYaxis()->SetTitle("nEntries");
  h_cutflow_wgt->GetYaxis()->SetTitle("nEntries with gen weights");

  if (isMC)
  {
    // PUObs
    h_genpuobs    ->GetXaxis()->SetTitle("nPU observed");
    h_genpuobs    ->GetYaxis()->SetTitle("nEntries");
    h_genpuobs_wgt->GetXaxis()->SetTitle("nPU observed");
    h_genpuobs_wgt->GetYaxis()->SetTitle("nEvents with weights");
    
    // PUTrue
    h_genputrue    ->GetXaxis()->SetTitle("nPU True");
    h_genputrue    ->GetYaxis()->SetTitle("nEntries");
    h_genputrue_wgt->GetXaxis()->SetTitle("nPU True");
    h_genputrue_wgt->GetYaxis()->SetTitle("nEvents with weights");
  }

  // SumW2
  h_cutflow    ->Sumw2();
  h_cutflow_wgt->Sumw2();
  if (isMC)
  {
    h_genpuobs     ->Sumw2();
    h_genpuobs_wgt ->Sumw2();
    h_genputrue    ->Sumw2();
    h_genputrue_wgt->Sumw2();
  }
}

void DisPho::MakeAndFillConfigTree()
{
  // ROOT gets confused with things declared const...
  // so have to "capture" configs in temps... has to be better way to do this

  // blinding
  auto blindSF_tmp = blindSF;
  auto applyBlindSF_tmp = applyBlindSF;
  auto blindMET_tmp = blindMET;
  auto applyBlindMET_tmp = applyBlindMET;
  configtree->Branch("blindSF", &blindSF_tmp);
  configtree->Branch("applyBlindSF", &applyBlindSF_tmp);
  configtree->Branch("blindMET", &blindMET_tmp);
  configtree->Branch("applyBlindMET", &applyBlindMET_tmp);

  // object prep
  auto jetpTmin_tmp = jetpTmin;
  auto jetEtamax_tmp = jetEtamax;
  auto jetIDmin_tmp = jetIDmin;
  auto rhEmin_tmp = rhEmin;
  auto phpTmin_tmp = phpTmin;
  auto phIDmin_tmp = phIDmin;
  configtree->Branch("jetpTmin", &jetpTmin_tmp);
  configtree->Branch("jetEtamax", &jetEtamax_tmp);
  configtree->Branch("jetIDmin", &jetIDmin_tmp);
  configtree->Branch("rhEmin", &rhEmin_tmp);
  configtree->Branch("phpTmin", &phpTmin_tmp);
  configtree->Branch("phIDmin", &phIDmin_tmp);

  // object extra pruning
  auto seedTimemin_tmp = seedTimemin;
  auto nPhosmax_tmp = nPhosmax;
  configtree->Branch("seedTimemin", &seedTimemin_tmp);
  configtree->Branch("nPhosmax", &nPhosmax_tmp);

  // photon storing options
  auto splitPho_tmp = splitPho;
  auto onlyGED_tmp = onlyGED;
  auto onlyOOT_tmp = onlyOOT;
  configtree->Branch("splitPho", &splitPho_tmp);
  configtree->Branch("onlyGED", &onlyGED_tmp);
  configtree->Branch("onlyOOT", &onlyOOT_tmp);

  // lepton prep cuts
  auto ellowpTmin_tmp = ellowpTmin;
  auto elhighpTmin_tmp = elhighpTmin;
  auto mulowpTmin_tmp = mulowpTmin;
  auto muhighpTmin_tmp = muhighpTmin;
  configtree->Branch("ellowpTmin", &ellowpTmin_tmp);
  configtree->Branch("elhighpTmin", &elhighpTmin_tmp);
  configtree->Branch("mulowpTmin", &mulowpTmin_tmp);
  configtree->Branch("muhighpTmin", &muhighpTmin_tmp);

  // rec hit storing options
  auto storeRecHits_tmp = storeRecHits;
  configtree->Branch("storeRecHits", &storeRecHits_tmp);

  // pre-selection vars
  auto applyTrigger_tmp = applyTrigger;
  auto minHT_tmp = minHT;
  auto applyHT_tmp = applyHT;
  auto phgoodpTmin_tmp = phgoodpTmin;
  auto phgoodIDmin_tmp = phgoodIDmin;
  auto applyPhGood_tmp = applyPhGood;
  configtree->Branch("applyTrigger", &applyTrigger_tmp);
  configtree->Branch("minHT", &minHT_tmp);
  configtree->Branch("applyHT", &applyHT_tmp);
  configtree->Branch("phgoodpTmin", &phgoodpTmin_tmp);
  configtree->Branch("phgoodIDmin", &phgoodIDmin_tmp);
  configtree->Branch("applyPhGood", &applyPhGood_tmp);

  // dR matching criteria
  auto dRmin_tmp = dRmin;
  auto pTres_tmp = pTres;
  auto gendRmin_tmp = gendRmin;
  auto genpTres_tmp = genpTres;
  auto trackdRmin_tmp = trackdRmin;
  auto trackpTmin_tmp = trackpTmin;
  auto genjetdRmin_tmp = genjetdRmin;
  auto genjetpTfactor_tmp = genjetpTfactor;
  auto leptondRmin_tmp = leptondRmin;
  configtree->Branch("dRmin", &dRmin_tmp);
  configtree->Branch("pTres", &pTres_tmp);
  configtree->Branch("gendRmin", &gendRmin_tmp);
  configtree->Branch("genpTres", &genpTres_tmp);
  configtree->Branch("trackdRmin", &trackdRmin_tmp);
  configtree->Branch("trackpTmin", &trackpTmin_tmp);
  configtree->Branch("genjetdRmin", &genjetdRmin_tmp);
  configtree->Branch("genjetpTfactor", &genjetpTfactor_tmp);
  configtree->Branch("leptondRmin", &leptondRmin_tmp);

  // JER extra info
  auto smearjetEmin_tmp = smearjetEmin;
  configtree->Branch("smearjetEmin", &smearjetEmin_tmp);

  // trigger info
  auto inputPaths_tmp = inputPaths;
  auto inputFilters_tmp = inputFilters;
  configtree->Branch("inputPaths", &inputPaths_tmp);
  configtree->Branch("inputFilters", &inputFilters_tmp);

  // met flag info
  auto inputFlags_tmp = inputFlags;
  configtree->Branch("inputFlags", &inputFlags_tmp);

  // MC info
  auto isGMSB_tmp = isGMSB;
  auto isHVDS_tmp = isHVDS;
  auto isBkgd_tmp = isBkgd;
  auto isToy_tmp  = isToy;
  auto isADD_tmp  = isADD;
  auto xsec_tmp = xsec;
  auto filterEff_tmp = filterEff;
  auto BR_tmp = BR;
  configtree->Branch("isGMSB", &isGMSB_tmp);
  configtree->Branch("isHVDS", &isHVDS_tmp);
  configtree->Branch("isBkgd", &isBkgd_tmp);
  configtree->Branch("isToy" , &isToy_tmp);
  configtree->Branch("isADD" , &isADD_tmp);
  configtree->Branch("xsec", &xsec_tmp);
  configtree->Branch("filterEff", &filterEff_tmp);
  configtree->Branch("BR", &BR_tmp);

  // Fill tree just once, after configs have been read in
  configtree->Fill();
}

void DisPho::MakeEventTree()
{
  // rho info
  disphotree->Branch("rho", &rho);

  // Generic MC Info
  if (isMC)
  {
    disphotree->Branch("genwgt", &genwgt);
    disphotree->Branch("genpuobs", &genpuobs);
    disphotree->Branch("genputrue", &genputrue);

    disphotree->Branch("gent0", &gent0);
    disphotree->Branch("genx0", &genx0);
    disphotree->Branch("geny0", &geny0);
    disphotree->Branch("genz0", &genz0);
  }

  // GMSB Info
  if (isGMSB)
  {
    disphotree->Branch("nNeutoPhGr", &nNeutoPhGr);

    gmsbBranches.resize(Config::nGMSBs);
    for (auto igmsb = 0; igmsb < Config::nGMSBs; igmsb++)
    {
      auto & gmsbBranch = gmsbBranches[igmsb];

      disphotree->Branch(Form("genNmass_%i",igmsb), &gmsbBranch.genNmass_);
      disphotree->Branch(Form("genNE_%i",igmsb), &gmsbBranch.genNE_);
      disphotree->Branch(Form("genNpt_%i",igmsb), &gmsbBranch.genNpt_);
      disphotree->Branch(Form("genNphi_%i",igmsb), &gmsbBranch.genNphi_);
      disphotree->Branch(Form("genNeta_%i",igmsb), &gmsbBranch.genNeta_);
      
      disphotree->Branch(Form("genNprodvx_%i",igmsb), &gmsbBranch.genNprodvx_);
      disphotree->Branch(Form("genNprodvy_%i",igmsb), &gmsbBranch.genNprodvy_);
      disphotree->Branch(Form("genNprodvz_%i",igmsb), &gmsbBranch.genNprodvz_);
      
      disphotree->Branch(Form("genNdecayvx_%i",igmsb), &gmsbBranch.genNdecayvx_);
      disphotree->Branch(Form("genNdecayvy_%i",igmsb), &gmsbBranch.genNdecayvy_);
      disphotree->Branch(Form("genNdecayvz_%i",igmsb), &gmsbBranch.genNdecayvz_);

      disphotree->Branch(Form("genphE_%i",igmsb), &gmsbBranch.genphE_);
      disphotree->Branch(Form("genphpt_%i",igmsb), &gmsbBranch.genphpt_);
      disphotree->Branch(Form("genphphi_%i",igmsb), &gmsbBranch.genphphi_);
      disphotree->Branch(Form("genpheta_%i",igmsb), &gmsbBranch.genpheta_);
      disphotree->Branch(Form("genphmatch_%i",igmsb), &gmsbBranch.genphmatch_);
      
      disphotree->Branch(Form("gengrmass_%i",igmsb), &gmsbBranch.gengrmass_);
      disphotree->Branch(Form("gengrE_%i",igmsb), &gmsbBranch.gengrE_);
      disphotree->Branch(Form("gengrpt_%i",igmsb), &gmsbBranch.gengrpt_);
      disphotree->Branch(Form("gengrphi_%i",igmsb), &gmsbBranch.gengrphi_);
      disphotree->Branch(Form("gengreta_%i",igmsb), &gmsbBranch.gengreta_);
    } // end loop over nGBMBs
  } // end block over isGMSB

  // HVDS Info
  if (isHVDS)
  {
    disphotree->Branch("nvPions", &nvPions);

    hvdsBranches.resize(Config::nHVDSs);
    for (auto ihvds = 0; ihvds < Config::nHVDSs; ihvds++)
    {
      auto & hvdsBranch = hvdsBranches[ihvds];

      disphotree->Branch(Form("genvPionmass_%i",ihvds), &hvdsBranch.genvPionmass_);
      disphotree->Branch(Form("genvPionE_%i",ihvds), &hvdsBranch.genvPionE_);
      disphotree->Branch(Form("genvPionpt_%i",ihvds), &hvdsBranch.genvPionpt_);
      disphotree->Branch(Form("genvPionphi_%i",ihvds), &hvdsBranch.genvPionphi_);
      disphotree->Branch(Form("genvPioneta_%i",ihvds), &hvdsBranch.genvPioneta_);
      
      disphotree->Branch(Form("genvPionprodvx_%i",ihvds), &hvdsBranch.genvPionprodvx_);
      disphotree->Branch(Form("genvPionprodvy_%i",ihvds), &hvdsBranch.genvPionprodvy_);
      disphotree->Branch(Form("genvPionprodvz_%i",ihvds), &hvdsBranch.genvPionprodvz_);
      
      disphotree->Branch(Form("genvPiondecayvx_%i",ihvds), &hvdsBranch.genvPiondecayvx_);
      disphotree->Branch(Form("genvPiondecayvy_%i",ihvds), &hvdsBranch.genvPiondecayvy_);
      disphotree->Branch(Form("genvPiondecayvz_%i",ihvds), &hvdsBranch.genvPiondecayvz_);
      
      disphotree->Branch(Form("genHVph0E_%i",ihvds), &hvdsBranch.genHVph0E_);
      disphotree->Branch(Form("genHVph0pt_%i",ihvds), &hvdsBranch.genHVph0pt_);
      disphotree->Branch(Form("genHVph0phi_%i",ihvds), &hvdsBranch.genHVph0phi_);
      disphotree->Branch(Form("genHVph0eta_%i",ihvds), &hvdsBranch.genHVph0eta_);
      disphotree->Branch(Form("genHVph0match_%i",ihvds), &hvdsBranch.genHVph0match_);
      
      disphotree->Branch(Form("genHVph1E_%i",ihvds), &hvdsBranch.genHVph1E_);
      disphotree->Branch(Form("genHVph1pt_%i",ihvds), &hvdsBranch.genHVph1pt_);
      disphotree->Branch(Form("genHVph1phi_%i",ihvds), &hvdsBranch.genHVph1phi_);
      disphotree->Branch(Form("genHVph1eta_%i",ihvds), &hvdsBranch.genHVph1eta_);
      disphotree->Branch(Form("genHVph1match_%i",ihvds), &hvdsBranch.genHVph1match_);
    } // end loop over nHVDSs
  } // end block over HVDS

  // ToyMC Info
  if (isToy)
  {
    disphotree->Branch("nToyPhs", &nToyPhs);

    toyBranches.resize(Config::nToys);
    for (auto itoy = 0; itoy < Config::nToys; itoy++)
    {
      auto & toyBranch = toyBranches[itoy];

      disphotree->Branch(Form("genphE_%i",itoy), &toyBranch.genphE_);
      disphotree->Branch(Form("genphpt_%i",itoy), &toyBranch.genphpt_);
      disphotree->Branch(Form("genphphi_%i",itoy), &toyBranch.genphphi_);
      disphotree->Branch(Form("genpheta_%i",itoy), &toyBranch.genpheta_);
      
      disphotree->Branch(Form("genphmatch_%i",itoy), &toyBranch.genphmatch_);
      disphotree->Branch(Form("genphmatch_ptres_%i",itoy), &toyBranch.genphmatch_ptres_);
      disphotree->Branch(Form("genphmatch_status_%i",itoy), &toyBranch.genphmatch_status_);
    } // end loop over nToys
  } // end block over ToyMC

  // Run, Lumi, Event info
  disphotree->Branch("run", &run);
  disphotree->Branch("lumi", &lumi);
  disphotree->Branch("event", &event, "event/l");
   
  // Trigger Info
  disphotree->Branch("hltSignal", &hltSignal);
  disphotree->Branch("hltRefPhoID", &hltRefPhoID);
  disphotree->Branch("hltRefDispID", &hltRefDispID);
  disphotree->Branch("hltRefHT", &hltRefHT);
  disphotree->Branch("hltPho50", &hltPho50);
  disphotree->Branch("hltPho200", &hltPho200);
  disphotree->Branch("hltDiPho70", &hltDiPho70);
  disphotree->Branch("hltDiPho3022M90", &hltDiPho3022M90);
  disphotree->Branch("hltDiPho30PV18PV", &hltDiPho30PV18PV);
  disphotree->Branch("hltEle32WPT", &hltEle32WPT);
  disphotree->Branch("hltDiEle33MW", &hltDiEle33MW);
  disphotree->Branch("hltJet500", &hltJet500);

  // MET Filter Info
  disphotree->Branch("metPV", &metPV);
  disphotree->Branch("metBeamHalo", &metBeamHalo);
  disphotree->Branch("metHBHENoise", &metHBHENoise);
  disphotree->Branch("metHBHEisoNoise", &metHBHEisoNoise);
  disphotree->Branch("metECALTP", &metECALTP);
  disphotree->Branch("metPFMuon", &metPFMuon);
  disphotree->Branch("metPFChgHad", &metPFChgHad);
  disphotree->Branch("metEESC", &metEESC);
  disphotree->Branch("metECALCalib", &metECALCalib);
  disphotree->Branch("metECALBadCalib", &metECALBadCalib);

  // Vertex info
  disphotree->Branch("nvtx", &nvtx);
  disphotree->Branch("vtxX", &vtxX);
  disphotree->Branch("vtxY", &vtxY);
  disphotree->Branch("vtxZ", &vtxZ);
  
  // MET info
  disphotree->Branch("t1pfMETpt", &t1pfMETpt);
  disphotree->Branch("t1pfMETphi", &t1pfMETphi);
  disphotree->Branch("t1pfMETsumEt", &t1pfMETsumEt);

  disphotree->Branch("t1pfMETptUncorr", &t1pfMETptUncorr);
  disphotree->Branch("t1pfMETphiUncorr", &t1pfMETphiUncorr);
  disphotree->Branch("t1pfMETsumEtUncorr", &t1pfMETsumEtUncorr);

  // GEN MET info
  if (isMC)
  { 
    disphotree->Branch("genMETpt", &genMETpt);
    disphotree->Branch("genMETphi", &genMETphi);
  }

  // Jet info
  disphotree->Branch("njets", &njets);
  disphotree->Branch("jetE", &jetE);
  disphotree->Branch("jetpt", &jetpt);
  disphotree->Branch("jeteta", &jeteta);
  disphotree->Branch("jetphi", &jetphi);

  disphotree->Branch("jetID", &jetID);

  disphotree->Branch("jetNHF", &jetNHF);
  disphotree->Branch("jetNEMF", &jetNEMF);  
  disphotree->Branch("jetCHF", &jetCHF);
  disphotree->Branch("jetCEMF", &jetCEMF);
  disphotree->Branch("jetMUF", &jetMUF);
  disphotree->Branch("jetNHM", &jetNHM);
  disphotree->Branch("jetCHM", &jetCHM);

  if (isMC)
  {
    disphotree->Branch("jetscaleRel", &jetscaleRel);
    disphotree->Branch("jetsmearSF", &jetsmearSF);
    disphotree->Branch("jetsmearDownSF", &jetsmearDownSF);
    disphotree->Branch("jetsmearUpSF", &jetsmearUpSF);
    disphotree->Branch("jetisGen", &jetisGen);
  }

  // Electron Info
  disphotree->Branch("nelLowL", &nelLowL);
  disphotree->Branch("nelLowM", &nelLowM);
  disphotree->Branch("nelLowT", &nelLowT);
  disphotree->Branch("nelHighL", &nelHighL);
  disphotree->Branch("nelHighM", &nelHighM);
  disphotree->Branch("nelHighT", &nelHighT);

  // Muon Info
  disphotree->Branch("nmuLowL", &nmuLowL);
  disphotree->Branch("nmuLowM", &nmuLowM);
  disphotree->Branch("nmuLowT", &nmuLowT);
  disphotree->Branch("nmuHighL", &nmuHighL);
  disphotree->Branch("nmuHighM", &nmuHighM);
  disphotree->Branch("nmuHighT", &nmuHighT);

  // RecHit Info
  disphotree->Branch("nrechits", &nrechits);
  if (storeRecHits)
  {
    disphotree->Branch("rhX", &rhX);
    disphotree->Branch("rhY", &rhY);
    disphotree->Branch("rhZ", &rhZ);
    disphotree->Branch("rhE", &rhE);
    disphotree->Branch("rhtime", &rhtime);
    disphotree->Branch("rhtimeErr", &rhtimeErr);
    disphotree->Branch("rhTOF", &rhTOF);
    disphotree->Branch("rhID", &rhID);
    disphotree->Branch("rhisOOT", &rhisOOT);
    disphotree->Branch("rhisGS6", &rhisGS6);
    disphotree->Branch("rhisGS1", &rhisGS1);
    disphotree->Branch("rhadcToGeV", &rhadcToGeV);
    disphotree->Branch("rhped12", &rhped12);
    disphotree->Branch("rhped6", &rhped6);
    disphotree->Branch("rhped1", &rhped1);
    disphotree->Branch("rhpedrms12", &rhpedrms12);
    disphotree->Branch("rhpedrms6", &rhpedrms6);
    disphotree->Branch("rhpedrms1", &rhpedrms1);
  }

  // Photon Info
  disphotree->Branch("nphotons", &nphotons);

  phoBranches.resize(Config::nPhotons);
  for (auto iphoton = 0; iphoton < Config::nPhotons; iphoton++)
  {
    auto & phoBranch = phoBranches[iphoton];

    disphotree->Branch(Form("phoE_%i",iphoton), &phoBranch.E_);
    disphotree->Branch(Form("phopt_%i",iphoton), &phoBranch.pt_);
    disphotree->Branch(Form("phoeta_%i",iphoton), &phoBranch.eta_);
    disphotree->Branch(Form("phophi_%i",iphoton), &phoBranch.phi_);

    disphotree->Branch(Form("phoscE_%i",iphoton), &phoBranch.scE_);
    disphotree->Branch(Form("phosceta_%i",iphoton), &phoBranch.sceta_);
    disphotree->Branch(Form("phoscphi_%i",iphoton), &phoBranch.scphi_);

    disphotree->Branch(Form("phoHoE_%i",iphoton), &phoBranch.HoE_);
    disphotree->Branch(Form("phor9_%i",iphoton), &phoBranch.r9_);

    disphotree->Branch(Form("phoChgHadIso_%i",iphoton), &phoBranch.ChgHadIso_);
    disphotree->Branch(Form("phoNeuHadIso_%i",iphoton), &phoBranch.NeuHadIso_);
    disphotree->Branch(Form("phoPhoIso_%i",iphoton), &phoBranch.PhoIso_);

    disphotree->Branch(Form("phoEcalPFClIso_%i",iphoton), &phoBranch.EcalPFClIso_);
    disphotree->Branch(Form("phoHcalPFClIso_%i",iphoton), &phoBranch.HcalPFClIso_);
    disphotree->Branch(Form("phoTrkIso_%i",iphoton), &phoBranch.TrkIso_);

    disphotree->Branch(Form("phoChgHadIsoC_%i",iphoton), &phoBranch.ChgHadIsoC_);
    disphotree->Branch(Form("phoNeuHadIsoC_%i",iphoton), &phoBranch.NeuHadIsoC_);
    disphotree->Branch(Form("phoPhoIsoC_%i",iphoton), &phoBranch.PhoIsoC_);

    disphotree->Branch(Form("phoEcalPFClIsoC_%i",iphoton), &phoBranch.EcalPFClIsoC_);
    disphotree->Branch(Form("phoHcalPFClIsoC_%i",iphoton), &phoBranch.HcalPFClIsoC_);
    disphotree->Branch(Form("phoTrkIsoC_%i",iphoton), &phoBranch.TrkIsoC_);

    disphotree->Branch(Form("phosieie_%i",iphoton), &phoBranch.sieie_);
    disphotree->Branch(Form("phosipip_%i",iphoton), &phoBranch.sipip_);
    disphotree->Branch(Form("phosieip_%i",iphoton), &phoBranch.sieip_);

    disphotree->Branch(Form("phoe2x2_%i",iphoton), &phoBranch.e2x2_);
    disphotree->Branch(Form("phoe3x3_%i",iphoton), &phoBranch.e3x3_);
    disphotree->Branch(Form("phoe5x5_%i",iphoton), &phoBranch.e5x5_);

    disphotree->Branch(Form("phosmaj_%i",iphoton), &phoBranch.smaj_);
    disphotree->Branch(Form("phosmin_%i",iphoton), &phoBranch.smin_);
    disphotree->Branch(Form("phoalpha_%i",iphoton), &phoBranch.alpha_);

    if (storeRecHits)
    {
      disphotree->Branch(Form("phoseed_%i",iphoton), &phoBranch.seed_);
      disphotree->Branch(Form("phorecHits_%i",iphoton), &phoBranch.recHits_);
    }
    else 
    {
      disphotree->Branch(Form("phoseedX_%i",iphoton), &phoBranch.seedX_);
      disphotree->Branch(Form("phoseedY_%i",iphoton), &phoBranch.seedY_);
      disphotree->Branch(Form("phoseedZ_%i",iphoton), &phoBranch.seedZ_);
      disphotree->Branch(Form("phoseedE_%i",iphoton), &phoBranch.seedE_);
      disphotree->Branch(Form("phoseedtime_%i",iphoton), &phoBranch.seedtime_);
      disphotree->Branch(Form("phoseedtimeErr_%i",iphoton), &phoBranch.seedtimeErr_);
      disphotree->Branch(Form("phoseedTOF_%i",iphoton), &phoBranch.seedTOF_);
      disphotree->Branch(Form("phoseedID_%i",iphoton), &phoBranch.seedID_);
      disphotree->Branch(Form("phoseedisOOT_%i",iphoton), &phoBranch.seedisOOT_);
      disphotree->Branch(Form("phoseedisGS6_%i",iphoton), &phoBranch.seedisGS6_);
      disphotree->Branch(Form("phoseedisGS1_%i",iphoton), &phoBranch.seedisGS1_);
      disphotree->Branch(Form("phoseedadcToGeV_%i",iphoton), &phoBranch.seedadcToGeV_);
      disphotree->Branch(Form("phoseedped12_%i",iphoton), &phoBranch.seedped12_);
      disphotree->Branch(Form("phoseedped6_%i",iphoton), &phoBranch.seedped6_);
      disphotree->Branch(Form("phoseedped1_%i",iphoton), &phoBranch.seedped1_);
      disphotree->Branch(Form("phoseedpedrms12_%i",iphoton), &phoBranch.seedpedrms12_);
      disphotree->Branch(Form("phoseedpedrms6_%i",iphoton), &phoBranch.seedpedrms6_);
      disphotree->Branch(Form("phoseedpedrms1_%i",iphoton), &phoBranch.seedpedrms1_);
    }

    disphotree->Branch(Form("phosuisseX_%i",iphoton), &phoBranch.suisseX_);

    disphotree->Branch(Form("phoisOOT_%i",iphoton), &phoBranch.isOOT_);
    disphotree->Branch(Form("phoisEB_%i",iphoton), &phoBranch.isEB_);
    disphotree->Branch(Form("phoisHLT_%i",iphoton), &phoBranch.isHLT_);
    disphotree->Branch(Form("phoisTrk_%i",iphoton), &phoBranch.isTrk_);
    disphotree->Branch(Form("phopassEleVeto_%i",iphoton), &phoBranch.passEleVeto_);
    disphotree->Branch(Form("phohasPixSeed_%i",iphoton), &phoBranch.hasPixSeed_);
    disphotree->Branch(Form("phogedID_%i",iphoton), &phoBranch.gedID_);
    disphotree->Branch(Form("phoootID_%i",iphoton), &phoBranch.ootID_);

    if (isMC)
    {
      if (isGMSB || isHVDS) disphotree->Branch(Form("phoisSignal_%i",iphoton), &phoBranch.isSignal_);
      disphotree->Branch(Form("phoisGen_%i",iphoton), &phoBranch.isGen_);

      disphotree->Branch(Form("phoscaleAbs_%i",iphoton), &phoBranch.scaleAbs_);
      disphotree->Branch(Form("phosmearAbs_%i",iphoton), &phoBranch.smearAbs_);
    } // end block over isMC
  } // end loop over nPhotons
}

void DisPho::endJob() {}

void DisPho::beginRun(edm::Run const & iRun, edm::EventSetup const & iSetup) {}

void DisPho::endRun(edm::Run const & iRun, edm::EventSetup const & iSetup) {}

void DisPho::fillDescriptions(edm::ConfigurationDescriptions & descriptions)
{
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

DEFINE_FWK_MODULE(DisPho);
