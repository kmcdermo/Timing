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
  jetIDStoremin(iConfig.existsAs<int>("jetIDStoremin") ? iConfig.getParameter<int>("jetIDStoremin") : 3),

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
  genpTres(iConfig.existsAs<double>("genpTres") ? iConfig.getParameter<double>("genpTres") : 0.5),
  trackdRmin(iConfig.existsAs<double>("trackdRmin") ? iConfig.getParameter<double>("trackdRmin") : 0.2),
  trackpTmin(iConfig.existsAs<double>("trackpTmin") ? iConfig.getParameter<double>("trackpTmin") : 5.f),

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
  if (isGMSB || isHVDS || isBkgd || isToy)
  {
    isMC = true;
    genevtInfoToken = consumes<GenEventInfoProduct>             (iConfig.getParameter<edm::InputTag>("genevt"));
    gent0Token      = consumes<float>                           (iConfig.getParameter<edm::InputTag>("gent0"));
    genxyz0Token    = consumes<Point3D>                         (iConfig.getParameter<edm::InputTag>("genxyz0"));
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
  rho = rhosH.isValid() ? *(rhosH.product()) : 0.f;

  // MET
  edm::Handle<std::vector<pat::MET> > metsH;
  iEvent.getByToken(metsToken, metsH);

  // JETS
  edm::Handle<std::vector<pat::Jet> > jetsH;
  iEvent.getByToken(jetsToken, jetsH);
  std::vector<pat::Jet> jets; jets.reserve(jetsH->size());

  // RecHits
  edm::Handle<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > recHitsEBH;
  iEvent.getByToken(recHitsEBToken, recHitsEBH);
  const EcalRecHitCollection * recHitsEB = recHitsEBH.product();
  edm::Handle<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > recHitsEEH;
  iEvent.getByToken(recHitsEEToken, recHitsEEH);
  const EcalRecHitCollection * recHitsEE = recHitsEEH.product();
  uiiumap recHitMap;

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

  // geometry (from ECAL ELF)
  edm::ESHandle<CaloGeometry> calogeoH;
  iSetup.get<CaloGeometryRecord>().get(calogeoH);
  const CaloSubdetectorGeometry * barrelGeometry = calogeoH->getSubdetectorGeometry(DetId::Ecal, EcalBarrel);
  const CaloSubdetectorGeometry * endcapGeometry = calogeoH->getSubdetectorGeometry(DetId::Ecal, EcalEndcap);

  // GEN INFO
  edm::Handle<GenEventInfoProduct> genevtInfoH;
  edm::Handle<float>   gent0H;
  edm::Handle<Point3D> genxyz0H;
  edm::Handle<std::vector<PileupSummaryInfo> > pileupInfoH;
  edm::Handle<std::vector<reco::GenParticle> > genparticlesH;
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

    ///////////////////////
    //                   //
    // Event weight info //
    //                   //
    ///////////////////////
    DisPho::InitializeGenEvtBranches();
    if (genevtInfoH.isValid()) {genwgt = genevtInfoH->weight();}
  }
  const Float_t evtwgt = (isMC ? genwgt : 1.f);

  // Fill total cutflow regardless of cuts
  h_cutflow->Fill(cutflowLabelMap["All"]*1.f,evtwgt);

  ///////////////////////////
  //                       //
  // Event, lumi, run info //
  //                       //
  ///////////////////////////
  run   = iEvent.id().run();
  lumi  = iEvent.luminosityBlock();
  event = iEvent.id().event();

  //////////////
  //          //
  // Blinding //
  //          //
  //////////////
  if (event%blindSF!=0 && applyBlindSF) return;
  h_cutflow->Fill(cutflowLabelMap["nEvBlinding"]*1.f,evtwgt);

  if (metsH.isValid())
  {
    const pat::MET & t1pfMET = (*metsH)[0];
    if (t1pfMET.pt() > blindMET && applyBlindMET) return;
  }  
  h_cutflow->Fill(cutflowLabelMap["METBlinding"]*1.f,evtwgt);

  /////////////////////
  //                 //
  // Object Prepping //
  //                 //
  /////////////////////
  if (isGMSB) oot::PrepNeutralinos(genparticlesH,neutralinos);
  if (isHVDS) oot::PrepVPions(genparticlesH,vPions);
  if (isToy) oot::PrepToys(genparticlesH,toys);
  oot::PrepTriggerBits(triggerResultsH,iEvent,triggerBitMap);
  oot::PrepTriggerBits(triggerFlagsH,iEvent,triggerFlagMap);
  oot::PrepTriggerObjects(triggerResultsH,triggerObjectsH,iEvent,triggerObjectsByFilterMap);
  //  oot::PrepJets(jetsH,jets,jetpTmin,jetIDmin,jetEtamax);
  oot::PrepJets(jetsH,jets); // FIXME -- HACK!!!
  oot::PrepRecHits(recHitsEB,recHitsEE,recHitMap,rhEmin);
  oot::PrepPhotons(photonsH,ootPhotonsH,photons,rho,phpTmin,phIDmin);

  ///////////////////
  //               //
  // Extra Pruning //
  //               //
  ///////////////////

  /////////////////////////
  // No jet requirements //
  /////////////////////////
  njets = jets.size();
  jetHT = 0.f;
  for (const auto jet : jets) jetHT += jet.pt();


  ////////////////
  // Jet pt min //
  ////////////////
  jets.erase(std::remove_if(jets.begin(),jets.end(),
			    [&](const pat::Jet & jet)
			    {
			      return jet.pt() < jetpTmin;
			    }),jets.end());
  njetspt15 = jets.size();
  jetHTpt15 = 0.f;
  for (const auto jet : jets) jetHTpt15 += jet.pt();

  /////////////////
  // Jet eta max //
  /////////////////
  jets.erase(std::remove_if(jets.begin(),jets.end(),
			    [&](const pat::Jet & jet)
			    {
			      return std::abs(jet.eta()) > jetEtamax;
			    }),jets.end());
  njetseta3 = jets.size();
  jetHTeta3 = 0.f;
  for (const auto jet : jets) jetHTeta3 += jet.pt();

  ///////////////////////////
  // Photon,Jet DeltaR min //
  ///////////////////////////
  jets.erase(std::remove_if(jets.begin(),jets.end(),
			    [&](const pat::Jet & jet)
			    {
			      bool isNearPhoton = false;
			      for (const auto & photon : photons)
			      {
				if (Config::deltaR(jet.phi(),jet.eta(),photon.phi(),photon.eta()) < dRmin)
				{
				  isNearPhoton = true;
				  break;
				}
			      }
			      return isNearPhoton;
			    }),jets.end());
  njetsnopho = jets.size();
  jetHTnopho = 0.f;
  for (const auto jet : jets) jetHTnopho += jet.pt();

  ///////////////////////////////
  // Remove early time photons //
  ///////////////////////////////
  photons.erase(std::remove_if(photons.begin(),photons.end(),
			       [&](const oot::Photon & photon)
			       {
				 const pat::Photon & pho = photon.photon();
				 const reco::SuperClusterRef& phosc = pho.superCluster().isNonnull() ? pho.superCluster() : pho.parentSuperCluster();
				 const DetId & seedDetId = phosc->seed()->seed(); // get seed detid
				 const EcalRecHitCollection * recHits = ((seedDetId.subdetId() == EcalBarrel) ? recHitsEB : recHitsEE); // which recHits to use
				 EcalRecHitCollection::const_iterator seedHit = recHits->find(seedDetId); // get the underlying rechit
				 const float seedTime = ((seedHit != recHits->end()) ? seedHit->time() : -9999.f);
				 return (seedTime < seedTimemin);
			       }),photons.end());
				 
  ///////////////////////////// 
  //                         //
  // Photon Storing Options  //
  //                         //
  /////////////////////////////
  if (splitPho) oot::SplitPhotons(photons,2);       // split photons by OOT and GED (store at most two of each)
  if (onlyGED)  oot::StoreOnlyPho(photons,4,false); // store only GED photons, top four only
  if (onlyOOT)  oot::StoreOnlyPho(photons,4,true);  // store only OOT photons, top four only

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
  h_cutflow->Fill(cutflowLabelMap["Trigger"]*1.f,evtwgt);

  // HT pre-selection
  if (jetHT < minHT && applyHT) return;
  h_cutflow->Fill(cutflowLabelMap["H_{T}"]*1.f,evtwgt);

  // photon pre-selection: at least one good photon in event
  bool isphgood = false;
  int  iph = 0;
  if (photonsH.isValid() || ootPhotonsH.isValid()) 
  {
    for (const auto & photon : photons)
    {
      const auto & pho = photon.photon();

      if (iph >= 4) break;
      iph++;
      
      if (pho.pt() < phgoodpTmin) continue;
      if (phgoodIDmin != "none")
      {
	if (!photon.isOOT() && !pho.photonID(phgoodIDmin+"-ged")) continue;
	if ( photon.isOOT() && !pho.photonID(phgoodIDmin+"-oot")) continue;
      }
      
      isphgood = true; break;
    } 
  } // end check
  if (!isphgood && applyPhGood) return;
  h_cutflow->Fill(cutflowLabelMap["Good Photon"]*1.f,evtwgt);

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
      const Point3D xyz0 = *(genxyz0H.product());
      genx0 = xyz0.X();
      geny0 = xyz0.Y();
      genz0 = xyz0.Z();
    }
    if (gent0H.isValid()) // standard check on t0
    {
      gent0 = *(gent0H.product());
    }
    
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

    // ToyMC
    if (isToy) 
    {
      DisPho::InitializeToyBranches();
      if (genparticlesH.isValid())
      {
	nToyPhs = toys.size();
	if (nToyPhs > 0) DisPho::SetToyBranch(toys[0],toyBranch0,photons);
	if (nToyPhs > 1) DisPho::SetToyBranch(toys[1],toyBranch1,photons);
      }
    } // isHVDS
  } // isMC

  //////////////////
  //              //
  // Trigger Info //
  //              //
  //////////////////

  hltSignal = (triggerBitMap.count(Config::SignalPath.c_str()) ? triggerBitMap[Config::SignalPath.c_str()] : false);
  hltRefPhoID = (triggerBitMap.count(Config::RefPhoIDPath.c_str()) ? triggerBitMap[Config::RefPhoIDPath.c_str()] : false);
  hltRefDispID = (triggerBitMap.count(Config::RefDispIDPath.c_str()) ? triggerBitMap[Config::RefDispIDPath.c_str()] : false);
  hltRefHT = (triggerBitMap.count(Config::RefHTPath.c_str()) ? triggerBitMap[Config::RefHTPath.c_str()] : false);
  hltPho50 = (triggerBitMap.count(Config::Pho50Path.c_str()) ? triggerBitMap[Config::Pho50Path.c_str()] : false);
  hltPho200 = (triggerBitMap.count(Config::Pho200Path.c_str()) ? triggerBitMap[Config::Pho200Path.c_str()] : false);
  hltDiPho70 = (triggerBitMap.count(Config::DiPho70Path.c_str()) ? triggerBitMap[Config::DiPho70Path.c_str()] : false);
  hltDiPho3022M90 = (triggerBitMap.count(Config::DiPho3022M90Path.c_str()) ? triggerBitMap[Config::DiPho3022M90Path.c_str()] : false);
  hltDiPho30PV18PV = (triggerBitMap.count(Config::DiPho30PV18PVPath.c_str()) ? triggerBitMap[Config::DiPho30PV18PVPath.c_str()] : false);
  hltDiEle33MW = (triggerBitMap.count(Config::DiEle33MWPath.c_str()) ? triggerBitMap[Config::DiEle33MWPath.c_str()] : false);
  hltDiEle27WPT = (triggerBitMap.count(Config::DiEle27WPTPath.c_str()) ? triggerBitMap[Config::DiEle27WPTPath.c_str()] : false);
  hltJet500 = (triggerBitMap.count(Config::Jet500Path.c_str()) ? triggerBitMap[Config::Jet500Path.c_str()] : false);

  /////////////////////
  //                 //
  // MET Filter Info //
  //                 //
  /////////////////////

  metPV = (triggerFlagMap.count(Config::PVFlag.c_str()) ? triggerFlagMap[Config::PVFlag.c_str()] : false);
  metBeamHalo = (triggerFlagMap.count(Config::BeamHaloFlag.c_str()) ? triggerFlagMap[Config::BeamHaloFlag.c_str()] : false);
  metHBHENoise = (triggerFlagMap.count(Config::HBHENoiseFlag.c_str()) ? triggerFlagMap[Config::HBHENoiseFlag.c_str()] : false);
  metHBHEisoNoise = (triggerFlagMap.count(Config::HBHEisoNoiseFlag.c_str()) ? triggerFlagMap[Config::HBHEisoNoiseFlag.c_str()] : false);
  metECALTP = (triggerFlagMap.count(Config::ECALTPFlag.c_str()) ? triggerFlagMap[Config::ECALTPFlag.c_str()] : false);
  metPFMuon = (triggerFlagMap.count(Config::PFMuonFlag.c_str()) ? triggerFlagMap[Config::PFMuonFlag.c_str()] : false);
  metPFChgHad = (triggerFlagMap.count(Config::PFChgHadFlag.c_str()) ? triggerFlagMap[Config::PFChgHadFlag.c_str()] : false);
  metEESC = (triggerFlagMap.count(Config::EESCFlag.c_str()) ? triggerFlagMap[Config::EESCFlag.c_str()] : false);
  metECALCalib = (triggerFlagMap.count(Config::ECALCalibFlag.c_str()) ? triggerFlagMap[Config::ECALCalibFlag.c_str()] : false);

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

  ////////////////
  //            //
  // Loose Jets //
  //            //
  ////////////////

  jets.erase(std::remove_if(jets.begin(),jets.end(),
			    [&](const pat::Jet & jet)
			    {
			      return oot::GetPFJetID(jet) < 1; // 1 == loose
			    }),jets.end());
  njetsidL = jets.size();
  jetHTidL = 0.f;
  for (const auto jet : jets) jetHTidL += jet.pt();

  if (jetsH.isValid()) // check to make sure reco jets exist
  {
    if (njetsidL > 0) DisPho::SetJetBranch(jets[0],jetBranch0L);
    if (njetsidL > 1) DisPho::SetJetBranch(jets[1],jetBranch1L);
    if (njetsidL > 2) DisPho::SetJetBranch(jets[2],jetBranch2L);
    if (njetsidL > 3) DisPho::SetJetBranch(jets[3],jetBranch3L);
  }

  ////////////////
  //            //
  // Tight Jets //
  //            //
  ////////////////
  jets.erase(std::remove_if(jets.begin(),jets.end(),
			    [&](const pat::Jet & jet)
			    {
			      return oot::GetPFJetID(jet) < 2; // 2 == tight
			    }),jets.end());
  njetsidT = jets.size();
  jetHTidT = 0.f;
  for (const auto jet : jets) jetHTidT += jet.pt();

  if (jetsH.isValid()) // check to make sure reco jets exist
  {
    if (njetsidT > 0) DisPho::SetJetBranch(jets[0],jetBranch0T);
    if (njetsidT > 1) DisPho::SetJetBranch(jets[1],jetBranch1T);
    if (njetsidT > 2) DisPho::SetJetBranch(jets[2],jetBranch2T);
    if (njetsidT > 3) DisPho::SetJetBranch(jets[3],jetBranch3T);
  }

  /////////////////////////
  //                     //
  // Tight Lep Veto Jets //
  //                     //
  /////////////////////////
  jets.erase(std::remove_if(jets.begin(),jets.end(),
			    [&](const pat::Jet & jet)
			    {
			      return oot::GetPFJetID(jet) < 3; // 3 == tight lep veto
			    }),jets.end());
  njetsidTLV = jets.size();
  jetHTidTLV = 0.f;
  for (const auto jet : jets) jetHTidTLV += jet.pt();

  if (jetsH.isValid()) // check to make sure reco jets exist
  {
    if (njetsidTLV > 0) DisPho::SetJetBranch(jets[0],jetBranch0TLV);
    if (njetsidTLV > 1) DisPho::SetJetBranch(jets[1],jetBranch1TLV);
    if (njetsidTLV > 2) DisPho::SetJetBranch(jets[2],jetBranch2TLV);
    if (njetsidTLV > 3) DisPho::SetJetBranch(jets[3],jetBranch3TLV);
  }

  //////////////
  //          //
  // Rec Hits //
  //          //
  //////////////
  nrechits = recHitMap.size();
  if (storeRecHits)
  {
    DisPho::InitializeRecHitBranches();
    if (recHitsEBH.isValid() && recHitsEEH.isValid())
    {
      DisPho::SetRecHitBranches(recHitsEB,barrelGeometry,recHitMap);
      DisPho::SetRecHitBranches(recHitsEE,endcapGeometry,recHitMap);
    }
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
    if (nphotons > 0) DisPho::SetPhoBranch(photons[0],phoBranch0,recHitMap,recHitsEB,recHitsEE,tracksH);
    if (nphotons > 1) DisPho::SetPhoBranch(photons[1],phoBranch1,recHitMap,recHitsEB,recHitsEE,tracksH);
    if (nphotons > 2) DisPho::SetPhoBranch(photons[2],phoBranch2,recHitMap,recHitsEB,recHitsEE,tracksH);
    if (nphotons > 3) DisPho::SetPhoBranch(photons[3],phoBranch3,recHitMap,recHitsEB,recHitsEE,tracksH);

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
  disphotree->Fill();
}

void DisPho::InitializeGenEvtBranches()
{
  genwgt = -9999.f;
}

void DisPho::InitializeGenPointBranches()
{
  genx0 = -9999.f;
  geny0 = -9999.f;
  genz0 = -9999.f;
  gent0 = -9999.f;
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
    
    if (photon.pt() < ((1.f-genpTres) * gmsbBranch.genphpt_)) continue;
    if (photon.pt() > ((1.f+genpTres) * gmsbBranch.genphpt_)) continue;

    const float delR = Config::deltaR(gmsbBranch.genphphi_,gmsbBranch.genpheta_,photon.phi(),photon.eta());
    if (delR < mindR) 
    {
      mindR = delR;
      gmsbBranch.genphmatch_ = iph; 
    } // end check over deltaR

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
    if (tmppt < ((1.f-genpTres) * hvdsBranch.genHVph0pt_)) continue;
    if (tmppt > ((1.f+genpTres) * hvdsBranch.genHVph0pt_)) continue;

    const float delR0 = Config::deltaR(hvdsBranch.genHVph0phi_,hvdsBranch.genHVph0eta_,tmpphi,tmpeta);
    if (delR0 < mindR0) 
    {
      mindR0 = delR0;
      tmpph0 = iph;
    } // end check over deltaR
	      
    // check photon 2
    if (tmppt < ((1.f-genpTres) * hvdsBranch.genHVph1pt_)) continue;
    if (tmppt > ((1.f+genpTres) * hvdsBranch.genHVph1pt_)) continue;

    const float delR1 = Config::deltaR(hvdsBranch.genHVph1phi_,hvdsBranch.genHVph1eta_,tmpphi,tmpeta);
    if (delR1 < mindR1) 
    {
      mindR1 = delR1;
      tmpph1 = iph;
    } // end check over deltaR
    
    // now update iph
    iph++;
  } // end loop over reco photons
  
  // now save tmp photon iphs
  hvdsBranch.genHVph0match_ = tmpph0;
  hvdsBranch.genHVph1match_ = tmpph1;
} 

void DisPho::InitializeToyBranches()
{
  nToyPhs = -9999;
  DisPho::InitializeToyBranch(toyBranch0);
  DisPho::InitializeToyBranch(toyBranch1);
}

void DisPho::InitializeToyBranch(toyStruct & toyBranch)
{
  toyBranch.genphE_   = -9999.f;
  toyBranch.genphpt_  = -9999.f;
  toyBranch.genphphi_ = -9999.f;
  toyBranch.genpheta_ = -9999.f;

  toyBranch.genphmatch_ = -9999;
  toyBranch.genphmatch_ptres_ = -9999;
  toyBranch.genphmatch_status_ = -9999;
}

void DisPho::SetToyBranch(const reco::GenParticle & toy, toyStruct & toyBranch, const std::vector<oot::Photon> & photons)
{
  toyBranch.genphE_   = toy.energy();
  toyBranch.genphpt_  = toy.pt();
  toyBranch.genphphi_ = toy.phi();
  toyBranch.genpheta_ = toy.eta();

  int iph = 0;
  float mindR = dRmin, mindR_ptres = dRmin, mindR_status = dRmin;
  for (const auto & photon : photons)
  {
    if (iph > 4) break;
    
    const float delR = Config::deltaR(toyBranch.genphphi_,toyBranch.genpheta_,photon.phi(),photon.eta());
    if (delR < mindR) 
    {
      mindR = delR;
      toyBranch.genphmatch_ = iph; 
    } // end check over deltaR

    if ( (photon.pt() >= ((1.f-genpTres) * toyBranch.genphpt_)) && (photon.pt() <= ((1.f+genpTres) * toyBranch.genphpt_)) )
    {
      const float delR_ptres = Config::deltaR(toyBranch.genphphi_,toyBranch.genpheta_,photon.phi(),photon.eta());
      if (delR_ptres < mindR_ptres) 
      {
	mindR_ptres = delR_ptres;
	toyBranch.genphmatch_ptres_ = iph; 
      } // end check over deltaR
      
      if (toy.isPromptFinalState())
      {
	const float delR_status = Config::deltaR(toyBranch.genphphi_,toyBranch.genpheta_,photon.phi(),photon.eta());
	if (delR_status < mindR_status) 
	{
	  mindR_status = delR_status;
	  toyBranch.genphmatch_status_ = iph; 
	} // end check over deltaR
      } // end check over final state
    } // end check over pt res

    iph++;
  } // end loop over reco photons
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
  DisPho::InitializeJetBranch(jetBranch0L);
  DisPho::InitializeJetBranch(jetBranch1L);
  DisPho::InitializeJetBranch(jetBranch2L);
  DisPho::InitializeJetBranch(jetBranch3L);

  DisPho::InitializeJetBranch(jetBranch0T);
  DisPho::InitializeJetBranch(jetBranch1T);
  DisPho::InitializeJetBranch(jetBranch2T);
  DisPho::InitializeJetBranch(jetBranch3T);

  DisPho::InitializeJetBranch(jetBranch0TLV);
  DisPho::InitializeJetBranch(jetBranch1TLV);
  DisPho::InitializeJetBranch(jetBranch2TLV);
  DisPho::InitializeJetBranch(jetBranch3TLV);
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

  phoBranch.HoE_ = -9999.f;
  phoBranch.r9_ = -9999.f;

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

  if (storeRecHits)
  {
    phoBranch.seed_ = -1;
    phoBranch.recHits_.clear();
  }
  else
  {
    phoBranch.seedtime_ = -9999.f;
    phoBranch.seedE_    = -9999.f;
    phoBranch.seedID_   = 0; // non-ideal
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

void DisPho::SetPhoBranch(const oot::Photon& photon, phoStruct & phoBranch, const uiiumap & recHitMap,
			  const EcalRecHitCollection * recHitsEB, const EcalRecHitCollection * recHitsEE,
			  const edm::Handle<std::vector<reco::Track> > & tracksH)
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

  // ID-like variables
  phoBranch.HoE_ = pho.hadTowOverEm(); // used in ID + trigger (single tower HoverE)
  phoBranch.r9_  = pho.r9(); // used in slimming in PAT + trigger

  phoBranch.ChgHadIso_ = pho.chargedHadronIso();
  phoBranch.NeuHadIso_ = pho.neutralHadronIso();
  phoBranch.PhoIso_    = pho.photonIso();

  // More ID variables
  phoBranch.EcalPFClIso_ = pho.ecalPFClusterIso();
  phoBranch.HcalPFClIso_ = pho.hcalPFClusterIso();
  phoBranch.TrkIso_      = pho.trkSumPtHollowConeDR03();

  // Shower Shape Objects
  const reco::Photon::ShowerShape& phoshape = pho.full5x5_showerShapeVariables(); 

  // cluster shape variables
  phoBranch.Sieie_ = phoshape.sigmaIetaIeta;
  phoBranch.Sipip_ = phoshape.sigmaIphiIphi;
  phoBranch.Sieip_ = phoshape.sigmaIetaIphi;

  // other cluster shape variables
  phoBranch.e2x2_ = phoshape.e2x2;
  phoBranch.e3x3_ = phoshape.e3x3;
  phoBranch.e5x5_ = phoshape.e5x5;

  // use seed to get geometry and recHits
  const DetId & seedDetId = phosc->seed()->seed(); // seed detid
  const uint32_t seedRawId = seedDetId.rawId(); // crystal number
  const bool isEB = (seedDetId.subdetId() == EcalBarrel); // which subdet
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
  if (storeRecHits)
  {
    for (uiiumap::const_iterator rhiter = phrhIDmap.begin(); rhiter != phrhIDmap.end(); ++rhiter) // loop over only good rec hit ids
    {
      phoBranch.recHits_.emplace_back(rhiter->first);
    }
  }

  // save seed info + swiss cross
  if (recHitMap.count(seedRawId)) 
  {
    if (storeRecHits) 
    {
      // store just the seed for accessing through recHit branches
      phoBranch.seed_ = recHitMap.at(seedRawId);
    }
    else
    {
      // store basic info about the seed
      EcalRecHitCollection::const_iterator seedRecHit = recHits->find(seedDetId);
      if (seedRecHit != recHits->end()) // standard check (redundant)
      {
	phoBranch.seedtime_  = seedRecHit->time();
	phoBranch.seedE_     = seedRecHit->energy();
	phoBranch.seedID_    = seedRawId;
      }
    }
    
    // swiss cross
    if (recHits->size() > 0) phoBranch.suisseX_ = EcalTools::swissCross(seedDetId, *recHits, rhEmin);
  }

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
  phoBranch.isGen_ = oot::GenToObjectMatching(photon,genparticlesH,genpTres,dRmin);
}

int DisPho::CheckMatchHVDS(const int iph, const hvdsStruct& hvdsBranch)
{
  if      (iph == hvdsBranch.genHVph0match_ && iph != hvdsBranch.genHVph1match_) return 1;
  else if (iph != hvdsBranch.genHVph0match_ && iph == hvdsBranch.genHVph1match_) return 2;
  else if (iph == hvdsBranch.genHVph0match_ && iph == hvdsBranch.genHVph1match_) return 3;
  else                                                                           return 0;
}

void DisPho::beginJob() 
{
  edm::Service<TFileService> fs;
  
  // histograms needed
  h_cutflow = fs->make<TH1F>("h_cutflow", "Cut Flow", cutflowLabelMap.size(), 0, cutflowLabelMap.size());
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
    h_cutflow->GetXaxis()->SetBinLabel(cutflowLabelPair.second+1,cutflowLabelPair.first.c_str()); // +1 to account for bins in ROOT from [1,nBins]
  }
  h_cutflow->GetYaxis()->SetTitle("nEvents with weights");
  h_cutflow->Sumw2();
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
  int jetIDStoremin_tmp = jetIDStoremin;
  configtree->Branch("seedTimemin", &seedTimemin_tmp, "seedTimemin/F");
  configtree->Branch("jetIDStoremin", &jetIDStoremin_tmp, "jetIDStoremin/I");

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
  float genpTres_tmp = genpTres;
  float trackdRmin_tmp = trackdRmin;
  float trackpTmin_tmp = trackpTmin;
  configtree->Branch("dRmin", &dRmin_tmp, "dRmin/F");
  configtree->Branch("pTres", &pTres_tmp, "pTres/F");
  configtree->Branch("genpTres", &genpTres_tmp, "genpTres/F");
  configtree->Branch("trackdRmin", &trackdRmin_tmp, "trackdRmin/F");
  configtree->Branch("trackpTmin", &trackpTmin_tmp, "trackpTmin/F");

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
  float xsec_tmp = xsec;
  float filterEff_tmp = filterEff;
  float BR_tmp = BR;
  configtree->Branch("isGMSB", &isGMSB_tmp, "isGMSB/O");
  configtree->Branch("isHVDS", &isHVDS_tmp, "isHVDS/O");
  configtree->Branch("isBkgd", &isBkgd_tmp, "isBkgd/O");
  configtree->Branch("isToy" , &isToy_tmp , "isToy/O");
  configtree->Branch("xsec", &xsec_tmp, "xsec/F");
  configtree->Branch("filterEff", &filterEff_tmp, "filterEff/F");
  configtree->Branch("BR", &BR_tmp, "BR/F");

  // Fill tree just once, after configs have been read in
  configtree->Fill();
}

void DisPho::MakeEventTree()
{
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
    DisPho::MakeGMSBBranch(0,gmsbBranch0);
    DisPho::MakeGMSBBranch(1,gmsbBranch1);
  }

  // HVDS Info
  if (isHVDS)
  {
    disphotree->Branch("nvPions", &nvPions, "nvPions/I");
    DisPho::MakeHVDSBranch(0,hvdsBranch0);
    DisPho::MakeHVDSBranch(1,hvdsBranch1);
    DisPho::MakeHVDSBranch(2,hvdsBranch2);
    DisPho::MakeHVDSBranch(3,hvdsBranch3);
  }

  // ToyMC Info
  if (isToy)
  {
    disphotree->Branch("nToyPhs", &nToyPhs, "nToyPhs/I");
    DisPho::MakeToyBranch(0,toyBranch0);
    DisPho::MakeToyBranch(1,toyBranch1);
  }

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
  disphotree->Branch("hltDiEle33MW", &hltDiEle33MW, "hltDiEle33MW/O");
  disphotree->Branch("hltDiEle27WPT", &hltDiEle27WPT, "hltDiEle27WPT/O");
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

  // rho info
  disphotree->Branch("rho", &rho, "rho/F");

  // MET info
  disphotree->Branch("t1pfMETpt", &t1pfMETpt, "t1pfMETpt/F");
  disphotree->Branch("t1pfMETphi", &t1pfMETphi, "t1pfMETphi/F");
  disphotree->Branch("t1pfMETsumEt", &t1pfMETsumEt, "t1pfMETsumEt/F");

  // HT Info
  disphotree->Branch("jetHT", &jetHT, "jetHT/F");
  disphotree->Branch("njets", &njets, "njets/I");
  disphotree->Branch("jetHTpt15", &jetHTpt15, "jetHTpt15/F");
  disphotree->Branch("njetspt15", &njetspt15, "njetspt15/I");
  disphotree->Branch("jetHTeta3", &jetHTeta3, "jetHTeta3/F");
  disphotree->Branch("njetseta3", &njetseta3, "njetseta3/I");
  disphotree->Branch("jetHTnopho", &jetHTnopho, "jetHTnopho/F");
  disphotree->Branch("njetsnopho", &njetsnopho, "njetsnopho/I");
  disphotree->Branch("jetHTidL", &jetHTidL, "jetHTidL/F");
  disphotree->Branch("njetsidL", &njetsidL, "njetsidL/I");
  disphotree->Branch("jetHTidT", &jetHTidT, "jetHTidT/F");
  disphotree->Branch("njetsidT", &njetsidT, "njetsidT/I");
  disphotree->Branch("jetHTidTLV", &jetHTidTLV, "jetHTidTLV/F");
  disphotree->Branch("njetsidTLV", &njetsidTLV, "njetsidTLV/I");

  // Jet Info
  DisPho::MakeJetBranch(0,"L",jetBranch0L);
  DisPho::MakeJetBranch(1,"L",jetBranch1L);
  DisPho::MakeJetBranch(2,"L",jetBranch2L);
  DisPho::MakeJetBranch(3,"L",jetBranch3L);

  DisPho::MakeJetBranch(0,"T",jetBranch0T);
  DisPho::MakeJetBranch(1,"T",jetBranch1T);
  DisPho::MakeJetBranch(2,"T",jetBranch2T);
  DisPho::MakeJetBranch(3,"T",jetBranch3T);

  DisPho::MakeJetBranch(0,"TLV",jetBranch0TLV);
  DisPho::MakeJetBranch(1,"TLV",jetBranch1TLV);
  DisPho::MakeJetBranch(2,"TLV",jetBranch2TLV);
  DisPho::MakeJetBranch(3,"TLV",jetBranch3TLV);

  // RecHit Info
  disphotree->Branch("nrechits", &nrechits, "nrechits/I");
  if (storeRecHits)
  {
    disphotree->Branch("rheta", &rheta);
    disphotree->Branch("rhphi", &rhphi);
    disphotree->Branch("rhE", &rhE);
    disphotree->Branch("rhtime", &rhtime);
    disphotree->Branch("rhOOT", &rhOOT);
    disphotree->Branch("rhID", &rhID);
  }

  // Photon Info
  disphotree->Branch("nphotons", &nphotons, "nphotons/I");
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
  disphotree->Branch(Form("genNmass_%i",i), &gmsbBranch.genNmass_, Form("genNmass_%i/F",i));
  disphotree->Branch(Form("genNE_%i",i), &gmsbBranch.genNE_, Form("genNE_%i/F",i));
  disphotree->Branch(Form("genNpt_%i",i), &gmsbBranch.genNpt_, Form("genNpt_%i/F",i));
  disphotree->Branch(Form("genNphi_%i",i), &gmsbBranch.genNphi_, Form("genNphi_%i/F",i));
  disphotree->Branch(Form("genNeta_%i",i), &gmsbBranch.genNeta_, Form("genNeta_%i/F",i));

  disphotree->Branch(Form("genNprodvx_%i",i), &gmsbBranch.genNprodvx_, Form("genNprodvx_%i/F",i));
  disphotree->Branch(Form("genNprodvy_%i",i), &gmsbBranch.genNprodvy_, Form("genNprodvy_%i/F",i));
  disphotree->Branch(Form("genNprodvz_%i",i), &gmsbBranch.genNprodvz_, Form("genNprodvz_%i/F",i));

  disphotree->Branch(Form("genNdecayvx_%i",i), &gmsbBranch.genNdecayvx_, Form("genNdecayvx_%i/F",i));
  disphotree->Branch(Form("genNdecayvy_%i",i), &gmsbBranch.genNdecayvy_, Form("genNdecayvy_%i/F",i));
  disphotree->Branch(Form("genNdecayvz_%i",i), &gmsbBranch.genNdecayvz_, Form("genNdecayvz_%i/F",i));

  disphotree->Branch(Form("genphE_%i",i), &gmsbBranch.genphE_, Form("genphE_%i/F",i));
  disphotree->Branch(Form("genphpt_%i",i), &gmsbBranch.genphpt_, Form("genphpt_%i/F",i));
  disphotree->Branch(Form("genphphi_%i",i), &gmsbBranch.genphphi_, Form("genphphi_%i/F",i));
  disphotree->Branch(Form("genpheta_%i",i), &gmsbBranch.genpheta_, Form("genpheta_%i/F",i));
  disphotree->Branch(Form("genphmatch_%i",i), &gmsbBranch.genphmatch_, Form("genphmatch_%i/I",i));

  disphotree->Branch(Form("gengrmass_%i",i), &gmsbBranch.gengrmass_, Form("gengrmass_%i/F",i));
  disphotree->Branch(Form("gengrE_%i",i), &gmsbBranch.gengrE_, Form("gengrE_%i/F",i));
  disphotree->Branch(Form("gengrpt_%i",i), &gmsbBranch.gengrpt_, Form("gengrpt_%i/F",i));
  disphotree->Branch(Form("gengrphi_%i",i), &gmsbBranch.gengrphi_, Form("gengrphi_%i/F",i));
  disphotree->Branch(Form("gengreta_%i",i), &gmsbBranch.gengreta_, Form("gengreta_%i/F",i));
}

void DisPho::MakeHVDSBranch(const int i, hvdsStruct& hvdsBranch)
{
  disphotree->Branch(Form("genvPionmass_%i",i), &hvdsBranch.genvPionmass_, Form("genvPionmass_%i/F",i));
  disphotree->Branch(Form("genvPionE_%i",i), &hvdsBranch.genvPionE_, Form("genvPionE_%i/F",i));
  disphotree->Branch(Form("genvPionpt_%i",i), &hvdsBranch.genvPionpt_, Form("genvPionpt_%i/F",i));
  disphotree->Branch(Form("genvPionphi_%i",i), &hvdsBranch.genvPionphi_, Form("genvPionphi_%i/F",i));
  disphotree->Branch(Form("genvPioneta_%i",i), &hvdsBranch.genvPioneta_, Form("genvPioneta_%i/F",i));

  disphotree->Branch(Form("genvPionprodvx_%i",i), &hvdsBranch.genvPionprodvx_, Form("genvPionprodvx_%i/F",i));
  disphotree->Branch(Form("genvPionprodvy_%i",i), &hvdsBranch.genvPionprodvy_, Form("genvPionprodvy_%i/F",i));
  disphotree->Branch(Form("genvPionprodvz_%i",i), &hvdsBranch.genvPionprodvz_, Form("genvPionprodvz_%i/F",i));

  disphotree->Branch(Form("genvPiondecayvx_%i",i), &hvdsBranch.genvPiondecayvx_, Form("genvPiondecayvx_%i/F",i));
  disphotree->Branch(Form("genvPiondecayvy_%i",i), &hvdsBranch.genvPiondecayvy_, Form("genvPiondecayvy_%i/F",i));
  disphotree->Branch(Form("genvPiondecayvz_%i",i), &hvdsBranch.genvPiondecayvz_, Form("genvPiondecayvz_%i/F",i));

  disphotree->Branch(Form("genHVph0E_%i",i), &hvdsBranch.genHVph0E_, Form("genHVph0E_%i/F",i));
  disphotree->Branch(Form("genHVph0pt_%i",i), &hvdsBranch.genHVph0pt_, Form("genHVph0pt_%i/F",i));
  disphotree->Branch(Form("genHVph0phi_%i",i), &hvdsBranch.genHVph0phi_, Form("genHVph0phi_%i/F",i));
  disphotree->Branch(Form("genHVph0eta_%i",i), &hvdsBranch.genHVph0eta_, Form("genHVph0eta_%i/F",i));
  disphotree->Branch(Form("genHVph0match_%i",i), &hvdsBranch.genHVph0match_, Form("genHVph0match_%i/I",i));

  disphotree->Branch(Form("genHVph1E_%i",i), &hvdsBranch.genHVph1E_, Form("genHVph1E_%i/F",i));
  disphotree->Branch(Form("genHVph1pt_%i",i), &hvdsBranch.genHVph1pt_, Form("genHVph1pt_%i/F",i));
  disphotree->Branch(Form("genHVph1phi_%i",i), &hvdsBranch.genHVph1phi_, Form("genHVph1phi_%i/F",i));
  disphotree->Branch(Form("genHVph1eta_%i",i), &hvdsBranch.genHVph1eta_, Form("genHVph1eta_%i/F",i));
  disphotree->Branch(Form("genHVph1match_%i",i), &hvdsBranch.genHVph1match_, Form("genHVph1match_%i/I",i));
}

void DisPho::MakeToyBranch(const int i, toyStruct& toyBranch)
{
  disphotree->Branch(Form("genphE_%i",i), &toyBranch.genphE_, Form("genphE_%i/F",i));
  disphotree->Branch(Form("genphpt_%i",i), &toyBranch.genphpt_, Form("genphpt_%i/F",i));
  disphotree->Branch(Form("genphphi_%i",i), &toyBranch.genphphi_, Form("genphphi_%i/F",i));
  disphotree->Branch(Form("genpheta_%i",i), &toyBranch.genpheta_, Form("genpheta_%i/F",i));

  disphotree->Branch(Form("genphmatch_%i",i), &toyBranch.genphmatch_, Form("genphmatch_%i/I",i));
  disphotree->Branch(Form("genphmatch_ptres_%i",i), &toyBranch.genphmatch_ptres_, Form("genphmatch_ptres_%i/I",i));
  disphotree->Branch(Form("genphmatch_status_%i",i), &toyBranch.genphmatch_status_, Form("genphmatch_status_%i/I",i));
}

void DisPho::MakeJetBranch(const int i, const TString& label, jetStruct& jetBranch)
{
  disphotree->Branch(Form("jetE_%i_%s",i,label.Data()), &jetBranch.E_, Form("jetE_%i_%s/F",i,label.Data()));
  disphotree->Branch(Form("jetpt_%i_%s",i,label.Data()), &jetBranch.Pt_, Form("jetpt_%i_%s/F",i,label.Data()));
  disphotree->Branch(Form("jeteta_%i_%s",i,label.Data()), &jetBranch.Eta_, Form("jeteta_%i_%s/F",i,label.Data()));
  disphotree->Branch(Form("jetphi_%i_%s",i,label.Data()), &jetBranch.Phi_, Form("jetphi_%i_%s/F",i,label.Data()));
}

void DisPho::MakePhoBranch(const int i, phoStruct& phoBranch)
{
  disphotree->Branch(Form("phoE_%i",i), &phoBranch.E_, Form("phoE_%i/F",i));
  disphotree->Branch(Form("phopt_%i",i), &phoBranch.Pt_, Form("phopt_%i/F",i));
  disphotree->Branch(Form("phoeta_%i",i), &phoBranch.Eta_, Form("phoeta_%i/F",i));
  disphotree->Branch(Form("phophi_%i",i), &phoBranch.Phi_, Form("phophi_%i/F",i));

  disphotree->Branch(Form("phoscE_%i",i), &phoBranch.scE_, Form("phoscE_%i/F",i));
  disphotree->Branch(Form("phosceta_%i",i), &phoBranch.scEta_, Form("phosceta_%i/F",i));
  disphotree->Branch(Form("phoscphi_%i",i), &phoBranch.scPhi_, Form("phoscphi_%i/F",i));

  disphotree->Branch(Form("phoHoE_%i",i), &phoBranch.HoE_, Form("phoHoE_%i/F",i));
  disphotree->Branch(Form("phor9_%i",i), &phoBranch.r9_, Form("phor9_%i/F",i));

  disphotree->Branch(Form("phoChgHadIso_%i",i), &phoBranch.ChgHadIso_, Form("phoChgHadIso_%i/F",i));
  disphotree->Branch(Form("phoNeuHadIso_%i",i), &phoBranch.NeuHadIso_, Form("phoNeuHadIso_%i/F",i));
  disphotree->Branch(Form("phoPhoIso_%i",i), &phoBranch.PhoIso_, Form("phoPhoIso_%i/F",i));

  disphotree->Branch(Form("phoEcalPFClIso_%i",i), &phoBranch.EcalPFClIso_, Form("phoEcalPFClIso_%i/F",i));
  disphotree->Branch(Form("phoHcalPFClIso_%i",i), &phoBranch.HcalPFClIso_, Form("phoHcalPFClIso_%i/F",i));
  disphotree->Branch(Form("phoTrkIso_%i",i), &phoBranch.TrkIso_, Form("phoTrkIso_%i/F",i));

  disphotree->Branch(Form("phosieie_%i",i), &phoBranch.Sieie_, Form("phosieie_%i/F",i));
  disphotree->Branch(Form("phosipip_%i",i), &phoBranch.Sipip_, Form("phosipip_%i/F",i));
  disphotree->Branch(Form("phosieip_%i",i), &phoBranch.Sieip_, Form("phosieip_%i/F",i));

  disphotree->Branch(Form("phoe2x2_%i",i), &phoBranch.e2x2_, Form("phoe2x2_%i/F",i));
  disphotree->Branch(Form("phoe3x3_%i",i), &phoBranch.e3x3_, Form("phoe3x3_%i/F",i));
  disphotree->Branch(Form("phoe5x5_%i",i), &phoBranch.e5x5_, Form("phoe5x5_%i/F",i));

  disphotree->Branch(Form("phosmaj_%i",i), &phoBranch.Smaj_, Form("phosmaj_%i/F",i));
  disphotree->Branch(Form("phosmin_%i",i), &phoBranch.Smin_, Form("phosmin_%i/F",i));
  disphotree->Branch(Form("phoalpha_%i",i), &phoBranch.alpha_, Form("phoalpha_%i/F",i));

  if (storeRecHits)
  {
    disphotree->Branch(Form("phoseed_%i",i), &phoBranch.seed_, Form("phoseed_%i/I",i));
    disphotree->Branch(Form("phorecHits_%i",i), &phoBranch.recHits_);
  }
  else 
  {
    disphotree->Branch(Form("phoseedtime_%i",i), &phoBranch.seedtime_, Form("phoseedtime_%i/F",i));
    disphotree->Branch(Form("phoseedE_%i",i), &phoBranch.seedE_, Form("phoseedE_%i/F",i));
    disphotree->Branch(Form("phoseedID_%i",i), &phoBranch.seedID_, Form("phoseedID_%i/i",i));
  }

  disphotree->Branch(Form("phosuisseX_%i",i), &phoBranch.suisseX_, Form("phosuisseX_%i/F",i));

  disphotree->Branch(Form("phoisOOT_%i",i), &phoBranch.isOOT_, Form("phoisOOT_%i/O",i));
  disphotree->Branch(Form("phoisEB_%i",i), &phoBranch.isEB_, Form("phoisEB_%i/O",i));
  disphotree->Branch(Form("phoisHLT_%i",i), &phoBranch.isHLT_, Form("phoisHLT_%i/O",i));
  disphotree->Branch(Form("phoisTrk_%i",i), &phoBranch.isTrk_, Form("phoisTrk_%i/O",i));
  disphotree->Branch(Form("phopassEleVeto_%i",i), &phoBranch.passEleVeto_, Form("phopassEleVeto_%i/O",i));
  disphotree->Branch(Form("phohasPixSeed_%i",i), &phoBranch.hasPixSeed_, Form("phohasPixSeed_%i/O",i));
  disphotree->Branch(Form("phogedID_%i",i), &phoBranch.gedID_, Form("phogedID_%i/I",i));
  disphotree->Branch(Form("phoootID_%i",i), &phoBranch.ootID_, Form("phoootID_%i/I",i));
}

void DisPho::MakePhoBranchMC(const int i, phoStruct& phoBranch)
{
  if (isGMSB || isHVDS) disphotree->Branch(Form("phoisSignal_%i",i), &phoBranch.isSignal_, Form("phoisSignal_%i/I",i));
  disphotree->Branch(Form("phoisGen_%i",i), &phoBranch.isGen_, Form("phoisGen_%i/O",i));
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
