#include "RECOSkim.h"

RECOSkim::RECOSkim(const edm::ParameterSet& iConfig): 
  // vertexes
  verticesTag(iConfig.getParameter<edm::InputTag>("vertices")),

  // rhos
  rhosTag(iConfig.getParameter<edm::InputTag>("rhos")),

  // mets
  metsTag(iConfig.getParameter<edm::InputTag>("mets")),  

  // jets
  jetsTag(iConfig.getParameter<edm::InputTag>("jets")),  

  // photons
  photonsTag(iConfig.getParameter<edm::InputTag>("photons")),  

  //recHits
  recHitCollectionEBToken(consumes<EcalRecHitCollection>(iConfig.getParameter<edm::InputTag>("recHitCollectionEB"))),
  recHitCollectionEEToken(consumes<EcalRecHitCollection>(iConfig.getParameter<edm::InputTag>("recHitCollectionEE")))
{
  usesResource();
  usesResource("TFileService");

  // vertices
  verticesToken = consumes<std::vector<reco::Vertex> > (verticesTag);

  // rhos
  rhosToken = consumes<double> (rhosTag);

  // mets
  metsToken = consumes<std::vector<reco::MET> > (metsTag);

  // jets
  jetsToken = consumes<std::vector<reco::Jet> > (jetsTag);

  // photons 
  photonsToken = consumes<std::vector<reco::Photon> > (photonsTag);
}

RECOSkim::~RECOSkim() {}

void RECOSkim::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) 
{
  // VERTICES
  edm::Handle<std::vector<reco::Vertex> > verticesH;
  iEvent.getByToken(verticesToken, verticesH);
  
  // RHOS
  edm::Handle<double> rhosH;
  iEvent.getByToken(rhosToken, rhosH);

  // MET
  edm::Handle<std::vector<reco::MET> > metsH;
  iEvent.getByToken(metsToken, metsH);

  // JETS
  edm::Handle<std::vector<reco::Jet> > jetsH;
  iEvent.getByToken(jetsToken, jetsH);

  // PHOTONS
  edm::Handle<std::vector<reco::Photon> > photonsH;
  iEvent.getByToken(photonsToken, photonsH);

  // geometry (from ECAL ELF)
  edm::ESHandle<CaloGeometry> calogeoH;
  iSetup.get<CaloGeometryRecord>().get(calogeoH);
  const CaloSubdetectorGeometry * barrelGeometry = calogeoH->getSubdetectorGeometry(DetId::Ecal, EcalBarrel);
  const CaloSubdetectorGeometry * endcapGeometry = calogeoH->getSubdetectorGeometry(DetId::Ecal, EcalEndcap);

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
  RECOSkim::InitializePVBranches();
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

  const float rho = rhosH->product();

  //////////////////
  //              //
  // Type1 PF Met //
  //              //
  //////////////////
  RECOSkim::InitializeMETBranches();
  if (metsH.isValid())
  {
    const reco::MET & t1pfMET = (*metsH)[0];

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
  RECOSkim::ClearJetBranches();
  if (jetsH.isValid()) // check to make sure reco jets exist
  {
    njets = jetsH->size();
    if (njets > 0) RECOSkim::InitializeJetBranches();
    int ijet = 0;
    for (std::vector<reco::Jet>::const_iterator jetiter = jetsH->begin(); jetiter != jetsH->end(); ++jetiter)
    {
      jetE  [ijet] = jetiter->energy();
      jetpt [ijet] = jetiter->pt();
      jetphi[ijet] = jetiter->phi();
      jeteta[ijet] = jetiter->eta();

      ijet++;
    } // end loop over reco jets
  } // end check over reco jets
  
  //////////////////
  //              //
  // Reco Photons //
  //              //
  //////////////////
  RECOSkim::ClearRecoPhotonBranches();
  if (photonsH.isValid()) // standard handle check
  {
    // ECALELF tools
    EcalClusterLazyTools * clustertools = new EcalClusterLazyTools (iEvent, iSetup, recHitCollectionEBToken, recHitCollectionEEToken);

    nphotons = photonsH->size();
    if (nphotons > 0) RECOSkim::InitializeRecoPhotonBranches();

    int iph = 0;
    for (std::vector<reco::Photon>::const_iterator phiter = photonsH->begin(); phiter != photonsH->end(); ++phiter) // loop over photon vector
    {
      // super cluster from photon
      const reco::SuperClusterRef& phsc = phiter->superCluster().isNonnull() ? phiter->superCluster() : phiter->parentSuperCluster();

      // Shower Shape Objects
      const ShowerShape& phshape = phiter->full5x5_showerShapeVariables(); // phiter->showerShapeVariables();

      // standard photon branches
      phE  [iph] = phiter->energy();
      phpt [iph] = phiter->pt();
      phphi[iph] = phiter->phi();
      pheta[iph] = phiter->eta();

      // ID-like variables
      phHoE  [iph] = phiter->hadronicOverEm(); // phiter->hadTowOverEm();
      phsieie[iph] = phshape.sigmaIetaIeta;
      phr9   [iph] = phshape.e3x3/phsc->rawEnergy();

      
      // cluster shape variables
      const float see = phshape.sigmaIetaIeta;
      const float spp = phshape.sigmaIphiIphi;
      const float sep = phshape.sigmaIetaIphi;
      const float disc = std::sqrt((spp-see)*(spp-see)+4.f*sep*sep);
      phsmaj[iph] = (spp+see+disc)/2.f;
      phsmin[iph] = (spp+see-disc)/2.f;
	
      if (phsc.isNonnull()) // check to make sure supercluster is good
      {
	phscE[iph] = phsc->energy();

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
      
	phnrh[iph] = phrhIDmap.size();
	for (uiiumap::const_iterator rhiter = phrhIDmap.begin(); rhiter != phrhIDmap.end(); ++rhiter) // loop over only good rec hit ids
        {
	  const DetId recHitId(rhiter->first);
	  // seed rechit info 
	  if (seedDetId.rawId() == recHitId) 
	  { 
	    EcalRecHitCollection::const_iterator recHit = recHits->find(recHitId); // get the underlying rechit
	    const auto recHitPos = isEB ? barrelGeometry->getGeometry(recHitId)->getPosition() : endcapGeometry->getGeometry(recHitId)->getPosition();
	    phseedphi [iph] = recHitPos.phi();
	    phseedeta [iph] = recHitPos.eta();
	    phseedE   [iph] = recHit->energy();
	    phseedtime[iph] = recHit->time();
	    phseedOOT [iph] = int(recHit->checkFlag(EcalRecHit::kOutOfTime));
	  } // end check over seed rechit
	} // end loop over rec hit id map
      } // end check over super cluster
      iph++; // increment photon counter
    } // end loop over photon vector
    delete clustertools; // delete cluster tools once done with loop over photons
  } // end check over photon handle valid

  ///////////////
  //           //
  // Fill Tree //
  //           //
  ///////////////
  tree->Fill();      
}    

void RECOSkim::InitializePVBranches()
{
  nvtx = -9999; 
  vtxX = -9999.f; vtxY = -9999.f; vtxZ = -9999.f;
}

void RECOSkim::InitializeMETBranches()
{
  t1pfMETpt = -9999.f; t1pfMETphi = -9999.f; t1pfMETsumEt = -9999.f;
}

void RECOSkim::ClearJetBranches()
{
  njets = -9999;

  jetE.clear();
  jetpt.clear();
  jetphi.clear();
  jeteta.clear();
}

void RECOSkim::InitializeJetBranches()
{
  jetE.resize(njets);
  jetpt.resize(njets);
  jetphi.resize(njets);
  jeteta.resize(njets);
  
  for (int ijet = 0; ijet < njets; ijet++)
  {
    jetE  [ijet] = -9999.f;
    jetpt [ijet] = -9999.f;
    jetphi[ijet] = -9999.f;
    jeteta[ijet] = -9999.f;
  }
}

void RECOSkim::ClearRecoPhotonBranches()
{
  nphotons = -9999;

  phE.clear();
  phpt.clear();
  phphi.clear(); 
  pheta.clear(); 
  
  phHoE.clear();
  phsieie.clear();
  phr9.clear();
  phsmaj.clear();
  phsmin.clear();
  
  phscE.clear(); 

  phnrh.clear();
  phseedphi.clear();
  phseedeta.clear(); 
  phseedE.clear(); 
  phseedtime.clear();
  phseedOOT.clear();
}

void RECOSkim::InitializeRecoPhotonBranches()
{
  phE.resize(nphotons);
  phpt.resize(nphotons);
  phphi.resize(nphotons);
  pheta.resize(nphotons);

  phHoE.resize(nphotons);
  phsieie.resize(nphotons);
  phr9.resize(nphotons);
  phsmaj.resize(nphotons);
  phsmin.resize(nphotons);
  
  phscE.resize(nphotons);

  phnrh.resize(nphotons);
  phseedphi.resize(nphotons);
  phseedeta.resize(nphotons);
  phseedE.resize(nphotons);
  phseedtime.resize(nphotons);
  phseedOOT.resize(nphotons);

  for (int iph = 0; iph < nphotons; iph++)
  {
    phE   [iph] = -9999.f; 
    phpt  [iph] = -9999.f; 
    phphi [iph] = -9999.f; 
    pheta [iph] = -9999.f; 
    
    phhOe  [iph] = -9999.f;
    phsieie[iph] = -9999.f;
    phr9   [iph] = -9999.f;
    phsmaj [iph] = -9999.f;
    phsmin [iph] = -9999.f;
    
    phscE[iph] = -9999.f; 

    phnrh     [iph] = -9999;
    phseedphi [iph] = -9999.f;
    phseedeta [iph] = -9999.f;
    phseedE   [iph] = -9999.f;
    phseedtime[iph] = -9999.f;
    phseedOOT [iph] = -9999;
  }
}

void RECOSkim::beginJob() 
{
  edm::Service<TFileService> fs;
  tree = fs->make<TTree>("tree"     , "tree");

  // Run, Lumi, Event info
  tree->Branch("event"                , &event                , "event/I");
  tree->Branch("run"                  , &run                  , "run/I");
  tree->Branch("lumi"                 , &lumi                 , "lumi/I");

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
   
  // Photon Info
  tree->Branch("nphotons"             , &nphotons             , "nphotons/I");
  tree->Branch("phE"                  , &phE);
  tree->Branch("phpt"                 , &phpt);
  tree->Branch("phphi"                , &phphi);
  tree->Branch("pheta"                , &pheta);

  tree->Branch("phHoE"                , &phHoE);
  tree->Branch("phsieie"              , &phsieie);
  tree->Branch("phr9"                 , &phr9);
  tree->Branch("phsmaj"               , &phsmaj);
  tree->Branch("phsmin"               , &phsmin);

  tree->Branch("phscE"                , &phscE);

  tree->Branch("phnrh"                , &phnrh);
  tree->Branch("phseedphi"            , &phseedphi);
  tree->Branch("phseedeta"            , &phseedeta);
  tree->Branch("phseedE"              , &phseedE);
  tree->Branch("phseedtime"           , &phseedtime);
  tree->Branch("phseedOOT"            , &phseedOOT);
}

void RECOSkim::endJob() {}

void RECOSkim::beginRun(edm::Run const& iRun, edm::EventSetup const& iSetup) {}

void RECOSkim::endRun(edm::Run const&, edm::EventSetup const&) {}

void RECOSkim::fillDescriptions(edm::ConfigurationDescriptions& descriptions) 
{
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

DEFINE_FWK_MODULE(RECOSkim);
