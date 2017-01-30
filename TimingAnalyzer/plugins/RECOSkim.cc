#include "RECOSkim.h"

RECOSkim::RECOSkim(const edm::ParameterSet& iConfig): 
  // vertexes
  verticesTag(iConfig.getParameter<edm::InputTag>("vertices")),

  // rhos
  rhosTag(iConfig.getParameter<edm::InputTag>("rhos")),

  // trigger info
  triggersTag(iConfig.getParameter<edm::InputTag>("triggers")),

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

  // trigger
  triggersToken = consumes<edm::TriggerResults> (triggersTag);

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

  // TRIGGERS
  edm::Handle<edm::TriggerResults> triggersH;
  iEvent.getByToken(triggersToken, triggersH);

  // MET
  edm::Handle<std::vector<reco::PFMET> > metsH;
  iEvent.getByToken(metsToken, metsH);

  // JETS
  edm::Handle<std::vector<reco::PFJet> > jetsH;
  iEvent.getByToken(jetsToken, jetsH);
  std::vector<reco::PFJet> jets = *jetsH;

  // PHOTONS
  edm::Handle<std::vector<reco::Photon> > photonsH;
  iEvent.getByToken(photonsToken, photonsH);
  std::vector<reco::Photon> photons = *photonsH;
  
  // geometry (from ECAL ELF)
  edm::ESHandle<CaloGeometry> caloGeoH;
  iSetup.get<CaloGeometryRecord>().get(caloGeoH);
  const CaloSubdetectorGeometry * barrelGeometry = caloGeoH->getSubdetectorGeometry(DetId::Ecal, EcalBarrel);
  const CaloSubdetectorGeometry * endcapGeometry = caloGeoH->getSubdetectorGeometry(DetId::Ecal, EcalEndcap);

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
  // Trigger Info //
  //              //
  //////////////////

  hltdoubleph = false;

  // Which triggers fired
  if (triggersH.isValid())
  {
    for (std::size_t i = 0; i < triggerPathsVector.size(); i++) 
    {
      if (triggerPathsMap[triggerPathsVector[i]] == -1) continue;	
      if (i == 0 && triggersH->accept(triggerPathsMap[triggerPathsVector[i]])) hltdoubleph = true; // Double photon trigger: 36, 22
    }
  }

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
  RECOSkim::PrepJets(jetsH,jets);
  if (jetsH.isValid()) // check to make sure reco jets exist
  {
    njets = jets.size(); // already trimmed to top 5 pT jets
    if (njets > 0) RECOSkim::InitializeJetBranches();
    int ijet = 0;
    for (std::vector<reco::PFJet>::const_iterator jetiter = jets.begin(); jetiter != jets.end(); ++jetiter)
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
  RECOSkim::PrepPhotons(photonsH,photons);
  if (photonsH.isValid()) // standard handle check
  {
    // ECALELF tools
    EcalClusterLazyTools * clustertools = new EcalClusterLazyTools (iEvent, iSetup, recHitCollectionEBToken, recHitCollectionEEToken);

    nphotons = photons.size();
    if (nphotons > 0) RECOSkim::InitializeRecoPhotonBranches();

    int iph = 0;
    for (std::vector<reco::Photon>::const_iterator phiter = photons.begin(); phiter != photons.end(); ++phiter) // loop over photon vector
    {
      // standard photon branches
      phE  [iph] = phiter->energy();
      phpt [iph] = phiter->pt();
      phphi[iph] = phiter->phi();
      pheta[iph] = phiter->eta();

      // super cluster from photon
      const reco::SuperClusterRef& phsc = phiter->superCluster().isNonnull() ? phiter->superCluster() : phiter->parentSuperCluster();
      phscE  [iph] = phsc->energy();
      phscphi[iph] = phsc->phi();
      phsceta[iph] = phsc->eta();
      const float sceta = std::abs(phsceta[iph]);

      // Shower Shape Objects
      const reco::Photon::ShowerShape& phshape = phiter->full5x5_showerShapeVariables(); // phiter->showerShapeVariables();
      // cluster shape variables
      phsieie[iph] = phshape.sigmaIetaIeta; 
      phsipip[iph] = phshape.sigmaIphiIphi; 
      phsieip[iph] = phshape.sigmaIetaIphi; 

      // ID-like variables
      phHoE   [iph] = phiter->hadronicOverEm(); // phiter->hadTowOverEm();
      phChgIso[iph] = std::max(phiter->chargedHadronIso() - (rho * RECOSkim::GetChargedHadronEA(sceta)),0.f);
      phNeuIso[iph] = std::max(phiter->neutralHadronIso() - (rho * RECOSkim::GetNeutralHadronEA(sceta)),0.f);
      phPhIso [iph] = std::max(phiter->photonIso()        - (rho * RECOSkim::GetGammaEA        (sceta)),0.f);
      phr9    [iph] = phshape.e3x3/phsc->rawEnergy();

      // store similar ints if pass individual selections
      phHoE_b   [iph] = RECOSkim::PassHoE   (sceta,phHoE   [iph]);
      phsieie_b [iph] = RECOSkim::PassSieie (sceta,phsieie [iph]);
      phChgIso_b[iph] = RECOSkim::PassChgIso(sceta,phChgIso[iph]);
      phNeuIso_b[iph] = RECOSkim::PassNeuIso(sceta,phNeuIso[iph],phpt[iph]);
      phPhIso_b [iph] = RECOSkim::PassPhIso (sceta,phPhIso [iph],phpt[iph]);
	
      /////////////////
      //             //
      // RecHit Info //
      //             //
      /////////////////
      // get geometry and recHits
      const DetId seedDetId = phsc->seed()->seed(); //seed detid
      const bool  isEB = (seedDetId.subdetId() == EcalBarrel); //which subdet
      const EcalRecHitCollection * recHits = isEB ? clustertools->getEcalEBRecHitCollection() : clustertools->getEcalEERecHitCollection();

      if ( recHits->size() != 0 ) //just to be safe
      {
	// 2nd moments from official calculation
	const Cluster2ndMoments ph2ndMoments = noZS::EcalClusterTools::cluster2ndMoments( *phsc, *recHits);
	// radius of semi-major,minor axis is the inverse square root of the eigenvalues of the covariance matrix
	phsmaj[iph] = ph2ndMoments.sMaj;
	phsmin[iph] = ph2ndMoments.sMin;
      }

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

	// seed rechit info + swiss crossssss
	if (seedDetId.rawId() == recHitId) 
	{ 
	  const auto recHitPos = isEB ? barrelGeometry->getGeometry(recHitId)->getPosition() : endcapGeometry->getGeometry(recHitId)->getPosition();
	  phseedE   [iph] = recHit->energy();
	  phseedphi [iph] = recHitPos.phi();
	  phseedeta [iph] = recHitPos.eta();
	  phseedtime[iph] = recHit->time();
	  phseedOOT [iph] = int(recHit->checkFlag(EcalRecHit::kOutOfTime));

	  // swiss cross
	  phsuisseX [iph] = EcalTools::swissCross(recHitId,(*recHits),0.f);
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

void RECOSkim::PrepJets(const edm::Handle<std::vector<reco::PFJet> > & jetsH, std::vector<reco::PFJet> & jets)
{
  if (jetsH.isValid()) // standard handle check
  {
    std::sort(jets.begin(),jets.end(),sortByJetPt);
    jets.resize(10); // keep a maximum of 10, now that they are sorted
  }
}  

void RECOSkim::PrepPhotons(const edm::Handle<std::vector<reco::Photon> > & photonsH, std::vector<reco::Photon> & photons)
{
  if (photonsH.isValid()) // standard handle check
  {
    std::sort(photons.begin(),photons.end(),sortByPhotonPt);
  }
}  

float RECOSkim::GetChargedHadronEA(const float eta)
{
  if      (eta <  1.0)                  return 0.0360;
  else if (eta >= 1.0   && eta < 1.479) return 0.0377;
  else if (eta >= 1.479 && eta < 2.0  ) return 0.0306;
  else if (eta >= 2.0   && eta < 2.2  ) return 0.0283;
  else if (eta >= 2.2   && eta < 2.3  ) return 0.0254;
  else if (eta >= 2.3   && eta < 2.4  ) return 0.0217;
  else if (eta >= 2.4   && eta < 5.0  ) return 0.0167;
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
  else if (eta >= 2.4   && eta < 5.0  ) return 0.0591;
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
  else if (eta >= 2.4   && eta < 5.0  ) return 0.1998;
  else                                  return 0.;
}

int RECOSkim::PassHoE(const float eta, const float HoE)
{ 
  if (eta < 1.479) // 1.4442
  {
    if      (HoE < 0.0269) return 3; 
    else if (HoE < 0.0396) return 2; 
    else if (HoE < 0.0597) return 1; 
    else                   return 0;
  }
  else if (eta > 1.479 && eta < 2.5) // 1.566
  {
    if      (HoE < 0.0213) return 3; 
    else if (HoE < 0.0219) return 2; 
    else if (HoE < 0.0481) return 1; 
    else                   return 0;
  }
  else                     return 0;
}

int RECOSkim::PassSieie(const float eta, const float Sieie)
{ 
  if (eta < 1.479)
  {
    if      (Sieie < 0.00994) return 3; 
    else if (Sieie < 0.01022) return 2; 
    else if (Sieie < 0.01031) return 1; 
    else                      return 0;
  }
  else if (eta > 1.479 && eta < 2.5)
  {
    if      (Sieie < 0.03000) return 3; 
    else if (Sieie < 0.03001) return 2; 
    else if (Sieie < 0.03013) return 1; 
    else                      return 0;
  }
  else                        return 0;
}

int RECOSkim::PassChgIso(const float eta, const float ChgIso)
{ 
  if (eta < 1.479)
  {
    if      (ChgIso < 0.202) return 3; 
    else if (ChgIso < 0.441) return 2; 
    else if (ChgIso < 1.295) return 1; 
    else                     return 0;
  }
  else if (eta > 1.479 && eta < 2.5)
  {
    if      (ChgIso < 0.034) return 3; 
    else if (ChgIso < 0.442) return 2; 
    else if (ChgIso < 1.011) return 1; 
    else                     return 0;
  }
  else                       return 0;
}

int RECOSkim::PassNeuIso(const float eta, const float NeuIso, const float pt)
{ 
  if (eta < 1.479)
  {
    const float ptdep = 0.0148*pt+0.000017*pt*pt;
    if      (NeuIso < (0.264 +ptdep)) return 3; 
    else if (NeuIso < (2.725 +ptdep)) return 2; 
    else if (NeuIso < (10.910+ptdep)) return 1; 
    else                              return 0;
  }
  else if (eta > 1.479 && eta < 2.5)
  {
    const float ptdep = 0.0163*pt+0.000014*pt*pt;
    if      (NeuIso < (0.586 +ptdep)) return 3; 
    else if (NeuIso < (1.715 +ptdep)) return 2; 
    else if (NeuIso < (5.931 +ptdep)) return 1; 
    else                              return 0;
  }
  else                                return 0;
}

int RECOSkim::PassPhIso(const float eta, const float PhIso, const float pt)
{ 
  if (eta < 1.479)
  {
    const float ptdep = 0.0047*pt;
    if      (PhIso < (2.362+ptdep)) return 3; 
    else if (PhIso < (2.571+ptdep)) return 2; 
    else if (PhIso < (3.630+ptdep)) return 1; 
    else                            return 0;
  }
  else if (eta > 1.479 && eta < 2.5)
  {
    const float ptdep = 0.0034*pt;
    if      (PhIso < (2.617+ptdep)) return 3; 
    else if (PhIso < (3.863+ptdep)) return 2; 
    else if (PhIso < (6.641+ptdep)) return 1; 
    else                            return 0;
  }
  else                              return 0;
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

  phscE.clear(); 
  phscphi.clear(); 
  phsceta.clear(); 

  phsieie.clear();
  phsipip.clear();
  phsieip.clear();
  phsmaj.clear();
  phsmin.clear();
  
  phHoE.clear();
  phChgIso.clear();
  phNeuIso.clear();
  phPhIso.clear();
  phr9.clear();
  phsuisseX.clear();

  phHoE_b.clear();
  phsieie_b.clear();
  phChgIso_b.clear();
  phNeuIso_b.clear();
  phPhIso_b.clear();

  phnrh.clear();
  phnrhEcut.clear();
  phnrhOOT.clear();
  phseedE.clear(); 
  phseedphi.clear();
  phseedeta.clear(); 
  phseedtime.clear();
  phseedOOT.clear();
}

void RECOSkim::InitializeRecoPhotonBranches()
{
  phE.resize(nphotons);
  phpt.resize(nphotons);
  phphi.resize(nphotons);
  pheta.resize(nphotons);

  phscE.resize(nphotons);
  phscphi.resize(nphotons);
  phsceta.resize(nphotons);

  phsieie.resize(nphotons);
  phsipip.resize(nphotons);
  phsieip.resize(nphotons);
  phsmaj.resize(nphotons);
  phsmin.resize(nphotons);

  phHoE.resize(nphotons);
  phChgIso.resize(nphotons);
  phNeuIso.resize(nphotons);
  phPhIso.resize(nphotons);
  phsuisseX.resize(nphotons);
  phr9.resize(nphotons);
  
  phHoE_b.resize(nphotons);
  phsieie_b.resize(nphotons);
  phChgIso_b.resize(nphotons);
  phNeuIso_b.resize(nphotons);
  phPhIso_b.resize(nphotons);

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
    phE   [iph] = -9999.f; 
    phpt  [iph] = -9999.f; 
    phphi [iph] = -9999.f; 
    pheta [iph] = -9999.f; 
    
    phscE  [iph] = -9999.f; 
    phscphi[iph] = -9999.f; 
    phsceta[iph] = -9999.f; 

    phsieie[iph] = -9999.f;
    phsipip[iph] = -9999.f;
    phsieip[iph] = -9999.f;
    phsmaj [iph] = -9999.f;
    phsmin [iph] = -9999.f;

    phHoE    [iph] = -9999.f;
    phChgIso [iph] = -9999.f;
    phNeuIso [iph] = -9999.f;
    phPhIso  [iph] = -9999.f;
    phr9     [iph] = -9999.f;
    phsuisseX[iph] = -9999.f;
    
    phHoE_b   [iph] = -9999;
    phsieie_b [iph] = -9999;
    phChgIso_b[iph] = -9999;
    phNeuIso_b[iph] = -9999;
    phPhIso_b [iph] = -9999;

    phnrh     [iph] = -9999;
    phnrhEcut [iph] = -9999;
    phnrhOOT  [iph] = -9999;
    phseedE   [iph] = -9999.f;
    phseedphi [iph] = -9999.f;
    phseedeta [iph] = -9999.f;
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

  // Trigger info
  tree->Branch("hltdoubleph"          , &hltdoubleph          , "hltdoubleph/O");

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

  tree->Branch("phscE"                , &phscE);
  tree->Branch("phscphi"              , &phscphi);
  tree->Branch("phsceta"              , &phsceta);

  tree->Branch("phsieie"              , &phsieie);
  tree->Branch("phsipip"              , &phsipip);
  tree->Branch("phsieip"              , &phsieip);
  tree->Branch("phsmaj"               , &phsmaj);
  tree->Branch("phsmin"               , &phsmin);

  tree->Branch("phHoE"                , &phHoE);
  tree->Branch("phChgIso"             , &phChgIso);
  tree->Branch("phNeuIso"             , &phNeuIso);
  tree->Branch("phPhIso"              , &phPhIso);
  tree->Branch("phr9"                 , &phr9);
  tree->Branch("phsuisseX"            , &phsuisseX);

  tree->Branch("phHoE_b"              , &phHoE_b);
  tree->Branch("phsieie_b"            , &phsieie_b);
  tree->Branch("phChgIso_b"           , &phChgIso_b);
  tree->Branch("phNeuIso_b"           , &phNeuIso_b);
  tree->Branch("phPhIso_b"            , &phPhIso_b);

  tree->Branch("phnrh"                , &phnrh);
  tree->Branch("phnrhEcut"            , &phnrhEcut);
  tree->Branch("phnrhOOT"             , &phnrhOOT);
  tree->Branch("phseedE"              , &phseedE);
  tree->Branch("phseedphi"            , &phseedphi);
  tree->Branch("phseedeta"            , &phseedeta);
  tree->Branch("phseedtime"           , &phseedtime);
  tree->Branch("phseedOOT"            , &phseedOOT);
}

void RECOSkim::endJob() {}

void RECOSkim::beginRun(edm::Run const& iRun, edm::EventSetup const& iSetup) 
{
  triggerPathsVector.push_back("HLT_Photon36_R9Id85_OR_CaloId24b40e_Iso50T80L_Photon22_AND_HE10_R9Id65_Eta2_Mass15_v");

  HLTConfigProvider hltConfig;
  bool changedConfig = false;
  hltConfig.init(iRun, iSetup, triggersTag.process(), changedConfig);
  
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

void RECOSkim::endRun(edm::Run const&, edm::EventSetup const&) {}

void RECOSkim::fillDescriptions(edm::ConfigurationDescriptions& descriptions) 
{
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

DEFINE_FWK_MODULE(RECOSkim);
