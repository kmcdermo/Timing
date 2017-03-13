#include "PhotonDump.h"

PhotonDump::PhotonDump(const edm::ParameterSet& iConfig): 
  // triggers
  dumpTriggerMenu  (iConfig.existsAs<bool>("dumpTriggerMenu") ? iConfig.getParameter<bool>("dumpTriggerMenu") : false),
  isHLT2           (iConfig.existsAs<bool>("isHLT2")          ? iConfig.getParameter<bool>("isHLT2")          : false),
  isHLT3           (iConfig.existsAs<bool>("isHLT3")          ? iConfig.getParameter<bool>("isHLT3")          : false),
  triggerResultsTag(iConfig.getParameter<edm::InputTag>("triggerResults")),

  // vertexes
  verticesTag(iConfig.getParameter<edm::InputTag>("vertices")),

  // rhos
  rhosTag(iConfig.getParameter<edm::InputTag>("rhos")),

  // mets
  metsTag(iConfig.getParameter<edm::InputTag>("mets")),  

  // jets
  jetsTag(iConfig.getParameter<edm::InputTag>("jets")),  

  // photons + ids
  photonLooseIdMapTag (iConfig.getParameter<edm::InputTag>("loosePhotonID")),  
  photonMediumIdMapTag(iConfig.getParameter<edm::InputTag>("mediumPhotonID")),  
  photonTightIdMapTag (iConfig.getParameter<edm::InputTag>("tightPhotonID")),  
  photonsTag          (iConfig.getParameter<edm::InputTag>("photons")),  
  
  //recHits
  dumpRHs              (iConfig.existsAs<bool>("dumpRHs") ? iConfig.getParameter<bool>("dumpRHs") : false),
  recHitCollectionEBTAG(consumes<EcalRecHitCollection>(iConfig.getParameter<edm::InputTag>( "recHitCollectionEB" ))),
  recHitCollectionEETAG(consumes<EcalRecHitCollection>(iConfig.getParameter<edm::InputTag>( "recHitCollectionEE" ))),

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

  //vertex
  verticesToken = consumes<std::vector<reco::Vertex> > (verticesTag);

  // rhos
  rhosToken = consumes<double> (rhosTag);

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
  std::vector<pat::Jet> jets = *jetsH;

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
  std::vector<pat::Photon> photons = *photonsH;

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

  if (isMC)
  {
    iEvent.getByToken(genevtInfoToken, genevtInfoH);
    iEvent.getByToken(pileupInfoToken, pileupInfoH);
    iEvent.getByToken(genpartsToken,   genparticlesH);
    iEvent.getByToken(genjetsToken,    genjetsH);
  }

  // do some prepping of objects
  PhotonDump::PrepJets(jetsH,jets);
  PhotonDump::PrepPhotons(photonsH,photonLooseIdMap,photonMediumIdMap,photonTightIdMap,photons);

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
    if (isHLT2)
    {
     for (std::size_t i = 0; i < triggerNames.size(); i++)
     {
       if (triggerIndex[triggerNames[i]] == -1) continue;	
       if (i == 0  && triggerResultsH->accept(triggerIndex[triggerNames[i]])) hltpho120_met40       = true;
       if (i == 1  && triggerResultsH->accept(triggerIndex[triggerNames[i]])) hltpho175             = true;
       if (i == 2  && triggerResultsH->accept(triggerIndex[triggerNames[i]])) hltdoublepho60        = true;
       if (i == 3  && triggerResultsH->accept(triggerIndex[triggerNames[i]])) hltdispho45           = true; 
       if (i == 4  && triggerResultsH->accept(triggerIndex[triggerNames[i]])) hltdispho45_jet50     = true; 
       if (i == 5  && triggerResultsH->accept(triggerIndex[triggerNames[i]])) hltdispho45_dijet50   = true; 
       if (i == 6  && triggerResultsH->accept(triggerIndex[triggerNames[i]])) hltdispho45_dijet35   = true; 
       if (i == 7  && triggerResultsH->accept(triggerIndex[triggerNames[i]])) hltdispho45_trijet35  = true; 
       if (i == 8  && triggerResultsH->accept(triggerIndex[triggerNames[i]])) hltdispho45_el100veto = true; 
       if (i == 9  && triggerResultsH->accept(triggerIndex[triggerNames[i]])) hltdispho45_notkveto  = true; 
       if (i == 10 && triggerResultsH->accept(triggerIndex[triggerNames[i]])) hltdispho50           = true; 
       if (i == 11 && triggerResultsH->accept(triggerIndex[triggerNames[i]])) hltdispho60           = true; 
       if (i == 12 && triggerResultsH->accept(triggerIndex[triggerNames[i]])) hltdispho60_jet50     = true; 
       if (i == 13 && triggerResultsH->accept(triggerIndex[triggerNames[i]])) hltdispho60_dijet50   = true; 
       if (i == 14 && triggerResultsH->accept(triggerIndex[triggerNames[i]])) hltdispho60_dijet35   = true; 
       if (i == 15 && triggerResultsH->accept(triggerIndex[triggerNames[i]])) hltdispho60_trijet35  = true; 
       if (i == 16 && triggerResultsH->accept(triggerIndex[triggerNames[i]])) hltdispho60_notkveto  = true; 
       if (i == 17 && triggerResultsH->accept(triggerIndex[triggerNames[i]])) hltdispho80           = true; 
       if (i == 18 && triggerResultsH->accept(triggerIndex[triggerNames[i]])) hltdispho100          = true;
     } // end loop over trigger names
    } // end check over HLT2
    else if (isHLT3)
    {
     for (std::size_t i = 0; i < triggerNames.size(); i++)
     {
       if (triggerIndex[triggerNames[i]] == -1) continue;	
       if (i == 0  && triggerResultsH->accept(triggerIndex[triggerNames[i]])) hltdispho45           = true;
       if (i == 1  && triggerResultsH->accept(triggerIndex[triggerNames[i]])) hltdispho45_notkveto  = true;
       if (i == 2  && triggerResultsH->accept(triggerIndex[triggerNames[i]])) hltdispho45_notime    = true;
       if (i == 3  && triggerResultsH->accept(triggerIndex[triggerNames[i]])) hltdispho45_nosmaj    = true;
       if (i == 4  && triggerResultsH->accept(triggerIndex[triggerNames[i]])) hltdispho45_nosmin    = true;
       if (i == 5  && triggerResultsH->accept(triggerIndex[triggerNames[i]])) hltdispho45_nosieie   = true;
       if (i == 6  && triggerResultsH->accept(triggerIndex[triggerNames[i]])) hltdispho45_nor9      = true;
       if (i == 7  && triggerResultsH->accept(triggerIndex[triggerNames[i]])) hltdispho45_nohoe     = true;
       if (i == 8  && triggerResultsH->accept(triggerIndex[triggerNames[i]])) hltdispho45_noet      = true;
       if (i == 9  && triggerResultsH->accept(triggerIndex[triggerNames[i]])) hltdispho45_nol1match = true;
     }
    }
  } // end check over valid triggerResults

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
	
	nNeutralino = 0;
	nNeutoPhGr  = 0;
	std::vector<reco::GenParticle> neutralinos;
	for (std::vector<reco::GenParticle>::const_iterator gpiter = genparticlesH->begin(); gpiter != genparticlesH->end(); ++gpiter) // loop over gen particles
        {
	  if (nNeutoPhGr == 2) break;

	  if (gpiter->pdgId() == 1000022 && gpiter->numberOfDaughters() == 2)
	  {
	    nNeutralino++;
	    
	    if ((gpiter->daughter(0)->pdgId() == 22 && gpiter->daughter(1)->pdgId() == 1000039) ||
		(gpiter->daughter(1)->pdgId() == 22 && gpiter->daughter(0)->pdgId() == 1000039)) 
	    {
	      nNeutoPhGr++;
	      neutralinos.push_back((*gpiter));
	    } // end conditional over matching daughter ids
	  } // end conditional over neutralino id
	} // end loop over gen particles

	std::sort(neutralinos.begin(),neutralinos.end(),sortByGenParticlePt);

	nNeutoPhGr = 0; // reuse
	for (std::vector<reco::GenParticle>::const_iterator gpiter = neutralinos.begin(); gpiter != neutralinos.end(); ++gpiter) // loop over neutralinos
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
	    int phdaughter = (gpiter->daughter(0)->pdgId() == 22)?0:1;
	    
	    genph1E    = gpiter->daughter(phdaughter)->energy();
	    genph1pt   = gpiter->daughter(phdaughter)->pt();
	    genph1phi  = gpiter->daughter(phdaughter)->phi();
	    genph1eta  = gpiter->daughter(phdaughter)->eta();
	    
	    // check for a reco match!
	    if (photonsH.isValid()) // standard check
	    {
	      int   iph   = 0;
	      float mindR = 0.3; // at least this much
	      for (std::vector<pat::Photon>::const_iterator phiter = photons.begin(); phiter != photons.end(); ++phiter) // loop over photon vector 
	      {
		const float tmppt  = phiter->pt();
		const float tmpphi = phiter->phi();
		const float tmpeta = phiter->eta();
		
		if (std::abs(tmppt-genph1pt)/genph1pt < 0.5)
		{
		  const float delR = deltaR(genph1phi,genph1eta,tmpphi,tmpeta);
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
	    int grdaughter = (gpiter->daughter(0)->pdgId() == 1000039)?0:1;

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
	    int phdaughter = (gpiter->daughter(0)->pdgId() == 22)?0:1;
		
	    genph2E    = gpiter->daughter(phdaughter)->energy();
	    genph2pt   = gpiter->daughter(phdaughter)->pt();
	    genph2phi  = gpiter->daughter(phdaughter)->phi();
	    genph2eta  = gpiter->daughter(phdaughter)->eta();
	      
	    // check for a reco match!
	    if (photonsH.isValid()) // standard check
	    {
	      int   iph   = 0;
	      float mindR = 0.3; // at least this much
	      for (std::vector<pat::Photon>::const_iterator phiter = photons.begin(); phiter != photons.end(); ++phiter) // loop over photon vector 
	      {
		const float tmppt  = phiter->pt();
		const float tmpphi = phiter->phi();
		const float tmpeta = phiter->eta();
		
		if (std::abs(tmppt-genph2pt)/genph2pt < 0.5)
		{
		  const float delR = deltaR(genph2phi,genph2eta,tmpphi,tmpeta);
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
	    int grdaughter = (gpiter->daughter(0)->pdgId() == 1000039)?0:1;

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
	std::vector<reco::GenParticle> vPions;
    	for (std::vector<reco::GenParticle>::const_iterator gpiter = genparticlesH->begin(); gpiter != genparticlesH->end(); ++gpiter) // loop over gen particles
        {
    	  if (gpiter->pdgId() == 4900111 && gpiter->numberOfDaughters() == 2)
    	  {
	    if (gpiter->daughter(0)->pdgId() == 22 && gpiter->daughter(1)->pdgId() == 22)
	    {
	      nvPions++;
	      vPions.push_back((*gpiter));
	    } // end check over both gen photons	
	  } // end check over vPions
	} // end loop over gen particles

	std::sort(vPions.begin(),vPions.end(),sortByGenParticlePt);

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
	  
	  int leading    = (gpiter->daughter(0)->pt()>gpiter->daughter(1)->pt())?0:1;
	  int subleading = (gpiter->daughter(0)->pt()>gpiter->daughter(1)->pt())?1:0;
	  
	  genHVph1E  .push_back(gpiter->daughter(leading)->energy());
	  genHVph1pt .push_back(gpiter->daughter(leading)->pt());
	  genHVph1phi.push_back(gpiter->daughter(leading)->phi());
	  genHVph1eta.push_back(gpiter->daughter(leading)->eta());
	  
	  genHVph2E  .push_back(gpiter->daughter(subleading)->energy());
	  genHVph2pt .push_back(gpiter->daughter(subleading)->pt());
	  genHVph2phi.push_back(gpiter->daughter(subleading)->phi());
	  genHVph2eta.push_back(gpiter->daughter(subleading)->eta());
	  
	  // check for a reco match!
	  if (photonsH.isValid()) // standard check
	  {
	    int   tmpph1 = -9999, tmpph2 = -9999;
	    int   iph = 0;
	    float mindR1 = 0.3, mindR2 = 0.3; // at least this much
	    for (std::vector<pat::Photon>::const_iterator phiter = photons.begin(); phiter != photons.end(); ++phiter) // loop over photon vector 
	    {
	      const float tmppt  = phiter->pt();
	      const float tmpphi = phiter->phi();
	      const float tmpeta = phiter->eta();
		  
	      // check photon 1
	      if (std::abs(tmppt-genHVph1pt.back())/genHVph1pt.back() < 0.5)
	      {
		const float delR = deltaR(genHVph1phi.back(),genHVph1eta.back(),tmpphi,tmpeta);
		if (delR < mindR1) 
		{
		  mindR1 = delR;
		  tmpph1 = iph;
		} // end check over deltaR
	      } // end check over pt resolution
	      
	      // check photon 2
	      if (std::abs(tmppt-genHVph2pt.back())/genHVph2pt.back() < 0.5)
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
	    float mindR = 0.4;
	    for (std::vector<pat::Jet>::const_iterator jetiter = jets.begin(); jetiter != jets.end(); ++jetiter) // loop over reco jets for match
	    {
	      if (std::abs(genjetpt[igjet]-jetiter->pt())/genjetpt[igjet] < 0.5) // pt resolution check
	      {
		const float delR = deltaR(jetiter->phi(),gjetiter->eta(),jetiter->phi(),jetiter->eta());
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

    if (isGMSB)
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
  if (photonsH.isValid()) // standard handle check
  {
    // ECALELF tools
    EcalClusterLazyTools * clustertools = new EcalClusterLazyTools (iEvent, iSetup, recHitCollectionEBTAG, recHitCollectionEETAG);

    nphotons = photons.size();
    if (nphotons > 0) PhotonDump::InitializeRecoPhotonBranches();

    int iph = 0;
    for (std::vector<pat::Photon>::const_iterator phiter = photons.begin(); phiter != photons.end(); ++phiter) // loop over photon vector
    {
      // standard photon branches
      phE  [iph] = phiter->energy();
      phpt [iph] = phiter->pt();
      phphi[iph] = phiter->phi();
      pheta[iph] = phiter->eta();

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
	phisMatched[iph] = PhotonDump::PhotonMatching((*phiter),genparticlesH);
      }

      // super cluster from photon
      const reco::SuperClusterRef& phsc = phiter->superCluster().isNonnull() ? phiter->superCluster() : phiter->parentSuperCluster();
      phscE  [iph] = phsc->energy();
      phsceta[iph] = phsc->eta();
      phscphi[iph] = phsc->phi();
      const float sceta = std::abs(phsceta[iph]);

      // Shower Shape Objects
      const reco::Photon::ShowerShape& phshape = phiter->full5x5_showerShapeVariables(); // phiter->showerShapeVariables();

      // ID-like variables
      phHoE   [iph] = phiter->hadronicOverEm(); // phiter->hadTowOverEm();
      phr9    [iph] = phshape.e3x3/phsc->rawEnergy(); // http://cmslxr.fnal.gov/source/DataFormats/EgammaCandidates/interface/Photon.h#0239
      phChgIso[iph] = std::max(phiter->chargedHadronIso() - (rho * PhotonDump::GetChargedHadronEA(sceta)),0.f);
      phNeuIso[iph] = std::max(phiter->neutralHadronIso() - (rho * PhotonDump::GetNeutralHadronEA(sceta)),0.f);
      phIso   [iph] = std::max(phiter->photonIso()        - (rho * PhotonDump::GetGammaEA        (sceta)),0.f);

      // cluster shape variables
      phsieie[iph] = phshape.sigmaIetaIeta;
      phsipip[iph] = phshape.sigmaIphiIphi;
      phsieip[iph] = phshape.sigmaIetaIphi;

      // 0 --> did not pass anything, 1 --> loose pass, 2 --> medium pass, 3 --> tight pass
      if      (phiter->photonID("tight"))  {phVID[iph] = 3;}
      else if (phiter->photonID("medium")) {phVID[iph] = 2;}
      else if (phiter->photonID("loose"))  {phVID[iph] = 1;}
      else                                 {phVID[iph] = 0;}

      // store similar ints if pass individual selections
      phHoE_b   [iph] = PhotonDump::PassHoE   (sceta,phHoE   [iph]);
      phsieie_b [iph] = PhotonDump::PassSieie (sceta,phsieie [iph]);
      phChgIso_b[iph] = PhotonDump::PassChgIso(sceta,phChgIso[iph]);
      phNeuIso_b[iph] = PhotonDump::PassNeuIso(sceta,phNeuIso[iph],phpt[iph]);
      phIso_b   [iph] = PhotonDump::PassPhIso (sceta,phIso   [iph],phpt[iph]);
   
      // if (true) PhotonDump::DumpVIDs((*phiter),iph,sceta);

      // use seed to get geometry and recHits
      const DetId seedDetId = phsc->seed()->seed(); //seed detid
      const bool isEB = (seedDetId.subdetId() == EcalBarrel); //which subdet
      const EcalRecHitCollection * recHits = isEB ? clustertools->getEcalEBRecHitCollection() : clustertools->getEcalEERecHitCollection();

      // 2nd moments from official calculation
      if ( recHits->size() )
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
    delete clustertools; // delete cluster tools once done with loop over photons
  } // end check over photon handle valid

  ///////////////
  //           //
  // Fill Tree //
  //           //
  ///////////////
  tree->Fill();      
}    

void PhotonDump::PrepJets(const edm::Handle<std::vector<pat::Jet> > & jetsH, std::vector<pat::Jet> & jets)
{
  if (jetsH.isValid()) // standard handle check
  {
    std::sort(jets.begin(),jets.end(),sortByJetPt);
  }
}  

void PhotonDump::PrepPhotons(const edm::Handle<std::vector<pat::Photon> > & photonsH, 
			     const edm::ValueMap<bool> & photonLooseIdMap, 
			     const edm::ValueMap<bool> & photonMediumIdMap, 
			     const edm::ValueMap<bool> & photonTightIdMap, 
			     std::vector<pat::Photon> & photons)
{
  if (photonsH.isValid()) // standard handle check
  {
    // create and initialize temp id-value vector
    std::vector<std::vector<pat::Photon::IdPair> > idpairs(photons.size());
    for (std::size_t iph = 0; iph < idpairs.size(); iph++)
    {
      idpairs[iph].resize(3);
      idpairs[iph][0] = {"loose" ,false};
      idpairs[iph][1] = {"medium",false};
      idpairs[iph][2] = {"tight" ,false};
    }

    int iphH = 0; // dumb counter because iterators only work with VID
    for (std::vector<pat::Photon>::const_iterator phiter = photonsH->begin(); phiter != photonsH->end(); ++phiter) // loop over photon vector
    {
      // Get the VID of the photon
      const edm::Ptr<pat::Photon> photonPtr(photonsH, phiter - photonsH->begin());

      // store VID in temp struct
      // loose > medium > tight
      if (photonLooseIdMap [photonPtr]) idpairs[iphH][0].second = true;
      if (photonMediumIdMap[photonPtr]) idpairs[iphH][1].second = true;
      if (photonTightIdMap [photonPtr]) idpairs[iphH][2].second = true;
      
      iphH++;
    }
    
    // set the ID-value for each photon in other collection
    for (std::size_t iph = 0; iph < photons.size(); iph++)
    {
      photons[iph].setPhotonIDs(idpairs[iph]);
    }
    
    // now finally sort vector by pT
    std::sort(photons.begin(),photons.end(),sortByPhotonPt);
  }
}  

bool PhotonDump::PhotonMatching(const pat::Photon & photon, const edm::Handle<std::vector<reco::GenParticle> > & genparticlesH)
{
  if (genparticlesH.isValid()) // make sure gen particles exist
  {
    float dRmin = 0.1;
    for (std::vector<reco::GenParticle>::const_iterator gpiter = genparticlesH->begin(); gpiter != genparticlesH->end(); ++gpiter)
    {
      if (gpiter->pdgId() == 22 && gpiter->isPromptFinalState())
	{
	  if (std::abs(gpiter->pt()-photon.pt())/photon.pt() < 0.5)
	  {
	    const float dR = deltaR(photon.phi(),photon.eta(),gpiter->phi(),gpiter->eta());
	    if (dR < dRmin) 
	    {
	      return true;
	    } // end check over dRmin
	  } // end check over pT resolution
	} // end check over gen particle status 
      } // end loop over gen particles
    } // end check over gen particles exist

  return false;      
} 

float PhotonDump::GetChargedHadronEA(const float eta)
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

float PhotonDump::GetNeutralHadronEA(const float eta) 
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

float PhotonDump::GetGammaEA(const float eta) 
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

int PhotonDump::PassHoE(const float eta, const float HoE)
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

int PhotonDump::PassSieie(const float eta, const float Sieie)
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

int PhotonDump::PassChgIso(const float eta, const float ChgIso)
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

int PhotonDump::PassNeuIso(const float eta, const float NeuIso, const float pt)
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

int PhotonDump::PassPhIso(const float eta, const float PhIso, const float pt)
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

void PhotonDump::DumpTriggerMenu(const HLTConfigProvider& hltConfig, const std::vector<std::string>& pathNames, edm::Run const& iRun)
{
  std::cout << "Run Number: " << iRun.run() << std::endl;
  for (std::size_t j = 0; j < pathNames.size(); j++)
  {
    std::cout << "   " << pathNames[j].c_str() << " : " << hltConfig.triggerIndex(pathNames[j]) << " - " << j << std::endl;
  }
  std::cout << "--------------------------" << std::endl;
  for (std::size_t i = 0; i < triggerNames.size(); i++)
  {
    std::cout << "   " << triggerNames[i].c_str() << " : " << i << " - " << triggerIndex[triggerNames[i]] << std::endl;
  }
  std::cout << "--------------------------" << std::endl;
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

void PhotonDump::DumpVIDs(const pat::Photon & photon, int iph, float sceta)
{
  int VIDval = 0;
  TString VIDstr = "loose";
  if      (VIDstr.EqualTo("loose"))  VIDval = 1;
  else if (VIDstr.EqualTo("medium")) VIDval = 2;
  else if (VIDstr.EqualTo("tight"))  VIDval = 3;
  
  if (photon.photonID(VIDstr.Data()))
  {
    bool check = (phHoE_b[iph] >= VIDval && phsieie_b[iph] >= VIDval && phChgIso_b[iph] >= VIDval && phNeuIso_b[iph] >= VIDval && phIso_b[iph] >= VIDval);
    if (!check)
    {
      std::cout << "VID: "     << phVID[iph]      << " eta: " << sceta         << std::endl
		<< " HoE:    " << phHoE_b[iph]    << " val: " << phHoE[iph]    << std::endl
		<< " Sieie:  " << phsieie_b[iph]  << " val: " << phsieie[iph]  << std::endl
		<< " chgIso: " << phChgIso_b[iph] << " val: " << phChgIso[iph] << std::endl
		<< " neuIso: " << phNeuIso_b[iph] << " val: " << phNeuIso[iph] << std::endl
		<< " phIso:  " << phIso_b[iph]    << " val: " << phIso[iph]    << std::endl 
		<< std::endl;
    }
  }
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
  if (isHLT2)
  {
    hltpho120_met40       = false;
    hltpho175             = false;
    hltdoublepho60        = false;
    hltdispho45           = false; 
    hltdispho45_jet50     = false; 
    hltdispho45_dijet50   = false; 
    hltdispho45_dijet35   = false; 
    hltdispho45_trijet35  = false; 
    hltdispho45_el100veto = false; 
    hltdispho45_notkveto  = false; 
    hltdispho50           = false; 
    hltdispho60           = false; 
    hltdispho60_jet50     = false; 
    hltdispho60_dijet50   = false; 
    hltdispho60_dijet35   = false; 
    hltdispho60_trijet35  = false; 
    hltdispho60_notkveto  = false; 
    hltdispho80           = false; 
    hltdispho100          = false;
  }
  else if (isHLT3)
  {
    hltdispho45           = false; 
    hltdispho45_notkveto  = false;
    hltdispho45_notime    = false;
    hltdispho45_nosmaj    = false;
    hltdispho45_nosmin    = false;
    hltdispho45_nosieie   = false;
    hltdispho45_nor9      = false;
    hltdispho45_nohoe     = false;
    hltdispho45_noet      = false;
    hltdispho45_nol1match = false;
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
 
  if (isGMSB)
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

  phE.clear();
  phpt.clear();
  phphi.clear(); 
  pheta.clear(); 

  if (isGMSB || isHVDS) phmatch.clear();
  if (isMC) phisMatched.clear();

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

  phVID.clear();

  phHoE_b.clear();
  phsieie_b.clear();
  phChgIso_b.clear();
  phNeuIso_b.clear();
  phIso_b.clear();

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
  phE.resize(nphotons);
  phpt.resize(nphotons);
  phphi.resize(nphotons);
  pheta.resize(nphotons);

  if (isGMSB || isHVDS) phmatch.resize(nphotons);
  if (isMC) phisMatched.resize(nphotons);
  
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

  phHoE_b.resize(nphotons);
  phsieie_b.resize(nphotons);
  phChgIso_b.resize(nphotons);
  phNeuIso_b.resize(nphotons);
  phIso_b.resize(nphotons);

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

    if (isGMSB || isHVDS) phmatch [iph] = -9999;
    if (isMC) phisMatched[iph] = false;

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

    phHoE_b   [iph] = -9999;
    phsieie_b [iph] = -9999;
    phChgIso_b[iph] = -9999;
    phNeuIso_b[iph] = -9999;
    phIso_b   [iph] = -9999;

    phnrh    [iph] = -9999;
    phseedpos[iph] = -9999;
  }
}

void PhotonDump::InitializeRecoRecHitBranches(int iph)
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
  if (isHLT2)
  {
    tree->Branch("hltpho120_met40"      , &hltpho120_met40      , "hltpho120_met40/O");
    tree->Branch("hltpho175"            , &hltpho175            , "hltpho175/O");
    tree->Branch("hltdoublepho60"       , &hltdoublepho60       , "hltdoublepho60/O");
    tree->Branch("hltdispho45"          , &hltdispho45          , "hltdispho45/O");
    tree->Branch("hltdispho45_jet50"    , &hltdispho45_jet50    , "hltdispho45_jet50/O");
    tree->Branch("hltdispho45_dijet50"  , &hltdispho45_dijet50  , "hltdispho45_dijet50/O");
    tree->Branch("hltdispho45_dijet35"  , &hltdispho45_dijet35  , "hltdispho45_dijet35/O");
    tree->Branch("hltdispho45_trijet35" , &hltdispho45_trijet35 , "hltdispho45_trijet35/O");
    tree->Branch("hltdispho45_el100veto", &hltdispho45_el100veto, "hltdispho45_el100veto/O");
    tree->Branch("hltdispho45_notkveto" , &hltdispho45_notkveto , "hltdispho45_notkveto/O");
    tree->Branch("hltdispho50"          , &hltdispho50          , "hltdispho50/O");
    tree->Branch("hltdispho60"          , &hltdispho60          , "hltdispho60/O");
    tree->Branch("hltdispho60_dijet50"  , &hltdispho60_dijet50  , "hltdispho60_dijet50/O");
    tree->Branch("hltdispho60_dijet35"  , &hltdispho60_dijet35  , "hltdispho60_dijet35/O");
    tree->Branch("hltdispho60_trijet35" , &hltdispho60_trijet35 , "hltdispho60_trijet35/O");
    tree->Branch("hltdispho60_notkveto" , &hltdispho60_notkveto , "hltdispho60_notkveto/O");
    tree->Branch("hltdispho80"          , &hltdispho80          , "hltdispho80/O");
    tree->Branch("hltdispho100"         , &hltdispho100         , "hltdispho100/O");
  }
  else if (isHLT3)
  {
    tree->Branch("hltdispho45"          , &hltdispho45          , "hltdispho45/O");
    tree->Branch("hltdispho45_notkveto" , &hltdispho45_notkveto , "hltdispho45_notkveto/O");
    tree->Branch("hltdispho45_notime"   , &hltdispho45_notime   , "hltdispho45_notime/O");
    tree->Branch("hltdispho45_nosmaj"   , &hltdispho45_nosmaj   , "hltdispho45_nosmaj/O");
    tree->Branch("hltdispho45_nosmin"   , &hltdispho45_nosmin   , "hltdispho45_nosmin/O");
    tree->Branch("hltdispho45_nosieie"  , &hltdispho45_nosieie  , "hltdispho45_nosieie/O");
    tree->Branch("hltdispho45_nor9"     , &hltdispho45_nor9     , "hltdispho45_nor9/O");
    tree->Branch("hltdispho45_nohoe"    , &hltdispho45_nohoe    , "hltdispho45_nohoe/O");
    tree->Branch("hltdispho45_noet"     , &hltdispho45_noet     , "hltdispho45_noet/O");
    tree->Branch("hltdispho45_nol1match", &hltdispho45_nol1match, "hltdispho45_nol1match/O");
  }

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
  
  if (isGMSB)
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

  tree->Branch("phE"                  , &phE);
  tree->Branch("phpt"                 , &phpt);
  tree->Branch("phphi"                , &phphi);
  tree->Branch("pheta"                , &pheta);

  if (isGMSB || isHVDS) tree->Branch("phmatch", &phmatch);
  if (isMC) tree->Branch("phisMatched", &phisMatched);

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

  tree->Branch("phHoE_b"              , &phHoE_b);
  tree->Branch("phsieie_b"            , &phsieie_b);
  tree->Branch("phChgIso_b"           , &phChgIso_b);
  tree->Branch("phNeuIso_b"           , &phNeuIso_b);
  tree->Branch("phIso_b"              , &phIso_b);

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

void PhotonDump::beginRun(edm::Run const& iRun, edm::EventSetup const& iSetup) 
{
  if (isHLT2) // test menu for displaced photons
  {
    triggerNames.push_back("HLT_Photon120_R9Id90_HE10_Iso40_EBOnly_PFMET40_v"); // i = 0
    triggerNames.push_back("HLT_Photon175_v");                                  // i = 1
    triggerNames.push_back("HLT_DoublePhoton60_v");                             // i = 2
    triggerNames.push_back("HLT_DisplacedPhoton45_v");                          // i = 3
    triggerNames.push_back("HLT_DisplacedPhoton45_PFJet50_v");                  // i = 4
    triggerNames.push_back("HLT_DisplacedPhoton45_DiPFJet50_v");                // i = 5
    triggerNames.push_back("HLT_DisplacedPhoton45_DiPFJet35_v");                // i = 6
    triggerNames.push_back("HLT_DisplacedPhoton45_TriPFJet35_v");               // i = 7
    triggerNames.push_back("HLT_DisplacedPhoton45_El100Veto_v");                // i = 8
    triggerNames.push_back("HLT_DisplacedPhoton45_noTrackVeto_v");              // i = 9
    triggerNames.push_back("HLT_DisplacedPhoton50_v");                          // i = 10
    triggerNames.push_back("HLT_DisplacedPhoton60_v");                          // i = 11
    triggerNames.push_back("HLT_DisplacedPhoton60_PFJet50_v");                  // i = 12
    triggerNames.push_back("HLT_DisplacedPhoton60_DiPFJet50_v");                // i = 13
    triggerNames.push_back("HLT_DisplacedPhoton60_DiPFJet35_v");                // i = 14
    triggerNames.push_back("HLT_DisplacedPhoton60_TriPFJet35_v");               // i = 15
    triggerNames.push_back("HLT_DisplacedPhoton60_noTrackVeto_v");              // i = 16
    triggerNames.push_back("HLT_DisplacedPhoton80_v");                          // i = 17
    triggerNames.push_back("HLT_DisplacedPhoton100_v");                         // i = 18
  }
  else if (isHLT3) // remove cuts on at a time for DisplacedPhoton45
  {
    triggerNames.push_back("HLT_DisplacedPhoton45_v");                          // i = 0
    triggerNames.push_back("HLT_DisplacedPhoton45_noTrackVeto_v");              // i = 1
    triggerNames.push_back("HLT_DisplacedPhoton45_noTime_v");                   // i = 2
    triggerNames.push_back("HLT_DisplacedPhoton45_noSmaj_v");                   // i = 3
    triggerNames.push_back("HLT_DisplacedPhoton45_noSmin_v");                   // i = 4
    triggerNames.push_back("HLT_DisplacedPhoton45_noSieie_v");                  // i = 5
    triggerNames.push_back("HLT_DisplacedPhoton45_noR9_v");                     // i = 6
    triggerNames.push_back("HLT_DisplacedPhoton45_noHoE_v");                    // i = 7
    triggerNames.push_back("HLT_DisplacedPhoton45_noEt_v");                     // i = 8
    triggerNames.push_back("HLT_DisplacedPhoton45_noL1Match_v");                // i = 9
  }

  // initialize triggerIndex, key: Name, value: Index 
  for (std::size_t i = 0; i < triggerNames.size(); i++)
  {
    triggerIndex[triggerNames[i]] = -1;
  }
  
  HLTConfigProvider hltConfig;
  bool changed = false;
  hltConfig.init(iRun, iSetup, triggerResultsTag.process(), changed);
  const std::vector<std::string>& pathNames = hltConfig.triggerNames();
  for (std::size_t i = 0; i < triggerNames.size(); i++)
  {
    TPRegexp pattern(triggerNames[i]);
    for (std::size_t j = 0; j < pathNames.size(); j++)
    {
      if (TString(pathNames[j]).Contains(pattern))
      {
	triggerIndex[triggerNames[i]] = j; //hltConfig.triggerIndex(pathNames[j]);
	break;
      }
    }
  }

  if (dumpTriggerMenu) PhotonDump::DumpTriggerMenu(hltConfig,pathNames,iRun);
}

void PhotonDump::endRun(edm::Run const&, edm::EventSetup const&) 
{
  // reset trigger info
  triggerNames.clear();
  triggerIndex.clear();
}

void PhotonDump::fillDescriptions(edm::ConfigurationDescriptions& descriptions) 
{
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

DEFINE_FWK_MODULE(PhotonDump);
