#include "GENAnalyzer.h"

GENAnalyzer::GENAnalyzer(const edm::ParameterSet& iConfig)
{
  usesResource();
  usesResource("TFileService");

  genevtInfoToken = consumes<GenEventInfoProduct>             (iConfig.getParameter<edm::InputTag>("genevt"));
  genpartsToken   = consumes<std::vector<reco::GenParticle> > (iConfig.getParameter<edm::InputTag>("genparts"));   
  genjetsToken    = consumes<std::vector<reco::GenJet> >      (iConfig.getParameter<edm::InputTag>("genjets"));   
  genmetsToken    = consumes<std::vector<reco::GenMET> >      (iConfig.getParameter<edm::InputTag>("genmets"));   
}

GENAnalyzer::~GENAnalyzer() {}

void GENAnalyzer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) 
{
  // GEN INFO
  edm::Handle<GenEventInfoProduct> genevtInfoH;
  iEvent.getByToken(genevtInfoToken, genevtInfoH);
  edm::Handle<std::vector<reco::GenParticle> > genparticlesH;
  iEvent.getByToken(genpartsToken,   genparticlesH);
  edm::Handle<std::vector<reco::GenJet> > genjetsH;
  iEvent.getByToken(genjetsToken,    genjetsH);
  edm::Handle<std::vector<reco::GenMET> > genmetsH;
  iEvent.getByToken(genmetsToken,    genmetsH);

  ///////////////////////////
  //                       //
  // Event, lumi, run info //
  //                       //
  ///////////////////////////
  run   = iEvent.id().run();
  lumi  = iEvent.luminosityBlock();
  event = iEvent.id().event();

  ///////////////////////
  //                   //
  // Event weight info //
  //                   //
  ///////////////////////
  GENAnalyzer::InitializeGenEvtBranches();
  if (genevtInfoH.isValid()) {genwgt = genevtInfoH->weight();}

  ///////////////////////
  //                   //
  // Gen particle info //
  //                   //
  ///////////////////////
  
  if (genparticlesH.isValid()) // make sure gen particles exist --> only do this for GMSB
  {
    for (std::vector<reco::GenParticle>::const_iterator gpiter = genparticlesH->begin(); gpiter != genparticlesH->end(); ++gpiter) // loop over gen particles
    {
      if (gpiter->pdgId() == 4900111) 
      {
	std::cout << gpiter->pdgId() << " " << gpiter->status() << std::endl;
	if (gpiter->numberOfMothers() != 0) std::cout << " mothers: ";
	for (auto mgpiter = gpiter->motherRefVector().begin(); mgpiter != gpiter->motherRefVector().end(); ++mgpiter)
	{
	  std::cout << (*mgpiter)->pdgId() << " ";
	}
	if (gpiter->numberOfMothers() != 0) std::cout << std::endl;
	
	// dump daughters second
	if (gpiter->numberOfDaughters() != 0) std::cout << " daughters: ";
	for (auto dgpiter = gpiter->daughterRefVector().begin(); dgpiter != gpiter->daughterRefVector().end(); ++dgpiter)
	{
	  std::cout << (*dgpiter)->pdgId() << " ";
	}
	if (gpiter->numberOfDaughters() != 0) std::cout << std::endl;
      }
    } // end loop over gen particles
    std::cout << "-------------------" << std::endl;
  } // end check for gen particles
  std::cout << "===================" << std::endl << std::endl;
  ///////////////////
  //               //
  // Gen Jets info //
  //               //
  ///////////////////
  GENAnalyzer::ClearGenJetsBranches();
  if (genjetsH.isValid()) // make sure gen particles exist
  {
    std::vector<reco::GenJet> genjets = *genjetsH;
    std::sort(genjets.begin(),genjets.end(),sortByJetPt);

    ngenjets = genjets.size();
    for (std::vector<reco::GenJet>::const_iterator gjetiter = genjets.begin(); gjetiter != genjets.end(); ++gjetiter) // loop over genjets
    {
      genjetE  .push_back(gjetiter->energy());
      genjetpt .push_back(gjetiter->pt());
      genjetphi.push_back(gjetiter->phi());
      genjeteta.push_back(gjetiter->eta());
    } // end check over genjets
  }

  /////////////
  //         //
  // GEN MET //
  //         //
  /////////////
  GENAnalyzer::ClearGenMETsBranches();
  if (genmetsH.isValid())
  {
    std::vector<reco::GenMET> genmets = *genmetsH;
    std::sort(genmets.begin(),genmets.end(),sortByMETPt);

    ngenmets = genmets.size();
    for (std::vector<reco::GenMET>::const_iterator gmetiter = genmets.begin(); gmetiter != genmets.end(); ++gmetiter) // loop over genmets
    { 
      genMETpt   .push_back(gmetiter->pt());
      genMETphi  .push_back(gmetiter->phi());
      genMETsumEt.push_back(gmetiter->sumEt());
    }
  }

  ///////////////
  //           //
  // Fill Tree //
  //           //
  ///////////////
  tree->Fill();      
}    

void GENAnalyzer::InitializeGenEvtBranches()
{
  genwgt = -9999.f;
}

void GENAnalyzer::ClearGenJetsBranches()
{
  ngenjets = -9999;

  genjetE.clear();
  genjetpt.clear();
  genjetphi.clear();
  genjeteta.clear();
}

void GENAnalyzer::ClearGenMETsBranches()
{
  ngenmets = -9999;

  genMETpt.clear();
  genMETphi.clear(); 
  genMETsumEt.clear();
}

void GENAnalyzer::beginJob() 
{
  edm::Service<TFileService> fs;
  tree = fs->make<TTree>("tree"     , "tree");

  // Run, Lumi, Event info
  tree->Branch("event"                  , &event                , "event/I");
  tree->Branch("run"                    , &run                  , "run/I");
  tree->Branch("lumi"                   , &lumi                 , "lumi/I");
   
  // Generator
  tree->Branch("genwgt"               , &genwgt               , "genwgt/F");

  tree->Branch("ngenjets"             , &ngenjets             , "ngenjets/I");
  tree->Branch("genjetE"              , &genjetE);
  tree->Branch("genjetpt"             , &genjetpt);
  tree->Branch("genjetphi"            , &genjetphi);
  tree->Branch("genjeteta"            , &genjeteta);

  tree->Branch("ngenmets"             , &ngenmets             , "ngenmets/I");
  tree->Branch("genMETpt"             , &genMETpt);
  tree->Branch("genMETphi"            , &genMETphi);
  tree->Branch("genMETsumEt"          , &genMETsumEt);
}

void GENAnalyzer::endJob() {}

void GENAnalyzer::beginRun(edm::Run const& iRun, edm::EventSetup const& iSetup) {}

void GENAnalyzer::endRun(edm::Run const&, edm::EventSetup const&) {}

void GENAnalyzer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) 
{
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

DEFINE_FWK_MODULE(GENAnalyzer);
