#include "Timing/TimingAnalyzer/plugins/PhotonDump.hh"

PhotonDump::PhotonDump(const edm::ParameterSet& iConfig): 
  // dR matching criteria
  dRmin(iConfig.existsAs<double>("dRmin") ? iConfig.getParameter<double>("dRmin") : 0.4),
  pTres(iConfig.existsAs<double>("pTres") ? iConfig.getParameter<double>("pTres") : 0.5),

  // triggers
  inputPaths       (iConfig.existsAs<std::string>("inputPaths")   ? iConfig.getParameter<std::string>("inputPaths") : ""),
  inputFilters     (iConfig.existsAs<std::string>("inputFilters") ? iConfig.getParameter<std::string>("inputFilters") : ""),
  triggerResultsTag(iConfig.getParameter<edm::InputTag>("triggerResults")),
  triggerObjectsTag(iConfig.getParameter<edm::InputTag>("triggerObjects")),

  // vertexes
  verticesTag(iConfig.getParameter<edm::InputTag>("vertices")),

  // rhos
  rhosTag(iConfig.getParameter<edm::InputTag>("rhos")),

  // mets
  metsTag(iConfig.getParameter<edm::InputTag>("mets")),  

  // jets
  jetsTag(iConfig.getParameter<edm::InputTag>("jets")),  

  // photons + ids
  photonsTag          (iConfig.getParameter<edm::InputTag>("photons")),
  photonLooseIdMapTag (iConfig.getParameter<edm::InputTag>("photonLooseID")),
  photonMediumIdMapTag(iConfig.getParameter<edm::InputTag>("photonMediumID")),
  photonTightIdMapTag (iConfig.getParameter<edm::InputTag>("photonTightID")),

  // ootPhotons + ids
  ootPhotonsTag          (iConfig.getParameter<edm::InputTag>("ootPhotons")),
  ootPhotonLooseIdMapTag (iConfig.getParameter<edm::InputTag>("ootPhotonLooseID")),
  ootPhotonMediumIdMapTag(iConfig.getParameter<edm::InputTag>("ootPhotonMediumID")),
  ootPhotonTightIdMapTag (iConfig.getParameter<edm::InputTag>("ootPhotonTightID")),
  
  //recHits
  dumpRHs     (iConfig.existsAs<bool>("dumpRHs") ? iConfig.getParameter<bool>("dumpRHs") : false),
  recHitsEBTag(iConfig.getParameter<edm::InputTag>("recHitsEB")),  
  recHitsEETag(iConfig.getParameter<edm::InputTag>("recHitsEE")),

  ///////////// GEN INFO
  // isMC or Data --> default Data
  isGMSB (iConfig.existsAs<bool>("isGMSB")  ? iConfig.getParameter<bool>("isGMSB")  : false),
  isHVDS (iConfig.existsAs<bool>("isHVDS")  ? iConfig.getParameter<bool>("isHVDS")  : false),
  isBkg  (iConfig.existsAs<bool>("isBkg")   ? iConfig.getParameter<bool>("isBkg")   : false),
  dumpIds(iConfig.existsAs<bool>("dumpIds") ? iConfig.getParameter<bool>("dumpIds") : false)
{
  usesResource();
  usesResource("TFileService");

  // triggers
  triggerResultsToken = consumes<edm::TriggerResults> (triggerResultsTag);
  triggerObjectsToken = consumes<std::vector<pat::TriggerObjectStandAlone> > (triggerObjectsTag);

  // read in from a stream the trigger paths for saving
  oot::ReadInTriggerNames(inputPaths,pathNames,triggerBitMap);

  // read in from a stream the hlt objects/labels to match to
  oot::ReadInFilterNames(inputFilters,filterNames,triggerObjectsByFilterMap);

  //vertex
  verticesToken = consumes<std::vector<reco::Vertex> > (verticesTag);

  // rhos
  rhosToken = consumes<double> (rhosTag);

  // mets
  metsToken = consumes<std::vector<pat::MET> > (metsTag);

  // jets
  jetsToken = consumes<std::vector<pat::Jet> > (jetsTag);

  // photons + ids
  photonsToken           = consumes<std::vector<pat::Photon> > (photonsTag);
  photonLooseIdMapToken  = consumes<edm::ValueMap<bool> > (photonLooseIdMapTag);
  photonMediumIdMapToken = consumes<edm::ValueMap<bool> > (photonMediumIdMapTag);
  photonTightIdMapToken  = consumes<edm::ValueMap<bool> > (photonTightIdMapTag);

  // ootPhotons + ids
  if (not ootPhotonsTag.label().empty())
  {
    ootPhotonsToken           = consumes<std::vector<pat::Photon> > (ootPhotonsTag);
    ootPhotonLooseIdMapToken  = consumes<edm::ValueMap<bool> > (ootPhotonLooseIdMapTag);
    ootPhotonMediumIdMapToken = consumes<edm::ValueMap<bool> > (ootPhotonMediumIdMapTag);
    ootPhotonTightIdMapToken  = consumes<edm::ValueMap<bool> > (ootPhotonTightIdMapTag);
  }

  // rechits
  recHitsEBToken = consumes<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > (recHitsEBTag);
  recHitsEEToken = consumes<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > (recHitsEETag);

  // only for simulated samples
  if (isGMSB || isHVDS || isBkg)
  {
    isMC = true;
    genevtInfoToken = consumes<GenEventInfoProduct>             (iConfig.getParameter<edm::InputTag>("genevt"));
    pileupInfoToken = consumes<std::vector<PileupSummaryInfo> > (iConfig.getParameter<edm::InputTag>("pileup"));
    genpartsToken   = consumes<std::vector<reco::GenParticle> > (iConfig.getParameter<edm::InputTag>("genparts"));   
    genjetsToken    = consumes<std::vector<reco::GenJet> >      (iConfig.getParameter<edm::InputTag>("genjets"));   
  }
  else 
  {
    isMC = false;
  }
}

PhotonDump::~PhotonDump() {}

void PhotonDump::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) 
{
  // TRIGGERS
  edm::Handle<edm::TriggerResults> triggerResultsH;
  iEvent.getByToken(triggerResultsToken, triggerResultsH);

  edm::Handle<std::vector<pat::TriggerObjectStandAlone> > triggerObjectsH;
  iEvent.getByToken(triggerObjectsToken, triggerObjectsH);

  // VERTICES
  edm::Handle<std::vector<reco::Vertex> > verticesH;
  iEvent.getByToken(verticesToken, verticesH);
  
  // MET
  edm::Handle<std::vector<pat::MET> > metsH;
  iEvent.getByToken(metsToken, metsH);

  // RHOS
  edm::Handle<double> rhosH;
  iEvent.getByToken(rhosToken, rhosH);

  // JETS
  edm::Handle<std::vector<pat::Jet> > jetsH;
  iEvent.getByToken(jetsToken, jetsH);
  std::vector<pat::Jet> jets; jets.reserve(jetsH->size());

  // PHOTONS + IDS
  edm::Handle<std::vector<pat::Photon> > photonsH;
  iEvent.getByToken(photonsToken, photonsH);
  int phosize = photonsH->size();
  edm::Handle<edm::ValueMap<bool> > photonLooseIdMapH;
  iEvent.getByToken(photonLooseIdMapToken, photonLooseIdMapH);
  edm::Handle<edm::ValueMap<bool> > photonMediumIdMapH;
  iEvent.getByToken(photonMediumIdMapToken, photonMediumIdMapH);
  edm::Handle<edm::ValueMap<bool> > photonTightIdMapH;
  iEvent.getByToken(photonTightIdMapToken, photonTightIdMapH);

  edm::Handle<std::vector<pat::Photon> > ootPhotonsH;
  edm::Handle<edm::ValueMap<bool> > ootPhotonLooseIdMapH;
  edm::Handle<edm::ValueMap<bool> > ootPhotonMediumIdMapH;
  edm::Handle<edm::ValueMap<bool> > ootPhotonTightIdMapH;
  if (not ootPhotonsToken.isUninitialized())
  {
    iEvent.getByToken(ootPhotonsToken, ootPhotonsH);
    phosize += ootPhotonsH->size();
    iEvent.getByToken(ootPhotonLooseIdMapToken, ootPhotonLooseIdMapH);
    iEvent.getByToken(ootPhotonMediumIdMapToken, ootPhotonMediumIdMapH);
    iEvent.getByToken(ootPhotonTightIdMapToken, ootPhotonTightIdMapH);
  }
  std::vector<oot::Photon> photons; photons.reserve(phosize);

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

  // GEN INFO
  edm::Handle<GenEventInfoProduct> genevtInfoH;
  edm::Handle<std::vector<PileupSummaryInfo> > pileupInfoH;
  edm::Handle<std::vector<reco::GenParticle> > genparticlesH;
  edm::Handle<std::vector<reco::GenJet> > genjetsH;
  genPartVec neutralinos;
  genPartVec vPions;
 
  if (isMC)
  {
    iEvent.getByToken(genevtInfoToken, genevtInfoH);
    iEvent.getByToken(pileupInfoToken, pileupInfoH);
    iEvent.getByToken(genpartsToken,   genparticlesH);
    iEvent.getByToken(genjetsToken,    genjetsH);
  }

  // do some prepping of objects
  oot::PrepTriggerBits(triggerResultsH,iEvent,triggerBitMap);
  oot::PrepTriggerObjects(triggerResultsH,triggerObjectsH,iEvent,triggerObjectsByFilterMap);
  oot::PrepJets(jetsH,jets);
  oot::PrepPhotons(photonsH,photonLooseIdMapH,photonMediumIdMapH,photonTightIdMapH,
		   ootPhotonsH,ootPhotonLooseIdMapH,ootPhotonMediumIdMapH,ootPhotonTightIdMapH,
		   photons);
  if (isGMSB) oot::PrepNeutralinos(genparticlesH,neutralinos);
  if (isHVDS) oot::PrepVPions(genparticlesH,vPions);
	
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
  PhotonDump::InitializeTriggerBranches();
  if (triggerResultsH.isValid())
  {
    for (std::size_t ipath = 0; ipath < pathNames.size(); ipath++)
    {
      triggerBits[ipath] = triggerBitMap[pathNames[ipath]];
    }
  } // end check over valid TriggerResults

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

  /////////////
  //         //
  // MC INFO //
  //         //
  /////////////
  if (isMC) 
  {
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
    if (isGMSB) 
    {
      PhotonDump::InitializeGMSBBranches();
      if (genparticlesH.isValid()) // make sure gen particles exist --> only do this for GMSB
      {
	// Dump gen particle pdgIds
	if (dumpIds) PhotonDump::DumpGenIds(genparticlesH); 

	nNeutoPhGr = 0;
	for (genPartVec::const_iterator gpiter = neutralinos.begin(); gpiter != neutralinos.end(); ++gpiter) // loop over neutralinos
	{
	  nNeutoPhGr++;
	  if (nNeutoPhGr == 1)
	  { 
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
	    const int phdaughter = (gpiter->daughter(0)->pdgId() == 22)?0:1;
	    
	    genph1E    = gpiter->daughter(phdaughter)->energy();
	    genph1pt   = gpiter->daughter(phdaughter)->pt();
	    genph1phi  = gpiter->daughter(phdaughter)->phi();
	    genph1eta  = gpiter->daughter(phdaughter)->eta();
	    
	    // check for a reco match!
	    if (photonsH.isValid() || ootPhotonsH.isValid()) // standard check
	    {
	      int   iph   = 0;
	      float mindR = dRmin;
	      for (std::vector<oot::Photon>::const_iterator phiter = photons.begin(); phiter != photons.end(); ++phiter) // loop over photon vector 
	      {
		if (std::abs(phiter->pt()-genph1pt)/genph1pt < pTres)
		{
		  const float delR = deltaR(genph1phi,genph1eta,phiter->phi(),phiter->eta());
		  if (delR < mindR) 
		  {
		    mindR = delR;
		    genph1match = iph; 
		  } // end check over deltaR
		} // end check over pt resolution
		iph++;
	      } // end loop over reco photons
	    } // end check for reco match

	    // set gravitino daughter stuff
	    const int grdaughter = (gpiter->daughter(0)->pdgId() == 1000039)?0:1;

	    gengr1mass = gpiter->daughter(grdaughter)->mass();
	    gengr1E    = gpiter->daughter(grdaughter)->energy();
	    gengr1pt   = gpiter->daughter(grdaughter)->pt();
	    gengr1phi  = gpiter->daughter(grdaughter)->phi();
	    gengr1eta  = gpiter->daughter(grdaughter)->eta();
	  } // end block over first matched neutralino -> photon + gravitino
       	  else if (nNeutoPhGr == 2)
	  {
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
	    const int phdaughter = (gpiter->daughter(0)->pdgId() == 22)?0:1;
		
	    genph2E    = gpiter->daughter(phdaughter)->energy();
	    genph2pt   = gpiter->daughter(phdaughter)->pt();
	    genph2phi  = gpiter->daughter(phdaughter)->phi();
	    genph2eta  = gpiter->daughter(phdaughter)->eta();
	      
	    // check for a reco match!
	    if (photonsH.isValid() || ootPhotonsH.isValid()) // standard check
	    {
	      int   iph   = 0;
	      float mindR = dRmin;
	      for (std::vector<oot::Photon>::const_iterator phiter = photons.begin(); phiter != photons.end(); ++phiter) // loop over photon vector 
	      {
		if (std::abs(phiter->pt()-genph2pt)/genph2pt < pTres)
		{
		  const float delR = deltaR(genph2phi,genph2eta,phiter->phi(),phiter->eta());
		  if (delR < mindR) 
		  {
		    mindR = delR;
		    genph2match = iph;
		  } // end check over deltaR
		} // end check over pt resolution
		iph++;
	      } // end loop over reco photons
	    } // end check for reco match
		
	    // set gravitino daughter stuff
	    const int grdaughter = (gpiter->daughter(0)->pdgId() == 1000039)?0:1;

	    gengr2mass = gpiter->daughter(grdaughter)->mass();
	    gengr2E    = gpiter->daughter(grdaughter)->energy();
	    gengr2pt   = gpiter->daughter(grdaughter)->pt();
	    gengr2phi  = gpiter->daughter(grdaughter)->phi();
	    gengr2eta  = gpiter->daughter(grdaughter)->eta();
	  } // end block over second matched neutralino -> photon + gravitino
	} // end loop over good neutralinos
      } // end check for gen particles
    } // end check over isGMSB
 
    if (isHVDS) 
    {
      PhotonDump::ClearHVDSBranches();
      if (genparticlesH.isValid()) // make sure gen particles exist --> only do this for GMSB
      {
	if (dumpIds) PhotonDump::DumpGenIds(genparticlesH); 

	PhotonDump::InitializeHVDSBranches();
	nvPions = vPions.size();
	for (std::vector<reco::GenParticle>::const_iterator gpiter = vPions.begin(); gpiter != vPions.end(); ++gpiter)
	{
	  // set neutralino parameters
	  genvPionmass.push_back(gpiter->mass());
	  genvPionE   .push_back(gpiter->energy());
	  genvPionpt  .push_back(gpiter->pt());
	  genvPionphi .push_back(gpiter->phi());
	  genvPioneta .push_back(gpiter->eta());
	  
	  // vPion production vertex
	  genvPionprodvx.push_back(gpiter->vx());
	  genvPionprodvy.push_back(gpiter->vy());
	  genvPionprodvz.push_back(gpiter->vz());
	  
	  // vPion decay vertex (same for both daughters unless really screwed up)
	  genvPiondecayvx.push_back(gpiter->daughter(0)->vx());
	  genvPiondecayvy.push_back(gpiter->daughter(0)->vy());
	  genvPiondecayvz.push_back(gpiter->daughter(0)->vz());
	  
	  const int leading    = (gpiter->daughter(0)->pt()>gpiter->daughter(1)->pt())?0:1;
	  const int subleading = (gpiter->daughter(0)->pt()>gpiter->daughter(1)->pt())?1:0;
	  
	  genHVph1E  .push_back(gpiter->daughter(leading)->energy());
	  genHVph1pt .push_back(gpiter->daughter(leading)->pt());
	  genHVph1phi.push_back(gpiter->daughter(leading)->phi());
	  genHVph1eta.push_back(gpiter->daughter(leading)->eta());
	  
	  genHVph2E  .push_back(gpiter->daughter(subleading)->energy());
	  genHVph2pt .push_back(gpiter->daughter(subleading)->pt());
	  genHVph2phi.push_back(gpiter->daughter(subleading)->phi());
	  genHVph2eta.push_back(gpiter->daughter(subleading)->eta());
	  
	  // check for a reco match!
	  if (photonsH.isValid() || ootPhotonsH.isValid()) // standard check
	  {
	    int   tmpph1 = -9999, tmpph2 = -9999;
	    int   iph = 0;
	    float mindR1 = dRmin, mindR2 = dRmin; // at least this much
	    for (std::vector<oot::Photon>::const_iterator phiter = photons.begin(); phiter != photons.end(); ++phiter) // loop over photon vector 
	    {
	      const float tmppt  = phiter->pt();
	      const float tmpphi = phiter->phi();
	      const float tmpeta = phiter->eta();
		  
	      // check photon 1
	      if (std::abs(tmppt-genHVph1pt.back())/genHVph1pt.back() < pTres)
	      {
		const float delR = deltaR(genHVph1phi.back(),genHVph1eta.back(),tmpphi,tmpeta);
		if (delR < mindR1) 
		{
		  mindR1 = delR;
		  tmpph1 = iph;
		} // end check over deltaR
	      } // end check over pt resolution
	      
	      // check photon 2
	      if (std::abs(tmppt-genHVph2pt.back())/genHVph2pt.back() < pTres)
	      {
		const float delR = deltaR(genHVph2phi.back(),genHVph2eta.back(),tmpphi,tmpeta);
		if (delR < mindR2) 
		{
		  mindR2 = delR;
		  tmpph2 = iph;
		} // end check over deltaR
	      } // end check over pt resolution
	      
	      // now update iph
	      iph++;
	    } // end loop over reco photons
	    
	    // now save tmp photon iphs
	    genHVph1match.push_back(tmpph1);
	    genHVph2match.push_back(tmpph2);
	  } // end check for reco match
	} // end loop over good vPions
      } // end check over valid gen particles
    } // end check over isHVDS
    
    ///////////////////
    //               //
    // Gen Jets info //
    //               //
    ///////////////////
    if (isGMSB) // too many jets in background samples
    {
      PhotonDump::ClearGenJetBranches();
      if (genjetsH.isValid()) // make sure gen particles exist
      {
	ngenjets = genjetsH->size();
	if (ngenjets > 0) PhotonDump::InitializeGenJetBranches();
	int igjet = 0;
	for (std::vector<reco::GenJet>::const_iterator gjetiter = genjetsH->begin(); gjetiter != genjetsH->end(); ++gjetiter) // loop over genjets
        {
	  genjetE  [igjet] = gjetiter->energy();
	  genjetpt [igjet] = gjetiter->pt();
	  genjetphi[igjet] = gjetiter->phi();
	  genjeteta[igjet] = gjetiter->eta();

	  // check for gen to reco match
	  if (jetsH.isValid()) // check to make sure reco (AK4) jets exist
	  {
	    int   ijet  = 0;
	    float mindR = dRmin;
	    for (std::vector<pat::Jet>::const_iterator jetiter = jets.begin(); jetiter != jets.end(); ++jetiter) // loop over reco jets for match
	    {
	      if (std::abs(genjetpt[igjet]-jetiter->pt())/genjetpt[igjet] < pTres) // pt resolution check
	      {
		const float delR = deltaR(genjetphi[igjet],genjeteta[igjet],jetiter->phi(),jetiter->eta());
		if (delR < mindR) // deltaR matching conditional
	        {
		  mindR = delR;
		  genjetmatch[igjet] = ijet;
		} // end check over deltaR
	      } // end check over pt resolution
	      ijet++;
	    } // end loop over reco jets
	  } // end check over reco jets 

	  igjet++;
	} // end loop over genjets
      } // end check over genjets
    } // end check over isGMSB
  } // end block over isMC

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

    if (isGMSB || isHVDS)
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
    njets = jets.size();
    if (njets > 0) PhotonDump::InitializeJetBranches();
    int ijet = 0;
    for (std::vector<pat::Jet>::const_iterator jetiter = jets.begin(); jetiter != jets.end(); ++jetiter)
    {
      jetE  [ijet] = jetiter->energy();
      jetpt [ijet] = jetiter->pt();
      jetphi[ijet] = jetiter->phi();
      jeteta[ijet] = jetiter->eta();

      // check if reco jet is matched to gen jet
      if (isGMSB)
      {
	for (std::size_t igjet = 0; igjet < genjetmatch.size(); igjet++) // loop over gen jet matches
        {
	  if (genjetmatch[igjet] == ijet) 
	  {
	    jetmatch[ijet] = int(igjet);
	    break;
	  } // end conditional over check
	} // end loop over gen jets matches
      } // end block over isGMSB

      ijet++;
    } // end loop over reco jets
  } // end check over reco jets
  
  //////////////////
  //              //
  // Reco Photons //
  //              //
  //////////////////
  PhotonDump::ClearRecoPhotonBranches();
  if (photonsH.isValid() || ootPhotonsH.isValid()) // standard handle check
  {
    nphotons = photons.size();
    if (nphotons > 0) PhotonDump::InitializeRecoPhotonBranches();

    int iph = 0;
    for (std::vector<oot::Photon>::const_iterator phiter = photons.begin(); phiter != photons.end(); ++phiter) // loop over photon vector
    {
      const pat::Photon & photon = phiter->photon();

      // from ootPhoton collection
      phisOOT[iph] = phiter->isOOT();

      // standard photon branches
      phE  [iph] = photon.energy();
      phpt [iph] = photon.pt();
      phphi[iph] = photon.phi();
      pheta[iph] = photon.eta();

      // check for HLT filter matches!
      strBitMap isHLTMatched; 
      for (const auto & filter : filterNames) isHLTMatched[filter] = false;
      oot::HLTToObjectMatching(triggerObjectsByFilterMap,isHLTMatched,*phiter,pTres,dRmin);
      for (std::size_t ifilter = 0; ifilter < filterNames.size(); ifilter++)
      {
	phIsHLTMatched[iph][ifilter] = isHLTMatched[filterNames[ifilter]];
      }

      // Check for gen level match
      if (isMC) 
      {
	if (isGMSB)
	{
	  if      (iph == genph1match && iph != genph2match) phmatch[iph] = 1; // matched to photon from leading neutralino 
	  else if (iph == genph2match && iph != genph1match) phmatch[iph] = 2; // matched to photon from subleading neutralino
	  else if (iph == genph1match && iph == genph2match) phmatch[iph] = 3; // matched to both photons from both neutralinos (probably will never happen)
	  else                                               phmatch[iph] = 0; // no corresponding match
	} 
	if (isHVDS) 
	{
	  // reminder genHVph(1?2)match is a vector of size nvPions --> stores the index of reco photon that a gen photon from a dark pion is matched to
	  // since we can have many dipho pairs, store just if reco photon is matched to dark pion decay
	  // so meaning of 1,2, and 3 change

	  bool tmpph1match = false;
	  bool tmpph2match = false;
	  for (unsigned int jph = 0; jph < genHVph1match.size(); jph++)
	  {
	    if (genHVph1match[jph] == iph)
	    { 
	      tmpph1match = true;
	      break;
	    }
	  } // end loop over genHVph1matches
	  for (unsigned int jph = 0; jph < genHVph2match.size(); jph++)
	  {
	    if (genHVph2match[jph] == iph) 
	    {
	      tmpph2match = true;
	      break;
	    }
	  } // end loop over genHVph2matches

	  if      (tmpph1match && !tmpph2match) phmatch[iph] = 1; // 1 means "I am matched to some photon inside genHVph1match and not to any photon in genHVph2match"
	  else if (tmpph2match && !tmpph1match) phmatch[iph] = 2; // 2 means "I am matched to some photon inside genHVph2match and not to any photon in genHVph1match"
	  else if (tmpph1match &&  tmpph2match) phmatch[iph] = 3; // 3 means "I am matched to some photon inside genHVph1match and some photon in genHVph2match" (i.e. the generator photons merged into a single reco photon)
	  else                                  phmatch[iph] = 0; // no corresponding match
	} // end block over HVDS
	phIsGenMatched[iph] = oot::GenToObjectMatching(*phiter,genparticlesH,pTres,dRmin);
      }

      // super cluster from photon
      const reco::SuperClusterRef& phsc = photon.superCluster().isNonnull() ? photon.superCluster() : photon.parentSuperCluster();
      phscE  [iph] = phsc->energy();
      phsceta[iph] = phsc->eta();
      phscphi[iph] = phsc->phi();
      const float sceta = std::abs(phsceta[iph]);

      // Shower Shape Objects
      const reco::Photon::ShowerShape& phshape = photon.full5x5_showerShapeVariables(); // photon.showerShapeVariables();

      // ID-like variables
      phHoE   [iph] = photon.hadTowOverEm(); // used in ID
      phr9    [iph] = photon.r9();
      phChgIso[iph] = std::max(photon.chargedHadronIso() - (rho * oot::GetChargedHadronEA(sceta)),0.f);
      phNeuIso[iph] = std::max(photon.neutralHadronIso() - (rho * oot::GetNeutralHadronEA(sceta)),0.f);
      phIso   [iph] = std::max(photon.photonIso()        - (rho * oot::GetGammaEA        (sceta)),0.f);

      // cluster shape variables
      phsieie[iph] = phshape.sigmaIetaIeta;
      phsipip[iph] = phshape.sigmaIphiIphi;
      phsieip[iph] = phshape.sigmaIetaIphi;

      // 0 --> did not pass anything, 1 --> loose pass, 2 --> medium pass, 3 --> tight pass
      if      (photon.photonID("tight"))  {phVID[iph] = 3;}
      else if (photon.photonID("medium")) {phVID[iph] = 2;}
      else if (photon.photonID("loose"))  {phVID[iph] = 1;}
      else                                {phVID[iph] = 0;}

      // use seed to get geometry and recHits
      const DetId seedDetId = phsc->seed()->seed(); //seed detid
      const bool isEB = (seedDetId.subdetId() == EcalBarrel); //which subdet
      const EcalRecHitCollection * recHits = (isEB ? recHitsEBH : recHitsEEH).product();

      // 2nd moments from official calculation
      if (recHits->size() > 0)
      {
	const Cluster2ndMoments ph2ndMoments = noZS::EcalClusterTools::cluster2ndMoments( *phsc, *recHits);
	// radius of semi-major,minor axis is the inverse square root of the eigenvalues of the covariance matrix
	phsmaj [iph] = ph2ndMoments.sMaj;
	phsmin [iph] = ph2ndMoments.sMin;
	phalpha[iph] = ph2ndMoments.alpha;
      }

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

void PhotonDump::DumpGenIds(const edm::Handle<std::vector<reco::GenParticle> > & genparticlesH)
{
  std::cout << "event: " << event << std::endl;
  
  for (std::vector<reco::GenParticle>::const_iterator gpiter = genparticlesH->begin(); gpiter != genparticlesH->end(); ++gpiter) // loop over gen particles
  {
    if ((isGMSB && gpiter->pdgId() != 1000022) && (isHVDS && gpiter->pdgId() != 4900111)) continue;

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

void PhotonDump::DumpRecHitInfo(const int iph, const DetIdPairVec & hitsAndFractions, const EcalRecHitCollection *& recHits)
{
  std::cout << "event: " << event << std::endl;
  if (phE[iph] > 100.f && phmatch[iph] > 0 && phVID[iph] >= 2)
  { 
    if ((std::abs(pheta[iph]) < Config::etaEEmax) && !(std::abs(pheta[iph]) > Config::etaEBmax && std::abs(pheta[iph]) < Config::etaEEmin))  
    {
      if (phnrh[iph] == 0)
      {
	const int hafsize = hitsAndFractions.size();
	std::cout <<  "phE: " << phE[iph]   << " phphi: " << phphi[iph]   << " pheta: " << pheta[iph]   << std::endl;
	std::cout << " scE: " << phscE[iph] << " scphi: " << phscphi[iph] << " sceta: " << phsceta[iph] << " haf.size()= " << hafsize << std::endl;
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

void PhotonDump::InitializeTriggerBranches()
{
  for (std::size_t ipath = 0; ipath < pathNames.size(); ipath++)
  { 
    triggerBits[ipath] = false;
  }
}

void PhotonDump::InitializeGenEvtBranches()
{
  genwgt = -9999.f;
}

void PhotonDump::InitializeGenPUBranches()
{
  genpuobs = -9999; genputrue = -9999;
}

void PhotonDump::InitializeGMSBBranches()
{
  // Gen particle info
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

void PhotonDump::ClearHVDSBranches()
{
  nvPions = -9999;
  
  genvPionmass.clear();
  genvPionE   .clear();
  genvPionpt  .clear();
  genvPionphi .clear();
  genvPioneta .clear();
  
  genvPionprodvx.clear();
  genvPionprodvy.clear();
  genvPionprodvz.clear();
  
  genvPiondecayvx.clear();
  genvPiondecayvy.clear();
  genvPiondecayvz.clear();
  
  genHVph1E  .clear();
  genHVph1pt .clear();
  genHVph1phi.clear();
  genHVph1eta.clear();
  
  genHVph2E  .clear();
  genHVph2pt .clear();
  genHVph2phi.clear();
  genHVph2eta.clear();
  
  genHVph1match.clear();
  genHVph2match.clear();
}

void PhotonDump::InitializeHVDSBranches()
{
  nvPions = 0;
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
 
  if (isGMSB || isHVDS)
  {
    t1pfMETgenMETpt = -9999.f; t1pfMETgenMETphi = -9999.f; t1pfMETgenMETsumEt = -9999.f;
  }
}

void PhotonDump::ClearJetBranches()
{
  njets = -9999;

  if (isGMSB) jetmatch.clear();

  jetE.clear();
  jetpt.clear();
  jetphi.clear();
  jeteta.clear();
}

void PhotonDump::InitializeJetBranches()
{
  if (isGMSB) jetmatch.resize(njets);

  jetE.resize(njets);
  jetpt.resize(njets);
  jetphi.resize(njets);
  jeteta.resize(njets);
  
  for (int ijet = 0; ijet < njets; ijet++)
  {
    if (isGMSB) jetmatch[ijet] = -9999;

    jetE  [ijet] = -9999.f;
    jetpt [ijet] = -9999.f;
    jetphi[ijet] = -9999.f;
    jeteta[ijet] = -9999.f;
  }
}

void PhotonDump::ClearRecoPhotonBranches()
{
  nphotons = -9999;

  phisOOT.clear();

  phE.clear();
  phpt.clear();
  phphi.clear(); 
  pheta.clear(); 

  if (isGMSB || isHVDS) phmatch.clear();
  if (isMC) phIsGenMatched.clear();

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

  phVID.clear();

  phIsHLTMatched.clear();

  phnrh.clear();
  phseedpos.clear();

  phrheta.clear(); 
  phrhphi.clear(); 
  phrhE.clear(); 
  phrhtime.clear();
  phrhID.clear();
  phrhOOT.clear();
}

void PhotonDump::InitializeRecoPhotonBranches()
{
  phisOOT.resize(nphotons);

  phE.resize(nphotons);
  phpt.resize(nphotons);
  phphi.resize(nphotons);
  pheta.resize(nphotons);

  if (isGMSB || isHVDS) phmatch.resize(nphotons);
  if (isMC) phIsGenMatched.resize(nphotons);
  
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

  phVID.resize(nphotons);

  phIsHLTMatched.resize(nphotons);

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
    phisOOT[iph] = -1;

    phE  [iph] = -9999.f; 
    phpt [iph] = -9999.f; 
    phphi[iph] = -9999.f; 
    pheta[iph] = -9999.f; 

    if (isGMSB || isHVDS) phmatch [iph] = -9999;
    if (isMC) phIsGenMatched[iph] = false;

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

    phVID     [iph] = -9999;

    phnrh    [iph] = -9999;
    phseedpos[iph] = -9999;

    phIsHLTMatched[iph].resize(filterNames.size());
    for (std::size_t ifilter = 0; ifilter < filterNames.size(); ifilter++)
    {
      phIsHLTMatched[iph][ifilter] = 0; // false
    }
  }
}

void PhotonDump::InitializeRecoRecHitBranches(const int iph)
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

void PhotonDump::beginJob() 
{
  edm::Service<TFileService> fs;
  tree = fs->make<TTree>("tree"     , "tree");

  // Run, Lumi, Event info
  tree->Branch("event"                  , &event                , "event/l");
  tree->Branch("run"                    , &run                  , "run/i");
  tree->Branch("lumi"                   , &lumi                 , "lumi/i");
   
  // Trigger Info
  tree->Branch("triggerBits"            , &triggerBits);

  // Vertex info
  tree->Branch("nvtx"                   , &nvtx                 , "nvtx/I");
  tree->Branch("vtxX"                   , &vtxX                 , "vtxX/F");
  tree->Branch("vtxY"                   , &vtxY                 , "vtxY/F");
  tree->Branch("vtxZ"                   , &vtxZ                 , "vtxZ/F");

  if (isMC)
  {
    // Generator inf
    tree->Branch("genwgt"               , &genwgt               , "genwgt/F");
    tree->Branch("genpuobs"             , &genpuobs             , "genpuobs/I");
    tree->Branch("genputrue"            , &genputrue            , "genputrue/I");
  }

  if (isGMSB)
  {
    // Gen particle info
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

  // HVDS info
  if (isHVDS)
  {
    tree->Branch("nvPions"              , &nvPions             , "nvPions/I");

    tree->Branch("genvPionprodvx"       , &genvPionprodvx);
    tree->Branch("genvPionprodvy"       , &genvPionprodvy);
    tree->Branch("genvPionprodvz"       , &genvPionprodvz);

    tree->Branch("genvPiondecayvx"      , &genvPiondecayvx);
    tree->Branch("genvPiondecayvy"      , &genvPiondecayvy);
    tree->Branch("genvPiondecayvz"      , &genvPiondecayvz);

    tree->Branch("genvPionmass"         , &genvPionmass);
    tree->Branch("genvPionE"            , &genvPionE);   
    tree->Branch("genvPionpt"           , &genvPionpt);
    tree->Branch("genvPionphi"          , &genvPionphi);
    tree->Branch("genvPioneta"          , &genvPioneta);

    tree->Branch("genHVph1E"            , &genHVph1E);   
    tree->Branch("genHVph1pt"           , &genHVph1pt);
    tree->Branch("genHVph1phi"          , &genHVph1phi);
    tree->Branch("genHVph1eta"          , &genHVph1eta);

    tree->Branch("genHVph2E"            , &genHVph2E);   
    tree->Branch("genHVph2pt"           , &genHVph2pt);
    tree->Branch("genHVph2phi"          , &genHVph2phi);
    tree->Branch("genHVph2eta"          , &genHVph2eta);

    tree->Branch("genHVph1match"        , &genHVph1match);
    tree->Branch("genHVph2match"        , &genHVph2match);
  }

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
  
  if (isGMSB || isHVDS)
  {
    tree->Branch("t1pfMETgenMETpt"      , &t1pfMETgenMETpt      , "t1pfMETgenMETpt/F");
    tree->Branch("t1pfMETgenMETphi"     , &t1pfMETgenMETphi     , "t1pfMETgenMETphi/F");
    tree->Branch("t1pfMETgenMETsumEt"   , &t1pfMETgenMETsumEt   , "t1pfMETgenMETsumEt/F");
  }

  // Jet Info
  tree->Branch("njets"                , &njets                , "njets/I");
  if (isGMSB) tree->Branch("jetmatch" , &jetmatch);
  tree->Branch("jetE"                 , &jetE);
  tree->Branch("jetpt"                , &jetpt);
  tree->Branch("jetphi"               , &jetphi);
  tree->Branch("jeteta"               , &jeteta);
   
  // Photon Info
  tree->Branch("nphotons"             , &nphotons             , "nphotons/I");
  tree->Branch("phisOOT"              , &phisOOT);

  tree->Branch("phE"                  , &phE);
  tree->Branch("phpt"                 , &phpt);
  tree->Branch("phphi"                , &phphi);
  tree->Branch("pheta"                , &pheta);

  if (isGMSB || isHVDS) tree->Branch("phmatch", &phmatch);
  if (isMC) tree->Branch("phIsGenMatched", &phIsGenMatched);

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

  tree->Branch("phVID"                , &phVID);

  tree->Branch("phIsHLTMatched"       , &phIsHLTMatched);

  tree->Branch("phnrh"                , &phnrh);
  tree->Branch("phseedpos"            , &phseedpos);

  tree->Branch("phrheta"              , &phrheta);
  tree->Branch("phrhphi"              , &phrhphi);
  tree->Branch("phrhE"                , &phrhE);
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
