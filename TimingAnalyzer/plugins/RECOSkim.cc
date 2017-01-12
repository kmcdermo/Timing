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
  metsToken = consumes<std::vector<reco::PFMET> > (metsTag);

  // jets
  jetsToken = consumes<std::vector<reco::PFJet> > (jetsTag);

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
  edm::Handle<std::vector<reco::PFMET> > metsH;
  iEvent.getByToken(metsToken, metsH);

  // JETS
  edm::Handle<std::vector<reco::PFJet> > jetsH;
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

  const float rho = rhosH.isValid() ? *(rhosH.product()) : 0.f;

  //////////////////
  //              //
  // Type1 PF Met //
  //              //
  //////////////////
  RECOSkim::InitializeMETBranches();
  if (metsH.isValid())
  {
    const reco::PFMET & t1pfMET = (*metsH)[0];

    // Type1 PF MET (corrected)
    t1pfMETpt    = t1pfMET.pt();
    t1pfMETphi   = t1pfMET.phi();
    t1pfMETsumEt = t1pfMET.sumEt();
  }

  ////////////////////
  //                //
  // Jets (AK4 CHS) //
  //                //
  ////////////////////
  RECOSkim::ClearJetBranches();
  if (jetsH.isValid()) // check to make sure reco jets exist
  {
    njets = jetsH->size() >= 5 ? 5 : jetsH->size(); // save a max of 5 jets
    if (njets > 0) RECOSkim::InitializeJetBranches();
    int ijet = 0;
    for (std::vector<reco::PFJet>::const_iterator jetiter = jetsH->begin(); jetiter != jetsH->end(); ++jetiter)
    {
      if (ijet == njets) break;

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
      const float phsceta = std::abs(phsc->eta());
      phscE[iph] = phsc->energy();

      // Shower Shape Objects
      const reco::Photon::ShowerShape& phshape = phiter->full5x5_showerShapeVariables(); // phiter->showerShapeVariables();

      // standard photon branches
      phE  [iph] = phiter->energy();
      phpt [iph] = phiter->pt();
      phphi[iph] = phiter->phi();
      pheta[iph] = phiter->eta();

      // ID-like variables
      phHoE   [iph] = phiter->hadronicOverEm(); // phiter->hadTowOverEm();
      phsieie [iph] = phshape.sigmaIetaIeta;
      phr9    [iph] = phshape.e3x3/phsc->rawEnergy();
      phChgIso[iph] = phiter->chargedHadronIso() - (rho * RECOSkim::GetChargedHadronEA(phsceta));
      phNeuIso[iph] = phiter->neutralHadronIso() - (rho * RECOSkim::GetNeutralHadronEA(phsceta));
      phIso   [iph] = phiter->photonIso()        - (rho * RECOSkim::GetGammaEA        (phsceta));

      // cluster shape variables
      const float see  = phshape.sigmaIetaIeta;
      const float spp  = phshape.sigmaIphiIphi;
      const float sep  = phshape.sigmaIetaIphi;
      const float disc = std::sqrt((spp-see)*(spp-see)+4.f*sep*sep);
      phsmaj[iph] = (spp+see+disc)/2.f;
      phsmin[iph] = (spp+see-disc)/2.f;
	
      /////////////////
      //             //
      // RecHit Info //
      //             //
      /////////////////
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
      
      phnrh    [iph] = phrhIDmap.size();
      phnrhEcut[iph] = 0;
      phnrhOOT [iph] = 0;
      for (uiiumap::const_iterator rhiter = phrhIDmap.begin(); rhiter != phrhIDmap.end(); ++rhiter) // loop over only good rec hit ids
      {
	const DetId recHitId(rhiter->first);
	EcalRecHitCollection::const_iterator recHit = recHits->find(recHitId); // get the underlying rechit

	// count rec hits in photon passing E cut and OOT flag
	if (recHit->energy() < 1.f)
	{
	  phnrhEcut[iph]++;
	  if (recHit->checkFlag(EcalRecHit::kOutOfTime))
	  { 
	    phnrhOOT[iph]++;
	  }
	}

	// seed rechit info 
	if (seedDetId.rawId() == recHitId) 
	{ 
	  const auto recHitPos = isEB ? barrelGeometry->getGeometry(recHitId)->getPosition() : endcapGeometry->getGeometry(recHitId)->getPosition();
	  phseedphi [iph] = recHitPos.phi();
	  phseedeta [iph] = recHitPos.eta();
	  phseedE   [iph] = recHit->energy();
	  phseedtime[iph] = recHit->time();
	  phseedOOT [iph] = int(recHit->checkFlag(EcalRecHit::kOutOfTime));
	} // end check over seed rechit
      } // end loop over rec hit id map

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

float RECOSkim::GetChargedHadronEA(const float eta)
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

float RECOSkim::GetNeutralHadronEA(const float eta) 
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

float RECOSkim::GetGammaEA(const float eta) 
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

  phscE.clear(); 

  phE.clear();
  phpt.clear();
  phphi.clear(); 
  pheta.clear(); 
  
  phHoE.clear();
  phsieie.clear();
  phr9.clear();
  phChgIso.clear();
  phNeuIso.clear();
  phIso.clear();

  phsmaj.clear();
  phsmin.clear();
  
  phnrh.clear();
  phnrhEcut.clear();
  phnrhOOT.clear();
  phseedphi.clear();
  phseedeta.clear(); 
  phseedE.clear(); 
  phseedtime.clear();
  phseedOOT.clear();
}

void RECOSkim::InitializeRecoPhotonBranches()
{
  phscE.resize(nphotons);

  phE.resize(nphotons);
  phpt.resize(nphotons);
  phphi.resize(nphotons);
  pheta.resize(nphotons);

  phHoE.resize(nphotons);
  phsieie.resize(nphotons);
  phr9.resize(nphotons);
  phChgIso.resize(nphotons);
  phNeuIso.resize(nphotons);
  phIso.resize(nphotons);

  phsmaj.resize(nphotons);
  phsmin.resize(nphotons);
  
  phnrh.resize(nphotons);
  phnrhEcut.resize(nphotons);
  phnrhOOT.resize(nphotons);
  phseedphi.resize(nphotons);
  phseedeta.resize(nphotons);
  phseedE.resize(nphotons);
  phseedtime.resize(nphotons);
  phseedOOT.resize(nphotons);

  for (int iph = 0; iph < nphotons; iph++)
  {
    phscE[iph] = -9999.f; 

    phE   [iph] = -9999.f; 
    phpt  [iph] = -9999.f; 
    phphi [iph] = -9999.f; 
    pheta [iph] = -9999.f; 
    
    phHoE   [iph] = -9999.f;
    phsieie [iph] = -9999.f;
    phr9    [iph] = -9999.f;
    phChgIso[iph] = -9999.f;
    phNeuIso[iph] = -9999.f;
    phIso   [iph] = -9999.f;

    phsmaj[iph] = -9999.f;
    phsmin[iph] = -9999.f;
    
    phnrh     [iph] = -9999;
    phnrhEcut [iph] = -9999;
    phnrhOOT  [iph] = -9999;
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

  tree->Branch("phscE"                , &phscE);

  tree->Branch("phE"                  , &phE);
  tree->Branch("phpt"                 , &phpt);
  tree->Branch("phphi"                , &phphi);
  tree->Branch("pheta"                , &pheta);

  tree->Branch("phHoE"                , &phHoE);
  tree->Branch("phsieie"              , &phsieie);
  tree->Branch("phr9"                 , &phr9);
  tree->Branch("phChgIso"             , &phChgIso);
  tree->Branch("phNeuIso"             , &phNeuIso);
  tree->Branch("phIso"                , &phIso);

  tree->Branch("phsmaj"               , &phsmaj);
  tree->Branch("phsmin"               , &phsmin);

  tree->Branch("phnrh"                , &phnrh);
  tree->Branch("phnrhEcut"            , &phnrhEcut);
  tree->Branch("phnrhOOT"             , &phnrhOOT);
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
