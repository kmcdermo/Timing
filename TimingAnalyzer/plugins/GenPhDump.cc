#include "GenPhDump.h"

GenPhDump::GenPhDump(const edm::ParameterSet& iConfig): 
  // photons + ids
  photonLooseIdMapTag  (iConfig.getParameter<edm::InputTag>("loosePhotonID")),  
  photonMediumIdMapTag (iConfig.getParameter<edm::InputTag>("mediumPhotonID")),  
  photonTightIdMapTag  (iConfig.getParameter<edm::InputTag>("tightPhotonID")),  
  photonsTag           (iConfig.getParameter<edm::InputTag>("photons")),  

  // gen particle info
  prunedGenParticlesTag(iConfig.getParameter<edm::InputTag>("prunedGenParticles"))
{
  usesResource();
  usesResource("TFileService");

  // photons + ids
  photonLooseIdMapToken   = consumes<edm::ValueMap<bool> >      (photonLooseIdMapTag);
  photonMediumIdMapToken  = consumes<edm::ValueMap<bool> >      (photonMediumIdMapTag);
  photonTightIdMapToken   = consumes<edm::ValueMap<bool> >      (photonTightIdMapTag);
  photonsToken            = consumes<std::vector<pat::Photon> > (photonsTag);

  prunedGenParticlesToken = consumes<std::vector<reco::GenParticle> >      (prunedGenParticlesTag);
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
  std::vector<pat::Photon> photons = *photonsH;

  // GEN INFO
  edm::Handle<std::vector<reco::GenParticle> > prunedGenParticlesH;
  iEvent.getByToken(prunedGenParticlesToken, prunedGenParticlesH);

  GenPhDump::PrepPhotons(photonsH,photonLooseIdMap,photonMediumIdMap,photonTightIdMap,photons); // enormous amount of bullsh*t to get photons in shape 

  //////////////////
  //              //
  // Reco Photons //
  //              //
  //////////////////

  for (std::vector<pat::Photon>::const_iterator phiter = photons.begin(); phiter != photons.end(); ++phiter) // loop over photon vector
  {
    isMatched = false;// reset variables
    //    int igp = -1; // best matched gp index
        
    if (!phiter->photonID("medium")) continue;
        
    if (prunedGenParticlesH.isValid()) // make sure gen particles exist
    {
      float dRmin = 1e9;
      //      int jgp = 0;
      for (std::vector<reco::GenParticle>::const_iterator gpiter = prunedGenParticlesH->begin(); gpiter != prunedGenParticlesH->end(); ++gpiter)
      {
	if (gpiter->pdgId() == 22 && gpiter->isPromptFinalState())
	{
	  if (std::abs(gpiter->pt()-phiter->pt())/phiter->pt() < 0.5)
	  {
	    const float dR = deltaR(phiter->phi(),phiter->eta(),gpiter->phi(),gpiter->eta());
	    if (dR < dRmin) 
	    {
	      dRmin = dR;
	      //	      igp   = jgp;
	    } // end check over dRmin
	  } // end check over pT resolution
	} // end check over gen particle status 
	//	jgp++;
      } // end loop over gen particles
      if (dRmin < 0.1) isMatched = true;
    } // end check over gen particles exist
    
    phpt = phiter->pt();

    tree->Fill();
  } // end loop over reco photons
}

void GenPhDump::PrepPhotons(const edm::Handle<std::vector<pat::Photon> > & photonsH, 
			    const edm::ValueMap<bool> & photonLooseIdMap, 
			    const edm::ValueMap<bool> & photonMediumIdMap, 
			    const edm::ValueMap<bool> & photonTightIdMap, 
			    std::vector<pat::Photon> & photons)
{
  if (photonsH.isValid()) // standard handle check
  {
    // create and initialize temp id-value vector
    std::vector<std::vector<pat::Photon::IdPair> > idpairs(photons.size());
    for (size_t iph = 0; iph < idpairs.size(); iph++)
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
    for (size_t iph = 0; iph < photons.size(); iph++)
    {
      photons[iph].setPhotonIDs(idpairs[iph]);
    }
    
    // now finally sort vector by pT
    std::sort(photons.begin(),photons.end(),sortByPhotonPt);
  }
}  

void GenPhDump::beginJob() 
{
  edm::Service<TFileService> fs;
  tree = fs->make<TTree>("tree","tree");

  tree->Branch("isMatched"    , &isMatched      , "isMatched/O");
  tree->Branch("phpt"         , &phpt           , "phpt/F");
}

void GenPhDump::endJob() {}

void GenPhDump::beginRun(edm::Run const& iRun, edm::EventSetup const& iSetup) {}

void GenPhDump::endRun(edm::Run const&, edm::EventSetup const&) {}

void GenPhDump::fillDescriptions(edm::ConfigurationDescriptions& descriptions) 
{
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

DEFINE_FWK_MODULE(GenPhDump);
