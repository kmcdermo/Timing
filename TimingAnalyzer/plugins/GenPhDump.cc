#include "GenPhDump.h"

GenPhDump::GenPhDump(const edm::ParameterSet& iConfig): 
  // photons + ids
  photonLooseIdMapTag  (iConfig.getParameter<edm::InputTag>("loosePhotonID")),  
  photonMediumIdMapTag (iConfig.getParameter<edm::InputTag>("mediumPhotonID")),  
  photonTightIdMapTag  (iConfig.getParameter<edm::InputTag>("tightPhotonID")),  
  photonsTag           (iConfig.getParameter<edm::InputTag>("photons")),  

  // gen particle info
  prunedGenParticlesTag(iConfig.getParameter<edm::InputTag>("prunedGenParticles")),  
  packedGenParticlesTag(iConfig.getParameter<edm::InputTag>("packedGenParticles"))
{
  usesResource();
  usesResource("TFileService");

  // photons + ids
  photonLooseIdMapToken   = consumes<edm::ValueMap<bool> >      (photonLooseIdMapTag);
  photonMediumIdMapToken  = consumes<edm::ValueMap<bool> >      (photonMediumIdMapTag);
  photonTightIdMapToken   = consumes<edm::ValueMap<bool> >      (photonTightIdMapTag);
  photonsToken            = consumes<std::vector<pat::Photon> > (photonsTag);

  prunedGenParticlesToken = consumes<std::vector<reco::GenParticle> >      (prunedGenParticlesTag);
  packedGenParticlesToken = consumes<std::vector<pat::PackedGenParticle> > (packedGenParticlesTag);
}

GenPhDump::~GenPhDump() {}

void GenPhDump::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) 
{
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

  // GEN INFO
  edm::Handle<std::vector<reco::GenParticle> > prunedGenParticlesH;
  iEvent.getByToken(prunedGenParticlesToken, prunedGenParticlesH);

  edm::Handle<std::vector<pat::PackedGenParticle> > packedGenParticlesH;
  iEvent.getByToken(packedGenParticlesToken, packedGenParticlesH);
  
  ///////////////////////
  //                   //
  // Gen particle info //
  //                   //
  ///////////////////////
  
  int genph1match = 0, genph2match;
  if (prunedGenParticlesH.isValid()) // make sure gen particles exist
  {
    bool firstMother  = false; // bool for first neutralino found
    bool secondMother = false; // bool for second neutralino found
    for (std::vector<reco::GenParticle>::const_iterator gpiter = prunedGenParticlesH->begin(); gpiter != prunedGenParticlesH->end(); ++gpiter) // loop over gen particles
    {
      if (firstMother && secondMother) break; // have two matches!
      
      if (gpiter->pdgId() == 1000022 && gpiter->numberOfDaughters() == 2)
      {
	if ((gpiter->daughter(0)->pdgId() == 22 && gpiter->daughter(1)->pdgId() == 1000039) ||
	    (gpiter->daughter(1)->pdgId() == 22 && gpiter->daughter(0)->pdgId() == 1000039)) 
	{
	  if (!firstMother && !secondMother)
	  {
	    // set photon daughter stuff
	    int phdaughter = -1; // determine which one is the photon daughter
	    if      (gpiter->daughter(0)->pdgId() == 22) {phdaughter = 0;}
	    else if (gpiter->daughter(1)->pdgId() == 22) {phdaughter = 1;}

	    const float genph1E   = gpiter->daughter(phdaughter)->energy();
	    const float genph1pt  = gpiter->daughter(phdaughter)->pt();
	    const float genph1phi = gpiter->daughter(phdaughter)->phi();
	    const float genph1eta = gpiter->daughter(phdaughter)->eta();

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

	    // set this to ensure not to overwrite first mother info
	    firstMother = true;
	  } // end block over first matched neutralino -> photon + gravitino
	  else if (firstMother && !secondMother)
	  {
	    // set photon daughter stuff
	    int phdaughter = -1; // determine which one is the photon daughter
	    if      (gpiter->daughter(0)->pdgId() == 22) {phdaughter = 0;}
	    else if (gpiter->daughter(1)->pdgId() == 22) {phdaughter = 1;}
	    
	    const float genph2E   = gpiter->daughter(phdaughter)->energy();
	    const float genph2pt  = gpiter->daughter(phdaughter)->pt();
	    const float genph2phi = gpiter->daughter(phdaughter)->phi();
	    const float genph2eta = gpiter->daughter(phdaughter)->eta();
	      
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
	    
	    secondMother = true;
	  } // end block over second matched neutralino -> photon + gravitino
	} // end conditional over matching daughter ids
      } // end conditional over neutralino id
    } // end loop over gen particles
  } // end check for gen particles

  //////////////////
  //              //
  // Reco Photons //
  //              //
  //////////////////
  if (photonsH.isValid()) // standard handle check
  {
    for (std::vector<pat::Photon>::const_iterator phiter = photonsH->begin(); phiter != photonsH->end(); ++phiter) // loop over photon vector
    {
      // Get the VID of the photon
      const edm::Ptr<pat::Photon> photonPtr(photonsH, phiter - photonsH->begin());

      // loose > medium > tight
      if (photonLooseIdMap [photonPtr]) ;
      if (photonMediumIdMap[photonPtr]) ;
      if (photonTightIdMap [photonPtr]) ;
    }
  }
}    

void GenPhDump::beginJob() {}

void GenPhDump::endJob() {}

void GenPhDump::beginRun(edm::Run const& iRun, edm::EventSetup const& iSetup) {}

void GenPhDump::endRun(edm::Run const&, edm::EventSetup const&) {}

void GenPhDump::fillDescriptions(edm::ConfigurationDescriptions& descriptions) 
{
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

DEFINE_FWK_MODULE(GenDump);
