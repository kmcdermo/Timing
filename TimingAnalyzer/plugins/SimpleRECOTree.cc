#include "SimpleRECOTree.h"
#include "FWCore/Utilities/interface/isFinite.h"

SimpleRECOTree::SimpleRECOTree(const edm::ParameterSet& iConfig): 
  // rhos
  rhosTag(iConfig.getParameter<edm::InputTag>("rhos")),

  // photons
  photonsTag  (iConfig.getParameter<edm::InputTag>("photons")),  
  
  // pfcluster isos
  ecalIsoTag  (iConfig.getParameter<edm::InputTag>("ecalIso")),  
  hcalIsoTag  (iConfig.getParameter<edm::InputTag>("hcalIso")),  

  //recHits
  recHitsEBTag(iConfig.getParameter<edm::InputTag>("recHitsEB")),  
  recHitsEETag(iConfig.getParameter<edm::InputTag>("recHitsEE"))
{
  usesResource();
  usesResource("TFileService");

  // rhos
  rhosToken = consumes<double> (rhosTag);

  // photons
  photonsToken = consumes<std::vector<reco::Photon> > (photonsTag);

  // pfcluster isos
  ecalIsoToken = consumes<edm::ValueMap<float> > (ecalIsoTag);
  hcalIsoToken = consumes<edm::ValueMap<float> > (hcalIsoTag);

  // rechits
  recHitsEBToken = consumes<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > (recHitsEBTag);
  recHitsEEToken = consumes<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > (recHitsEETag);
}

SimpleRECOTree::~SimpleRECOTree() {}

void SimpleRECOTree::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) 
{
  // RHOS
  edm::Handle<double> rhosH;
  iEvent.getByToken(rhosToken, rhosH);

  // PHOTONS
  edm::Handle<std::vector<reco::Photon> > photonsH;
  iEvent.getByToken(photonsToken, photonsH);
  std::vector<RecoPhoton> photons(photonsH->size());

  // PFCluster Isolations
  edm::Handle<edm::ValueMap<float> > ecalIsoH;
  iEvent.getByToken(ecalIsoToken, ecalIsoH);
  edm::ValueMap<float> ecalIso = *ecalIsoH;

  edm::Handle<edm::ValueMap<float> > hcalIsoH;
  iEvent.getByToken(hcalIsoToken, hcalIsoH);
  edm::ValueMap<float> hcalIso = *hcalIsoH;

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

  // topology (from PhotonProducer)
  edm::ESHandle<CaloTopology> calotopoH;
  iSetup.get<CaloTopologyRecord>().get(calotopoH);
  const CaloTopology * topology = calotopoH.product();

  // do some prepping of objects
  SimpleRECOTree::PrepPhotons(photonsH,photons,ecalIso,hcalIso);

  ///////////////////////////
  //                       //
  // Event, lumi, run info //
  //                       //
  ///////////////////////////
  run   = iEvent.id().run();
  lumi  = iEvent.luminosityBlock();
  event = iEvent.id().event();

  ///////////////////
  //               //
  // FixedGrid Rho //
  //               //
  ///////////////////

  const float rho = rhosH.isValid() ? *(rhosH.product()) : 0.f;

  //////////////////
  //              //
  // Reco Photons //
  //              //
  //////////////////
  SimpleRECOTree::ClearRecoPhotonBranches();
  if (photonsH.isValid()) // standard handle check
  {
    nphotons = photons.size();
    if (nphotons > 0) SimpleRECOTree::InitializeRecoPhotonBranches();

    int iph = 0;
    for (std::vector<RecoPhoton>::const_iterator phiter = photons.begin(); phiter != photons.end(); ++phiter) // loop over photon vector
    {
      // standard photon branches
      phE  [iph] = phiter->photon.energy();
      phpt [iph] = phiter->photon.pt();
      phphi[iph] = phiter->photon.phi();
      pheta[iph] = phiter->photon.eta();

      // super cluster from photon
      const reco::SuperClusterRef& phsc = phiter->photon.superCluster().isNonnull() ? phiter->photon.superCluster() : phiter->photon.parentSuperCluster();
      phscE  [iph] = phsc->energy();
      phsceta[iph] = phsc->eta();
      phscphi[iph] = phsc->phi();
      const float sceta = std::abs(phsceta[iph]);

      // ID-like variables
      phHoE   [iph] = phiter->photon.hadTowOverEm(); // : hcal energy behind BC in SC (used in slimming), phiter->photon.hadronicOverEm() : total hadronic fraction (used in ID)
      phr9    [iph] = phiter->photon.r9(); // ?? should this be full5x5_r9()?? --> r9() used in slimming (full == noZS)
      phChgIso[iph] = std::max(phiter->photon.chargedHadronIso() - (rho * SimpleRECOTree::GetChargedHadronEA(sceta)),0.f);
      phNeuIso[iph] = std::max(phiter->photon.neutralHadronIso() - (rho * SimpleRECOTree::GetNeutralHadronEA(sceta)),0.f);
      phIso   [iph] = std::max(phiter->photon.photonIso()        - (rho * SimpleRECOTree::GetGammaEA        (sceta)),0.f);

      // use seed to get geometry and recHits
      const reco::CaloClusterPtr& phbc = phsc->seed();
      const DetId seedDetId = phbc->seed(); //seed detid
      const bool isEB = (seedDetId.subdetId() == EcalBarrel); //which subdet
      const EcalRecHitCollection * recHits = (isEB ? recHitsEBH : recHitsEEH).product();

      // need recHits to calculate shape variables
      if (recHits->size() > 0)
      {
	// For now sipip and sieip not in shower shape for standard photon producer
	std::vector<float> localCov = noZS::EcalClusterTools::localCovariances( *phbc, recHits, topology);
	// cluster shape variables
	phsieie[iph] = std::sqrt(localCov[0]);
	phsipip[iph] = (!edm::isFinite(localCov[2]) ? 0. : std::sqrt(localCov[2]));
	phsieip[iph] = localCov[1];

	// 2nd moments from official calculation
	const Cluster2ndMoments ph2ndMoments = noZS::EcalClusterTools::cluster2ndMoments( *phsc, *recHits);
	// radius of semi-major,minor axis is the inverse square root of the eigenvalues of the covariance matrix
	phsmaj [iph] = ph2ndMoments.sMaj;
	phsmin [iph] = ph2ndMoments.sMin;
	phalpha[iph] = ph2ndMoments.alpha;
      }

      // PF Cluster Isolations
      phEcalIso[iph] = phiter->ecalIso;
      phHcalIso[iph] = phiter->hcalIso;

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
      if (phnrh[iph] > 0) SimpleRECOTree::InitializeRecoRecHitBranches(iph);
      int irh = 0;
      for (uiiumap::const_iterator rhiter = phrhIDmap.begin(); rhiter != phrhIDmap.end(); ++rhiter) // loop over only good rec hit ids
      {
	const uint32_t rhID = rhiter->first;
	
	const DetId recHitId(rhID);
	EcalRecHitCollection::const_iterator recHit = recHits->find(recHitId); // get the underlying rechit
	
	const auto recHitPos = isEB ? barrelGeometry->getGeometry(recHitId)->getPosition() : endcapGeometry->getGeometry(recHitId)->getPosition();
	
	// save position, energy, and time of each rechit to a vector
	phrheta [iph][irh] = recHitPos.eta();
	phrhphi [iph][irh] = recHitPos.phi();
	phrhE   [iph][irh] = recHit->energy();
	phrhtime[iph][irh] = recHit->time();
	phrhID  [iph][irh] = int(rhID);
	phrhOOT [iph][irh] = int(recHit->checkFlag(EcalRecHit::kOutOfTime));
	
	// extra info from the SEED
	if (seedDetId.rawId() == recHitId) 
	{ 
	  phseedpos[iph] = irh; // save the position in the vector of the seed 
	  phsuisseX[iph] = EcalTools::swissCross(recHitId,(*recHits),1.f); // http://cmslxr.fnal.gov/source/RecoEcal/EgammaCoreTools/interface/EcalTools.h
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

void SimpleRECOTree::PrepPhotons(const edm::Handle<std::vector<reco::Photon> > & photonsH, std::vector<RecoPhoton> & photons,
				 const edm::ValueMap<float> & ecalIso, const edm::ValueMap<float> & hcalIso)
{
  if (photonsH.isValid()) // standard handle check
  {
    int iph = 0;
    for (std::vector<reco::Photon>::const_iterator phiter = photonsH->begin(); phiter != photonsH->end(); ++phiter)
    {
      // get ptr to photon
      const edm::Ptr<reco::Photon> photonPtr(photonsH, phiter - photonsH->begin());
      
      photons[iph].photon  = (*phiter);
      photons[iph].ecalIso = ecalIso[photonPtr];
      photons[iph].hcalIso = hcalIso[photonPtr];

      iph++;
    }

    std::sort(photons.begin(),photons.end(),sortByPhotonPt);
  }
}  

float SimpleRECOTree::GetChargedHadronEA(const float eta)
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

float SimpleRECOTree::GetNeutralHadronEA(const float eta) 
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

float SimpleRECOTree::GetGammaEA(const float eta) 
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

void SimpleRECOTree::ClearRecoPhotonBranches()
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

  phEcalIso.clear();
  phHcalIso.clear();

  phnrh.clear();
  phseedpos.clear();

  phrheta.clear(); 
  phrhphi.clear(); 
  phrhE.clear(); 
  phrhtime.clear();
  phrhID.clear();
  phrhOOT.clear();
}

void SimpleRECOTree::InitializeRecoPhotonBranches()
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

  phEcalIso.resize(nphotons);
  phHcalIso.resize(nphotons);

  phnrh.resize(nphotons);
  phseedpos.resize(nphotons);

  phrheta.resize(nphotons);
  phrhphi.resize(nphotons);
  phrhE.resize(nphotons);
  phrhtime.resize(nphotons);
  phrhID.resize(nphotons);
  phrhOOT.resize(nphotons);

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

    phEcalIso[iph] = -9999.f;
    phHcalIso[iph] = -9999.f;

    phnrh    [iph] = -9999;
    phseedpos[iph] = -9999;
  }
}

void SimpleRECOTree::InitializeRecoRecHitBranches(const int iph)
{
  phrheta [iph].resize(phnrh[iph]);
  phrhphi [iph].resize(phnrh[iph]);
  phrhE   [iph].resize(phnrh[iph]);
  phrhtime[iph].resize(phnrh[iph]);
  phrhID  [iph].resize(phnrh[iph]);
  phrhOOT [iph].resize(phnrh[iph]);

  for (int irh = 0; irh < phnrh[iph]; irh++)
  {
    phrheta [iph][irh] = -9999.f;
    phrhphi [iph][irh] = -9999.f;
    phrhE   [iph][irh] = -9999.f;
    phrhtime[iph][irh] = -9999.f;
    phrhID  [iph][irh] = -9999;
    phrhOOT [iph][irh] = -9999;
  }
}

void SimpleRECOTree::beginJob() 
{
  edm::Service<TFileService> fs;
  tree = fs->make<TTree>("tree"     , "tree");

  // Run, Lumi, Event info
  tree->Branch("event"                , &event                , "event/l");
  tree->Branch("run"                  , &run                  , "run/i");
  tree->Branch("lumi"                 , &lumi                 , "lumi/i");
   
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

  tree->Branch("phEcalIso"            , &phEcalIso);
  tree->Branch("phHcalIso"            , &phHcalIso);

  tree->Branch("phnrh"                , &phnrh);
  tree->Branch("phseedpos"            , &phseedpos);

  tree->Branch("phrheta"              , &phrheta);
  tree->Branch("phrhphi"              , &phrhphi);
  tree->Branch("phrhE"                , &phrhE);
  tree->Branch("phrhtime"             , &phrhtime);
  tree->Branch("phrhID"               , &phrhID);
  tree->Branch("phrhOOT"              , &phrhOOT);
}

void SimpleRECOTree::endJob() {}

void SimpleRECOTree::beginRun(edm::Run const& iRun, edm::EventSetup const& iSetup) {}

void SimpleRECOTree::endRun(edm::Run const&, edm::EventSetup const&) {}

void SimpleRECOTree::fillDescriptions(edm::ConfigurationDescriptions& descriptions) 
{
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

DEFINE_FWK_MODULE(SimpleRECOTree);
