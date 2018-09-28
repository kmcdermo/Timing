#include "Timing/TimingAnalyzer/plugins/DisPho.hh"

DisPho::DisPho(const edm::ParameterSet& iConfig): 
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

  // photon storing
  splitPho(iConfig.existsAs<bool>("splitPho") ? iConfig.getParameter<bool>("splitPho") : false),
  onlyGED (iConfig.existsAs<bool>("onlyGED")  ? iConfig.getParameter<bool>("onlyGED")  : false),
  onlyOOT (iConfig.existsAs<bool>("onlyOOT")  ? iConfig.getParameter<bool>("onlyOOT")  : false),

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

  // JER extra info
  smearjetEmin(iConfig.existsAs<double>("smearjetEmin") ? iConfig.getParameter<double>("smearjetEmin") : 0.01),

  // triggers
  inputPaths       (iConfig.existsAs<std::string>("inputPaths")   ? iConfig.getParameter<std::string>("inputPaths") : ""),
  inputFilters     (iConfig.existsAs<std::string>("inputFilters") ? iConfig.getParameter<std::string>("inputFilters") : ""),
  triggerResultsTag(iConfig.getParameter<edm::InputTag>("triggerResults")),
  triggerObjectsTag(iConfig.getParameter<edm::InputTag>("triggerObjects")),
  
  // MET flags
  inputFlags     (iConfig.existsAs<std::string>("inputFlags") ? iConfig.getParameter<std::string>("inputFlags") : ""),
  triggerFlagsTag(iConfig.getParameter<edm::InputTag>("triggerFlags")),

  // tracks
  tracksTag(iConfig.getParameter<edm::InputTag>("tracks")),

  // vertices
  verticesTag(iConfig.getParameter<edm::InputTag>("vertices")),

  // rhos
  rhosTag(iConfig.getParameter<edm::InputTag>("rhos")),

  // mets
  metsTag(iConfig.getParameter<edm::InputTag>("mets")),  

  // jets
  jetsTag(iConfig.getParameter<edm::InputTag>("jets")),  

  //recHits
  recHitsEBTag(iConfig.getParameter<edm::InputTag>("recHitsEB")),  
  recHitsEETag(iConfig.getParameter<edm::InputTag>("recHitsEE")),

  // photons + ids
  photonsTag(iConfig.getParameter<edm::InputTag>("photons")),

  // ootPhotons + ids
  ootPhotonsTag(iConfig.getParameter<edm::InputTag>("ootPhotons")),

  ///////////// GEN INFO
  // isMC or Data --> default Data
  isGMSB(iConfig.existsAs<bool>("isGMSB") ? iConfig.getParameter<bool>("isGMSB") : false),
  isHVDS(iConfig.existsAs<bool>("isHVDS") ? iConfig.getParameter<bool>("isHVDS") : false),
  isBkgd(iConfig.existsAs<bool>("isBkgd") ? iConfig.getParameter<bool>("isBkgd") : false),
  isToy (iConfig.existsAs<bool>("isToy")  ? iConfig.getParameter<bool>("isToy")  : false),
  isADD (iConfig.existsAs<bool>("isADD")  ? iConfig.getParameter<bool>("isADD")  : false),
  
  xsec(iConfig.existsAs<double>("xsec") ? iConfig.getParameter<double>("xsec") : 1.0),
  filterEff(iConfig.existsAs<double>("filterEff") ? iConfig.getParameter<double>("filterEff") : 1.0),
  BR(iConfig.existsAs<double>("BR") ? iConfig.getParameter<double>("BR") : 1.0)
{
  usesResource();
  usesResource("TFileService");

  // labels for cut flow histogram
  std::vector<std::string> cutflowLabelVec = {"All","nEvBlinding","METBlinding","Trigger","H_{T}","Good Photon"};
  int ibin = 0;
  for (const auto & cutflowLabel : cutflowLabelVec)
  {
    cutflowLabelMap[cutflowLabel] = ibin++;
  }

  // triggers
  triggerResultsToken = consumes<edm::TriggerResults> (triggerResultsTag);
  triggerObjectsToken = consumes<std::vector<pat::TriggerObjectStandAlone> > (triggerObjectsTag);

  // read in from a stream the trigger paths for saving
  oot::ReadInTriggerNames(inputPaths,pathNames,triggerBitMap);

  // read in from a stream the hlt objects/labels to match to
  oot::ReadInFilterNames(inputFilters,filterNames,triggerObjectsByFilterMap);

  // MET flags
  triggerFlagsToken = consumes<edm::TriggerResults> (triggerFlagsTag);

  // read in from a stream the trigger paths for saving
  oot::ReadInTriggerNames(inputFlags,flagNames,triggerFlagMap);

  // tracks 
  tracksToken = consumes<std::vector<reco::Track> > (tracksTag);

  // vertices
  verticesToken = consumes<std::vector<reco::Vertex> > (verticesTag);

  // rhos
  rhosToken = consumes<double> (rhosTag);

  // mets
  metsToken = consumes<std::vector<pat::MET> > (metsTag);

  // jets
  jetsToken = consumes<std::vector<pat::Jet> > (jetsTag);

  // rechits
  recHitsEBToken = consumes<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > (recHitsEBTag);
  recHitsEEToken = consumes<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > (recHitsEETag);

  // photons + ids
  photonsToken = consumes<std::vector<pat::Photon> > (photonsTag);
  if (not ootPhotonsTag.label().empty())
  {
    ootPhotonsToken = consumes<std::vector<pat::Photon> > (ootPhotonsTag);
  }

  // only for simulated samples
  if (isGMSB || isHVDS || isBkgd || isToy || isADD)
  {
    isMC = true;
    genevtInfoToken = consumes<GenEventInfoProduct>             (iConfig.getParameter<edm::InputTag>("genevt"));
    gent0Token      = consumes<float>                           (iConfig.getParameter<edm::InputTag>("gent0"));
    genxyz0Token    = consumes<Point3D>                         (iConfig.getParameter<edm::InputTag>("genxyz0"));
    pileupInfoToken = consumes<std::vector<PileupSummaryInfo> > (iConfig.getParameter<edm::InputTag>("pileup"));
    genpartsToken   = consumes<std::vector<reco::GenParticle> > (iConfig.getParameter<edm::InputTag>("genparts"));
    genjetsToken    = consumes<std::vector<reco::GenJet> >      (iConfig.getParameter<edm::InputTag>("genjets"));
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

  // MET FLAGS
  edm::Handle<edm::TriggerResults> triggerFlagsH;
  iEvent.getByToken(triggerFlagsToken, triggerFlagsH);

  // TRACKS
  edm::Handle<std::vector<reco::Track> > tracksH;
  iEvent.getByToken(tracksToken, tracksH);

  // VERTICES
  edm::Handle<std::vector<reco::Vertex> > verticesH;
  iEvent.getByToken(verticesToken, verticesH);
  
  // RHOS
  edm::Handle<double> rhosH;
  iEvent.getByToken(rhosToken, rhosH);

  // MET
  edm::Handle<std::vector<pat::MET> > metsH;
  iEvent.getByToken(metsToken, metsH);

  // JETS
  edm::Handle<std::vector<pat::Jet> > jetsH;
  iEvent.getByToken(jetsToken, jetsH);
  std::vector<pat::Jet> jets; jets.reserve(jetsH->size());

  // ECAL RECHITS
  edm::Handle<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > recHitsEBH;
  iEvent.getByToken(recHitsEBToken, recHitsEBH);
  const EcalRecHitCollection * recHitsEB = recHitsEBH.product();
  edm::Handle<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > recHitsEEH;
  iEvent.getByToken(recHitsEEToken, recHitsEEH);
  const EcalRecHitCollection * recHitsEE = recHitsEEH.product();
  uiiumap recHitMap;

  /////////////
  // PHOTONS //
  /////////////

  // GEDPHOTONS + IDS
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

  // GEOMETRY : https://gitlab.cern.ch/shervin/ECALELF
  edm::ESHandle<CaloGeometry> caloGeoH;
  iSetup.get<CaloGeometryRecord>().get(caloGeoH);
  const CaloSubdetectorGeometry * barrelGeometry = caloGeoH->getSubdetectorGeometry(DetId::Ecal, EcalBarrel);
  const CaloSubdetectorGeometry * endcapGeometry = caloGeoH->getSubdetectorGeometry(DetId::Ecal, EcalEndcap);

  //////////////////
  // ECAL RECORDS // 
  //////////////////

  // Laser constants : http://cmslxr.fnal.gov/source/RecoEcal/EgammaCoreTools/interface/EcalClusterLazyTools.h
  edm::ESHandle<EcalLaserDbService> laserH;
  iSetup.get<EcalLaserDbRecord>().get(laserH);

  // Intercalibration constants : http://cmslxr.fnal.gov/source/RecoEcal/EgammaCoreTools/interface/EcalClusterLazyTools.h
  edm::ESHandle<EcalIntercalibConstants> interCalibH;
  const EcalIntercalibConstantMap *      interCalibMap;
  iSetup.get<EcalIntercalibConstantsRcd>().get(interCalibH);
  interCalibMap = &interCalibH->getMap();
  
  // ADCToGeV : http://cmslxr.fnal.gov/source/RecoEcal/EgammaCoreTools/interface/EcalClusterLazyTools.h
  edm::ESHandle<EcalADCToGeVConstant> adcToGeVH;
  iSetup.get<EcalADCToGeVConstantRcd>().get(adcToGeVH);

  // Pedestals : https://github.com/ferriff/usercode/blob/master/DBDump/plugins/DBDump.cc
  edm::ESHandle<EcalPedestals> pedestalsH;
  iSetup.get<EcalPedestalsRcd>().get(pedestalsH);

  /////////////////
  // JET DB INFO //
  /////////////////

  // JECs : https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookJetEnergyCorrections#CorrPatJets  
  edm::ESHandle<JetCorrectorParametersCollection> jetCorrH;
  iSetup.get<JetCorrectionsRecord>().get("AK4PFchs",jetCorrH); 
  JetCorrectionUncertainty jetCorrUnc((*jetCorrH)["Uncertainty"]);

  // JERs : https://twiki.cern.ch/twiki/bin/view/CMSPublic/WorkBookJetEnergyResolution#Accessing_factors_from_Global_Ta
  JME::JetResolution jetRes = JME::JetResolution::get(iSetup, "AK4PFchs_pt");
  JME::JetResolutionScaleFactor jetRes_sf = JME::JetResolutionScaleFactor::get(iSetup, "AK4PFchs");

  //////////////
  // GEN INFO //
  //////////////

  edm::Handle<GenEventInfoProduct> genevtInfoH;
  edm::Handle<float>   gent0H;
  edm::Handle<Point3D> genxyz0H;
  edm::Handle<std::vector<PileupSummaryInfo> > pileupInfoH;
  edm::Handle<std::vector<reco::GenParticle> > genparticlesH;
  edm::Handle<std::vector<reco::GenJet> > genjetsH;
  genPartVec neutralinos;
  genPartVec vPions;
  genPartVec toys;

  if (isMC)
  {
    iEvent.getByToken(genevtInfoToken, genevtInfoH);
    iEvent.getByToken(genxyz0Token   , genxyz0H);
    iEvent.getByToken(gent0Token     , gent0H);
    iEvent.getByToken(pileupInfoToken, pileupInfoH);
    iEvent.getByToken(genpartsToken  , genparticlesH);
    iEvent.getByToken(genjetsToken   , genjetsH);

    ///////////////////////
    //                   //
    // Event weight info //
    //                   //
    ///////////////////////
    DisPho::InitializeGenEvtBranches();
    if (genevtInfoH.isValid()) 
    {
      DisPho::SetGenEvtBranches(genevtInfoH);
    }

    /////////////////////
    //                 //
    // Gen pileup info //
    //                 //
    /////////////////////
    DisPho::InitializeGenPUBranches();
    if (pileupInfoH.isValid()) // standard check for pileup
    {
      DisPho::SetGenPUBranches(pileupInfoH);
    } // end check over pileup
  }

  //////////////////////
  //                  //
  // Set Event Weight //
  //                  //
  //////////////////////
  const Float_t wgt = (isMC ? genwgt : 1.f);

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

  //////////////
  //          //
  // Blinding //
  //          //
  //////////////
  if (event%blindSF!=0 && applyBlindSF) return;
  h_cutflow    ->Fill((cutflowLabelMap["nEvBlinding"]*1.f));
  h_cutflow_wgt->Fill((cutflowLabelMap["nEvBlinding"]*1.f),wgt);

  if (metsH.isValid())
  {
    if ((*metsH)[0].pt() > blindMET && applyBlindMET) return;
  }  
  h_cutflow    ->Fill((cutflowLabelMap["METBlinding"]*1.f));
  h_cutflow_wgt->Fill((cutflowLabelMap["METBlinding"]*1.f),wgt);

  /////////
  //     //
  // Rho //
  //     //
  /////////
  DisPho::InitializeRhoBranches();
  if (rhosH.isValid())
  {
    DisPho::SetRhoBranches(rhosH);
  }

  /////////////////////
  //                 //
  // Object Prepping //
  //                 //
  /////////////////////
  if (isGMSB) oot::PrepNeutralinos(genparticlesH,neutralinos);
  if (isHVDS) oot::PrepVPions(genparticlesH,vPions);
  if (isToy)  oot::PrepToys(genparticlesH,toys);
  oot::PrepTriggerBits(triggerResultsH,iEvent,triggerBitMap);
  oot::PrepTriggerBits(triggerFlagsH,iEvent,triggerFlagMap);
  oot::PrepTriggerObjects(triggerResultsH,triggerObjectsH,iEvent,triggerObjectsByFilterMap);
  oot::PrepJets(jetsH,jets,jetpTmin,jetEtamax,jetIDmin);
  oot::PrepRecHits(recHitsEB,recHitsEE,recHitMap,rhEmin);
  oot::PrepPhotons(photonsH,ootPhotonsH,photons,rho,phpTmin,phIDmin);

  ///////////////////
  //               //
  // Extra Pruning //
  //               //
  ///////////////////
  oot::PrunePhotons(photons,recHitsEB,recHitsEE,seedTimemin);
  oot::PruneJets(jets,photons,dRmin);
		
  /////////////////////////////
  //                         //
  // Photon Storing Options  //
  //                         //
  /////////////////////////////
  if (splitPho) oot::SplitPhotons(photons,Config::nPhotons/2);     // split photons by OOT and GED (store at most nPhotons/2 of each)
  if (onlyGED)  oot::StoreOnlyPho(photons,Config::nPhotons,false); // store only GED photons, top nPhotons only
  if (onlyOOT)  oot::StoreOnlyPho(photons,Config::nPhotons,true);  // store only OOT photons, top nPhotons only
  
  ///////////////////////////////
  //                           //
  // Object Counts for Storing //
  //                           //
  ///////////////////////////////
  const int nJets    = std::min(int(jets.size()),Config::nJets);
  const int nRecHits = recHitMap.size();
  const int nPhotons = std::min(int(photons.size()),Config::nPhotons);

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
  h_cutflow    ->Fill((cutflowLabelMap["Trigger"]*1.f));
  h_cutflow_wgt->Fill((cutflowLabelMap["Trigger"]*1.f),wgt);

  // HT pre-selection
  auto jetHT = 0.f;
  for (auto ijet = 0; ijet < nJets; ijet++) jetHT += jets[ijet].pt();
  if (jetHT < minHT && applyHT) return;
  h_cutflow    ->Fill((cutflowLabelMap["H_{T}"]*1.f));
  h_cutflow_wgt->Fill((cutflowLabelMap["H_{T}"]*1.f),wgt);

  // photon pre-selection: at least one good photon in event
  bool isphgood = false;
  if (photonsH.isValid() || ootPhotonsH.isValid()) 
  {
    for (auto iphoton = 0; iphoton < nPhotons; iphoton++)
    {
      const auto & photon = photons[iphoton];
      const auto & pho = photon.photon();

      //      if (iphoton > 0) break; // only consider first photon

      if (pho.pt() < phgoodpTmin) continue;

      // const float sceta = std::abs(pho.superCluster()->eta());
      // if (sceta > Config::etaEBcutoff) continue;

      if (phgoodIDmin != "none")
      {
	if (!photon.isOOT() && !pho.photonID(phgoodIDmin+"-ged")) continue;
	if ( photon.isOOT() && !pho.photonID(phgoodIDmin+"-oot")) continue;
      }
      
      isphgood = true; break;
    } 
  } // end check
  if (!isphgood && applyPhGood) return;
  h_cutflow    ->Fill((cutflowLabelMap["Good Photon"]*1.f));
  h_cutflow_wgt->Fill((cutflowLabelMap["Good Photon"]*1.f),wgt);

  /////////////
  //         //
  // MC Info //
  //         //
  /////////////
  if (isMC) 
  {
    ////////////////
    //            //
    // xyzt0 info //
    //            //
    ////////////////
    DisPho::InitializeGenPointBranches();
    if (genxyz0H.isValid()) // standard check 3D point
    {
      DisPho::SetGenXYZ0Branches(genxyz0H);
    }
    if (gent0H.isValid()) // standard check on t0
    {
      DisPho::SetGenT0Branches(gent0H);
    }
      
    ///////////////////////
    //                   //
    // Gen particle info //
    //                   //
    ///////////////////////
    // GMSB
    if (isGMSB) 
    {
      DisPho::InitializeGMSBBranches();
      if (genparticlesH.isValid() && (photonsH.isValid() || ootPhotonsH.isValid()))
      {
	DisPho::SetGMSBBranches(neutralinos,photons,nPhotons);
      } // check genparticles are okay
    } // isGMSB

    // HVDS
    if (isHVDS) 
    {
      DisPho::InitializeHVDSBranches();
      if (genparticlesH.isValid() && (photonsH.isValid() || ootPhotonsH.isValid()))
      {
	DisPho::SetHVDSBranches(vPions,photons,nPhotons);
      } // check genparticles are okay
    } // isHVDS

    // ToyMC
    if (isToy) 
    {
      DisPho::InitializeToyBranches();
      if (genparticlesH.isValid() && (photonsH.isValid() || ootPhotonsH.isValid()))
      {
	DisPho::SetToyBranches(toys,photons,nPhotons);
      } // check genparticles are okay
    } // isHVDS
  } // isMC

  ///////////////////////////
  //                       //
  // Event, lumi, run info //
  //                       //
  ///////////////////////////
  DisPho::SetRecordInfo(iEvent);

  //////////////////
  //              //
  // Trigger Info //
  //              //
  //////////////////
  DisPho::SetTriggerBranches();

  /////////////////////
  //                 //
  // MET Filter Info //
  //                 //
  /////////////////////
  DisPho::SetMETFilterBranches();

  /////////////////////////
  //                     //   
  // Primary Vertex info //
  //                     //
  /////////////////////////
  DisPho::InitializePVBranches();
  if (verticesH.isValid())
  {
    DisPho::SetPVBranches(verticesH);
  }

  //////////////////
  //              //
  // Type1 PF Met //
  //              //
  //////////////////
  DisPho::InitializeMETBranches();
  if (metsH.isValid())
  {
    const auto & t1pfMET = (*metsH)[0];
    
    DisPho::SetMETBranches(t1pfMET);
  }

  /////////////////////////
  //                     //
  // Jets (AK4 standard) //
  //                     //
  /////////////////////////
  DisPho::InitializeJetBranches(nJets);
  if (isMC) DisPho::InitializeJetBranchesMC(nJets);
  if (jetsH.isValid()) // check to make sure reco jets exist
  {
    DisPho::SetJetBranches(jets,nJets);
    if (isMC && jetCorrH.isValid() && genjetsH.isValid()) DisPho::SetJetBranchesMC(jets,nJets,genjetsH,jetCorrUnc,jetRes,jetRes_sf);
  }

  //////////////
  //          //
  // Rec Hits //
  //          //
  //////////////
  DisPho::InitializeRecHitBranches(nRecHits);
  if (recHitsEBH.isValid() && recHitsEEH.isValid() && caloGeoH.isValid() && adcToGeVH.isValid() && laserH.isValid() && interCalibH.isValid() && pedestalsH.isValid())
  {
    DisPho::SetRecHitBranches(recHitsEB,barrelGeometry,recHitsEE,endcapGeometry,
			      recHitMap,iEvent,laserH,interCalibMap,adcToGeVH,pedestalsH);
  }

  //////////////////
  //              //
  // Reco Photons //
  //              //
  //////////////////
  DisPho::InitializePhoBranches();
  if (isMC) DisPho::InitializePhoBranchesMC();
  if ((photonsH.isValid() || ootPhotonsH.isValid()) && recHitsEBH.isValid() && recHitsEEH.isValid() && tracksH.isValid()) // standard handle check
  {
    DisPho::SetPhoBranches(photons,nPhotons,recHitMap,recHitsEB,recHitsEE,tracksH);
    if (isMC && genparticlesH.isValid()) DisPho::SetPhoBranchesMC(photons,nPhotons,genparticlesH);
  }

  ///////////////
  //           //
  // Fill Tree //
  //           //
  ///////////////
  disphotree->Fill();
}

void DisPho::InitializeRhoBranches()
{
  rho = 0.f;
}

void DisPho::SetRhoBranches(const edm::Handle<double> & rhosH)
{
  rho = *(rhosH.product());
}

void DisPho::InitializeGenEvtBranches()
{
  genwgt = -9999.f;
}

void DisPho::SetGenEvtBranches(const edm::Handle<GenEventInfoProduct> & genevtInfoH)
{
  genwgt = genevtInfoH->weight();
}

void DisPho::InitializeGenPointBranches()
{
  genx0 = -9999.f;
  geny0 = -9999.f;
  genz0 = -9999.f;
  gent0 = -9999.f;
}

void DisPho::SetGenXYZ0Branches(const edm::Handle<Point3D> & genxyz0H)
{
  const auto & genxyz0 = *(genxyz0H.product());

  genx0 = genxyz0.X();
  geny0 = genxyz0.Y();
  genz0 = genxyz0.Z();
}

void DisPho::SetGenT0Branches(const edm::Handle<float> & gent0H)
{
  gent0 = *(gent0H.product());
}

void DisPho::InitializeGenPUBranches()
{
  genpuobs = -9999; 
  genputrue = -9999;
}

void DisPho::SetGenPUBranches(edm::Handle<std::vector<PileupSummaryInfo> > & pileupInfoH)
{
  for (const auto & puinfo : *pileupInfoH)
  {
    if (puinfo.getBunchCrossing() == 0) 
    {
      genpuobs  = puinfo.getPU_NumInteractions();
      genputrue = puinfo.getTrueNumInteractions();
      
      break;
    } // end check over correct BX
  } // end loop over PU
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

void DisPho::SetGMSBBranches(const std::vector<reco::GenParticle> & neutralinos, const std::vector<oot::Photon> & photons, const int nPhotons)
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
      
      if (photon.pt() < ((1.f-genpTres) * gmsbBranch.genphpt_)) continue;
      if (photon.pt() > ((1.f+genpTres) * gmsbBranch.genphpt_)) continue;
      
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

void DisPho::SetHVDSBranches(const std::vector<reco::GenParticle> & vPions, const std::vector<oot::Photon> & photons, const int nPhotons)
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
      const auto tmppt = photon.pt();

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

void DisPho::SetToyBranches(const std::vector<reco::GenParticle> & toys, const std::vector<oot::Photon> & photons, const int nPhotons)
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
      
      if ( (photon.pt() >= ((1.f-genpTres) * toyBranch.genphpt_)) && (photon.pt() <= ((1.f+genpTres) * toyBranch.genphpt_)) )
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
  hltSignal = (triggerBitMap.count(Config::SignalPath.c_str()) ? triggerBitMap[Config::SignalPath.c_str()] : false);
  hltRefPhoID = (triggerBitMap.count(Config::RefPhoIDPath.c_str()) ? triggerBitMap[Config::RefPhoIDPath.c_str()] : false);
  hltRefDispID = (triggerBitMap.count(Config::RefDispIDPath.c_str()) ? triggerBitMap[Config::RefDispIDPath.c_str()] : false);
  hltRefHT = (triggerBitMap.count(Config::RefHTPath.c_str()) ? triggerBitMap[Config::RefHTPath.c_str()] : false);
  hltPho50 = (triggerBitMap.count(Config::Pho50Path.c_str()) ? triggerBitMap[Config::Pho50Path.c_str()] : false);
  hltPho200 = (triggerBitMap.count(Config::Pho200Path.c_str()) ? triggerBitMap[Config::Pho200Path.c_str()] : false);
  hltDiPho70 = (triggerBitMap.count(Config::DiPho70Path.c_str()) ? triggerBitMap[Config::DiPho70Path.c_str()] : false);
  hltDiPho3022M90 = (triggerBitMap.count(Config::DiPho3022M90Path.c_str()) ? triggerBitMap[Config::DiPho3022M90Path.c_str()] : false);
  hltDiPho30PV18PV = (triggerBitMap.count(Config::DiPho30PV18PVPath.c_str()) ? triggerBitMap[Config::DiPho30PV18PVPath.c_str()] : false);
  hltEle32WPT = (triggerBitMap.count(Config::Ele32WPTPath.c_str()) ? triggerBitMap[Config::Ele32WPTPath.c_str()] : false);
  hltDiEle33MW = (triggerBitMap.count(Config::DiEle33MWPath.c_str()) ? triggerBitMap[Config::DiEle33MWPath.c_str()] : false);
  hltJet500 = (triggerBitMap.count(Config::Jet500Path.c_str()) ? triggerBitMap[Config::Jet500Path.c_str()] : false);
}

void DisPho::SetMETFilterBranches()
{
  metPV = (triggerFlagMap.count(Config::PVFlag.c_str()) ? triggerFlagMap[Config::PVFlag.c_str()] : false);
  metBeamHalo = (triggerFlagMap.count(Config::BeamHaloFlag.c_str()) ? triggerFlagMap[Config::BeamHaloFlag.c_str()] : false);
  metHBHENoise = (triggerFlagMap.count(Config::HBHENoiseFlag.c_str()) ? triggerFlagMap[Config::HBHENoiseFlag.c_str()] : false);
  metHBHEisoNoise = (triggerFlagMap.count(Config::HBHEisoNoiseFlag.c_str()) ? triggerFlagMap[Config::HBHEisoNoiseFlag.c_str()] : false);
  metECALTP = (triggerFlagMap.count(Config::ECALTPFlag.c_str()) ? triggerFlagMap[Config::ECALTPFlag.c_str()] : false);
  metPFMuon = (triggerFlagMap.count(Config::PFMuonFlag.c_str()) ? triggerFlagMap[Config::PFMuonFlag.c_str()] : false);
  metPFChgHad = (triggerFlagMap.count(Config::PFChgHadFlag.c_str()) ? triggerFlagMap[Config::PFChgHadFlag.c_str()] : false);
  metEESC = (triggerFlagMap.count(Config::EESCFlag.c_str()) ? triggerFlagMap[Config::EESCFlag.c_str()] : false);
  metECALCalib = (triggerFlagMap.count(Config::ECALCalibFlag.c_str()) ? triggerFlagMap[Config::ECALCalibFlag.c_str()] : false);
}

void DisPho::InitializePVBranches()
{
  nvtx = -9999; 
  vtxX = -9999.f;
  vtxY = -9999.f;
  vtxZ = -9999.f;
}

void DisPho::SetPVBranches(const edm::Handle<std::vector<reco::Vertex> > & verticesH)
{
  nvtx = verticesH->size();
  
  const auto & primevtx = (*verticesH)[0];

  vtxX = primevtx.position().x();
  vtxY = primevtx.position().y();
  vtxZ = primevtx.position().z();
}

void DisPho::InitializeMETBranches()
{
  t1pfMETpt    = -9999.f;
  t1pfMETphi   = -9999.f;
  t1pfMETsumEt = -9999.f;
}

void DisPho::SetMETBranches(const pat::MET & t1pfMET)
{
  t1pfMETpt    = t1pfMET.pt();
  t1pfMETphi   = t1pfMET.phi();
  t1pfMETsumEt = t1pfMET.sumEt();
}
			    
void DisPho::InitializeJetBranches(const int nJets)
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

void DisPho::SetJetBranches(const std::vector<pat::Jet> & jets, const int nJets)
{
  njets = jets.size();

  for (auto ijet = 0; ijet < nJets; ijet++)
  {
    const auto & jet = jets[ijet];
    
    jetE  [ijet] = jet.energy();
    jetpt [ijet] = jet.pt();
    jetphi[ijet] = jet.phi();
    jeteta[ijet] = jet.eta();

    jetID [ijet] = oot::GetPFJetID(jet);

    jetNHF [ijet] = jet.neutralHadronEnergyFraction();
    jetNEMF[ijet] = jet.neutralEmEnergyFraction();
    jetCHF [ijet] = jet.chargedHadronEnergyFraction();
    jetCEMF[ijet] = jet.chargedEmEnergyFraction();
    jetMUF [ijet] = jet.muonEnergyFraction();
    jetNHM [ijet] = jet.neutralMultiplicity();
    jetCHM [ijet] = jet.chargedMultiplicity();
  }
}

void DisPho::InitializeJetBranchesMC(const int nJets)
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

void DisPho::SetJetBranchesMC(const std::vector<pat::Jet> & jets, const int nJets, const edm::Handle<std::vector<reco::GenJet> > & genjetsH, 
			      JetCorrectionUncertainty & jetCorrUnc, const JME::JetResolution & jetRes, const JME::JetResolutionScaleFactor & jetRes_sf)
{
  // JER procedure explanation from https://twiki.cern.ch/twiki/bin/viewauth/CMS/JetResolution#Smearing_procedures
  // JER implementation copied from https://github.com/cms-sw/cmssw/blob/master/PhysicsTools/PatUtils/interface/SmearedJetProducerT.h#L208-L215
  const auto runNum_uint = static_cast <unsigned int> (run);
  const auto lumiNum_uint = static_cast <unsigned int> (lumi);
  const auto evNum_uint = static_cast <unsigned int> (event);
  const auto jet0eta = uint32_t(jets.empty() ? 0 : jets[0].eta()/0.01);
  std::mt19937 mt_rand(1 + jet0eta + (lumiNum_uint<<10) + (runNum_uint<<20) + evNum_uint);

  // get genjets
  const auto & genjets = *(genjetsH);

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
    auto igenjet = DisPho::GenJetMatcher(jet,genjets,jer);

    if (igenjet > 0) // if matched, use scaling method
    {
      const auto ptres = 1.f - (genjets[igenjet].pt() / pt);

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

int DisPho::GenJetMatcher(const pat::Jet & jet, const std::vector<reco::GenJet> & genjets, const float jer)
{
  auto igenjet = -1;
  auto mindR = genjetdRmin;

  for (auto i = 0U; i < genjets.size(); i++)
  {
    const auto & genjet = genjets[i];
    const auto delR = reco::deltaR(jet, genjet);
    
    if (delR < mindR)
    {
      if (std::abs(genjet.pt() - jet.pt()) < (genjetpTfactor * jer * jet.pt()))
      {
	mindR = delR;
	igenjet = i;
      }
    }
  }
  
  return igenjet;
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

void DisPho::InitializeRecHitBranches(const int nRecHits)
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

void DisPho::SetRecHitBranches(const EcalRecHitCollection * recHitsEB, const CaloSubdetectorGeometry * barrelGeometry,
			       const EcalRecHitCollection * recHitsEE, const CaloSubdetectorGeometry * endcapGeometry,
			       const uiiumap & recHitMap, const edm::Event & iEvent,
			       const edm::ESHandle<EcalLaserDbService> & laserH, const EcalIntercalibConstantMap * interCalibMap, 
			       const edm::ESHandle<EcalADCToGeVConstant> & adcToGeVH, const edm::ESHandle<EcalPedestals> & pedestalsH)
{
  nrechits = recHitMap.size();
  
  DisPho::SetRecHitBranches(recHitsEB,barrelGeometry,recHitMap,iEvent,laserH,interCalibMap,adcToGeVH->getEBValue(),pedestalsH);
  DisPho::SetRecHitBranches(recHitsEE,endcapGeometry,recHitMap,iEvent,laserH,interCalibMap,adcToGeVH->getEEValue(),pedestalsH);
}

void DisPho::SetRecHitBranches(const EcalRecHitCollection * recHits, const CaloSubdetectorGeometry * geometry,
			       const uiiumap & recHitMap, const edm::Event & iEvent, 
			       const edm::ESHandle<EcalLaserDbService> & laserH, const EcalIntercalibConstantMap * interCalibMap,
			       const float adcToGeV, const edm::ESHandle<EcalPedestals> & pedestalsH)
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
      const auto laser = laserH->getLaserCorrection(recHitId,iEvent.time());
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
    phoBranch.scPhi_ = -9999.f;
    phoBranch.scEta_ = -9999.f;

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

void DisPho::SetPhoBranches(const std::vector<oot::Photon> photons, const int nPhotons, const uiiumap & recHitMap,
			    const EcalRecHitCollection * recHitsEB, const EcalRecHitCollection * recHitsEE,
			    const edm::Handle<std::vector<reco::Track> > & tracksH)
{
  nphotons = photons.size();
  
  for (auto iphoton = 0; iphoton < nPhotons; iphoton++)
  {
    // get objects
    const auto & photon = photons[iphoton];
    auto & phoBranch = phoBranches[iphoton];
    const auto & pho = photon.photon();
    
    // basic kinematic with v2: https://twiki.cern.ch/twiki/bin/viewauth/CMS/EgammaMiniAODV2#Applying_the_Energy_Scale_and_sm
    const auto phop4 = pho.p4() * (pho.pat::Photon::userFloat("ecalEnergyPostCorr") / pho.pat::Photon::energy());
    phoBranch.E_   = phop4.energy();
    phoBranch.pt_  = phop4.pt();
    phoBranch.phi_ = phop4.phi();
    phoBranch.eta_ = phop4.eta();

    // save pt for later use
    const auto phopt = phoBranch.Pt_;

    // super cluster info from photon
    const auto & phosc = pho.superCluster().isNonnull() ? pho.superCluster() : pho.parentSuperCluster();
    const auto scEta = phosc->eta();

    phoBranch.scE_   = phosc->energy();
    phoBranch.scphi_ = phosc->phi();
    phoBranch.sceta_ = phosc->eta();

    // ID-like variables
    phoBranch.HoE_ = pho.hadTowOverEm(); // used in ID + trigger (single tower HoverE)
    phoBranch.r9_  = pho.r9(); // used in slimming in PAT + trigger

    // PF Isolations : GED ID
    phoBranch.ChgHadIso_ = pho.chargedHadronIso();
    phoBranch.NeuHadIso_ = pho.neutralHadronIso();
    phoBranch.PhoIso_    = pho.photonIso();

    // PF Cluster Isos : OOT ID
    phoBranch.EcalPFClIso_ = pho.ecalPFClusterIso();
    phoBranch.HcalPFClIso_ = pho.hcalPFClusterIso();
    phoBranch.TrkIso_      = pho.trkSumPtHollowConeDR03();

    // corrected values for isolations
    phoBranch.ChgHadIsoC_ = std::max(phoBranch.ChgHadIso_ - (rho * oot::GetChargedHadronEA(scEta))                                              ,0.f);
    phoBranch.NeuHadIsoC_ = std::max(phoBranch.NeuHadIso_ - (rho * oot::GetNeutralHadronEA(scEta)) - (oot::GetNeutralHadronPtScale(scEta,phopt)),0.f);
    phoBranch.PhoIsoC_    = std::max(phoBranch.PhoIso_    - (rho * oot::GetGammaEA        (scEta)) - (oot::GetGammaPtScale        (scEta,phopt)),0.f);

    phoBranch.EcalPFClIsoC_ = std::max(phoBranch.EcalPFClIso_ - (rho * oot::GetEcalPFClEA(scEta)) - (oot::GetEcalPFClPtScale(scEta,phopt)),0.f);
    phoBranch.HcalPFClIsoC_ = std::max(phoBranch.HcalPFClIso_ - (rho * oot::GetHcalPFClEA(scEta)) - (oot::GetHcalPFClPtScale(scEta,phopt)),0.f);
    phoBranch.TrkIsoC_      = std::max(phoBranch.TrkIso_      - (rho * oot::GetTrackEA   (scEta)) - (oot::GetTrackPtScale   (scEta,phopt)),0.f);

    // Shower Shape Objects
    const auto & phoshape = pho.full5x5_showerShapeVariables(); 

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
    const bool isEB = (seedDetId.subdetId() == EcalBarrel); // which subdet
    const auto recHits = (isEB ? recHitsEB : recHitsEE); 

    // 2nd moments from official calculation
    if (recHits->size() > 0)
    {
      const auto & ph2ndMoments = noZS::EcalClusterTools::cluster2ndMoments( *phosc, *recHits);
      // radius of semi-major,minor axis is the inverse square root of the eigenvalues of the covariance matrix
      phoBranch.smaj_  = ph2ndMoments.sMaj;
      phoBranch.smin_  = ph2ndMoments.sMin;
      phoBranch.alpha_ = ph2ndMoments.alpha;
    }

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
    }
  
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
      }
    
      // swiss cross
      if (recHits->size() > 0) phoBranch.suisseX_ = EcalTools::swissCross(seedDetId, *recHits, rhEmin);
    } // end check over if seed exists
    
    // some standard booleans
    phoBranch.isOOT_ = photon.isOOT();
    phoBranch.isEB_  = isEB;

    // HLT Matching!
    strBitMap isHLTMatched;
    for (const auto & filter : filterNames) isHLTMatched[filter] = false;
    oot::HLTToObjectMatching(triggerObjectsByFilterMap,isHLTMatched,photon,pTres,dRmin);
    const std::string filter = Config::DispIDFilter.c_str();
    phoBranch.isHLT_ = (isHLTMatched.count(filter.c_str()) ? isHLTMatched[filter.c_str()] : false);

    // check for simple track veto
    phoBranch.isTrk_ = oot::TrackToObjectMatching(tracksH,photon,trackpTmin,trackdRmin);

    // other track vetoes
    phoBranch.passEleVeto_ = pho.passElectronVeto();
    phoBranch.hasPixSeed_  = pho.hasPixelSeed();
  
    // 0 --> did not pass anything, 1 --> loose pass, 2 --> medium pass, 3 --> tight pass
    // GED first
    if      (pho.photonID("tight-ged"))  {phoBranch.gedID_ = 3;}
    else if (pho.photonID("medium-ged")) {phoBranch.gedID_ = 2;}
    else if (pho.photonID("loose-ged"))  {phoBranch.gedID_ = 1;}
    else                                 {phoBranch.gedID_ = 0;}
    // OOT second
    if      (pho.photonID("tight-oot"))  {phoBranch.ootID_ = 3;}
    else if (pho.photonID("loose-oot"))  {phoBranch.ootID_ = 1;}
    else                                 {phoBranch.ootID_ = 0;}
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

void DisPho::SetPhoBranchesMC(const std::vector<oot::Photon> photons, const int nPhotons, 
			      const edm::Handle<std::vector<reco::GenParticle> > & genparticlesH)
{
  for (auto iphoton = 0; iphoton < nPhotons; iphoton++)
  {
    // get objects
    const auto & photon = photons[iphoton];
    auto & phoBranch = phoBranches[iphoton];
    const auto & pho = photon.photon();
    
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
    phoBranch.isGen_ = oot::GenToObjectMatching(photon,genparticlesH,genpTres,gendRmin);
    
    // scale and smearing uncs
    const auto phoE = phoBranch.E_; // assumed this already set!!!
      
    // eval up/down, then save bigger one (diff is order of one of MeV's...
    const auto down_scale = std::abs(phoE-pho.pat::Photon::userFloat("energyScaleDown"));
    const auto up_scale   = std::abs(pho.pat::Photon::userFloat("energyScaleUp")-phoE);
    phoBranch.scaleAbs_   = (up_scale > down_scale ? up_scale : down_scale);

    // eval up/down, then save bigger one (diff is order of one of MeV's...
    const auto down_smear = std::abs(phoE-pho.pat::Photon::userFloat("energySigmaDown"));
    const auto up_smear   = std::abs(pho.pat::Photon::userFloat("energySigmaUp")-phoE);
    phoBranch.smearAbs_   = (up_smear > down_smear ? up_smear : down_smear);
  }
}

int DisPho::CheckMatchHVDS(const int iphoton, const hvdsStruct& hvdsBranch)
{
  if      (iphoton == hvdsBranch.genHVph0match_ && iphoton != hvdsBranch.genHVph1match_) return 1;
  else if (iphoton != hvdsBranch.genHVph0match_ && iphoton == hvdsBranch.genHVph1match_) return 2;
  else if (iphoton == hvdsBranch.genHVph0match_ && iphoton == hvdsBranch.genHVph1match_) return 3;
  else                                                                                   return 0;
}

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
  unsigned int blindSF_tmp = blindSF;
  bool applyBlindSF_tmp = applyBlindSF;
  float blindMET_tmp = blindMET;
  bool applyBlindMET_tmp = applyBlindMET;
  configtree->Branch("blindSF", &blindSF_tmp, "blindSF/i");
  configtree->Branch("applyBlindSF", &applyBlindSF_tmp, "applyBlindSF/O");
  configtree->Branch("blindMET", &blindMET_tmp, "blindMET/F");
  configtree->Branch("applyBlindMET", &applyBlindMET_tmp, "applyBlindMET/O");

  // object prep
  float jetpTmin_tmp = jetpTmin;
  float jetEtamax_tmp = jetEtamax;
  int jetIDmin_tmp = jetIDmin;
  float rhEmin_tmp = rhEmin;
  float phpTmin_tmp = phpTmin;
  std::string phIDmin_tmp = phIDmin;
  configtree->Branch("jetpTmin", &jetpTmin_tmp, "jetpTmin/F");
  configtree->Branch("jetEtamax", &jetEtamax_tmp, "jetEtamax/F");
  configtree->Branch("jetIDmin", &jetIDmin_tmp, "jetIDmin/I");
  configtree->Branch("rhEmin", &rhEmin_tmp, "rhEmin/F");
  configtree->Branch("phpTmin", &phpTmin_tmp, "phpTmin/F");
  configtree->Branch("phIDmin", &phIDmin_tmp);

  // object extra pruning
  float seedTimemin_tmp = seedTimemin;
  configtree->Branch("seedTimemin", &seedTimemin_tmp, "seedTimemin/F");

  // photon storing options
  bool splitPho_tmp = splitPho;
  bool onlyGED_tmp = onlyGED;
  bool onlyOOT_tmp = onlyOOT;
  configtree->Branch("splitPho", &splitPho_tmp, "splitPho/O");
  configtree->Branch("onlyGED", &onlyGED_tmp, "onlyGED/O");
  configtree->Branch("onlyOOT", &onlyOOT_tmp, "onlyOOT/O");

  // rec hit storing options
  bool storeRecHits_tmp = storeRecHits;
  configtree->Branch("storeRecHits", &storeRecHits_tmp, "storeRecHits/O");

  // pre-selection vars
  bool applyTrigger_tmp = applyTrigger;
  float minHT_tmp = minHT;
  bool applyHT_tmp = applyHT;
  float phgoodpTmin_tmp = phgoodpTmin;
  std::string phgoodIDmin_tmp = phgoodIDmin;
  bool applyPhGood_tmp = applyPhGood;
  configtree->Branch("applyTrigger", &applyTrigger_tmp, "applyTrigger/O");
  configtree->Branch("minHT", &minHT_tmp, "minHT/F");
  configtree->Branch("applyHT", &applyHT_tmp, "applyHT/O");
  configtree->Branch("phgoodpTmin", &phgoodpTmin_tmp, "phgoodpTmin/F");
  configtree->Branch("phgoodIDmin", &phgoodIDmin_tmp);
  configtree->Branch("applyPhGood", &applyPhGood_tmp, "applyPhGood/O");

  // dR matching criteria
  float dRmin_tmp = dRmin;
  float pTres_tmp = pTres;
  float gendRmin_tmp = gendRmin;
  float genpTres_tmp = genpTres;
  float trackdRmin_tmp = trackdRmin;
  float trackpTmin_tmp = trackpTmin;
  float genjetdRmin_tmp = genjetdRmin;
  float genjetpTfactor_tmp = genjetpTfactor;
  configtree->Branch("dRmin", &dRmin_tmp, "dRmin/F");
  configtree->Branch("pTres", &pTres_tmp, "pTres/F");
  configtree->Branch("gendRmin", &gendRmin_tmp, "gendRmin/F");
  configtree->Branch("genpTres", &genpTres_tmp, "genpTres/F");
  configtree->Branch("trackdRmin", &trackdRmin_tmp, "trackdRmin/F");
  configtree->Branch("trackpTmin", &trackpTmin_tmp, "trackpTmin/F");
  configtree->Branch("genjetdRmin", &genjetdRmin_tmp, "genjetdRmin/F");
  configtree->Branch("genjetpTfactor", &genjetpTfactor_tmp, "genjetpTfactor/F");

  // JER extra info
  float smearjetEmin_tmp = smearjetEmin;
  configtree->Branch("smearjetEmin", &smearjetEmin_tmp, "smearjetEmin/F");

  // trigger info
  std::string inputPaths_tmp = inputPaths;
  std::string inputFilters_tmp = inputFilters;
  configtree->Branch("inputPaths", &inputPaths_tmp);
  configtree->Branch("inputFilters", &inputFilters_tmp);

  // met flag info
  std::string inputFlags_tmp = inputFlags;
  configtree->Branch("inputFlags", &inputFlags_tmp);

  // MC info
  bool isGMSB_tmp = isGMSB;
  bool isHVDS_tmp = isHVDS;
  bool isBkgd_tmp = isBkgd;
  bool isToy_tmp  = isToy;
  bool isADD_tmp  = isADD;
  float xsec_tmp = xsec;
  float filterEff_tmp = filterEff;
  float BR_tmp = BR;
  configtree->Branch("isGMSB", &isGMSB_tmp, "isGMSB/O");
  configtree->Branch("isHVDS", &isHVDS_tmp, "isHVDS/O");
  configtree->Branch("isBkgd", &isBkgd_tmp, "isBkgd/O");
  configtree->Branch("isToy" , &isToy_tmp , "isToy/O");
  configtree->Branch("isADD" , &isADD_tmp , "isADD/O");
  configtree->Branch("xsec", &xsec_tmp, "xsec/F");
  configtree->Branch("filterEff", &filterEff_tmp, "filterEff/F");
  configtree->Branch("BR", &BR_tmp, "BR/F");

  // Fill tree just once, after configs have been read in
  configtree->Fill();
}

void DisPho::MakeEventTree()
{
  // rho info
  disphotree->Branch("rho", &rho, "rho/F");

  // Generic MC Info
  if (isMC)
  {
    disphotree->Branch("genwgt", &genwgt, "genwgt/F");
    disphotree->Branch("genx0", &genx0, "genx0/F");
    disphotree->Branch("geny0", &geny0, "geny0/F");
    disphotree->Branch("genz0", &genz0, "genz0/F");
    disphotree->Branch("gent0", &gent0, "gent0/F");
    disphotree->Branch("genpuobs", &genpuobs, "genpuobs/I");
    disphotree->Branch("genputrue", &genputrue, "genputrue/I");
  }

  // GMSB Info
  if (isGMSB)
  {
    disphotree->Branch("nNeutoPhGr", &nNeutoPhGr, "nNeutoPhGr/I");

    gmsbBranches.resize(Config::nGMSBs);
    for (auto igmsb = 0; igmsb < Config::nGMSBs; igmsb++)
    {
      auto & gmsbBranch = gmsbBranches[igmsb];

      disphotree->Branch(Form("genNmass_%i",igmsb), &gmsbBranch.genNmass_, Form("genNmass_%i/F",igmsb));
      disphotree->Branch(Form("genNE_%i",igmsb), &gmsbBranch.genNE_, Form("genNE_%i/F",igmsb));
      disphotree->Branch(Form("genNpt_%i",igmsb), &gmsbBranch.genNpt_, Form("genNpt_%i/F",igmsb));
      disphotree->Branch(Form("genNphi_%i",igmsb), &gmsbBranch.genNphi_, Form("genNphi_%i/F",igmsb));
      disphotree->Branch(Form("genNeta_%i",igmsb), &gmsbBranch.genNeta_, Form("genNeta_%i/F",igmsb));
      
      disphotree->Branch(Form("genNprodvx_%i",igmsb), &gmsbBranch.genNprodvx_, Form("genNprodvx_%i/F",igmsb));
      disphotree->Branch(Form("genNprodvy_%i",igmsb), &gmsbBranch.genNprodvy_, Form("genNprodvy_%i/F",igmsb));
      disphotree->Branch(Form("genNprodvz_%i",igmsb), &gmsbBranch.genNprodvz_, Form("genNprodvz_%i/F",igmsb));
      
      disphotree->Branch(Form("genNdecayvx_%i",igmsb), &gmsbBranch.genNdecayvx_, Form("genNdecayvx_%i/F",igmsb));
      disphotree->Branch(Form("genNdecayvy_%i",igmsb), &gmsbBranch.genNdecayvy_, Form("genNdecayvy_%i/F",igmsb));
      disphotree->Branch(Form("genNdecayvz_%i",igmsb), &gmsbBranch.genNdecayvz_, Form("genNdecayvz_%i/F",igmsb));

      disphotree->Branch(Form("genphE_%i",igmsb), &gmsbBranch.genphE_, Form("genphE_%i/F",igmsb));
      disphotree->Branch(Form("genphpt_%i",igmsb), &gmsbBranch.genphpt_, Form("genphpt_%i/F",igmsb));
      disphotree->Branch(Form("genphphi_%i",igmsb), &gmsbBranch.genphphi_, Form("genphphi_%i/F",igmsb));
      disphotree->Branch(Form("genpheta_%i",igmsb), &gmsbBranch.genpheta_, Form("genpheta_%i/F",igmsb));
      disphotree->Branch(Form("genphmatch_%i",igmsb), &gmsbBranch.genphmatch_, Form("genphmatch_%i/I",igmsb));
      
      disphotree->Branch(Form("gengrmass_%i",igmsb), &gmsbBranch.gengrmass_, Form("gengrmass_%i/F",igmsb));
      disphotree->Branch(Form("gengrE_%i",igmsb), &gmsbBranch.gengrE_, Form("gengrE_%i/F",igmsb));
      disphotree->Branch(Form("gengrpt_%i",igmsb), &gmsbBranch.gengrpt_, Form("gengrpt_%i/F",igmsb));
      disphotree->Branch(Form("gengrphi_%i",igmsb), &gmsbBranch.gengrphi_, Form("gengrphi_%i/F",igmsb));
      disphotree->Branch(Form("gengreta_%i",igmsb), &gmsbBranch.gengreta_, Form("gengreta_%i/F",igmsb));
    } // end loop over nGBMBs
  } // end block over isGMSB

  // HVDS Info
  if (isHVDS)
  {
    disphotree->Branch("nvPions", &nvPions, "nvPions/I");

    hvdsBranches.resize(Config::nHVDSs);
    for (auto ihvds = 0; ihvds < Config::nHVDSs; ihvds++)
    {
      auto & hvdsBranch = hvdsBranches[ihvds];

      disphotree->Branch(Form("genvPionmass_%i",ihvds), &hvdsBranch.genvPionmass_, Form("genvPionmass_%i/F",ihvds));
      disphotree->Branch(Form("genvPionE_%i",ihvds), &hvdsBranch.genvPionE_, Form("genvPionE_%i/F",ihvds));
      disphotree->Branch(Form("genvPionpt_%i",ihvds), &hvdsBranch.genvPionpt_, Form("genvPionpt_%i/F",ihvds));
      disphotree->Branch(Form("genvPionphi_%i",ihvds), &hvdsBranch.genvPionphi_, Form("genvPionphi_%i/F",ihvds));
      disphotree->Branch(Form("genvPioneta_%i",ihvds), &hvdsBranch.genvPioneta_, Form("genvPioneta_%i/F",ihvds));
      
      disphotree->Branch(Form("genvPionprodvx_%i",ihvds), &hvdsBranch.genvPionprodvx_, Form("genvPionprodvx_%i/F",ihvds));
      disphotree->Branch(Form("genvPionprodvy_%i",ihvds), &hvdsBranch.genvPionprodvy_, Form("genvPionprodvy_%i/F",ihvds));
      disphotree->Branch(Form("genvPionprodvz_%i",ihvds), &hvdsBranch.genvPionprodvz_, Form("genvPionprodvz_%i/F",ihvds));
      
      disphotree->Branch(Form("genvPiondecayvx_%i",ihvds), &hvdsBranch.genvPiondecayvx_, Form("genvPiondecayvx_%i/F",ihvds));
      disphotree->Branch(Form("genvPiondecayvy_%i",ihvds), &hvdsBranch.genvPiondecayvy_, Form("genvPiondecayvy_%i/F",ihvds));
      disphotree->Branch(Form("genvPiondecayvz_%i",ihvds), &hvdsBranch.genvPiondecayvz_, Form("genvPiondecayvz_%i/F",ihvds));
      
      disphotree->Branch(Form("genHVph0E_%i",ihvds), &hvdsBranch.genHVph0E_, Form("genHVph0E_%i/F",ihvds));
      disphotree->Branch(Form("genHVph0pt_%i",ihvds), &hvdsBranch.genHVph0pt_, Form("genHVph0pt_%i/F",ihvds));
      disphotree->Branch(Form("genHVph0phi_%i",ihvds), &hvdsBranch.genHVph0phi_, Form("genHVph0phi_%i/F",ihvds));
      disphotree->Branch(Form("genHVph0eta_%i",ihvds), &hvdsBranch.genHVph0eta_, Form("genHVph0eta_%i/F",ihvds));
      disphotree->Branch(Form("genHVph0match_%i",ihvds), &hvdsBranch.genHVph0match_, Form("genHVph0match_%i/I",ihvds));
      
      disphotree->Branch(Form("genHVph1E_%i",ihvds), &hvdsBranch.genHVph1E_, Form("genHVph1E_%i/F",ihvds));
      disphotree->Branch(Form("genHVph1pt_%i",ihvds), &hvdsBranch.genHVph1pt_, Form("genHVph1pt_%i/F",ihvds));
      disphotree->Branch(Form("genHVph1phi_%i",ihvds), &hvdsBranch.genHVph1phi_, Form("genHVph1phi_%i/F",ihvds));
      disphotree->Branch(Form("genHVph1eta_%i",ihvds), &hvdsBranch.genHVph1eta_, Form("genHVph1eta_%i/F",ihvds));
      disphotree->Branch(Form("genHVph1match_%i",ihvds), &hvdsBranch.genHVph1match_, Form("genHVph1match_%i/I",ihvds));
    } // end loop over nHVDSs
  } // end block over HVDS

  // ToyMC Info
  if (isToy)
  {
    disphotree->Branch("nToyPhs", &nToyPhs, "nToyPhs/I");

    toyBranches.resize(Config::nToys);
    for (auto itoy = 0; itoy < Config::nToys; itoy++)
    {
      auto & toyBranch = toyBranches[itoy];

      disphotree->Branch(Form("genphE_%i",itoy), &toyBranch.genphE_, Form("genphE_%i/F",itoy));
      disphotree->Branch(Form("genphpt_%i",itoy), &toyBranch.genphpt_, Form("genphpt_%i/F",itoy));
      disphotree->Branch(Form("genphphi_%i",itoy), &toyBranch.genphphi_, Form("genphphi_%i/F",itoy));
      disphotree->Branch(Form("genpheta_%i",itoy), &toyBranch.genpheta_, Form("genpheta_%i/F",itoy));
      
      disphotree->Branch(Form("genphmatch_%i",itoy), &toyBranch.genphmatch_, Form("genphmatch_%i/I",itoy));
      disphotree->Branch(Form("genphmatch_ptres_%i",itoy), &toyBranch.genphmatch_ptres_, Form("genphmatch_ptres_%i/I",itoy));
      disphotree->Branch(Form("genphmatch_status_%i",itoy), &toyBranch.genphmatch_status_, Form("genphmatch_status_%i/I",itoy));
    } // end loop over nToys
  } // end block over ToyMC

  // Run, Lumi, Event info
  disphotree->Branch("run", &run, "run/i");
  disphotree->Branch("lumi", &lumi, "lumi/i");
  disphotree->Branch("event", &event, "event/l");
   
  // Trigger Info
  disphotree->Branch("hltSignal", &hltSignal, "hltSignal/O");
  disphotree->Branch("hltRefPhoID", &hltRefPhoID, "hltRefPhoID/O");
  disphotree->Branch("hltRefDispID", &hltRefDispID, "hltRefDispID/O");
  disphotree->Branch("hltRefHT", &hltRefHT, "hltRefHT/O");
  disphotree->Branch("hltPho50", &hltPho50, "hltPho50/O");
  disphotree->Branch("hltPho200", &hltPho200, "hltPho200/O");
  disphotree->Branch("hltDiPho70", &hltDiPho70, "hltDiPho70/O");
  disphotree->Branch("hltDiPho3022M90", &hltDiPho3022M90, "hltDiPho3022M90/O");
  disphotree->Branch("hltDiPho30PV18PV", &hltDiPho30PV18PV, "hltDiPho30PV18PV/O");
  disphotree->Branch("hltEle32WPT", &hltEle32WPT, "hltEle32WPT/O");
  disphotree->Branch("hltDiEle33MW", &hltDiEle33MW, "hltDiEle33MW/O");
  disphotree->Branch("hltJet500", &hltJet500, "hltJet500/O");

  // MET Filter Info
  disphotree->Branch("metPV", &metPV, "metPV/O");
  disphotree->Branch("metBeamHalo", &metBeamHalo, "metBeamHalo/O");
  disphotree->Branch("metHBHENoise", &metHBHENoise, "metHBHENoise/O");
  disphotree->Branch("metHBHEisoNoise", &metHBHEisoNoise, "metHBHEisoNoise/O");
  disphotree->Branch("metECALTP", &metECALTP, "metECALTP/O");
  disphotree->Branch("metPFMuon", &metPFMuon, "metPFMuon/O");
  disphotree->Branch("metPFChgHad", &metPFChgHad, "metPFChgHad/O");
  disphotree->Branch("metEESC", &metEESC, "metEESC/O");
  disphotree->Branch("metECALCalib", &metECALCalib, "metECALCalib/O");

  // Vertex info
  disphotree->Branch("nvtx", &nvtx, "nvtx/I");
  disphotree->Branch("vtxX", &vtxX, "vtxX/F");
  disphotree->Branch("vtxY", &vtxY, "vtxY/F");
  disphotree->Branch("vtxZ", &vtxZ, "vtxZ/F");
  
  // MET info
  disphotree->Branch("t1pfMETpt", &t1pfMETpt, "t1pfMETpt/F");
  disphotree->Branch("t1pfMETphi", &t1pfMETphi, "t1pfMETphi/F");
  disphotree->Branch("t1pfMETsumEt", &t1pfMETsumEt, "t1pfMETsumEt/F");

  // Jet info
  disphotree->Branch("njets", &njets, "njets/I");
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

  // RecHit Info
  disphotree->Branch("nrechits", &nrechits, "nrechits/I");
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
  disphotree->Branch("nphotons", &nphotons, "nphotons/I");

  phoBranches.resize(Config::nPhotons);
  for (auto iphoton = 0; iphoton < Config::nPhotons; iphoton++)
  {
    auto & phoBranch = phoBranches[iphoton];

    disphotree->Branch(Form("phoE_%i",iphoton), &phoBranch.E_, Form("phoE_%i/F",iphoton));
    disphotree->Branch(Form("phopt_%i",iphoton), &phoBranch.pt_, Form("phopt_%i/F",iphoton));
    disphotree->Branch(Form("phoeta_%i",iphoton), &phoBranch.eta_, Form("phoeta_%i/F",iphoton));
    disphotree->Branch(Form("phophi_%i",iphoton), &phoBranch.phi_, Form("phophi_%i/F",iphoton));

    disphotree->Branch(Form("phoscE_%i",iphoton), &phoBranch.scE_, Form("phoscE_%i/F",iphoton));
    disphotree->Branch(Form("phosceta_%i",iphoton), &phoBranch.sceta_, Form("phosceta_%i/F",iphoton));
    disphotree->Branch(Form("phoscphi_%i",iphoton), &phoBranch.scphi_, Form("phoscphi_%i/F",iphoton));

    disphotree->Branch(Form("phoHoE_%i",iphoton), &phoBranch.HoE_, Form("phoHoE_%i/F",iphoton));
    disphotree->Branch(Form("phor9_%i",iphoton), &phoBranch.r9_, Form("phor9_%i/F",iphoton));

    disphotree->Branch(Form("phoChgHadIso_%i",iphoton), &phoBranch.ChgHadIso_, Form("phoChgHadIso_%i/F",iphoton));
    disphotree->Branch(Form("phoNeuHadIso_%i",iphoton), &phoBranch.NeuHadIso_, Form("phoNeuHadIso_%i/F",iphoton));
    disphotree->Branch(Form("phoPhoIso_%i",iphoton), &phoBranch.PhoIso_, Form("phoPhoIso_%i/F",iphoton));

    disphotree->Branch(Form("phoEcalPFClIso_%i",iphoton), &phoBranch.EcalPFClIso_, Form("phoEcalPFClIso_%i/F",iphoton));
    disphotree->Branch(Form("phoHcalPFClIso_%i",iphoton), &phoBranch.HcalPFClIso_, Form("phoHcalPFClIso_%i/F",iphoton));
    disphotree->Branch(Form("phoTrkIso_%i",iphoton), &phoBranch.TrkIso_, Form("phoTrkIso_%i/F",iphoton));

    disphotree->Branch(Form("phoChgHadIsoC_%i",iphoton), &phoBranch.ChgHadIsoC_, Form("phoChgHadIsoC_%i/F",iphoton));
    disphotree->Branch(Form("phoNeuHadIsoC_%i",iphoton), &phoBranch.NeuHadIsoC_, Form("phoNeuHadIsoC_%i/F",iphoton));
    disphotree->Branch(Form("phoPhoIsoC_%i",iphoton), &phoBranch.PhoIsoC_, Form("phoPhoIsoC_%i/F",iphoton));

    disphotree->Branch(Form("phoEcalPFClIsoC_%i",iphoton), &phoBranch.EcalPFClIsoC_, Form("phoEcalPFClIsoC_%i/F",iphoton));
    disphotree->Branch(Form("phoHcalPFClIsoC_%i",iphoton), &phoBranch.HcalPFClIsoC_, Form("phoHcalPFClIsoC_%i/F",iphoton));
    disphotree->Branch(Form("phoTrkIsoC_%i",iphoton), &phoBranch.TrkIsoC_, Form("phoTrkIsoC_%i/F",iphoton));

    disphotree->Branch(Form("phosieie_%i",iphoton), &phoBranch.sieie_, Form("phosieie_%i/F",iphoton));
    disphotree->Branch(Form("phosipip_%i",iphoton), &phoBranch.sipip_, Form("phosipip_%i/F",iphoton));
    disphotree->Branch(Form("phosieip_%i",iphoton), &phoBranch.sieip_, Form("phosieip_%i/F",iphoton));

    disphotree->Branch(Form("phoe2x2_%i",iphoton), &phoBranch.e2x2_, Form("phoe2x2_%i/F",iphoton));
    disphotree->Branch(Form("phoe3x3_%i",iphoton), &phoBranch.e3x3_, Form("phoe3x3_%i/F",iphoton));
    disphotree->Branch(Form("phoe5x5_%i",iphoton), &phoBranch.e5x5_, Form("phoe5x5_%i/F",iphoton));

    disphotree->Branch(Form("phosmaj_%i",iphoton), &phoBranch.smaj_, Form("phosmaj_%i/F",iphoton));
    disphotree->Branch(Form("phosmin_%i",iphoton), &phoBranch.smin_, Form("phosmin_%i/F",iphoton));
    disphotree->Branch(Form("phoalpha_%i",iphoton), &phoBranch.alpha_, Form("phoalpha_%i/F",iphoton));

    if (storeRecHits)
    {
      disphotree->Branch(Form("phoseed_%i",iphoton), &phoBranch.seed_, Form("phoseed_%i/I",iphoton));
      disphotree->Branch(Form("phorecHits_%i",iphoton), &phoBranch.recHits_);
    }
    else 
    {
      disphotree->Branch(Form("phoseedX_%i",iphoton), &phoBranch.seedX_, Form("phoseedX_%i/F",iphoton));
      disphotree->Branch(Form("phoseedY_%i",iphoton), &phoBranch.seedY_, Form("phoseedY_%i/F",iphoton));
      disphotree->Branch(Form("phoseedZ_%i",iphoton), &phoBranch.seedZ_, Form("phoseedZ_%i/F",iphoton));
      disphotree->Branch(Form("phoseedE_%i",iphoton), &phoBranch.seedE_, Form("phoseedE_%i/F",iphoton));
      disphotree->Branch(Form("phoseedtime_%i",iphoton), &phoBranch.seedtime_, Form("phoseedtime_%i/F",iphoton));
      disphotree->Branch(Form("phoseedtimeErr_%i",iphoton), &phoBranch.seedtimeErr_, Form("phoseedtimeErr_%i/F",iphoton));
      disphotree->Branch(Form("phoseedTOF_%i",iphoton), &phoBranch.seedTOF_, Form("phoseedTOF_%i/F",iphoton));
      disphotree->Branch(Form("phoseedID_%i",iphoton), &phoBranch.seedID_, Form("phoseedID_%i/i",iphoton));
      disphotree->Branch(Form("phoseedisOOT_%i",iphoton), &phoBranch.seedisOOT_, Form("phoseedisOOT_%i/i",iphoton));
      disphotree->Branch(Form("phoseedisGS6_%i",iphoton), &phoBranch.seedisGS6_, Form("phoseedisGS6_%i/i",iphoton));
      disphotree->Branch(Form("phoseedisGS1_%i",iphoton), &phoBranch.seedisGS1_, Form("phoseedisGS1_%i/i",iphoton));
      disphotree->Branch(Form("phoseedadcToGeV_%i",iphoton), &phoBranch.seedadcToGeV_, Form("phoseedadcToGeV_%i/i",iphoton));
      disphotree->Branch(Form("phoseedped12_%i",iphoton), &phoBranch.seedped12_, Form("phoseedped12_%i/i",iphoton));
      disphotree->Branch(Form("phoseedped6_%i",iphoton), &phoBranch.seedped6_, Form("phoseedped6_%i/i",iphoton));
      disphotree->Branch(Form("phoseedped1_%i",iphoton), &phoBranch.seedped1_, Form("phoseedped1_%i/i",iphoton));
      disphotree->Branch(Form("phoseedpedrms12_%i",iphoton), &phoBranch.seedpedrms12_, Form("phoseedpedrms12_%i/i",iphoton));
      disphotree->Branch(Form("phoseedpedrms6_%i",iphoton), &phoBranch.seedpedrms6_, Form("phoseedpedrms6_%i/i",iphoton));
      disphotree->Branch(Form("phoseedpedrms1_%i",iphoton), &phoBranch.seedpedrms1_, Form("phoseedpedrms1_%i/i",iphoton));
    }

    disphotree->Branch(Form("phosuisseX_%i",iphoton), &phoBranch.suisseX_, Form("phosuisseX_%i/F",iphoton));

    disphotree->Branch(Form("phoisOOT_%i",iphoton), &phoBranch.isOOT_, Form("phoisOOT_%i/O",iphoton));
    disphotree->Branch(Form("phoisEB_%i",iphoton), &phoBranch.isEB_, Form("phoisEB_%i/O",iphoton));
    disphotree->Branch(Form("phoisHLT_%i",iphoton), &phoBranch.isHLT_, Form("phoisHLT_%i/O",iphoton));
    disphotree->Branch(Form("phoisTrk_%i",iphoton), &phoBranch.isTrk_, Form("phoisTrk_%i/O",iphoton));
    disphotree->Branch(Form("phopassEleVeto_%i",iphoton), &phoBranch.passEleVeto_, Form("phopassEleVeto_%i/O",iphoton));
    disphotree->Branch(Form("phohasPixSeed_%i",iphoton), &phoBranch.hasPixSeed_, Form("phohasPixSeed_%i/O",iphoton));
    disphotree->Branch(Form("phogedID_%i",iphoton), &phoBranch.gedID_, Form("phogedID_%i/I",iphoton));
    disphotree->Branch(Form("phoootID_%i",iphoton), &phoBranch.ootID_, Form("phoootID_%i/I",iphoton));

    if (isMC)
    {
      if (isGMSB || isHVDS) disphotree->Branch(Form("phoisSignal_%i",iphoton), &phoBranch.isSignal_, Form("phoisSignal_%i/I",iphoton));
      disphotree->Branch(Form("phoisGen_%i",iphoton), &phoBranch.isGen_, Form("phoisGen_%i/O",iphoton));

      disphotree->Branch(Form("phoscaleAbs_%i",iphoton), &phoBranch.scaleAbs_, Form("phoscaleAbs_%i/F",iphoton));
      disphotree->Branch(Form("phosmearAbs_%i",iphoton), &phoBranch.smearAbs_, Form("phosmearAbs_%i/F",iphoton));
    } // end block over isMC
  } // end loop over nPhotons
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
