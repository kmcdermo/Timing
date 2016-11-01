#include "OOTRecHits_mAD.h"

OOTRecHits_mAD::OOTRecHits_mAD(const edm::ParameterSet& iConfig): 
  // rec hit energy cut
  addrhsInDelR(iConfig.existsAs<bool>("addrhsInDelR") ? iConfig.getParameter<bool>("addrhsInDelR") : false),
  delRcut     (iConfig.existsAs<double>("delRcut")    ? iConfig.getParameter<double>("delRcut")    : 1.0),

  // trigger info
  triggerResultsTag(iConfig.getParameter<edm::InputTag>("triggerResults")),

  // photons
  photonLooseIdMapTag (iConfig.getParameter<edm::InputTag>("loosePhotonID")),  
  photonMediumIdMapTag(iConfig.getParameter<edm::InputTag>("mediumPhotonID")),  
  photonTightIdMapTag (iConfig.getParameter<edm::InputTag>("tightPhotonID")),  
  photonsTag(iConfig.getParameter<edm::InputTag>("photons")),

  //recHits
  recHitsReducedEBTAG(consumes<EcalRecHitCollection>(iConfig.getParameter<edm::InputTag>( "recHitsReducedEB" ))),
  recHitsReducedEETAG(consumes<EcalRecHitCollection>(iConfig.getParameter<edm::InputTag>( "recHitsReducedEE" )))
{
  usesResource();
  usesResource("TFileService");

  // trigger token
  triggerResultsToken = consumes<edm::TriggerResults> (triggerResultsTag);
  
  // photon tokens
  photonLooseIdMapToken  = consumes<edm::ValueMap<bool> > (photonLooseIdMapTag);
  photonMediumIdMapToken = consumes<edm::ValueMap<bool> > (photonMediumIdMapTag);
  photonTightIdMapToken  = consumes<edm::ValueMap<bool> > (photonTightIdMapTag);
  photonsToken = consumes<std::vector<pat::Photon> > (photonsTag);
}

OOTRecHits_mAD::~OOTRecHits_mAD() {}

void OOTRecHits_mAD::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) 
{
  // TRIGGER
  edm::Handle<edm::TriggerResults> triggerResultsH;
  iEvent.getByToken(triggerResultsToken, triggerResultsH);

  // PHOTONS + IDS
  edm::Handle<edm::ValueMap<bool> > photonLooseIdMapH;
  iEvent.getByToken(photonLooseIdMapToken, photonLooseIdMapH);
  edm::ValueMap<bool> photonLooseIdMap = *photonLooseIdMapH;

  edm::Handle<edm::ValueMap<bool> > photonMediumIdMapH;
  iEvent.getByToken(photonMediumIdMapToken, photonMediumIdMapH);
  edm::ValueMap<bool> photonMediumIdMap = *photonMediumIdMapH;

  edm::Handle<edm::ValueMap<bool> > photonTightIdMapH;
  iEvent.getByToken(photonTightIdMapToken, photonTightIdMapH);
  edm::ValueMap<bool> photonTightIdMap = *photonTightIdMapH;

  edm::Handle<std::vector<pat::Photon> > photonsH;
  iEvent.getByToken(photonsToken, photonsH);

  // ECALELF tools
  EcalClusterLazyTools * clustertools = new EcalClusterLazyTools (iEvent, iSetup, recHitsReducedEBTAG, recHitsReducedEETAG);
  
  // geometry (from ECAL ELF)
  edm::ESHandle<CaloGeometry> geoHandle;
  iSetup.get<CaloGeometryRecord>().get(geoHandle);
  const CaloSubdetectorGeometry * barrelGeometry = geoHandle->getSubdetectorGeometry(DetId::Ecal, EcalBarrel);
  const CaloSubdetectorGeometry * endcapGeometry = geoHandle->getSubdetectorGeometry(DetId::Ecal, EcalEndcap);

  // Event, lumi, run info
  run   = iEvent.id().run();
  lumi  = iEvent.luminosityBlock();
  event = iEvent.id().event();

  if (lumi != 135 && lumi != 136 && lumi != 406 && lumi != 407) return;

  // Trigger info
  hltdoubleph60 = false;

  // Which triggers fired
  if (triggerResultsH.isValid())
  {
    for (std::size_t i = 0; i < triggerPathsVector.size(); i++) 
    {
      if (triggerPathsMap[triggerPathsVector[i]] == -1) continue;	
      if (i == 0 && triggerResultsH->accept(triggerPathsMap[triggerPathsVector[i]])) hltdoubleph60 = true; // Double photon trigger (60-60)
    }
  }

  if (photonsH.isValid()) // standard handle check
  {
    for (std::vector<pat::Photon>::const_iterator phiter = photonsH->begin(); phiter != photonsH->end(); ++phiter) // loop over photon vector
    {
      OOTRecHits_mAD::InitializePhotonBranches();
      // standard photon branches
      phE   = phiter->energy();
      phpt  = phiter->pt();
      phphi = phiter->phi();
      pheta = phiter->eta();

      // Get the VID of the photon
      const edm::Ptr<pat::Photon> photonPtr(photonsH, phiter - photonsH->begin());

      // loose > medium > tight
      if (photonLooseIdMap [photonPtr]) {phVID = 1;}
      if (photonMediumIdMap[photonPtr]) {phVID = 2;}
      if (photonTightIdMap [photonPtr]) {phVID = 3;}

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

  	const EcalRecHitCollection * recHits = isEB ? clustertools->getEcalEBRecHitCollection() : clustertools->getEcalEERecHitCollection();

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
      
  	phnrhs = phrhIDmap.size();
  	if (phnrhs > 0) OOTRecHits_mAD::InitializeRecHitBranches();
  	int irh = 0;
  	for (uiiumap::const_iterator rhiter = phrhIDmap.begin(); rhiter != phrhIDmap.end(); ++rhiter) // loop over only good rec hit ids
        {
  	  const uint32_t rhId = rhiter->first;
  	  const DetId recHitId(rhId);
  	  EcalRecHitCollection::const_iterator recHit = recHits->find(recHitId); // get the underlying rechit
	  
  	  const auto recHitPos = isEB ? barrelGeometry->getGeometry(recHitId)->getPosition() : endcapGeometry->getGeometry(recHitId)->getPosition();

  	  // save position, energy, and time of each rechit to a vector
  	  phrhEs   [irh] = recHit->energy();
  	  phrhphis [irh] = recHitPos.phi();
  	  phrhetas [irh] = recHitPos.eta();
  	  phrhdelRs[irh] = deltaR(phrhphis[irh],phrhetas[irh],phphi,pheta);
  	  phrhtimes[irh] = recHit->time();
  	  phrhIDs  [irh] = int(rhId);
  	  phrhOOTs [irh] = int(recHit->checkFlag(EcalRecHit::kOutOfTime));
	  
  	  // save the position in the vector of the seed 
  	  if (seedDetId.rawId() == rhId) { phseedpos = irh; }

  	  irh++; // increment rechit counter
  	} // end loop over rec hit id map

	// add extra hits if possible from full collection
	if (addrhsInDelR)
	{
	  int add = 0;
	  for (EcalRecHitCollection::const_iterator recHit = recHits->begin(); recHit != recHits->end(); ++recHit)
	  {
	    // first get detector id
	    const DetId recHitId = recHit->detid();
	    const uint32_t rhId  = recHitId.rawId();

	    // check previous map to make sure to not double count recHit ids!
	    if (phrhIDmap.count(rhId)) continue; 
		
	    // get position to see if it is within delR <= cut_value of photon
	    const auto recHitPos = isEB ? barrelGeometry->getGeometry(recHitId)->getPosition() : endcapGeometry->getGeometry(recHitId)->getPosition();
	    const float rhphi  = recHitPos.phi();
	    const float rheta  = recHitPos.eta();
	    const float rhdelR = deltaR(phphi,pheta,rhphi,rheta);
	    if (rhdelR < delRcut) 
	    {
	      // can add directly to the end of the phrh vector (seed pos already determined)
	      phrhEs   .push_back(recHit->energy());
	      phrhphis .push_back(rhphi);
	      phrhetas .push_back(rheta);
	      phrhdelRs.push_back(rhdelR);
	      phrhtimes.push_back(recHit->time());
	      phrhIDs  .push_back(int(rhId));
	      phrhOOTs .push_back(int(recHit->checkFlag(EcalRecHit::kOutOfTime)));

	      // increment counter for total number of additional hits
	      add++;
	    } // end check over matching delR 
	    phnrhs_add = add;
	  } // end loop over all rec hits in collection, woof!
	} // end block over looking at additional rec hits in reduced collection
      } // end check over super cluster
      phrhtree->Fill();
    } // end loop over photon vector
  } // end check over photon handle valid

  delete clustertools;
}

void OOTRecHits_mAD::InitializePhotonBranches()
{
  phE   = -9999.f;
  phpt  = -9999.f;
  phphi = -9999.f;
  pheta = -9999.f;
  phVID = -9999;

  phscphi = -9999.f;
  phsceta = -9999.f;
  phscE   = -9999.f;

  // rec hits
  phnrhs     = -9999;
  phnrhs_add = -9999;
  
  phrhEs   .clear();
  phrhphis .clear();
  phrhetas .clear();
  phrhdelRs.clear();
  phrhtimes.clear();
  phrhIDs  .clear();
  phrhOOTs .clear();
	  
  phseedpos = -9999;
}

void OOTRecHits_mAD::InitializeRecHitBranches()
{
  phrhEs   .resize(phnrhs);
  phrhphis .resize(phnrhs);
  phrhetas .resize(phnrhs);
  phrhdelRs.resize(phnrhs);
  phrhtimes.resize(phnrhs);
  phrhIDs  .resize(phnrhs);
  phrhOOTs .resize(phnrhs);

  for (int irh = 0; irh < phnrhs; irh++)
  {
    phrhEs   [irh] = -9999.f;
    phrhphis [irh] = -9999.f;
    phrhetas [irh] = -9999.f;
    phrhdelRs[irh] = -9999.f;
    phrhtimes[irh] = -9999.f;
    phrhIDs  [irh] = -9999;
    phrhOOTs [irh] = -9999;
  }
}

void OOTRecHits_mAD::beginJob() 
{
  edm::Service<TFileService> fs;

  // phrh tree
  phrhtree = fs->make<TTree>("phrhtree", "tree");

  phrhtree->Branch("event"                , &event                , "event/I");
  phrhtree->Branch("run"                  , &run                  , "run/I");
  phrhtree->Branch("lumi"                 , &lumi                 , "lumi/I");
  phrhtree->Branch("hltdoubleph60"        , &hltdoubleph60        , "hltdoubleph60/O");

  phrhtree->Branch("phE"                  , &phE                  , "phE/F");
  phrhtree->Branch("phpt"                 , &phpt                 , "phpt/F");
  phrhtree->Branch("phphi"                , &phphi                , "phphi/F");
  phrhtree->Branch("pheta"                , &pheta                , "pheta/F");
  phrhtree->Branch("phVID"                , &phVID                , "phVID/I");

  phrhtree->Branch("phscE"                , &phscE                , "phscE/F");
  phrhtree->Branch("phscphi"              , &phscphi              , "phscphi/F");
  phrhtree->Branch("phsceta"              , &phsceta              , "phsceta/F");

  phrhtree->Branch("phnrhs"               , &phnrhs               , "phnrhs/I");
  phrhtree->Branch("phnrhs_add"           , &phnrhs_add           , "phnrhs_add/I");
  phrhtree->Branch("phrhEs"               , &phrhEs);
  phrhtree->Branch("phrhphis"             , &phrhphis);
  phrhtree->Branch("phrhetas"             , &phrhetas);
  phrhtree->Branch("phrhdelRs"            , &phrhdelRs);
  phrhtree->Branch("phrhtimes"            , &phrhtimes);
  phrhtree->Branch("phrhIDs"              , &phrhIDs);
  phrhtree->Branch("phrhOOTs"             , &phrhOOTs);
  phrhtree->Branch("phseedpos"            , &phseedpos            , "phseedpos/I");
}

void OOTRecHits_mAD::endJob() {}

void OOTRecHits_mAD::beginRun(edm::Run const& iRun, edm::EventSetup const& iSetup) 
{
  triggerPathsVector.push_back("HLT_DoublePhoton60_v");
  
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

void OOTRecHits_mAD::endRun(edm::Run const&, edm::EventSetup const&) {}

void OOTRecHits_mAD::fillDescriptions(edm::ConfigurationDescriptions& descriptions) 
{
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

DEFINE_FWK_MODULE(OOTRecHits_mAD);
