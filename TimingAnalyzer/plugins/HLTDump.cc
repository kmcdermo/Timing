#include "HLTDump.h"

HLTDump::HLTDump(const edm::ParameterSet& iConfig): 
  // cuts
  phpTmin(iConfig.existsAs<double>("phpTmin") ? iConfig.getParameter<double>("phpTmin") : 20.f),
  jetpTmin(iConfig.existsAs<double>("jetpTmin") ? iConfig.getParameter<double>("jetpTmin") : 15.f),
  dRmin(iConfig.existsAs<double>("dRmin") ? iConfig.getParameter<double>("dRmin") : 0.4),
  pTres(iConfig.existsAs<double>("pTres") ? iConfig.getParameter<double>("pTres") : 0.5),
  saveTrigObjs(iConfig.existsAs<bool>("saveTrigObjs") ? iConfig.getParameter<bool>("saveTrigObjs") : false),
  
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

  // photons
  photonsTag(iConfig.getParameter<edm::InputTag>("photons")),  

  // ootPhotons
  ootPhotonsTag(iConfig.getParameter<edm::InputTag>("ootPhotons")),  
  
  //recHits
  recHitsEBTag(iConfig.getParameter<edm::InputTag>("recHitsEB")),  
  recHitsEETag(iConfig.getParameter<edm::InputTag>("recHitsEE"))
{
  usesResource();
  usesResource("TFileService");

  // triggers
  triggerResultsToken = consumes<edm::TriggerResults> (triggerResultsTag);
  triggerObjectsToken = consumes<std::vector<pat::TriggerObjectStandAlone> > (triggerObjectsTag);

  //  read in from a stream the trigger paths for saving
  if (file_exists(inputPaths))
  {
    std::fstream pathStream;
    pathStream.open(inputPaths.c_str(),std::ios::in);
    std::string path;
    while (pathStream >> path)
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
    std::string label;// instance, processName;
    while (filterStream >> label)
    {
      if (label != "") filterNames.push_back(label);
    }
    filterStream.close();

    // vector of vector of trigger objects
    triggerObjectsByFilter.resize(filterNames.size());
  } // check to make sure text file exists

  //vertex
  verticesToken = consumes<std::vector<reco::Vertex> > (verticesTag);

  // rhos
  rhosToken = consumes<double> (rhosTag);

  // mets
  metsToken = consumes<std::vector<pat::MET> > (metsTag);

  // jets
  jetsToken = consumes<std::vector<pat::Jet> > (jetsTag);

  // photons
  photonsToken = consumes<std::vector<pat::Photon> > (photonsTag);
  ootPhotonsToken = consumes<std::vector<pat::Photon> > (ootPhotonsTag);

  // rechits
  recHitsEBToken = consumes<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > (recHitsEBTag);
  recHitsEEToken = consumes<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > (recHitsEETag);
}

HLTDump::~HLTDump() {}

void HLTDump::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) 
{
  // VERTICES
  edm::Handle<std::vector<reco::Vertex> > verticesH;
  iEvent.getByToken(verticesToken, verticesH);
  
  // RHOS
  edm::Handle<double> rhosH;
  iEvent.getByToken(rhosToken, rhosH);

  // TRIGGERS
  edm::Handle<edm::TriggerResults> triggerResultsH;
  iEvent.getByToken(triggerResultsToken, triggerResultsH);

  edm::Handle<std::vector<pat::TriggerObjectStandAlone> > triggerObjectsH;
  iEvent.getByToken(triggerObjectsToken, triggerObjectsH);
  std::vector<pat::TriggerObjectStandAlone> triggerObjects = *triggerObjectsH;

  // MET
  edm::Handle<std::vector<pat::MET> > metsH;
  iEvent.getByToken(metsToken, metsH);

  // JETS
  edm::Handle<std::vector<pat::Jet> > jetsH;
  iEvent.getByToken(jetsToken, jetsH);
  std::vector<pat::Jet> jets; jets.reserve(jetsH->size());

  // PHOTONS
  edm::Handle<std::vector<pat::Photon> > photonsH;
  iEvent.getByToken(photonsToken, photonsH);

  edm::Handle<std::vector<pat::Photon> > ootPhotonsH;
  iEvent.getByToken(ootPhotonsToken, ootPhotonsH);

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

  std::vector<PatPhoton> photons;
  HLTDump::PrepPhotons(photonsH,ootPhotonsH,photons);

  ///////////////////////////
  //                       //
  // Event, lumi, run info //
  //                       //
  ///////////////////////////
  run   = iEvent.id().run();
  lumi  = iEvent.luminosityBlock();
  event = iEvent.id().event();

  /////////////////////////
  //                     //   
  // Primary Vertex info //
  //                     //
  /////////////////////////
  HLTDump::InitializePVBranches();
  if (verticesH.isValid()) 
  {
    nvtx = verticesH->size();
    const reco::Vertex & primevtx = (*verticesH)[0];
    vtxX = primevtx.position().x();
    vtxY = primevtx.position().y();
    vtxZ = primevtx.position().z();
  }

  ///////////////////
  //               //
  // FixedGrid Rho //
  //               //
  ///////////////////
  const float rho = rhosH.isValid() ? *(rhosH.product()) : 0.f;

  //////////////////
  //              //
  // Trigger Info //
  //              //
  //////////////////
  HLTDump::InitializeTriggerBranches();
  if (triggerResultsH.isValid())
  {
    const edm::TriggerNames &triggerNames = iEvent.triggerNames(*triggerResultsH);
    for (std::size_t itrig = 0; itrig < triggerNames.size(); itrig++)
    {
      TString triggerName = triggerNames.triggerName(itrig);
      for (std::size_t ipath = 0; ipath < pathNames.size(); ipath++)
      {
	if (triggerName.Contains(pathNames[ipath],TString::kExact)) triggerBits[ipath] = triggerResultsH->accept(itrig);
      } // end loop over user path names
    } // end loop over trigger names

    // store all the trigger objects needed to be checked later
    if (triggerObjectsH.isValid())
    {
      for (pat::TriggerObjectStandAlone triggerObject : triggerObjects) 
      {
	triggerObject.unpackPathNames(triggerNames);
	triggerObject.unpackFilterLabels(iEvent, *triggerResultsH);
	for (std::size_t ifilter = 0; ifilter < filterNames.size(); ifilter++)
	{	
	  if (triggerObject.hasFilterLabel(filterNames[ifilter])) triggerObjectsByFilter[ifilter].push_back(triggerObject);
	} // end loop over user filter names
      } // end loop over trigger objects
    } // end check over valid TriggerObjects
  } // end check over valid TriggerResults

  /////////////////
  //             //
  // HLT Objects //
  //             //
  /////////////////
  HLTDump::PrepTriggerObjects();

  if (saveTrigObjs) 
  {
    HLTDump::ClearTriggerObjectBranches();
    for (std::size_t ifilter = 0; ifilter < triggerObjectsByFilter.size(); ifilter++)
    {
      for (std::size_t iobject = 0; iobject < triggerObjectsByFilter[ifilter].size(); iobject++)
      {
	const pat::TriggerObjectStandAlone & triggerObject = triggerObjectsByFilter[ifilter][iobject];
	
	trigobjE  [ifilter][iobject] = triggerObject.energy();
	trigobjeta[ifilter][iobject] = triggerObject.eta();
	trigobjphi[ifilter][iobject] = triggerObject.phi();
	trigobjpt [ifilter][iobject] = triggerObject.pt();
      } // save trig objects
    } // end loop over filter objects
  } // end loop over filters

  //////////////////
  //              //
  // Type1 PF Met //
  //              //
  //////////////////
  HLTDump::InitializeMETBranches();
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
  
  HLTDump::ClearJetBranches();
  if (jetsH.isValid()) // check to make sure reco jets exist
  {
    njets = jets.size();
    if (njets > 0) HLTDump::InitializeJetBranches();

    int ijet = 0;
    for (std::vector<pat::Jet>::const_iterator jetiter = jets.begin(); jetiter != jets.end(); ++jetiter)
    {
      jetE  [ijet] = jetiter->energy();
      jetpt [ijet] = jetiter->pt();
      jetphi[ijet] = jetiter->phi();
      jeteta[ijet] = jetiter->eta();
      
      const float nhf = jetiter->neutralHadronEnergyFraction();
      const float chf = jetiter->chargedHadronEnergyFraction();

      const float nef = jetiter->neutralEmEnergyFraction();
      const float cef = jetiter->chargedEmEnergyFraction();

      const int cm = jetiter->chargedMultiplicity();
      const int nm = jetiter->neutralMultiplicity();

      const float eta = std::abs(jeteta[ijet]);

      if      (eta <= 2.4) jetidL[ijet] = nhf<0.99 && nef<0.99 && (cm+nm)>1 && chf > 0 && cm > 0 && cef < 0.99;
      else if (eta <= 2.7) jetidL[ijet] = nhf<0.99 && nef<0.99 && (cm+nm)>1;
      else if (eta <= 3.0) jetidL[ijet] = nef>0.01 && nhf<0.98 && nm>2;
      else                 jetidL[ijet] = nef<0.90 && nm>10;
      
      ijet++;
    } // end loop over reco jets
  } // end check over reco jets
  
  //////////////////
  //              //
  // Reco Photons //
  //              //
  //////////////////
  HLTDump::ClearRecoPhotonBranches();
  if (photonsH.isValid() && ootPhotonsH.isValid()) // standard handle check
  {
    nphotons = photons.size();
    if (nphotons > 0) HLTDump::InitializeRecoPhotonBranches();

    int iph = 0;
    for (std::vector<PatPhoton>::const_iterator phiter = photons.begin(); phiter != photons.end(); ++phiter) // loop over photon vector
    {
      // from ootPhoton collection
      phisOOT[iph] = phiter->isOOT_;
      
      // standard photon branches
      phE  [iph] = phiter->photon_.energy();
      phpt [iph] = phiter->photon_.pt();
      phphi[iph] = phiter->photon_.phi();
      pheta[iph] = phiter->photon_.eta();

      // check for HLT filter matches!
      HLTDump::HLTToPATPhotonMatching(iph);

      // super cluster from photon
      const reco::SuperClusterRef& phsc = phiter->photon_.superCluster().isNonnull() ? phiter->photon_.superCluster() : phiter->photon_.parentSuperCluster();
      phscE  [iph] = phsc->energy();
      phsceta[iph] = phsc->eta();
      phscphi[iph] = phsc->phi();

      // Shower Shape Objects
      const reco::Photon::ShowerShape& phshape = phiter->photon_.full5x5_showerShapeVariables(); // phiter->photon_.showerShapeVariables();

      // ID-like variables
      phHOvE   [iph] = phiter->photon_.hadronicOverEm(); // ID
      phHTowOvE[iph] = phiter->photon_.hadTowOverEm(); // close to trigger
      phr9     [iph] = phiter->photon_.r9();

      // pseudo-track veto
      phPixSeed[iph] = phiter->photon_.passElectronVeto();
      phEleVeto[iph] = phiter->photon_.hasPixelSeed();

      // cluster shape variables
      phsieie[iph] = phshape.sigmaIetaIeta;
      phsipip[iph] = phshape.sigmaIphiIphi;
      phsieip[iph] = phshape.sigmaIetaIphi;

      // PF Isolations
      const float sceta = std::abs(phsceta[iph]);
      phChgIso[iph] = std::max(phiter->photon_.chargedHadronIso() - (rho * HLTDump::GetChargedHadronEA(sceta)),0.f);
      phNeuIso[iph] = std::max(phiter->photon_.neutralHadronIso() - (rho * HLTDump::GetNeutralHadronEA(sceta)),0.f);
      phIso   [iph] = std::max(phiter->photon_.photonIso()        - (rho * HLTDump::GetGammaEA        (sceta)),0.f);

      // PF Cluster Isolations
      phPFClEcalIso[iph] = phiter->photon_.ecalPFClusterIso();
      phPFClHcalIso[iph] = phiter->photon_.hcalPFClusterIso();

      // Track Isolation (dR of outer cone < 0.3 as matching in trigger)
      phHollowTkIso[iph] = phiter->photon_.trkSumPtHollowConeDR03();
     
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

void HLTDump::PrepTriggerObjects()
{
  for (auto& triggerObjects : triggerObjectsByFilter)
  {
    std::sort(triggerObjects.begin(),triggerObjects.end(),sortByTrigObjPt);
  }
}

void HLTDump::PrepJets(const edm::Handle<std::vector<pat::Jet> > & jetsH, std::vector<pat::Jet> & jets)
{
  if (jetsH.isValid()) // standard handle check
  {
    for (const auto& jet : *jetsH)
    {
      if (jet.pt() > jetpTmin) jets.push_back(jet);
    }

    std::sort(jets.begin(),jets.end(),sortByJetPt);
  }
}  

void HLTDump::PrepPhotons(const edm::Handle<std::vector<pat::Photon> > & photonsH, 
			  const edm::Handle<std::vector<pat::Photon> > & ootPhotonsH,
			  std::vector<PatPhoton> & photons)
{
  if (photonsH.isValid()) // standard handle check
  {
    int iph = 0;
    photons.resize(photonsH->size());
    for (std::vector<pat::Photon>::const_iterator phiter = photonsH->begin(); phiter != photonsH->end(); ++phiter)
    {
      photons[iph].photon_ = *phiter;
      photons[iph].isOOT_  = false;
      iph++;
    }
  }

  if (ootPhotonsH.isValid()) // standard handle check
  {
    int iph = photons.size();
    photons.resize(photons.size()+ootPhotonsH->size());
    for (std::vector<pat::Photon>::const_iterator phiter = ootPhotonsH->begin(); phiter != ootPhotonsH->end(); ++phiter)
    {
      photons[iph].photon_ = *phiter;
      photons[iph].isOOT_  = true;
      iph++;
    }
  }

  std::sort(photons.begin(),photons.end(),sortByPhotonPt);
  photons.erase(std::remove_if(photons.begin(),photons.end(),
			       [=](const PatPhoton& photon)
			       { return photon.photon_.pt()<phpTmin; }
			       ),photons.end());
}  

void HLTDump::HLTToPATPhotonMatching(const int iph)
{
  for (std::size_t ifilter = 0; ifilter < triggerObjectsByFilter.size(); ifilter++)
  {
    for (std::size_t iobject = 0; iobject < triggerObjectsByFilter[ifilter].size(); iobject++)
    {
      const pat::TriggerObjectStandAlone & triggerObject = triggerObjectsByFilter[ifilter][iobject];
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

void HLTDump::InitializePVBranches()
{
  nvtx = -9999; 
  vtxX = -9999.f; vtxY = -9999.f; vtxZ = -9999.f;
}

void HLTDump::InitializeMETBranches()
{
  t1pfMETpt = -9999.f; t1pfMETphi = -9999.f; t1pfMETsumEt = -9999.f;
}

void HLTDump::ClearJetBranches()
{
  njets = -9999;

  jetE.clear();
  jetpt.clear();
  jetphi.clear();
  jeteta.clear();
  jetidL.clear();
}

void HLTDump::InitializeJetBranches()
{
  jetE.resize(njets);
  jetpt.resize(njets);
  jetphi.resize(njets);
  jeteta.resize(njets);
  jetidL.resize(njets);

  for (int ijet = 0; ijet < njets; ijet++)
  {
    jetE  [ijet] = -9999.f;
    jetpt [ijet] = -9999.f;
    jetphi[ijet] = -9999.f;
    jeteta[ijet] = -9999.f;
    jetidL[ijet] = false;
  }
}

void HLTDump::ClearRecoPhotonBranches()
{
  nphotons = -9999;

  phisOOT.clear();

  phE.clear();
  phpt.clear();
  phphi.clear(); 
  pheta.clear(); 

  phscE.clear(); 
  phsceta.clear(); 
  phscphi.clear(); 

  phHOvE.clear();
  phHTowOvE.clear();
  phr9.clear();

  phPixSeed.clear();
  phEleVeto.clear();

  phChgIso.clear();
  phNeuIso.clear();
  phIso.clear();
  phPFClEcalIso.clear();
  phPFClHcalIso.clear();
  phHollowTkIso.clear();

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
  phisOOT.resize(nphotons);

  phE.resize(nphotons);
  phpt.resize(nphotons);
  phphi.resize(nphotons);
  pheta.resize(nphotons);

  phscE.resize(nphotons);
  phsceta.resize(nphotons);
  phscphi.resize(nphotons);

  phHOvE.resize(nphotons);
  phHTowOvE.resize(nphotons);
  phr9.resize(nphotons);

  phPixSeed.resize(nphotons);
  phEleVeto.resize(nphotons);

  phChgIso.resize(nphotons);
  phNeuIso.resize(nphotons);
  phIso.resize(nphotons);
  phPFClEcalIso.resize(nphotons);
  phPFClHcalIso.resize(nphotons);
  phHollowTkIso.resize(nphotons);

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
    phisOOT[iph] = -9999.f;

    phE  [iph] = -9999.f; 
    phpt [iph] = -9999.f; 
    phphi[iph] = -9999.f; 
    pheta[iph] = -9999.f; 

    phscE  [iph] = -9999.f; 
    phsceta[iph] = -9999.f; 
    phscphi[iph] = -9999.f; 

    phHOvE   [iph] = -9999.f;
    phHTowOvE[iph] = -9999.f;
    phr9     [iph] = -9999.f;

    phPixSeed[iph] = false;
    phEleVeto[iph] = false;

    phChgIso     [iph] = -9999.f;
    phNeuIso     [iph] = -9999.f;
    phIso        [iph] = -9999.f;
    phPFClEcalIso[iph] = -9999.f;
    phPFClHcalIso[iph] = -9999.f;
    phHollowTkIso[iph] = -9999.f;

    phsieie[iph] = -9999.f;
    phsipip[iph] = -9999.f;
    phsieip[iph] = -9999.f;
    phsmaj [iph] = -9999.f;
    phsmin [iph] = -9999.f;
    phalpha[iph] = -9999.f;

    phnrh    [iph] = -9999;

    phIsHLTMatched[iph].resize(filterNames.size());
    for (std::size_t ifilter = 0; ifilter < filterNames.size(); ifilter++)
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
  tree = fs->make<TTree>("tree","tree");

  // Run, Lumi, Event info
  tree->Branch("event"                , &event                , "event/l");
  tree->Branch("run"                  , &run                  , "run/i");
  tree->Branch("lumi"                 , &lumi                 , "lumi/i");
   
  // Trigger Info
  tree->Branch("triggerBits"          , &triggerBits);

  // Vertex info
  tree->Branch("nvtx"                 , &nvtx                 , "nvtx/I");
  tree->Branch("vtxX"                 , &vtxX                 , "vtxX/F");
  tree->Branch("vtxY"                 , &vtxY                 , "vtxY/F");
  tree->Branch("vtxZ"                 , &vtxZ                 , "vtxZ/F");

  // MET info
  tree->Branch("t1pfMETpt"            , &t1pfMETpt            , "t1pfMETpt/F");
  tree->Branch("t1pfMETphi"           , &t1pfMETphi           , "t1pfMETphi/F");
  tree->Branch("t1pfMETsumEt"         , &t1pfMETsumEt         , "t1pfMETsumEt/F");

  // Jet Info
  tree->Branch("njets"                , &njets                , "njets/I");

  tree->Branch("jetE"                 , &jetE);
  tree->Branch("jetpt"                , &jetpt);
  tree->Branch("jetphi"               , &jetphi);
  tree->Branch("jeteta"               , &jeteta);
  tree->Branch("jetidL"               , &jetidL);
   
  // Trigger Object Info
  if (saveTrigObjs)
  {
    tree->Branch("trigobjE"             , &trigobjE);
    tree->Branch("trigobjeta"           , &trigobjeta);
    tree->Branch("trigobjphi"           , &trigobjphi);
    tree->Branch("trigobjpt"            , &trigobjpt);
  }

  // Photon Info
  tree->Branch("nphotons"             , &nphotons             , "nphotons/I");

  tree->Branch("phisOOT"              , &phisOOT);

  tree->Branch("phE"                  , &phE);
  tree->Branch("phpt"                 , &phpt);
  tree->Branch("phphi"                , &phphi);
  tree->Branch("pheta"                , &pheta);

  tree->Branch("phscE"                , &phscE);
  tree->Branch("phsceta"              , &phsceta);
  tree->Branch("phscphi"              , &phscphi);

  tree->Branch("phHOvE"               , &phHOvE);
  tree->Branch("phHTowOvE"            , &phHTowOvE);
  tree->Branch("phr9"                 , &phr9);

  tree->Branch("phPixSeed"            , &phPixSeed);
  tree->Branch("phEleVeto"            , &phEleVeto);

  tree->Branch("phChgIso"             , &phChgIso);
  tree->Branch("phNeuIso"             , &phNeuIso);
  tree->Branch("phIso"                , &phIso);
  tree->Branch("phPFClEcalIso"        , &phPFClEcalIso);
  tree->Branch("phPFClHcalIso"        , &phPFClHcalIso);
  tree->Branch("phHollowTkIso"        , &phHollowTkIso);

  tree->Branch("phsieie"              , &phsieie);
  tree->Branch("phsipip"              , &phsipip);
  tree->Branch("phsieip"              , &phsieip);
  tree->Branch("phsmaj"               , &phsmaj);
  tree->Branch("phsmin"               , &phsmin);
  tree->Branch("phalpha"              , &phalpha);

  tree->Branch("phIsHLTMatched"       , &phIsHLTMatched);

  tree->Branch("phnrh"                , &phnrh);

  tree->Branch("phseedeta"            , &phseedeta);
  tree->Branch("phseedphi"            , &phseedphi);
  tree->Branch("phseedE"              , &phseedE);
  tree->Branch("phseedtime"           , &phseedtime);
  tree->Branch("phseedID"             , &phseedID);
  tree->Branch("phseedOOT"            , &phseedOOT);
}

void HLTDump::endJob() {}

void HLTDump::beginRun(edm::Run const& iRun, edm::EventSetup const& iSetup) {}

void HLTDump::endRun(edm::Run const&, edm::EventSetup const&) {}

void HLTDump::fillDescriptions(edm::ConfigurationDescriptions& descriptions) 
{
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

DEFINE_FWK_MODULE(HLTDump);
