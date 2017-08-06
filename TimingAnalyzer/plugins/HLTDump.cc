#include "HLTDump.h"

HLTDump::HLTDump(const edm::ParameterSet& iConfig): 
  // cuts
  jetpTmin(iConfig.existsAs<double>("jetpTmin") ? iConfig.getParameter<double>("jetpTmin") : 15.f),
  dRmin(iConfig.existsAs<double>("dRmin") ? iConfig.getParameter<double>("dRmin") : 0.4),
  pTres(iConfig.existsAs<double>("pTres") ? iConfig.getParameter<double>("pTres") : 0.5),
  
  // triggers
  inputPaths       (iConfig.existsAs<std::string>("inputPaths")   ? iConfig.getParameter<std::string>("inputPaths") : ""),
  inputFilters     (iConfig.existsAs<std::string>("inputFilters") ? iConfig.getParameter<std::string>("inputFilters") : ""),
  triggerResultsTag(iConfig.getParameter<edm::InputTag>("triggerResults")),
  triggerEventTag  (iConfig.getParameter<edm::InputTag>("triggerEvent")),

  // rhos
  rhosTag(iConfig.getParameter<edm::InputTag>("rhos")),

  // jets
  jetsTag(iConfig.getParameter<edm::InputTag>("jets")),  

  // photons + ids
  photonsTag(iConfig.getParameter<edm::InputTag>("photons")),  
  
  //recHits
  recHitsEBTag(iConfig.getParameter<edm::InputTag>("recHitsEB")),  
  recHitsEETag(iConfig.getParameter<edm::InputTag>("recHitsEE"))
{
  usesResource();
  usesResource("TFileService");

  // triggers
  triggerResultsToken = consumes<edm::TriggerResults>   (triggerResultsTag);
  triggerEventToken   = consumes<trigger::TriggerEvent> (triggerEventTag);

  // read in from a stream the trigger paths for saving
  if (file_exists(inputPaths))
  {
    std::fstream pathStream;
    pathStream.open(inputPaths.c_str(),std::ios::in);
    int index;
    std::string path;
    while (pathStream >> index >> path)
    {
      if (path != "") pathNames.push_back(path);
    }
    pathStream.close();

    // branch to store trigger info
    triggerBits.resize(pathNames.size());
  } // check to make sure text file exists

  // read in from a stream the hlt objects/labels to match to
  if (file_exists(inputFilters))
  {
    std::fstream filterStream;
    filterStream.open(inputFilters.c_str(),std::ios::in);
    int index;
    std::string label;// instance, processName;
    while (filterStream >> index >> label)
    {
      if (label != "") filterTags.push_back(edm::InputTag(label));
    }
    filterStream.close();

    // vector of vector of trigger objects
    triggerObjectsByFilter.resize(filterTags.size());
  } // check to make sure text file exists

  // rhos
  rhosToken = consumes<double> (rhosTag);

  // jets
  jetsToken = consumes<std::vector<pat::Jet> > (jetsTag);

  // photons
  photonsToken = consumes<std::vector<pat::Photon> > (photonsTag);

  // rechits
  recHitsEBToken = consumes<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > (recHitsEBTag);
  recHitsEEToken = consumes<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > (recHitsEETag);
}

HLTDump::~HLTDump() {}

void HLTDump::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) 
{
  // TRIGGERS
  edm::Handle<edm::TriggerResults> triggerResultsH;
  iEvent.getByToken(triggerResultsToken, triggerResultsH);

  edm::Handle<trigger::TriggerEvent> triggerEventH;
  iEvent.getByToken(triggerEventToken, triggerEventH);

  // RHOS
  edm::Handle<double> rhosH;
  iEvent.getByToken(rhosToken, rhosH);

  // JETS
  edm::Handle<std::vector<pat::Jet> > jetsH;
  iEvent.getByToken(jetsToken, jetsH);
  std::vector<pat::Jet> jets = *jetsH;

  // PHOTONS
  edm::Handle<std::vector<pat::Photon> > photonsH;
  iEvent.getByToken(photonsToken, photonsH);
  std::vector<pat::Photon> photons = *photonsH;

  // RecHits
  edm::Handle<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > recHitsEBH;
  iEvent.getByToken(recHitsEBToken, recHitsEBH);
  edm::Handle<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > recHitsEEH;
  iEvent.getByToken(recHitsEEToken, recHitsEEH);

  // geometry (from ECAL ELF)
  edm::ESHandle<CaloGeometry> calogeoH;
  iSetup.get<CaloGeometryRecord>().get(calogeoH);
  const CaloSubdetectorGeometry * barrelGeometry = calogeoH->getSubdetectorGeometry(DetId::Ecal, EcalBarrel);
  const CaloSubdetectorGeometry * endcapGeometry = calogeoH->getSubdetectorGeometry(DetId::Ecal, EcalEndcap);

  // do some prepping of objects
  HLTDump::PrepJets(jetsH,jets);
  HLTDump::PrepPhotons(photonsH,photons);

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
  HLTDump::InitializeTriggerBranches();
  if (triggerResultsH.isValid())
  {
    for (std::size_t ipath = 0; ipath < pathNames.size(); ipath++)
    {
      if (pathIndex[pathNames[ipath]] == -1) continue;	
      if (triggerResultsH->accept(pathIndex[pathNames[ipath]])) triggerBits[ipath] = true;
    } // end loop over trigger names
  } // end check over valid TriggerResults

  // store all the trigger objects needed to be checked later
  if (triggerEventH.isValid())
  {
    // adapted from: https://github.com/prbbing/EXOTriggerTutorial/blob/master/TriggerEfficiencyMeasurement/plugins/TriggerEfficiencyAnalyzer.cc
    // http://cmslxr.fnal.gov/source/DataFormats/HLTReco/interface/TriggerObject.h?v=CMSSW_8_0_24
    const std::vector<trigger::TriggerObject> & triggerEventObjects = triggerEventH->getObjects();
    for (std::size_t ifilter = 0; ifilter < filterTags.size(); ifilter++)
    {
      const size_t filterIndex = triggerEventH->filterIndex(filterTags[ifilter]);
      if (filterIndex < triggerEventH->sizeFilters())
      {
	const std::vector<uint16_t> & objectKeys = triggerEventH->filterKeys(filterIndex);
	for (std::size_t ikey = 0; ikey < objectKeys.size(); ikey++)
	{
	  triggerObjectsByFilter[ifilter].push_back(triggerEventObjects[objectKeys[ikey]]);
	} // end loop over matching filter keys
      } // end check over filter existing!!!
    } // end loop over n filters by label
  } // end check over valid TriggerEvent

  /////////////////
  //             //
  // HLT Objects //
  //             //
  /////////////////

  HLTDump::ClearTriggerObjectBranches();
  for (std::size_t ifilter = 0; ifilter < triggerObjectsByFilter.size(); ifilter++)
  {
    HLTDump::PrepTrigObjs(triggerObjectsByFilter[ifilter]);
    
    for (std::size_t iobject = 0; iobject < triggerObjectsByFilter[ifilter].size(); iobject++)
    {
      const trigger::TriggerObject & triggerObject = triggerObjectsByFilter[ifilter][iobject];

      trigobjE  [ifilter][iobject] = triggerObject.energy();
      trigobjeta[ifilter][iobject] = triggerObject.eta();
      trigobjphi[ifilter][iobject] = triggerObject.phi();
      trigobjpt [ifilter][iobject] = triggerObject.pt();
    }
  }

  ///////////////////
  //               //
  // FixedGrid Rho //
  //               //
  ///////////////////

  const float rho = rhosH.isValid() ? *(rhosH.product()) : 0.f;

  /////////////////////////
  //                     //
  // Jets (AK4 standard) //
  //                     //
  /////////////////////////
  
  pfjetHT = -9999.f;
  if (jetsH.isValid()) // check to make sure reco jets exist
  {
    pfjetHT = 0.f;
    for (std::vector<pat::Jet>::const_iterator jetiter = jets.begin(); jetiter != jets.end(); ++jetiter)
    {
      if (jetiter->pt() < jetpTmin) break;
      pfjetHT += jetiter->pt();
    } // end loop over reco jets
  } // end check over reco jets
  
  //////////////////
  //              //
  // Reco Photons //
  //              //
  //////////////////
  HLTDump::ClearRecoPhotonBranches();
  if (photonsH.isValid()) // standard handle check
  {
    nphotons = photons.size();
    if (nphotons > 0) HLTDump::InitializeRecoPhotonBranches();

    int iph = 0;
    for (std::vector<pat::Photon>::const_iterator phiter = photons.begin(); phiter != photons.end(); ++phiter) // loop over photon vector
    {
      // standard photon branches
      phE  [iph] = phiter->energy();
      phpt [iph] = phiter->pt();
      phphi[iph] = phiter->phi();
      pheta[iph] = phiter->eta();

      // check for HLT filter matches!
      HLTDump::HLTToPATPhotonMatching(iph);

      // super cluster from photon
      const reco::SuperClusterRef& phsc = phiter->superCluster().isNonnull() ? phiter->superCluster() : phiter->parentSuperCluster();
      phscE  [iph] = phsc->energy();
      phsceta[iph] = phsc->eta();
      phscphi[iph] = phsc->phi();
      const float sceta = std::abs(phsceta[iph]);

      // Shower Shape Objects
      const reco::Photon::ShowerShape& phshape = phiter->full5x5_showerShapeVariables(); // phiter->showerShapeVariables();

      // ID-like variables
      phHoE   [iph] = phiter->hadronicOverEm(); // phiter->hadTowOverEm();
      phr9    [iph] = phshape.e3x3/phsc->rawEnergy(); // http://cmslxr.fnal.gov/source/DataFormats/EgammaCandidates/interface/Photon.h#0239
      phChgIso[iph] = std::max(phiter->chargedHadronIso() - (rho * HLTDump::GetChargedHadronEA(sceta)),0.f);
      phNeuIso[iph] = std::max(phiter->neutralHadronIso() - (rho * HLTDump::GetNeutralHadronEA(sceta)),0.f);
      phIso   [iph] = std::max(phiter->photonIso()        - (rho * HLTDump::GetGammaEA        (sceta)),0.f);

      // cluster shape variables
      phsieie[iph] = phshape.sigmaIetaIeta;
      phsipip[iph] = phshape.sigmaIphiIphi;
      phsieip[iph] = phshape.sigmaIetaIphi;

      // use seed to get geometry and recHits
      const DetId seedDetId = phsc->seed()->seed(); //seed detid
      const bool isEB = (seedDetId.subdetId() == EcalBarrel); //which subdet
      const EcalRecHitCollection * recHits = (isEB ? recHitsEBH : recHitsEEH).product();
      const EcalRecHitCollection::const_iterator seedRecHit = recHits->find(seedDetId);
      
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
	  // http://cmslxr.fnal.gov/source/RecoEcal/EgammaCoreTools/interface/EcalTools.h
	  phsuisseX[iph] = EcalTools::swissCross(seedDetId,(*recHits),1.f);

	  // radius of semi-major,minor axis is the inverse square root of the eigenvalues of the covariance matrix
	  const Cluster2ndMoments ph2ndMoments = noZS::EcalClusterTools::cluster2ndMoments( *phsc, *recHits);
	  phsmaj [iph] = ph2ndMoments.sMaj;
	  phsmin [iph] = ph2ndMoments.sMin;
	  phalpha[iph] = ph2ndMoments.alpha;

	  // map of rec hit ids
	  uiiumap phrhIDmap;
      
	  // all rechits in superclusters
	  const DetIdPairVec hitsAndFractions = phsc->hitsAndFractions();

	  for (DetIdPairVec::const_iterator hafitr = hitsAndFractions.begin(); hafitr != hitsAndFractions.end(); ++hafitr) // loop over all rec hits in SC
          {
	    const DetId recHitId = hafitr->first; // get detid of crystal
	    EcalRecHitCollection::const_iterator recHit = recHits->find(recHitId); // get the underlying rechit
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
  } // end check over photon handle valid

  ///////////////
  //           //
  // Fill Tree //
  //           //
  ///////////////
  tree->Fill();      
}    

void HLTDump::PrepTrigObjs(std::vector<trigger::TriggerObject>& triggerObjects)
{
  if (triggerObjects.size() > 0) 
  {
    std::sort(triggerObjects.begin(),triggerObjects.end(),sortByTrigObjPt);
  }
}

void HLTDump::PrepJets(const edm::Handle<std::vector<pat::Jet> > & jetsH, std::vector<pat::Jet> & jets)
{
  if (jetsH.isValid()) // standard handle check
  {
    std::sort(jets.begin(),jets.end(),sortByJetPt);
  }
}  

void HLTDump::PrepPhotons(const edm::Handle<std::vector<pat::Photon> > & photonsH, std::vector<pat::Photon> & photons)
{
  if (photonsH.isValid()) // standard handle check
  {
    std::sort(photons.begin(),photons.end(),sortByPhotonPt);
  }
}  

void HLTDump::HLTToPATPhotonMatching(const int iph)
{
  for (std::size_t ifilter = 0; ifilter < triggerObjectsByFilter.size(); ifilter++)
  {
    for (std::size_t iobject = 0; iobject < triggerObjectsByFilter[ifilter].size(); iobject++)
    {
      const trigger::TriggerObject & triggerObject = triggerObjectsByFilter[ifilter][iobject];
      if (std::abs(triggerObject.pt()-phpt[iph])/phpt[iph] < pTres)
      {
	if (deltaR(phphi[iph],pheta[iph],triggerObject.phi(),triggerObject.eta()) < dRmin)
	{
	  phIsHLTMatched[iph][ifilter] = true;
	}
      }
    }
  }
}

float HLTDump::GetChargedHadronEA(const float eta)
{
  if      (eta <  1.0)                  return 0.0360;
  else if (eta >= 1.0   && eta < 1.479) return 0.0377;
  else if (eta >= 1.479 && eta < 2.0  ) return 0.0306;
  else if (eta >= 2.0   && eta < 2.2  ) return 0.0283;
  else if (eta >= 2.2   && eta < 2.3  ) return 0.0254;
  else if (eta >= 2.3   && eta < 2.4  ) return 0.0217;
  else if (eta >= 2.4)                  return 0.0167;
  else                                  return 0.;
}

float HLTDump::GetNeutralHadronEA(const float eta) 
{
  if      (eta <  1.0)                  return 0.0597;
  else if (eta >= 1.0   && eta < 1.479) return 0.0807;
  else if (eta >= 1.479 && eta < 2.0  ) return 0.0629;
  else if (eta >= 2.0   && eta < 2.2  ) return 0.0197;
  else if (eta >= 2.2   && eta < 2.3  ) return 0.0184;
  else if (eta >= 2.3   && eta < 2.4  ) return 0.0284;
  else if (eta >= 2.4)                  return 0.0591;
  else                                  return 0.;
}

float HLTDump::GetGammaEA(const float eta) 
{
  if      (eta <  1.0)                  return 0.1210;
  else if (eta >= 1.0   && eta < 1.479) return 0.1107;
  else if (eta >= 1.479 && eta < 2.0  ) return 0.0699;
  else if (eta >= 2.0   && eta < 2.2  ) return 0.1056;
  else if (eta >= 2.2   && eta < 2.3  ) return 0.1457;
  else if (eta >= 2.3   && eta < 2.4  ) return 0.1719;
  else if (eta >= 2.4)                  return 0.1998;
  else                                  return 0.;
}

void HLTDump::InitializeTriggerBranches()
{
  for (std::size_t ipath = 0; ipath < pathNames.size(); ipath++)
  { 
    triggerBits[ipath] = false;
  }

  // clear all old trigger objects
  for (std::size_t ifilter = 0; ifilter < triggerObjectsByFilter.size(); ifilter++)
  {
    triggerObjectsByFilter[ifilter].clear();
  }
}

void HLTDump::ClearTriggerObjectBranches()
{
  trigobjE.clear();
  trigobjeta.clear();
  trigobjphi.clear();
  trigobjpt.clear();

  trigobjE.resize(triggerObjectsByFilter.size());
  trigobjeta.resize(triggerObjectsByFilter.size());
  trigobjphi.resize(triggerObjectsByFilter.size());
  trigobjpt.resize(triggerObjectsByFilter.size());

  for (std::size_t ifilter = 0; ifilter < triggerObjectsByFilter.size(); ifilter++)
  {
    trigobjE  [ifilter].resize(triggerObjectsByFilter[ifilter].size());
    trigobjeta[ifilter].resize(triggerObjectsByFilter[ifilter].size());
    trigobjphi[ifilter].resize(triggerObjectsByFilter[ifilter].size());
    trigobjpt [ifilter].resize(triggerObjectsByFilter[ifilter].size());
    for (std::size_t iobject = 0; iobject < triggerObjectsByFilter[ifilter].size(); iobject++)
    {
      trigobjE  [ifilter][iobject] = -9999.f;
      trigobjeta[ifilter][iobject] = -9999.f;
      trigobjphi[ifilter][iobject] = -9999.f;
      trigobjpt [ifilter][iobject] = -9999.f;
    }
  }
}

void HLTDump::ClearRecoPhotonBranches()
{
  nphotons = -9999;

  phE.clear();
  phpt.clear();
  phphi.clear(); 
  pheta.clear(); 

  phscE.clear(); 
  phsceta.clear(); 
  phscphi.clear(); 

  phHoE.clear();
  phr9.clear();
  phChgIso.clear();
  phNeuIso.clear();
  phIso.clear();
  phsuisseX.clear();

  phsieie.clear();
  phsipip.clear();
  phsieip.clear();
  phsmaj.clear();
  phsmin.clear();
  phalpha.clear();

  phIsHLTMatched.clear();

  phnrh.clear();

  phseedeta.clear(); 
  phseedphi.clear(); 
  phseedE.clear(); 
  phseedtime.clear();
  phseedID.clear();
  phseedOOT.clear();
}

void HLTDump::InitializeRecoPhotonBranches()
{
  phE.resize(nphotons);
  phpt.resize(nphotons);
  phphi.resize(nphotons);
  pheta.resize(nphotons);

  phscE.resize(nphotons);
  phsceta.resize(nphotons);
  phscphi.resize(nphotons);

  phHoE.resize(nphotons);
  phr9.resize(nphotons);
  phChgIso.resize(nphotons);
  phNeuIso.resize(nphotons);
  phIso.resize(nphotons);
  phsuisseX.resize(nphotons);

  phsieie.resize(nphotons);
  phsipip.resize(nphotons);
  phsieip.resize(nphotons);
  phsmaj.resize(nphotons);
  phsmin.resize(nphotons);
  phalpha.resize(nphotons);

  phIsHLTMatched.resize(nphotons);

  phnrh.resize(nphotons);

  phseedeta.resize(nphotons);
  phseedphi.resize(nphotons);
  phseedE.resize(nphotons);
  phseedtime.resize(nphotons);
  phseedID.resize(nphotons);
  phseedOOT.resize(nphotons);

  for (int iph = 0; iph < nphotons; iph++)
  {
    phE  [iph] = -9999.f; 
    phpt [iph] = -9999.f; 
    phphi[iph] = -9999.f; 
    pheta[iph] = -9999.f; 

    phscE  [iph] = -9999.f; 
    phsceta[iph] = -9999.f; 
    phscphi[iph] = -9999.f; 

    phHoE    [iph] = -9999.f;
    phr9     [iph] = -9999.f;
    phChgIso [iph] = -9999.f;
    phNeuIso [iph] = -9999.f;
    phIso    [iph] = -9999.f;
    phsuisseX[iph] = -9999.f;

    phsieie[iph] = -9999.f;
    phsipip[iph] = -9999.f;
    phsieip[iph] = -9999.f;
    phsmaj [iph] = -9999.f;
    phsmin [iph] = -9999.f;
    phalpha[iph] = -9999.f;

    phnrh    [iph] = -9999;

    phIsHLTMatched[iph].resize(filterTags.size());
    for (std::size_t ifilter = 0; ifilter < filterTags.size(); ifilter++)
    {
      phIsHLTMatched[iph][ifilter] = 0; // false
    }

    phseedeta [iph] = -9999.f;
    phseedphi [iph] = -9999.f;
    phseedE   [iph] = -9999.f;
    phseedtime[iph] = -9999.f;
    phseedID  [iph] = -9999.f;
    phseedOOT [iph] = -9999.f;
  }
}

void HLTDump::beginJob() 
{
  edm::Service<TFileService> fs;
  tree = fs->make<TTree>("tree"     , "tree");

  // Run, Lumi, Event info
  tree->Branch("event"                  , &event                , "event/l");
  tree->Branch("run"                    , &run                  , "run/i");
  tree->Branch("lumi"                   , &lumi                 , "lumi/i");
   
  // Trigger Info
  tree->Branch("triggerBits"            , &triggerBits);

  // Jet Info
  tree->Branch("pfjetHT"              , &pfjetHT                , "pfjetHT/F");
   
  // HLT Info
  tree->Branch("trigobjE"                  , &trigobjE);
  tree->Branch("trigobjeta"                , &trigobjeta);
  tree->Branch("trigobjphi"                , &trigobjphi);
  tree->Branch("trigobjpt"                 , &trigobjpt);

  // Photon Info
  tree->Branch("nphotons"             , &nphotons             , "nphotons/I");

  tree->Branch("phE"                  , &phE);
  tree->Branch("phpt"                 , &phpt);
  tree->Branch("phphi"                , &phphi);
  tree->Branch("pheta"                , &pheta);

  tree->Branch("phscE"                , &phscE);
  tree->Branch("phsceta"              , &phsceta);
  tree->Branch("phscphi"              , &phscphi);

  tree->Branch("phHoE"                , &phHoE);
  tree->Branch("phr9"                 , &phr9);
  tree->Branch("phChgIso"             , &phChgIso);
  tree->Branch("phNeuIso"             , &phNeuIso);
  tree->Branch("phIso"                , &phIso);
  tree->Branch("phsuisseX"            , &phsuisseX);

  tree->Branch("phsieie"              , &phsieie);
  tree->Branch("phsipip"              , &phsipip);
  tree->Branch("phsieip"              , &phsieip);
  tree->Branch("phsmaj"               , &phsmaj);
  tree->Branch("phsmin"               , &phsmin);
  tree->Branch("phalpha"              , &phalpha);

  tree->Branch("phIsHLTMatched"       , &phIsHLTMatched);

  tree->Branch("phnrh"                , &phnrh);

  tree->Branch("phseedeta"              , &phseedeta);
  tree->Branch("phseedphi"              , &phseedphi);
  tree->Branch("phseedE"                , &phseedE);
  tree->Branch("phseedtime"             , &phseedtime);
  tree->Branch("phseedID"               , &phseedID);
  tree->Branch("phseedOOT"              , &phseedOOT);
}

void HLTDump::endJob() {}

void HLTDump::beginRun(edm::Run const& iRun, edm::EventSetup const& iSetup) 
{
  // initialize pathIndex, key: Name, value: Index 
  for (std::size_t ipath = 0; ipath < pathNames.size(); ipath++)
  {
    pathIndex[pathNames[ipath]] = -1;
  }

  HLTConfigProvider hltConfig;
  bool changed = false;
  hltConfig.init(iRun, iSetup, triggerResultsTag.process(), changed);
  const std::vector<std::string>& triggerNames = hltConfig.triggerNames();
  for (std::size_t ipath = 0; ipath < pathNames.size(); ipath++)
  {
    TPRegexp pattern(pathNames[ipath]);
    for (std::size_t itrigger = 0; itrigger < triggerNames.size(); itrigger++)
    {
      if (TString(triggerNames[itrigger]).Contains(pattern))
      {
	pathIndex[pathNames[ipath]] = itrigger; //hltConfig.triggerIndex(pathNames[itrigger]);
	break;
      }
    }
  }
}

void HLTDump::endRun(edm::Run const&, edm::EventSetup const&) 
{
  // reset trigger info
  pathIndex.clear();
}

void HLTDump::fillDescriptions(edm::ConfigurationDescriptions& descriptions) 
{
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

DEFINE_FWK_MODULE(HLTDump);
