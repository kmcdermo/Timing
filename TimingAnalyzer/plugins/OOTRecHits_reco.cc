#include "OOTRecHits_reco.h"

OOTRecHits_reco::OOTRecHits_reco(const edm::ParameterSet& iConfig): 
  // trigger info
  triggerResultsTag(iConfig.getParameter<edm::InputTag>("triggerResults")),

  // photon rec hit analyis
  doPhRhs(iConfig.existsAs<bool>("doPhRhs") ? iConfig.getParameter<bool>("doPhRhs") : false),
  // rec hit energy cut
  addrhsInDelR(iConfig.existsAs<bool>("addrhsInDelR") ? iConfig.getParameter<bool>("addrhsInDelR") : false),
  delRcut     (iConfig.existsAs<double>("delRcut")    ? iConfig.getParameter<double>("delRcut")    : 1.0),

  // counting analysis
  doCount(iConfig.existsAs<bool>("doCount") ? iConfig.getParameter<bool>("doCount") : false),
  // rec hit energy cut
  applyrhEcut(iConfig.existsAs<bool>("applyrhEcut") ? iConfig.getParameter<bool>("applyrhEcut") : false),
  rhEcut     (iConfig.existsAs<double>("rhEcut")    ? iConfig.getParameter<double>("rhEcut")    : 1.0),

  photonsTag(iConfig.getParameter<edm::InputTag>("photons")),

  //recHits
  recHitsReducedEBTAG(consumes<EcalRecHitCollection>(iConfig.getParameter<edm::InputTag>( "recHitsReducedEB" ))),
  recHitsReducedEETAG(consumes<EcalRecHitCollection>(iConfig.getParameter<edm::InputTag>( "recHitsReducedEE" ))),
  recHitsFullEBTAG(consumes<EcalRecHitCollection>(iConfig.getParameter<edm::InputTag>( "recHitsFullEB" ))),
  recHitsFullEETAG(consumes<EcalRecHitCollection>(iConfig.getParameter<edm::InputTag>( "recHitsFullEE" )))
{
  usesResource();
  usesResource("TFileService");

  // trigger token
  triggerResultsToken = consumes<edm::TriggerResults> (triggerResultsTag);

  // photon token
  photonsToken = consumes<std::vector<reco::Photon> > (photonsTag);
}

OOTRecHits_reco::~OOTRecHits_reco() {}

void OOTRecHits_reco::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) 
{
  // TRIGGER
  edm::Handle<edm::TriggerResults> triggerResultsH;
  iEvent.getByToken(triggerResultsToken, triggerResultsH);

  // PHOTONS
  edm::Handle<std::vector<reco::Photon> > photonsH;
  iEvent.getByToken(photonsToken, photonsH);

  // ECALELF tools
  EcalClusterLazyTools * reducedtools = new EcalClusterLazyTools (iEvent, iSetup, recHitsReducedEBTAG, recHitsReducedEETAG);
  EcalClusterLazyTools * fulltools    = new EcalClusterLazyTools (iEvent, iSetup, recHitsFullEBTAG   , recHitsFullEETAG   );
  
  // geometry (from ECAL ELF)
  edm::ESHandle<CaloGeometry> geoHandle;
  iSetup.get<CaloGeometryRecord>().get(geoHandle);
  const CaloSubdetectorGeometry * barrelGeometry = geoHandle->getSubdetectorGeometry(DetId::Ecal, EcalBarrel);
  const CaloSubdetectorGeometry * endcapGeometry = geoHandle->getSubdetectorGeometry(DetId::Ecal, EcalEndcap);

  // Event, lumi, run info
  run   = iEvent.id().run();
  lumi  = iEvent.luminosityBlock();
  event = iEvent.id().event();

  // Trigger info
  hltdoubleel33 = false;
  hltdoubleel37 = false;

  // Which triggers fired
  if (triggerResultsH.isValid())
  {
    for (std::size_t i = 0; i < triggerPathsVector.size(); i++) 
    {
      if (triggerPathsMap[triggerPathsVector[i]] == -1) continue;	
      if (i == 0 && triggerResultsH->accept(triggerPathsMap[triggerPathsVector[i]])) hltdoubleel33 = true; // Double electron trigger (33-33)
      if (i == 1 && triggerResultsH->accept(triggerPathsMap[triggerPathsVector[i]])) hltdoubleel37 = true; // Double electron trigger (37-27)
    }
  }

  if (doPhRhs)
  {
    OOTRecHits_reco::PhotonRecHits(photonsH,fulltools,reducedtools,barrelGeometry,endcapGeometry);
  }  

  // Do all the counting!
  if (doCount)
  {
    nphotons = -9999;
    if (photonsH.isValid()) nphotons = photonsH->size();

    OOTRecHits_reco::ReducedToFullEB(reducedtools,fulltools);
    OOTRecHits_reco::ReducedToFullEE(reducedtools,fulltools);

    OOTRecHits_reco::FullToReducedEB(fulltools,reducedtools);
    OOTRecHits_reco::FullToReducedEE(fulltools,reducedtools);

    // fill the counting tree
    countingtree->Fill();    
  }

  delete reducedtools;
  delete fulltools;
}

void OOTRecHits_reco::PhotonRecHits(edm::Handle<std::vector<reco::Photon> > & photonsH, 
				    EcalClusterLazyTools *& fulltools, EcalClusterLazyTools *& reducedtools,
				    const CaloSubdetectorGeometry *& barrelGeometry, const CaloSubdetectorGeometry *& endcapGeometry)
{
  if (photonsH.isValid()) // standard handle check
  {
    for (std::vector<reco::Photon>::const_iterator phiter = photonsH->begin(); phiter != photonsH->end(); ++phiter) // loop over photon vector
    {
      OOTRecHits_reco::InitializePhotonBranches();
      // standard photon branches
      phE   = phiter->energy();
      phpt  = phiter->pt();
      phphi = phiter->phi();
      pheta = phiter->eta();

      // super cluster from photon
      const reco::SuperClusterRef& phsc = phiter->superCluster().isNonnull() ? phiter->superCluster() : phiter->parentSuperCluster();
      if (phsc.isNonnull()) // check to make sure supercluster is good
      {
  	phscE   = phsc->energy();
  	phscphi = phsc->position().phi();
  	phsceta = phsc->position().eta();

  	// use seed to get geometry and recHits
  	const DetId seedDetId = phsc->seed()->seed(); //seed detid
  	const bool isEB = (seedDetId.subdetId() == EcalBarrel); //which subdet

	//////////////////
	//              //
  	// Full recHits //
	//              //
	//////////////////
  	const EcalRecHitCollection * frecHits = isEB ? fulltools->getEcalEBRecHitCollection() : fulltools->getEcalEERecHitCollection();

  	// map of rec hit ids
  	uiiumap phfrhIDmap;

  	// all rechits in superclusters
  	const DetIdPairVec fhitsAndFractions = phsc->hitsAndFractions();
  	for (DetIdPairVec::const_iterator hafitr = fhitsAndFractions.begin(); hafitr != fhitsAndFractions.end(); ++hafitr) // loop over all rec hits in SC
  	{
  	  const DetId recHitId = hafitr->first; // get detid of crystal
  	  EcalRecHitCollection::const_iterator recHit = frecHits->find(recHitId); // get the underlying rechit
  	  if (recHit != frecHits->end()) // standard check
	  {
	    phfrhIDmap[recHitId.rawId()]++;	    
  	  } // end standard check recHit
  	} // end loop over hits and fractions

  	phnfrhs = phfrhIDmap.size();
  	if (phnfrhs > 0) OOTRecHits_reco::InitializeFullRecHitBranches();
  	int ifrh = 0;
  	for (uiiumap::const_iterator rhiter = phfrhIDmap.begin(); rhiter != phfrhIDmap.end(); ++rhiter) // loop over only good rec hit ids
        {
  	  const uint32_t rhId = rhiter->first;
  	  const DetId recHitId(rhId);
  	  EcalRecHitCollection::const_iterator recHit = frecHits->find(recHitId); // get the underlying rechit
	  
  	  const auto recHitPos = isEB ? barrelGeometry->getGeometry(recHitId)->getPosition() : endcapGeometry->getGeometry(recHitId)->getPosition();

  	  // save position, energy, and time of each rechit to a vector
  	  phfrhEs   [ifrh] = recHit->energy();
  	  phfrhphis [ifrh] = recHitPos.phi();
  	  phfrhetas [ifrh] = recHitPos.eta();
  	  phfrhdelRs[ifrh] = deltaR(phfrhphis[ifrh],phfrhetas[ifrh],phphi,pheta);
  	  phfrhtimes[ifrh] = recHit->time();
  	  phfrhIDs  [ifrh] = int(rhId);
  	  phfrhOOTs [ifrh] = int(recHit->checkFlag(EcalRecHit::kOutOfTime));
	  
  	  // save the position in the vector of the seed 
  	  if (seedDetId.rawId() == rhId) { phfseedpos = ifrh; }

  	  ifrh++; // increment rechit counter
  	} // end loop over rec hit id map
      
	// add extra hits if possible from full collection
	if (addrhsInDelR)
	{
	  int add = 0;
	  for (EcalRecHitCollection::const_iterator recHit = frecHits->begin(); recHit != frecHits->end(); ++recHit)
	  {
	    // first get detector id
	    const DetId recHitId = recHit->detid();
	    const uint32_t rhId  = recHitId.rawId();

	    // check previous map to make sure to not double count recHit ids!
	    if (phfrhIDmap.count(rhId)) continue; 
		
	    // get position to see if it is within delR <= cut_value of photon
	    const auto recHitPos = isEB ? barrelGeometry->getGeometry(recHitId)->getPosition() : endcapGeometry->getGeometry(recHitId)->getPosition();
	    const float rhphi  = recHitPos.phi();
	    const float rheta  = recHitPos.eta();
	    const float rhdelR = deltaR(phphi,pheta,rhphi,rheta);
	    if (rhdelR < delRcut) 
	    {
	      // can add directly to the end of the phrh vector (seed pos already determined)
	      phfrhEs   .push_back(recHit->energy());
	      phfrhphis .push_back(rhphi);
	      phfrhetas .push_back(rheta);
	      phfrhdelRs.push_back(rhdelR);
	      phfrhtimes.push_back(recHit->time());
	      phfrhIDs  .push_back(int(rhId));
	      phfrhOOTs .push_back(int(recHit->checkFlag(EcalRecHit::kOutOfTime)));

	      // increment counter for total number of additional hits
	      add++;
	    } // end check over matching delR 
	    phnfrhs_add = add;
	  } // end loop over all rec hits in collection, woof!
	} // end block over looking at additional rec hits in full collection

	/////////////////////
	//                 //
  	// Reduced recHits //
	//                 //
	/////////////////////
  	const EcalRecHitCollection * rrecHits = isEB ? reducedtools->getEcalEBRecHitCollection() : reducedtools->getEcalEERecHitCollection();

  	// map of rec hit ids
  	uiiumap phrrhIDmap;

  	// all rechits in superclusters
  	const DetIdPairVec rhitsAndFractions = phsc->hitsAndFractions();
  	for (DetIdPairVec::const_iterator hafitr = rhitsAndFractions.begin(); hafitr != rhitsAndFractions.end(); ++hafitr) // loop over all rec hits in SC
  	{
  	  const DetId recHitId = hafitr->first; // get detid of crystal
  	  EcalRecHitCollection::const_iterator recHit = rrecHits->find(recHitId); // get the underlying rechit
  	  if (recHit != rrecHits->end()) // standard check
          { 
	    phrrhIDmap[recHitId.rawId()]++;	    
	  } // end standard check recHit
  	} // end loop over hits and fractions
      
  	phnrrhs = phrrhIDmap.size();
  	if (phnrrhs > 0) OOTRecHits_reco::InitializeReducedRecHitBranches();
  	int irrh = 0;
  	for (uiiumap::const_iterator rhiter = phrrhIDmap.begin(); rhiter != phrrhIDmap.end(); ++rhiter) // loop over only good rec hit ids
        {
  	  const uint32_t rhId = rhiter->first;
  	  const DetId recHitId(rhId);
  	  EcalRecHitCollection::const_iterator recHit = rrecHits->find(recHitId); // get the underlying rechit
	  
  	  const auto recHitPos = isEB ? barrelGeometry->getGeometry(recHitId)->getPosition() : endcapGeometry->getGeometry(recHitId)->getPosition();

  	  // save position, energy, and time of each rechit to a vector
  	  phrrhEs   [irrh] = recHit->energy();
  	  phrrhphis [irrh] = recHitPos.phi();
  	  phrrhetas [irrh] = recHitPos.eta();
  	  phrrhdelRs[irrh] = deltaR(phrrhphis[irrh],phrrhetas[irrh],phphi,pheta);
  	  phrrhtimes[irrh] = recHit->time();
  	  phrrhIDs  [irrh] = int(rhId);
  	  phrrhOOTs [irrh] = int(recHit->checkFlag(EcalRecHit::kOutOfTime));
	  
  	  // save the position in the vector of the seed 
  	  if (seedDetId.rawId() == rhId) { phrseedpos = irrh; }

  	  irrh++; // increment rechit counter
  	} // end loop over rec hit id map

	// add extra hits if possible from full collection
	if (addrhsInDelR)
	{
	  int add = 0;
	  for (EcalRecHitCollection::const_iterator recHit = rrecHits->begin(); recHit != rrecHits->end(); ++recHit)
	  {
	    // first get detector id
	    const DetId recHitId = recHit->detid();
	    const uint32_t rhId  = recHitId.rawId();

	    // check previous map to make sure to not double count recHit ids!
	    if (phrrhIDmap.count(rhId)) continue; 
		
	    // get position to see if it is within delR <= cut_value of photon
	    const auto recHitPos = isEB ? barrelGeometry->getGeometry(recHitId)->getPosition() : endcapGeometry->getGeometry(recHitId)->getPosition();
	    const float rhphi  = recHitPos.phi();
	    const float rheta  = recHitPos.eta();
	    const float rhdelR = deltaR(phphi,pheta,rhphi,rheta);
	    if (rhdelR < delRcut) 
	    {
	      // can add directly to the end of the phrh vector (seed pos already determined)
	      phrrhEs   .push_back(recHit->energy());
	      phrrhphis .push_back(rhphi);
	      phrrhetas .push_back(rheta);
	      phrrhdelRs.push_back(rhdelR);
	      phrrhtimes.push_back(recHit->time());
	      phrrhIDs  .push_back(int(rhId));
	      phrrhOOTs .push_back(int(recHit->checkFlag(EcalRecHit::kOutOfTime)));

	      // increment counter for total number of additional hits
	      add++;
	    } // end check over matching delR 
	    phnrrhs_add = add;
	  } // end loop over all rec hits in collection, woof!
	} // end block over looking at additional rec hits in reduced collection
      } // end check over super cluster
      phrhtree->Fill();
    } // end loop over photon vector
  } // end check over photon handle valid
}

void OOTRecHits_reco::InitializePhotonBranches()
{
  phE   = -9999.f;
  phpt  = -9999.f;
  phphi = -9999.f;
  pheta = -9999.f;
  
  phscphi = -9999.f;
  phsceta = -9999.f;
  phscE   = -9999.f;

  // full hits
  phnfrhs     = -9999;
  phnfrhs_add = -9999;
  
  phfrhEs   .clear();
  phfrhphis .clear();
  phfrhetas .clear();
  phfrhdelRs.clear();
  phfrhtimes.clear();
  phfrhIDs  .clear();
  phfrhOOTs .clear();
	  
  phfseedpos = -9999;

  // reduced hits
  phnrrhs     = -9999;
  phnrrhs_add = -9999;
  
  phrrhEs   .clear();
  phrrhphis .clear();
  phrrhetas .clear();
  phrrhdelRs.clear();
  phrrhtimes.clear();
  phrrhIDs  .clear();
  phrrhOOTs .clear();
	  
  phrseedpos = -9999;
}

void OOTRecHits_reco::InitializeFullRecHitBranches()
{
  phfrhEs   .resize(phnfrhs);
  phfrhphis .resize(phnfrhs);
  phfrhetas .resize(phnfrhs);
  phfrhdelRs.resize(phnfrhs);
  phfrhtimes.resize(phnfrhs);
  phfrhIDs  .resize(phnfrhs);
  phfrhOOTs .resize(phnfrhs);

  for (int irh = 0; irh < phnfrhs; irh++)
  {
    phfrhEs   [irh] = -9999.f;
    phfrhphis [irh] = -9999.f;
    phfrhetas [irh] = -9999.f;
    phfrhdelRs[irh] = -9999.f;
    phfrhtimes[irh] = -9999.f;
    phfrhIDs  [irh] = -9999;
    phfrhOOTs [irh] = -9999;
  }
}

void OOTRecHits_reco::InitializeReducedRecHitBranches()
{
  phrrhEs   .resize(phnrrhs);
  phrrhphis .resize(phnrrhs);
  phrrhetas .resize(phnrrhs);
  phrrhdelRs.resize(phnrrhs);
  phrrhtimes.resize(phnrrhs);
  phrrhIDs  .resize(phnrrhs);
  phrrhOOTs .resize(phnrrhs);

  for (int irh = 0; irh < phnrrhs; irh++)
  {
    phrrhEs   [irh] = -9999.f;
    phrrhphis [irh] = -9999.f;
    phrrhetas [irh] = -9999.f;
    phrrhdelRs[irh] = -9999.f;
    phrrhtimes[irh] = -9999.f;
    phrrhIDs  [irh] = -9999;
    phrrhOOTs [irh] = -9999;
  }
}

void OOTRecHits_reco::ReducedToFullEB(EcalClusterLazyTools *& reducedtools, EcalClusterLazyTools *& fulltools)
{
  nReducedEB = 0; nR2FMatchedEB = 0; nReducedOOTEB = 0; nR2FMatchedOOTEB = 0;

  const EcalRecHitCollection * reducedRecHitsEB = reducedtools->getEcalEBRecHitCollection();
  for (EcalRecHitCollection::const_iterator irecHit = reducedRecHitsEB->begin(); irecHit != reducedRecHitsEB->end(); ++irecHit)
  {
    if (applyrhEcut && irecHit->energy() < rhEcut) continue;

    bool matched = false;

    const EcalRecHitCollection * fullRecHitsEB = fulltools->getEcalEBRecHitCollection();
    for (EcalRecHitCollection::const_iterator jrecHit = fullRecHitsEB->begin(); jrecHit != fullRecHitsEB->end(); ++jrecHit)
    {
      if (irecHit->detid().rawId() == jrecHit->detid().rawId())
      { 
  	matched = true;
  	break;
      }
    }
      
    if (matched) 
    {
      nR2FMatchedEB++;
      if (irecHit->checkFlag(EcalRecHit::kOutOfTime) == 1) 
      {
	nReducedOOTEB++; 
	nR2FMatchedOOTEB++;
      }
    }
    else 
    {
      if (irecHit->checkFlag(EcalRecHit::kOutOfTime) == 1) 
      {
	nReducedOOTEB++;
      }
    }
    nReducedEB++;
  }
}

void OOTRecHits_reco::ReducedToFullEE(EcalClusterLazyTools *& reducedtools, EcalClusterLazyTools *& fulltools)
{
  nReducedEE = 0; nR2FMatchedEE = 0; nReducedOOTEE = 0; nR2FMatchedOOTEE = 0;
  
  const EcalRecHitCollection * reducedRecHitsEE = reducedtools->getEcalEERecHitCollection();
  for (EcalRecHitCollection::const_iterator irecHit = reducedRecHitsEE->begin(); irecHit != reducedRecHitsEE->end(); ++irecHit)
  {
    if (applyrhEcut && irecHit->energy() < rhEcut) continue;

    bool matched = false;

    const EcalRecHitCollection * fullRecHitsEE = fulltools->getEcalEERecHitCollection();
    for (EcalRecHitCollection::const_iterator jrecHit = fullRecHitsEE->begin(); jrecHit != fullRecHitsEE->end(); ++jrecHit)
    {
      if (irecHit->detid().rawId() == jrecHit->detid().rawId())
      { 
  	matched = true;
  	break;
      }
    }
      
    if (matched) 
    {
      nR2FMatchedEE++;
      if (irecHit->checkFlag(EcalRecHit::kOutOfTime) == 1) 
      {
	nReducedOOTEE++; 
	nR2FMatchedOOTEE++;
      }
    }
    else 
    {
      if (irecHit->checkFlag(EcalRecHit::kOutOfTime) == 1) 
      {
	nReducedOOTEE++;
      }
    }
    nReducedEE++;
  }
}

void OOTRecHits_reco::FullToReducedEB(EcalClusterLazyTools *& fulltools, EcalClusterLazyTools *& reducedtools)
{
  nFullEB = 0; nF2RMatchedEB = 0; nFullOOTEB = 0; nF2RMatchedOOTEB = 0;

  const EcalRecHitCollection * fullRecHitsEB = fulltools->getEcalEBRecHitCollection();
  for (EcalRecHitCollection::const_iterator irecHit = fullRecHitsEB->begin(); irecHit != fullRecHitsEB->end(); ++irecHit)
  {
    if (applyrhEcut && irecHit->energy() < rhEcut) continue;

    bool matched = false;

    const EcalRecHitCollection * reducedRecHitsEB = reducedtools->getEcalEBRecHitCollection();
    for (EcalRecHitCollection::const_iterator jrecHit = reducedRecHitsEB->begin(); jrecHit != reducedRecHitsEB->end(); ++jrecHit)
    {
      if (irecHit->detid().rawId() == jrecHit->detid().rawId())
      { 
  	matched = true;
  	break;
      }
    }
      
    if (matched) 
    {
      nF2RMatchedEB++;
      if (irecHit->checkFlag(EcalRecHit::kOutOfTime) == 1) 
      {
	nFullOOTEB++; 
	nF2RMatchedOOTEB++;
      }
    }
    else 
    {
      if (irecHit->checkFlag(EcalRecHit::kOutOfTime) == 1) 
      {
	nFullOOTEB++;
      }
    }
    nFullEB++;
  }
}

void OOTRecHits_reco::FullToReducedEE(EcalClusterLazyTools *& fulltools, EcalClusterLazyTools *& reducedtools)
{
  nFullEE = 0; nF2RMatchedEE = 0; nFullOOTEE = 0; nF2RMatchedOOTEE = 0;

  const EcalRecHitCollection * fullRecHitsEE = fulltools->getEcalEERecHitCollection();
  for (EcalRecHitCollection::const_iterator irecHit = fullRecHitsEE->begin(); irecHit != fullRecHitsEE->end(); ++irecHit)
  {
    if (applyrhEcut && irecHit->energy() < rhEcut) continue;

    bool matched = false;

    const EcalRecHitCollection * reducedRecHitsEE = reducedtools->getEcalEERecHitCollection();
    for (EcalRecHitCollection::const_iterator jrecHit = reducedRecHitsEE->begin(); jrecHit != reducedRecHitsEE->end(); ++jrecHit)
    {
      if (irecHit->detid().rawId() == jrecHit->detid().rawId())
      { 
  	matched = true;
  	break;
      }
    }
      
    if (matched) 
    {
      nF2RMatchedEE++;
      if (irecHit->checkFlag(EcalRecHit::kOutOfTime) == 1) 
      {
	nFullOOTEE++; 
	nF2RMatchedOOTEE++;
      }
    }
    else 
    {
      if (irecHit->checkFlag(EcalRecHit::kOutOfTime) == 1) 
      {
	nFullOOTEE++;
      }
    }
    nFullEE++;
  }
}

void OOTRecHits_reco::beginJob() 
{
  edm::Service<TFileService> fs;

  // phrh tree
  phrhtree = fs->make<TTree>("phrhtree", "tree");

  // event level info
  phrhtree->Branch("event"                , &event                , "event/I");
  phrhtree->Branch("run"                  , &run                  , "run/I");
  phrhtree->Branch("lumi"                 , &lumi                 , "lumi/I");
  phrhtree->Branch("hltdouble33"          , &hltdouble33          , "hltdouble33/O");
  phrhtree->Branch("hltdouble37"          , &hltdouble37          , "hltdouble37/O");

  phrhtree->Branch("phE"                  , &phE                  , "phE/F");
  phrhtree->Branch("phpt"                 , &phpt                 , "phpt/F");
  phrhtree->Branch("phphi"                , &phphi                , "phphi/F");
  phrhtree->Branch("pheta"                , &pheta                , "pheta/F");

  phrhtree->Branch("phscE"                , &phscE                , "phscE/F");
  phrhtree->Branch("phscphi"              , &phscphi              , "phscphi/F");
  phrhtree->Branch("phsceta"              , &phsceta              , "phsceta/F");

  phrhtree->Branch("phnfrhs"              , &phnfrhs              , "phnfrhs/I");
  phrhtree->Branch("phnfrhs_add"          , &phnfrhs_add          , "phnfrhs_add/I");
  phrhtree->Branch("phfrhEs"              , &phfrhEs);
  phrhtree->Branch("phfrhphis"            , &phfrhphis);
  phrhtree->Branch("phfrhetas"            , &phfrhetas);
  phrhtree->Branch("phfrhdelRs"           , &phfrhdelRs);
  phrhtree->Branch("phfrhtimes"           , &phfrhtimes);
  phrhtree->Branch("phfrhIDs"             , &phfrhIDs);
  phrhtree->Branch("phfrhOOTs"            , &phfrhOOTs);
  phrhtree->Branch("phfseedpos"           , &phfseedpos           , "phfseedpos/I");

  phrhtree->Branch("phnrrhs"              , &phnrrhs              , "phnrrhs/I");
  phrhtree->Branch("phnrrhs_add"          , &phnrrhs_add          , "phnrrhs_add/I");
  phrhtree->Branch("phrrhEs"              , &phrrhEs);
  phrhtree->Branch("phrrhphis"            , &phrrhphis);
  phrhtree->Branch("phrrhetas"            , &phrrhetas);
  phrhtree->Branch("phrrhdelRs"           , &phrrhdelRs);
  phrhtree->Branch("phrrhtimes"           , &phrrhtimes);
  phrhtree->Branch("phrrhIDs"             , &phrrhIDs);
  phrhtree->Branch("phrrhOOTs"            , &phrrhOOTs);
  phrhtree->Branch("phrseedpos"           , &phrseedpos           , "phrseedpos/I");

  // counting tree
  countingtree = fs->make<TTree>("countingtree", "tree");

  countingtree->Branch("event"                , &event                , "event/I");
  countingtree->Branch("run"                  , &run                  , "run/I");
  countingtree->Branch("lumi"                 , &lumi                 , "lumi/I");
  countingtree->Branch("hltdouble33"          , &hltdouble33          , "hltdouble33/O");
  countingtree->Branch("hltdouble37"          , &hltdouble37          , "hltdouble37/O");
  countingtree->Branch("nphotons"             , &nphotons             , "nphotons/I");

  countingtree->Branch("nReducedEB"           , &nReducedEB           , "nReducedEB/I");
  countingtree->Branch("nReducedOOTEB"        , &nReducedOOTEB        , "nReducedOOTEB/I");
  countingtree->Branch("nR2FMatchedEB"        , &nR2FMatchedEB        , "nR2FMatchedEB/I");
  countingtree->Branch("nR2FMatchedOOTEB"     , &nR2FMatchedOOTEB     , "nR2FMatchedOOTEB/I");

  countingtree->Branch("nReducedEE"           , &nReducedEE           , "nReducedEE/I");
  countingtree->Branch("nReducedOOTEE"        , &nReducedOOTEE        , "nReducedOOTEE/I");
  countingtree->Branch("nR2FMatchedEE"        , &nR2FMatchedEE        , "nR2FMatchedEE/I");
  countingtree->Branch("nR2FMatchedOOTEE"     , &nR2FMatchedOOTEE     , "nR2FMatchedOOTEE/I");

  countingtree->Branch("nFullEB"              , &nFullEB              , "nFullEB/I");
  countingtree->Branch("nFullOOTEB"           , &nFullOOTEB           , "nFullOOTEB/I");
  countingtree->Branch("nF2RMatchedEB"        , &nF2RMatchedEB        , "nF2RMatchedEB/I");
  countingtree->Branch("nF2RMatchedOOTEB"     , &nF2RMatchedOOTEB     , "nF2RMatchedOOTEB/I");

  countingtree->Branch("nFullEE"              , &nFullEE              , "nFullEE/I");
  countingtree->Branch("nFullOOTEE"           , &nFullOOTEE           , "nFullOOTEE/I");
  countingtree->Branch("nF2RMatchedEE"        , &nF2RMatchedEE        , "nF2RMatchedEE/I");
  countingtree->Branch("nF2RMatchedOOTEE"     , &nF2RMatchedOOTEE     , "nF2RMatchedOOTEE/I");
}

void OOTRecHits_reco::endJob() {}

void OOTRecHits_reco::beginRun(edm::Run const& iRun, edm::EventSetup const& iSetup) {}

void OOTRecHits_reco::endRun(edm::Run const&, edm::EventSetup const&) 
{
  triggerPathsVector.push_back("HLT_DoubleEle33_CaloIdL_GsfTrkIdVL_v");
  triggerPathsVector.push_back("HLT_DoubleEle37_Ele27_CaloIdL_GsfTrkIdVL");
  
  HLTConfigProvider hltConfig;
  bool changedConfig = false;
  hltConfig.init(iRun, iSetup, triggerResultsTag.process(), changedConfig);
  
  for (size_t i = 0; i < triggerPathsVector.size(); i++) 
  {
    triggerPathsMap[triggerPathsVector[i]] = -1;
  }
  
  for(size_t i = 0; i < triggerPathsVector.size(); i++)
  {
    TPRegexp pattern(triggerPathsVector[i]);
    for(size_t j = 0; j < hltConfig.triggerNames().size(); j++)
    {
      std::string pathName = hltConfig.triggerNames()[j];
      if(TString(pathName).Contains(pattern))
      {
	triggerPathsMap[triggerPathsVector[i]] = j;
      }
    }
  }
}

void OOTRecHits_reco::fillDescriptions(edm::ConfigurationDescriptions& descriptions) 
{
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

DEFINE_FWK_MODULE(OOTRecHits_reco);
