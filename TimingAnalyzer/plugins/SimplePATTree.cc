#include "SimplePATTree.h"
#include "FWCore/Utilities/interface/isFinite.h"

SimplePATTree::SimplePATTree(const edm::ParameterSet& iConfig): 
  //photons
  photonsTag   (iConfig.getParameter<edm::InputTag>("photons")),  
  ootphotonsTag(iConfig.getParameter<edm::InputTag>("ootphotons")),  
  
  //recHits
  recHitsEBTag(iConfig.getParameter<edm::InputTag>("recHitsEB")),  
  recHitsEETag(iConfig.getParameter<edm::InputTag>("recHitsEE"))
{
  usesResource();
  usesResource("TFileService");

  // photons
  photonsToken    = consumes<std::vector<pat::Photon> > (photonsTag);
  ootphotonsToken = consumes<std::vector<pat::Photon> > (ootphotonsTag);

  // rechits
  recHitsEBToken = consumes<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > (recHitsEBTag);
  recHitsEEToken = consumes<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > (recHitsEETag);
}

SimplePATTree::~SimplePATTree() {}

void SimplePATTree::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) 
{
  // PHOTONS
  edm::Handle<std::vector<pat::Photon> > photonsH;
  iEvent.getByToken(photonsToken, photonsH);
  std::vector<PatPhoton> photons(photonsH->size());

  edm::Handle<std::vector<pat::Photon> > ootphotonsH;
  iEvent.getByToken(ootphotonsToken, ootphotonsH);
  std::vector<PatPhoton> ootphotons(ootphotonsH->size());

  // RecHits
  edm::Handle<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > recHitsEBH;
  iEvent.getByToken(recHitsEBToken, recHitsEBH);
  edm::Handle<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > recHitsEEH;
  iEvent.getByToken(recHitsEEToken, recHitsEEH);

  // do some prepping of objects
  SimplePATTree::PrepPhotons(photonsH,photons,false);
  SimplePATTree::PrepPhotons(ootphotonsH,ootphotons,true);
  photons.insert(photons.end(), ootphotons.begin(), ootphotons.end());

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
  // Reco Photons //
  //              //
  //////////////////
  SimplePATTree::ClearRecoPhotonBranches();
  if (photonsH.isValid()) // standard handle check
  {
    nphotons = photons.size();
    if (nphotons > 0) SimplePATTree::InitializeRecoPhotonBranches();

    int iph = 0;
    for (std::vector<PatPhoton>::const_iterator phiter = photons.begin(); phiter != photons.end(); ++phiter) // loop over photon vector
    {
      // which photon collection
      phIsOOT[iph] = phiter->isOOT_;

      // standard photon branches
      phE  [iph] = phiter->photon.energy();
      phpt [iph] = phiter->photon.pt();
      phphi[iph] = phiter->photon.phi();
      pheta[iph] = phiter->photon.eta();

      // ID-like variables
      phHoE  [iph] = phiter->photon.hadTowOverEm();
      phr9   [iph] = phiter->photon.r9();
      phsieie[iph] = phiter->photon.full5x5_sigmaIetaIeta();

      // super cluster from photon
      const reco::SuperClusterRef& phsc = phiter->photon.superCluster().isNonnull() ? phiter->photon.superCluster() : phiter->photon.parentSuperCluster();
      const reco::CaloClusterPtr& phbc = phsc->seed();
      const DetId seedDetId = phbc->seed(); //seed detid

      const EcalRecHitCollection * recHits = nullptr;
      if      (seedDetId.subdetId() == EcalBarrel) 
      {
	phIsEB[iph] = 1;
	recHits = recHitsEBH.product();
      }
      else if (seedDetId.subdetId() == EcalEndcap) 
      {
	phIsEB[iph] = 0;
	recHits = recHitsEEH.product();
      }

      // need recHits to calculate shape variables
      if (recHits != nullptr)
      {
	// 2nd moments from official calculation
	const Cluster2ndMoments ph2ndMoments = noZS::EcalClusterTools::cluster2ndMoments( *phsc, *recHits);
	// radius of semi-major,minor axis is the inverse square root of the eigenvalues of the covariance matrix
	phsmaj [iph] = ph2ndMoments.sMaj;
	phsmin [iph] = ph2ndMoments.sMin;
	phalpha[iph] = ph2ndMoments.alpha;
      }

      // PF Cluster Isolations
      phPFClEcalIso[iph] = phiter->photon.ecalPFClusterIso();
      phPFClHcalIso[iph] = phiter->photon.hcalPFClusterIso();

      // Track Isolation (dR of outer cone < 0.3 as matching in trigger)
      phHollowTkIso[iph] = phiter->photon.trkSumPtHollowConeDR03();

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
      int irh = 0;
      for (uiiumap::const_iterator rhiter = phrhIDmap.begin(); rhiter != phrhIDmap.end(); ++rhiter) // loop over only good rec hit ids
      {
	const uint32_t rhID = rhiter->first;
	
	const DetId recHitId(rhID);
	EcalRecHitCollection::const_iterator recHit = recHits->find(recHitId); // get the underlying rechit
	
	// extra info from the SEED
	if (seedDetId.rawId() == recHitId) 
	{ 
	  phseedE   [iph] = recHit->energy();
	  phseedtime[iph] = recHit->time();
	  phseedOOT [iph] = int(recHit->checkFlag(EcalRecHit::kOutOfTime));
	}
	
	irh++; // increment rechit counter
      } // end loop over rec hit id map
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

void SimplePATTree::PrepPhotons(const edm::Handle<std::vector<pat::Photon> > & photonsH, std::vector<PatPhoton> & photons, const bool isOOT)
{
  if (photonsH.isValid()) // standard handle check
  {
    int iph = 0;
    for (std::vector<pat::Photon>::const_iterator phiter = photonsH->begin(); phiter != photonsH->end(); ++phiter)
    {
      photons[iph].photon = (*phiter);
      photons[iph].isOOT_ = isOOT;
      iph++;
    }

    std::sort(photons.begin(),photons.end(),sortByPhotonPt);
  }
}  

void SimplePATTree::ClearRecoPhotonBranches()
{
  nphotons = -1;

  phIsOOT.clear();
  phIsEB.clear();

  phE.clear();
  phpt.clear();
  phphi.clear(); 
  pheta.clear(); 

  phHoE.clear();
  phr9.clear();
  phsieie.clear();

  phsmaj.clear();
  phsmin.clear();
  phalpha.clear();

  phPFClEcalIso.clear();
  phPFClHcalIso.clear();
  phHollowTkIso.clear();

  phnrh.clear();

  phseedE.clear(); 
  phseedtime.clear();
  phseedOOT.clear();
}

void SimplePATTree::InitializeRecoPhotonBranches()
{
  phIsOOT.resize(nphotons);
  phIsEB.resize(nphotons);
  
  phE.resize(nphotons);
  phpt.resize(nphotons);
  phphi.resize(nphotons);
  pheta.resize(nphotons);
  
  phHoE.resize(nphotons);
  phr9.resize(nphotons);
  phsieie.resize(nphotons);

  phsmaj.resize(nphotons);
  phsmin.resize(nphotons);
  phalpha.resize(nphotons);

  phPFClEcalIso.resize(nphotons);
  phPFClHcalIso.resize(nphotons);
  phHollowTkIso.resize(nphotons);

  phnrh.resize(nphotons);

  phseedE.resize(nphotons);
  phseedtime.resize(nphotons);
  phseedOOT.resize(nphotons);

  for (int iph = 0; iph < nphotons; iph++)
  {
    phIsOOT[iph] = -1;
    phIsEB [iph] = -1;

    phE  [iph] = -9999.f; 
    phpt [iph] = -9999.f; 
    phphi[iph] = -9999.f; 
    pheta[iph] = -9999.f; 

    phHoE  [iph] = -9999.f;
    phr9   [iph] = -9999.f;
    phsieie[iph] = -9999.f;

    phsmaj [iph] = -9999.f;
    phsmin [iph] = -9999.f;
    phalpha[iph] = -9999.f;

    phPFClEcalIso[iph] = -9999.f;
    phPFClHcalIso[iph] = -9999.f;
    phHollowTkIso[iph] = -9999.f;

    phnrh[iph] = -1;

    phseedE   [iph] = -9999.f;
    phseedtime[iph] = -9999.f;
    phseedOOT [iph] = -1;
  }
}

void SimplePATTree::beginJob() 
{
  edm::Service<TFileService> fs;
  tree = fs->make<TTree>("tree"     , "tree");

  // Run, Lumi, Event info
  tree->Branch("event"                , &event                , "event/l");
  tree->Branch("run"                  , &run                  , "run/i");
  tree->Branch("lumi"                 , &lumi                 , "lumi/i");
   
  // Photon Info
  tree->Branch("nphotons"             , &nphotons             , "nphotons/I");

  tree->Branch("phIsOOT"              , &phIsOOT);
  tree->Branch("phIsEB"               , &phIsEB);

  tree->Branch("phE"                  , &phE);
  tree->Branch("phpt"                 , &phpt);
  tree->Branch("phphi"                , &phphi);
  tree->Branch("pheta"                , &pheta);

  tree->Branch("phHoE"                , &phHoE);
  tree->Branch("phr9"                 , &phr9);
  tree->Branch("phsieie"              , &phsieie);

  tree->Branch("phsmaj"               , &phsmaj);
  tree->Branch("phsmin"               , &phsmin);
  tree->Branch("phalpha"              , &phalpha);

  tree->Branch("phPFClEcalIso"        , &phPFClEcalIso);
  tree->Branch("phPFClHcalIso"        , &phPFClHcalIso);
  tree->Branch("phHollowTkIso"        , &phHollowTkIso);

  tree->Branch("phnrh"                , &phnrh);

  tree->Branch("phseedE"              , &phseedE);
  tree->Branch("phseedtime"           , &phseedtime);
  tree->Branch("phseedOOT"            , &phseedOOT);
}

void SimplePATTree::endJob() {}

void SimplePATTree::beginRun(edm::Run const& iRun, edm::EventSetup const& iSetup) {}

void SimplePATTree::endRun(edm::Run const&, edm::EventSetup const&) {}

void SimplePATTree::fillDescriptions(edm::ConfigurationDescriptions& descriptions) 
{
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

DEFINE_FWK_MODULE(SimplePATTree);
