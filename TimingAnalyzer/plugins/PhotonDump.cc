#include "PhotonDump.h"

PhotonDump::PhotonDump(const edm::ParameterSet& iConfig): 
  // vertexes
  verticesTag(iConfig.getParameter<edm::InputTag>("vertices")),

  // mets
  metsTag(iConfig.getParameter<edm::InputTag>("mets")),  

  // jets
  jetsTag(iConfig.getParameter<edm::InputTag>("jets")),  

  // photons + ids
  photonLooseIdMapTag (iConfig.getParameter<edm::InputTag>("loosePhotonID")),  
  photonMediumIdMapTag(iConfig.getParameter<edm::InputTag>("mediumPhotonID")),  
  photonTightIdMapTag (iConfig.getParameter<edm::InputTag>("tightPhotonID")),  
  photonsTag(iConfig.getParameter<edm::InputTag>("photons")),  

  //recHits
  dumpRHs(iConfig.existsAs<bool>("dumpRHs") ? iConfig.getParameter<bool>("dumpRHs") : false),
  recHitCollectionEBTAG(consumes<EcalRecHitCollection>(iConfig.getParameter<edm::InputTag>( "recHitCollectionEB" ))),
  recHitCollectionEETAG(consumes<EcalRecHitCollection>(iConfig.getParameter<edm::InputTag>( "recHitCollectionEE" ))),

  ///////////// GEN INFO
  // isMC or Data --> default Data
  isMC   (iConfig.existsAs<bool>("isMC")    ? iConfig.getParameter<bool>("isMC")    : false),
  dumpIds(iConfig.existsAs<bool>("dumpIds") ? iConfig.getParameter<bool>("dumpIds") : false)
{
  usesResource();
  usesResource("TFileService");

  //vertex
  verticesToken = consumes<std::vector<reco::Vertex> > (verticesTag);

  // mets
  metsToken = consumes<std::vector<pat::MET> > (metsTag);

  // jets
  jetsToken = consumes<std::vector<pat::Jet> > (jetsTag);

  // photons + ids
  photonLooseIdMapToken  = consumes<edm::ValueMap<bool> > (photonLooseIdMapTag);
  photonMediumIdMapToken = consumes<edm::ValueMap<bool> > (photonMediumIdMapTag);
  photonTightIdMapToken  = consumes<edm::ValueMap<bool> > (photonTightIdMapTag);
  photonsToken           = consumes<std::vector<pat::Photon> > (photonsTag);

  // only for simulated samples
  if (isMC)
  {
    genevtInfoToken = consumes<GenEventInfoProduct>             (iConfig.getParameter<edm::InputTag>("genevt"));
    pileupInfoToken = consumes<std::vector<PileupSummaryInfo> > (iConfig.getParameter<edm::InputTag>("pileup"));
    genpartsToken   = consumes<std::vector<reco::GenParticle> > (iConfig.getParameter<edm::InputTag>("genparts"));   
    genjetsToken    = consumes<std::vector<reco::GenJet> >      (iConfig.getParameter<edm::InputTag>("genjets"));   
  }
}

PhotonDump::~PhotonDump() {}

void PhotonDump::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) 
{
  // VERTEX
  edm::Handle<std::vector<reco::Vertex> > verticesH;
  iEvent.getByToken(verticesToken, verticesH);
  
  // MET
  edm::Handle<std::vector<pat::MET> > metsH;
  iEvent.getByToken(metsToken, metsH);

  // JETS
  edm::Handle<std::vector<pat::Jet> > jetsH;
  iEvent.getByToken(jetsToken, jetsH);

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

  /////////////
  //         //
  // MC INFO //
  //         //
  /////////////
  if (isMC) 
  {
    edm::Handle<GenEventInfoProduct> genevtInfoH;
    iEvent.getByToken(genevtInfoToken, genevtInfoH);

    edm::Handle<std::vector<PileupSummaryInfo> > pileupInfoH;
    iEvent.getByToken(pileupInfoToken, pileupInfoH);

    edm::Handle<std::vector<reco::GenParticle> > genparticlesH;
    iEvent.getByToken(genpartsToken, genparticlesH);

    edm::Handle<std::vector<reco::GenJet> > genjetsH;
    iEvent.getByToken(genjetsToken, genjetsH);
  
    ///////////////////////
    //                   //
    // Event weight info //
    //                   //
    ///////////////////////
    PhotonDump::InitializeGenEvtBranches();
    if (genevtInfoH.isValid()) {genwgt = genevtInfoH->weight();}

    /////////////////////
    //                 //
    // Gen pileup info //
    //                 //
    /////////////////////
    PhotonDump::InitializeGenPUBranches();
    if (pileupInfoH.isValid()) // standard check for pileup
    {
      for (std::vector<PileupSummaryInfo>::const_iterator puiter = pileupInfoH->begin(); puiter != pileupInfoH->end(); ++puiter) 
      {
	if (puiter->getBunchCrossing() == 0) 
	{
	  genpuobs  = puiter->getPU_NumInteractions();
	  genputrue = puiter->getTrueNumInteractions();
	} // end check over correct BX
      } // end loop over PU
    } // end check over pileup

    ///////////////////////
    //                   //
    // Gen particle info //
    //                   //
    ///////////////////////
    PhotonDump::InitializeGenParticleBranches();
    if (genparticlesH.isValid()) // make sure gen particles exist
    {
      // Dump gen particle pdgIds
      if (dumpIds) PhotonDump::DumpGenIds(genparticlesH); 
      
      nNeutralino = 0;
      nNeutoPhGr  = 0;
      bool firstMother  = false; // bool for first neutralino found
      bool secondMother = false; // bool for second neutralino found
      for (std::vector<reco::GenParticle>::const_iterator gpiter = genparticlesH->begin(); gpiter != genparticlesH->end(); ++gpiter) // loop over gen particles
      {
	if (firstMother && secondMother) break; // have two matches!

	if (gpiter->pdgId() == 1000022 && gpiter->numberOfDaughters() == 2)
	{
	  nNeutralino++;

	  if ((gpiter->daughter(0)->pdgId() == 22 && gpiter->daughter(1)->pdgId() == 1000039) ||
	      (gpiter->daughter(1)->pdgId() == 22 && gpiter->daughter(0)->pdgId() == 1000039)) 
	  {
	    if (!firstMother && !secondMother)
	    {
	      nNeutoPhGr++;

	      // set neutralino parameters
	      genN1mass = gpiter->mass();
	      genN1E    = gpiter->energy();
	      genN1pt   = gpiter->pt();
	      genN1phi  = gpiter->phi();
	      genN1eta  = gpiter->eta();

	      // neutralino production vertex
	      genN1prodvx = gpiter->vx();
	      genN1prodvy = gpiter->vy();
	      genN1prodvz = gpiter->vz();
	      
	      // neutralino decay vertex (same for both daughters unless really screwed up)
	      genN1decayvx = gpiter->daughter(0)->vx();
	      genN1decayvy = gpiter->daughter(0)->vy();
	      genN1decayvz = gpiter->daughter(0)->vz();

	      // set photon daughter stuff
	      int phdaughter = -1; // determine which one is the photon daughter
	      if      (gpiter->daughter(0)->pdgId() == 22) {phdaughter = 0;}
	      else if (gpiter->daughter(1)->pdgId() == 22) {phdaughter = 1;}

	      genph1E    = gpiter->daughter(phdaughter)->energy();
	      genph1pt   = gpiter->daughter(phdaughter)->pt();
	      genph1phi  = gpiter->daughter(phdaughter)->phi();
	      genph1eta  = gpiter->daughter(phdaughter)->eta();

	      // check for a reco match!
	      if (photonsH.isValid()) // standard check
              {
		int iph = 0;
		for (std::vector<pat::Photon>::const_iterator phiter = photonsH->begin(); phiter != photonsH->end(); ++phiter) // loop over photon vector 
	        {
		  const float tmpphi = phiter->phi();
		  const float tmpeta = phiter->eta();
		  
		  if (deltaR(genph1phi,genph1eta,tmpphi,tmpeta) < 0.3) {genph1match = iph; break;}
		  
		  iph++;
		} // end loop over reco photons
	      } // end check for reco match
	    
	      int grdaughter = -1; // determine which one is the gravitino
	      if      (gpiter->daughter(0)->pdgId() == 1000039) {grdaughter = 0;}
	      else if (gpiter->daughter(1)->pdgId() == 1000039) {grdaughter = 1;}
	      
	      gengr1mass = gpiter->daughter(grdaughter)->mass();
	      gengr1E    = gpiter->daughter(grdaughter)->energy();
	      gengr1pt   = gpiter->daughter(grdaughter)->pt();
	      gengr1phi  = gpiter->daughter(grdaughter)->phi();
	      gengr1eta  = gpiter->daughter(grdaughter)->eta();
	      
	      // set this to ensure not to overwrite first mother info
	      firstMother = true;
	    } // end block over first matched neutralino -> photon + gravitino
	    else if (firstMother && !secondMother)
	    {
	      nNeutoPhGr++;

	      // set neutralino parameters
	      genN2mass = gpiter->mass();
	      genN2E    = gpiter->energy();
	      genN2pt   = gpiter->pt();
	      genN2phi  = gpiter->phi();
	      genN2eta  = gpiter->eta();
	      
	      // neutralino production vertex
	      genN2prodvx = gpiter->vx();
	      genN2prodvy = gpiter->vy();
	      genN2prodvz = gpiter->vz();
	      
	      // neutralino decay vertex (same for both daughters unless really screwed up)
	      genN2decayvx = gpiter->daughter(0)->vx();
	      genN2decayvy = gpiter->daughter(0)->vy();
	      genN2decayvz = gpiter->daughter(0)->vz();

	      // set photon daughter stuff
	      int phdaughter = -1; // determine which one is the photon daughter
	      if      (gpiter->daughter(0)->pdgId() == 22) {phdaughter = 0;}
	      else if (gpiter->daughter(1)->pdgId() == 22) {phdaughter = 1;}
	      
	      genph2E    = gpiter->daughter(phdaughter)->energy();
	      genph2pt   = gpiter->daughter(phdaughter)->pt();
	      genph2phi  = gpiter->daughter(phdaughter)->phi();
	      genph2eta  = gpiter->daughter(phdaughter)->eta();
	      
	      // check for a reco match!
	      if (photonsH.isValid()) // standard check
	      {
		int iph = 0;
		for (std::vector<pat::Photon>::const_iterator phiter = photonsH->begin(); phiter != photonsH->end(); ++phiter) // loop over photon vector 
		{
		  const float tmpphi = phiter->phi();
		  const float tmpeta = phiter->eta();
		  
		  if (deltaR(genph2phi,genph2eta,tmpphi,tmpeta) < 0.3) {genph2match = iph; break;}
		  
		  iph++;
		} // end loop over reco photons
	      } // end check for reco match
	      
	      int grdaughter = -1; // determine which one is the gravitino
	      if      (gpiter->daughter(0)->pdgId() == 1000039) {grdaughter = 0;}
	      else if (gpiter->daughter(1)->pdgId() == 1000039) {grdaughter = 1;}
	      
	      gengr2mass = gpiter->daughter(grdaughter)->mass();
	      gengr2E    = gpiter->daughter(grdaughter)->energy();
	      gengr2pt   = gpiter->daughter(grdaughter)->pt();
	      gengr2phi  = gpiter->daughter(grdaughter)->phi();
	      gengr2eta  = gpiter->daughter(grdaughter)->eta();
	      
	      // set this to ensure not to overwrite first mother info
	      secondMother = true;
	    } // end block over second matched neutralino -> photon + gravitino
	  } // end conditional over matching daughter ids
	} // end conditional over neutralino id
      } // end loop over gen particles
    } // end check for gen particles

    ///////////////////
    //               //
    // Gen Jets info //
    //               //
    ///////////////////
    PhotonDump::ClearGenJetBranches();
    if (genjetsH.isValid()) // make sure gen particles exist
    {
      ngenjets = genjetsH->size();
      if (ngenjets > 0) PhotonDump::InitializeGenJetBranches();
      int igjet = 0;
      for (std::vector<reco::GenJet>::const_iterator gjetiter = genjetsH->begin(); gjetiter != genjetsH->end(); ++gjetiter) // loop over genjets
      {
	// check for gen to reco match
	if (jetsH.isValid()) // check to make sure reco (AK4) jets exist
	{
	  int ijet = 0;
	  for (std::vector<pat::Jet>::const_iterator jetiter = jetsH->begin(); jetiter != jetsH->end(); ++jetiter) // loop over reco jets for match
	  {
	    if (deltaR(gjetiter->phi(),gjetiter->eta(),jetiter->phi(),jetiter->eta()) < 0.4) // deltaR matching conditional
	    {
	      genjetmatch[igjet] = ijet;
	      break;
	    }
	    ijet++;
	  } // end loop over reco jets
	} // end check over reco jets 

	genjetE  [igjet] = gjetiter->energy();
	genjetpt [igjet] = gjetiter->pt();
	genjetphi[igjet] = gjetiter->phi();
	genjeteta[igjet] = gjetiter->eta();

	igjet++;
      } // end loop over genjets
    } // end check over genjets
  } // end block over isMC

  /////////////////////////
  //                     //   
  // Primary Vertex info //
  //                     //
  /////////////////////////
  PhotonDump::InitializePVBranches();
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
  PhotonDump::InitializeMETBranches();
  if (metsH.isValid())
  {
    const pat::MET & t1pfMET = (*metsH)[0];

    // Type1 PF MET (corrected)
    t1pfMETpt    = t1pfMET.pt();
    t1pfMETphi   = t1pfMET.phi();
    t1pfMETsumEt = t1pfMET.sumEt();

    // T1PF MET Uncorrected
    t1pfMETuncorpt    = t1pfMET.pt();
    t1pfMETuncorphi   = t1pfMET.phi();
    t1pfMETuncorsumEt = t1pfMET.sumEt();

    // T1PF MET CaloMET
    t1pfMETcalopt    = t1pfMET.pt();
    t1pfMETcalophi   = t1pfMET.phi();
    t1pfMETcalosumEt = t1pfMET.sumEt();

    if (isMC)
    {
      // GEN MET 
      t1pfMETgenMETpt    = t1pfMET.genMET()->pt();
      t1pfMETgenMETphi   = t1pfMET.genMET()->phi();
      t1pfMETgenMETsumEt = t1pfMET.genMET()->sumEt();
    }
  }

  /////////////////////////
  //                     //
  // Jets (AK4 standard) //
  //                     //
  /////////////////////////
  PhotonDump::ClearJetBranches();
  if (jetsH.isValid()) // check to make sure reco jets exist
  {
    njets = jetsH->size();
    if (njets > 0) PhotonDump::InitializeJetBranches();
    int ijet = 0;
    for (std::vector<pat::Jet>::const_iterator jetiter = jetsH->begin(); jetiter != jetsH->end(); ++jetiter)
    {
      // check if reco jet is matched to gen jet
      if (isMC)
      {
	for (size_t igjet = 0; igjet < genjetmatch.size(); igjet++) // loop over gen jet matches
        {
	  if (genjetmatch[igjet] == ijet) 
	  {
	    jetmatch[ijet] = int(igjet);
	    break;
	  } // end conditional over check
	} // end loop over gen jets matches
      } // end block over isMC

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
  PhotonDump::ClearRecoPhotonBranches();
  if (photonsH.isValid()) // standard handle check
  {
    // ECALELF tools
    EcalClusterLazyTools * clustertools = new EcalClusterLazyTools (iEvent, iSetup, recHitCollectionEBTAG, recHitCollectionEETAG);

    nphotons = photonsH->size();
    if (nphotons > 0) PhotonDump::InitializeRecoPhotonBranches();

    int iph = 0;
    for (std::vector<pat::Photon>::const_iterator phiter = photonsH->begin(); phiter != photonsH->end(); ++phiter) // loop over photon vector
    {
      // Check for gen level match
      if (isMC)
      {
	if      (iph == genph1match) phmatch[iph] = 1;
	else if (iph == genph2match) phmatch[iph] = 2;
      }

      // Get the VID of the photon
      const edm::Ptr<pat::Photon> photonPtr(photonsH, phiter - photonsH->begin());

      // loose > medium > tight
      if (photonLooseIdMap [photonPtr]) {phVID[iph] = 1;}
      if (photonMediumIdMap[photonPtr]) {phVID[iph] = 2;}
      if (photonTightIdMap [photonPtr]) {phVID[iph] = 3;}

      // standard photon branches
      phE  [iph] = phiter->energy();
      phpt [iph] = phiter->pt();
      phphi[iph] = phiter->phi();
      pheta[iph] = phiter->eta();

      // cluster shape variables
      const float see = phiter->full5x5_showerShapeVariables().sigmaIetaIeta;
      const float spp = phiter->full5x5_showerShapeVariables().sigmaIphiIphi;
      const float sep = phiter->full5x5_showerShapeVariables().sigmaIetaIphi;
      const float disc = std::sqrt((spp-see)*(spp-see)+4.f*sep*sep);

      // lamba_1/2 = (spp+see-/+disc)/2
      
      phsmaj[iph] = std::sqrt(2.f/(spp+see-disc));
      phsmin[iph] = std::sqrt(2.f/(spp+see+disc));
	
      // super cluster from photon
      const reco::SuperClusterRef& phsc = phiter->superCluster().isNonnull() ? phiter->superCluster() : phiter->parentSuperCluster();
      if (phsc.isNonnull()) // check to make sure supercluster is good
      {
	phscX[iph] = phsc->position().x();
	phscY[iph] = phsc->position().y();
	phscZ[iph] = phsc->position().z();
	phscE[iph] = phsc->energy();

	// use seed to get geometry and recHits
	const DetId seedDetId = phsc->seed()->seed(); //seed detid
	const bool isEB = (seedDetId.subdetId() == EcalBarrel); //which subdet
	const EcalRecHitCollection * recHits = isEB ? clustertools->getEcalEBRecHitCollection() : clustertools->getEcalEERecHitCollection();

	// map of rec hit ids
	uiiumap phrhIDmap;

	// all rechits in superclusters
	const DetIdPairVec hitsAndFractions = phsc->hitsAndFractions();
	if (dumpRHs) PhotonDump::DumpRecHitInfo(iph,hitsAndFractions,recHits);

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
	if (phnrh[iph] > 0) PhotonDump::InitializeRecoRecHitBranches(iph);
	int irh = 0;
	for (uiiumap::const_iterator rhiter = phrhIDmap.begin(); rhiter != phrhIDmap.end(); ++rhiter) // loop over only good rec hit ids
        {
	  const uint32_t rhID = rhiter->first;

	  const DetId recHitId(rhID);
	  EcalRecHitCollection::const_iterator recHit = recHits->find(recHitId); // get the underlying rechit
	  
	  const auto recHitPos = isEB ? barrelGeometry->getGeometry(recHitId)->getPosition() : endcapGeometry->getGeometry(recHitId)->getPosition();

	  // save position, energy, and time of each rechit to a vector
	  phrhX   [iph][irh] = recHitPos.x();
	  phrhY   [iph][irh] = recHitPos.y();
	  phrhZ   [iph][irh] = recHitPos.z();
	  phrhE   [iph][irh] = recHit->energy();
	  phrhdelR[iph][irh] = deltaR(float(recHitPos.phi()),recHitPos.eta(),phphi[iph],pheta[iph]);
	  phrhtime[iph][irh] = recHit->time();
	  phrhID  [iph][irh] = int(rhID);
	  phrhOOT [iph][irh] = int(recHit->checkFlag(EcalRecHit::kOutOfTime));
	  
	  // save the position in the vector of the seed 
	  if (seedDetId.rawId() == recHitId) { phseedpos[iph] = irh; }

	  irh++; // increment rechit counter
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

void PhotonDump::DumpGenIds(const edm::Handle<std::vector<reco::GenParticle> > & genparticlesH)
{
  std::cout << "event: " << event << std::endl;
  
  for (std::vector<reco::GenParticle>::const_iterator gpiter = genparticlesH->begin(); gpiter != genparticlesH->end(); ++gpiter) // loop over gen particles
  {
    if (gpiter->pdgId() != 1000022) continue;

    std::cout << "particle id: " << gpiter->pdgId() << " (" << gpiter->mass() << ")" << std::endl;
    
    // dump mothers first
    if (gpiter->numberOfMothers() != 0) std::cout << " mothers: ";
    for (auto mgpiter = gpiter->motherRefVector().begin(); mgpiter != gpiter->motherRefVector().end(); ++mgpiter)
    {
      std::cout << (*mgpiter)->pdgId() << " (" << (*mgpiter)->mass() << ") ";
    }
    if (gpiter->numberOfMothers() != 0) std::cout << std::endl;

    // dump daughters second
    if (gpiter->numberOfDaughters() != 0) std::cout << " daughters: ";
    for (auto dgpiter = gpiter->daughterRefVector().begin(); dgpiter != gpiter->daughterRefVector().end(); ++dgpiter)
    {
      std::cout << (*dgpiter)->pdgId() << " (" << (*dgpiter)->mass() << ") ";
    }
    if (gpiter->numberOfDaughters() != 0) std::cout << std::endl;
  } // end loop over gen particles

  std::cout << "---------------------------------" << std::endl << std::endl;
}

void PhotonDump::DumpRecHitInfo(int iph, const DetIdPairVec & hitsAndFractions,	const EcalRecHitCollection *& recHits)
{
  std::cout << "event: " << event << std::endl;
  if (phE[iph] > 100.f && phmatch[iph] > 0 && phVID[iph] >= 2)
  { 
    if (std::abs(pheta[iph]) < 2.5 && !(std::abs(pheta[iph]) > 1.4442 && std::abs(pheta[iph]) < 1.566))  
    {
      if (phnrh[iph] == 0)
      {
	const int hafsize = hitsAndFractions.size();
	const float scphi = phi(phscX[iph],phscY[iph]);
	const float sceta = eta(phscX[iph],phscY[iph],phscZ[iph]);
	std::cout <<  "phE: " << phE[iph]   << " phphi: " << phphi[iph] << " pheta: " << pheta[iph] << std::endl;
	std::cout << " scE: " << phscE[iph] << " scphi: " << scphi      << " sceta: " << sceta      << " haf.size()= " << hafsize << std::endl;
	if (hafsize > 0) std::cout << " ids: ";
	uiiumap tmpmap;
	for (DetIdPairVec::const_iterator hafitr = hitsAndFractions.begin(); hafitr != hitsAndFractions.end(); ++hafitr) // loop over all rec hits in SC
        {
	  const DetId recHitId = hafitr->first; // get detid of crystal
	  std::cout << recHitId.rawId() << " ";
	  EcalRecHitCollection::const_iterator recHit = recHits->find(recHitId); // get the underlying rechit
	  if (recHit != recHits->end()) // standard check
          { 
	    tmpmap[recHitId.rawId()]++;
	  } // end standard check recHit
	} // end loop over hits and fractions
	if (hafsize > 0) std::cout << " ids: ";
      } // end check for nrhs == 0
    } // ensure within confines of ECAL
  } // gen matched photon (pt > 100, medium)
  std::cout << "---------------------------------" << std::endl << std::endl;
}

void PhotonDump::InitializeGenEvtBranches()
{
  genwgt = -9999.f;
}

void PhotonDump::InitializeGenPUBranches()
{
  genpuobs = -9999; genputrue = -9999;
}

void PhotonDump::InitializeGenParticleBranches()
{
  // Gen particle info
  nNeutralino = -9999;
  nNeutoPhGr  = -9999;

  genN1mass = -9999.f; genN1E = -9999.f; genN1pt = -9999.f; genN1phi = -9999.f; genN1eta = -9999.f;
  genN1prodvx = -9999.f; genN1prodvy = -9999.f; genN1prodvz = -9999.f;
  genN1decayvx = -9999.f; genN1decayvy = -9999.f; genN1decayvz = -9999.f;
  genph1E = -9999.f; genph1pt = -9999.f; genph1phi = -9999.f; genph1eta = -9999.f;
  genph1match = -9999;
  gengr1mass = -9999.f; gengr1E = -9999.f; gengr1pt = -9999.f; gengr1phi = -9999.f; gengr1eta = -9999.f;

  genN2mass = -9999.f; genN2E = -9999.f; genN2pt = -9999.f; genN2phi = -9999.f; genN2eta = -9999.f;
  genN2prodvx = -9999.f; genN2prodvy = -9999.f; genN2prodvz = -9999.f;
  genN2decayvx = -9999.f; genN2decayvy = -9999.f; genN2decayvz = -9999.f;
  genph2E = -9999.f; genph2pt = -9999.f; genph2phi = -9999.f; genph2eta = -9999.f;
  genph2match = -9999;
  gengr2mass = -9999.f; gengr2E = -9999.f; gengr2pt = -9999.f; gengr2phi = -9999.f; gengr2eta = -9999.f;
}

void PhotonDump::ClearGenJetBranches()
{
  ngenjets = -9999;

  genjetmatch.clear();

  genjetE.clear();
  genjetpt.clear();
  genjetphi.clear();
  genjeteta.clear();
}

void PhotonDump::InitializeGenJetBranches()
{
  genjetmatch.resize(ngenjets);

  genjetE.resize(ngenjets);
  genjetpt.resize(ngenjets);
  genjetphi.resize(ngenjets);
  genjeteta.resize(ngenjets);

  for (int igjet = 0; igjet < ngenjets; igjet++)
  {
    genjetmatch[igjet] = -9999;

    genjetE  [igjet] = -9999.f;
    genjetpt [igjet] = -9999.f;
    genjetphi[igjet] = -9999.f;
    genjeteta[igjet] = -9999.f;
  }
}

void PhotonDump::InitializePVBranches()
{
  nvtx = -9999; 
  vtxX = -9999.f; vtxY = -9999.f; vtxZ = -9999.f;
}

void PhotonDump::InitializeMETBranches()
{
  t1pfMETpt      = -9999.f; t1pfMETphi      = -9999.f; t1pfMETsumEt      = -9999.f;
  t1pfMETuncorpt = -9999.f; t1pfMETuncorphi = -9999.f; t1pfMETuncorsumEt = -9999.f;
  t1pfMETcalopt  = -9999.f; t1pfMETcalophi  = -9999.f; t1pfMETcalosumEt  = -9999.f;
 
  if (isMC)
  {
    t1pfMETgenMETpt = -9999.f; t1pfMETgenMETphi = -9999.f; t1pfMETgenMETsumEt = -9999.f;
  }
}

void PhotonDump::ClearJetBranches()
{
  njets = -9999;

  if (isMC) jetmatch.clear();

  jetE.clear();
  jetpt.clear();
  jetphi.clear();
  jeteta.clear();
}

void PhotonDump::InitializeJetBranches()
{
  jetmatch.resize(njets);

  jetE.resize(njets);
  jetpt.resize(njets);
  jetphi.resize(njets);
  jeteta.resize(njets);
  
  for (int ijet = 0; ijet < njets; ijet++)
  {
    jetmatch[ijet] = -9999;

    jetE  [ijet] = -9999.f;
    jetpt [ijet] = -9999.f;
    jetphi[ijet] = -9999.f;
    jeteta[ijet] = -9999.f;
  }
}

void PhotonDump::ClearRecoPhotonBranches()
{
  nphotons = -9999;

  if (isMC) phmatch.clear();
  phVID.clear();

  phE.clear();
  phpt.clear();
  phphi.clear(); 
  pheta.clear(); 
  phsmaj.clear();
  phsmin.clear();
  
  phscX.clear(); 
  phscY.clear(); 
  phscZ.clear(); 
  phscE.clear(); 

  phnrh.clear();

  phrhX.clear(); 
  phrhY.clear(); 
  phrhZ.clear(); 
  phrhE.clear(); 
  phrhdelR.clear(); 
  phrhtime.clear();
  phrhID.clear();
  phrhOOT.clear();

  phseedpos.clear();
}

void PhotonDump::InitializeRecoPhotonBranches()
{
  if (isMC) phmatch.resize(nphotons);
  phVID.resize(nphotons);

  phE.resize(nphotons);
  phpt.resize(nphotons);
  phphi.resize(nphotons);
  pheta.resize(nphotons);
  phsmaj.resize(nphotons);
  phsmin.resize(nphotons);
  
  phscX.resize(nphotons);
  phscY.resize(nphotons);
  phscZ.resize(nphotons);
  phscE.resize(nphotons);

  phnrh.resize(nphotons);

  phrhX.resize(nphotons);
  phrhY.resize(nphotons);
  phrhZ.resize(nphotons);
  phrhE.resize(nphotons);
  phrhdelR.resize(nphotons);
  phrhtime.resize(nphotons);
  phrhID.resize(nphotons);
  phrhOOT.resize(nphotons);

  phseedpos.resize(nphotons);

  for (int iph = 0; iph < nphotons; iph++)
  {
    if (isMC) phmatch[iph] = 0;
    phVID[iph] = 0;

    phE   [iph] = -9999.f; 
    phpt  [iph] = -9999.f; 
    phphi [iph] = -9999.f; 
    pheta [iph] = -9999.f; 
    phsmaj[iph] = -9999.f;
    phsmin[iph] = -9999.f;

    phscX [iph] = -9999.f; 
    phscY [iph] = -9999.f; 
    phscZ [iph] = -9999.f; 
    phscE [iph] = -9999.f; 

    phnrh [iph] = -9999;

    phseedpos[iph] = -9999;
  }
}

void PhotonDump::InitializeRecoRecHitBranches(int iph)
{
  phrhX   [iph].resize(phnrh[iph]);
  phrhY   [iph].resize(phnrh[iph]);
  phrhZ   [iph].resize(phnrh[iph]);
  phrhE   [iph].resize(phnrh[iph]);
  phrhdelR[iph].resize(phnrh[iph]);
  phrhtime[iph].resize(phnrh[iph]);
  phrhID  [iph].resize(phnrh[iph]);
  phrhOOT [iph].resize(phnrh[iph]);

  for (int irh = 0; irh < phnrh[iph]; irh++)
  {
    phrhX   [iph][irh] = -9999.f;
    phrhY   [iph][irh] = -9999.f;
    phrhZ   [iph][irh] = -9999.f;
    phrhE   [iph][irh] = -9999.f;
    phrhdelR[iph][irh] = -9999.f;
    phrhtime[iph][irh] = -9999.f;
    phrhID  [iph][irh] = -9999;
    phrhOOT [iph][irh] = -9999;
  }
}

void PhotonDump::beginJob() 
{
  edm::Service<TFileService> fs;
  tree = fs->make<TTree>("tree"     , "tree");

  // Run, Lumi, Event info
  tree->Branch("event"                , &event                , "event/I");
  tree->Branch("run"                  , &run                  , "run/I");
  tree->Branch("lumi"                 , &lumi                 , "lumi/I");
  
  if (isMC)
  {
    // Generator inf
    tree->Branch("genwgt"               , &genwgt               , "genwgt/F");
    tree->Branch("genpuobs"             , &genpuobs             , "genpuobs/I");
    tree->Branch("genputrue"            , &genputrue            , "genputrue/I");

    // Gen particle info
    tree->Branch("nNeutralino"          , &nNeutralino          , "nNeutralino/I");
    tree->Branch("nNeutoPhGr"           , &nNeutoPhGr           , "nNeutoPhGr/I");

    tree->Branch("genN1mass"            , &genN1mass            , "genN1mass/F");
    tree->Branch("genN1E"               , &genN1E               , "genN1E/F");
    tree->Branch("genN1pt"              , &genN1pt              , "genN1pt/F");
    tree->Branch("genN1phi"             , &genN1phi             , "genN1phi/F");
    tree->Branch("genN1eta"             , &genN1eta             , "genN1eta/F");
    tree->Branch("genN1prodvx"          , &genN1prodvx          , "genN1prodvx/F");
    tree->Branch("genN1prodvy"          , &genN1prodvy          , "genN1prodvy/F");
    tree->Branch("genN1prodvz"          , &genN1prodvz          , "genN1prodvz/F");
    tree->Branch("genN1decayvx"         , &genN1decayvx         , "genN1decayvx/F");
    tree->Branch("genN1decayvy"         , &genN1decayvy         , "genN1decayvy/F");
    tree->Branch("genN1decayvz"         , &genN1decayvz         , "genN1decayvz/F");
    tree->Branch("genph1E"              , &genph1E              , "genph1E/F");
    tree->Branch("genph1pt"             , &genph1pt             , "genph1pt/F");
    tree->Branch("genph1phi"            , &genph1phi            , "genph1phi/F");
    tree->Branch("genph1eta"            , &genph1eta            , "genph1eta/F");
    tree->Branch("genph1match"          , &genph1match          , "genph1match/I");
    tree->Branch("gengr1mass"           , &gengr1mass           , "gengr1mass/F");
    tree->Branch("gengr1E"              , &gengr1E              , "gengr1E/F");
    tree->Branch("gengr1pt"             , &gengr1pt             , "gengr1pt/F");
    tree->Branch("gengr1phi"            , &gengr1phi            , "gengr1phi/F");
    tree->Branch("gengr1eta"            , &gengr1eta            , "gengr1eta/F");

    tree->Branch("genN2mass"            , &genN2mass            , "genN2mass/F");
    tree->Branch("genN2E"               , &genN2E               , "genN2E/F");
    tree->Branch("genN2pt"              , &genN2pt              , "genN2pt/F");
    tree->Branch("genN2phi"             , &genN2phi             , "genN2phi/F");
    tree->Branch("genN2eta"             , &genN2eta             , "genN2eta/F");
    tree->Branch("genN2prodvx"          , &genN2prodvx          , "genN2prodvx/F");
    tree->Branch("genN2prodvy"          , &genN2prodvy          , "genN2prodvy/F");
    tree->Branch("genN2prodvz"          , &genN2prodvz          , "genN2prodvz/F");
    tree->Branch("genN2decayvx"         , &genN2decayvx         , "genN2decayvx/F");
    tree->Branch("genN2decayvy"         , &genN2decayvy         , "genN2decayvy/F");
    tree->Branch("genN2decayvz"         , &genN2decayvz         , "genN2decayvz/F");
    tree->Branch("genph2E"              , &genph2E              , "genph2E/F");
    tree->Branch("genph2pt"             , &genph2pt             , "genph2pt/F");
    tree->Branch("genph2phi"            , &genph2phi            , "genph2phi/F");
    tree->Branch("genph2eta"            , &genph2eta            , "genph2eta/F");
    tree->Branch("genph2match"          , &genph2match          , "genph2match/I");
    tree->Branch("gengr2mass"           , &gengr2mass           , "gengr2mass/F");
    tree->Branch("gengr2E"              , &gengr2E              , "gengr2E/F");
    tree->Branch("gengr2pt"             , &gengr2pt             , "gengr2pt/F");
    tree->Branch("gengr2phi"            , &gengr2phi            , "gengr2phi/F");
    tree->Branch("gengr2eta"            , &gengr2eta            , "gengr2eta/F");
  
    // GenJet Info
    tree->Branch("ngenjets"             , &ngenjets             , "ngenjets/I");
    tree->Branch("genjetmatch"          , &genjetmatch);
    tree->Branch("genjetE"              , &genjetE);
    tree->Branch("genjetpt"             , &genjetpt);
    tree->Branch("genjetphi"            , &genjetphi);
    tree->Branch("genjeteta"            , &genjeteta);
  }

  // Vertex info
  tree->Branch("nvtx"                 , &nvtx                 , "nvtx/I");
  tree->Branch("vtxX"                 , &vtxX                 , "vtxX/F");
  tree->Branch("vtxY"                 , &vtxY                 , "vtxY/F");
  tree->Branch("vtxZ"                 , &vtxZ                 , "vtxZ/F");

  // MET info
  tree->Branch("t1pfMETpt"            , &t1pfMETpt            , "t1pfMETpt/F");
  tree->Branch("t1pfMETphi"           , &t1pfMETphi           , "t1pfMETphi/F");
  tree->Branch("t1pfMETsumEt"         , &t1pfMETsumEt         , "t1pfMETsumEt/F");
  tree->Branch("t1pfMETuncorpt"       , &t1pfMETuncorpt       , "t1pfMETuncorpt/F");
  tree->Branch("t1pfMETuncorphi"      , &t1pfMETuncorphi      , "t1pfMETuncorphi/F");
  tree->Branch("t1pfMETuncorsumEt"    , &t1pfMETuncorsumEt    , "t1pfMETuncorsumEt/F");
  tree->Branch("t1pfMETcalopt"        , &t1pfMETcalopt        , "t1pfMETcalopt/F");
  tree->Branch("t1pfMETcalophi"       , &t1pfMETcalophi       , "t1pfMETcalophi/F");
  tree->Branch("t1pfMETcalosumEt"     , &t1pfMETcalosumEt     , "t1pfMETcalosumEt/F");
  
  if (isMC)
  {
    tree->Branch("t1pfMETgenMETpt"      , &t1pfMETgenMETpt      , "t1pfMETgenMETpt/F");
    tree->Branch("t1pfMETgenMETphi"     , &t1pfMETgenMETphi     , "t1pfMETgenMETphi/F");
    tree->Branch("t1pfMETgenMETsumEt"   , &t1pfMETgenMETsumEt   , "t1pfMETgenMETsumEt/F");
  }

  // Jet Info
  tree->Branch("njets"                , &njets                , "njets/I");
  if (isMC) tree->Branch("jetmatch"   , &jetmatch);
  tree->Branch("jetE"                 , &jetE);
  tree->Branch("jetpt"                , &jetpt);
  tree->Branch("jetphi"               , &jetphi);
  tree->Branch("jeteta"               , &jeteta);
   
  // Photon Info
  tree->Branch("nphotons"             , &nphotons             , "nphotons/I");
  if (isMC) tree->Branch("phmatch"    , &phmatch);
  tree->Branch("phVID"                , &phVID);
  tree->Branch("phE"                  , &phE);
  tree->Branch("phpt"                 , &phpt);
  tree->Branch("phphi"                , &phphi);
  tree->Branch("pheta"                , &pheta);
  tree->Branch("phsmaj"               , &phsmaj);
  tree->Branch("phsmin"               , &phsmin);

  tree->Branch("phscX"                , &phscX);
  tree->Branch("phscY"                , &phscY);
  tree->Branch("phscZ"                , &phscZ);
  tree->Branch("phscE"                , &phscE);
  tree->Branch("phnrh"                , &phnrh);

  tree->Branch("phseedpos"            , &phseedpos);

  tree->Branch("phrhX"                , &phrhX);
  tree->Branch("phrhY"                , &phrhY);
  tree->Branch("phrhZ"                , &phrhZ);
  tree->Branch("phrhE"                , &phrhE);
  tree->Branch("phrhdelR"             , &phrhdelR);
  tree->Branch("phrhtime"             , &phrhtime);
  tree->Branch("phrhID"               , &phrhID);
  tree->Branch("phrhOOT"              , &phrhOOT);
}

void PhotonDump::endJob() {}

void PhotonDump::beginRun(edm::Run const& iRun, edm::EventSetup const& iSetup) {}

void PhotonDump::endRun(edm::Run const&, edm::EventSetup const&) {}

void PhotonDump::fillDescriptions(edm::ConfigurationDescriptions& descriptions) 
{
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

DEFINE_FWK_MODULE(PhotonDump);
