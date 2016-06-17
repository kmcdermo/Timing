// basic C++ headers
#include <iostream>
#include <memory>
#include <vector>
#include <map>
#include <string>
#include <cmath>
#include <algorithm>
#include <tuple>

// FWCore
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/Common/interface/TriggerNames.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h" 

// HLT info
#include "HLTrigger/HLTcore/interface/HLTConfigProvider.h"

// Gen Info
#include "SimDataFormats/PileupSummaryInfo/interface/PileupSummaryInfo.h"
#include "SimDataFormats/GeneratorProducts/interface/GenEventInfoProduct.h"

// DataFormats
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/PatCandidates/interface/Electron.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "DataFormats/Common/interface/TriggerResults.h"
#include "DataFormats/Common/interface/ValueMap.h"

// Stolen from ECALELF dumper
#include "DataFormats/EcalDetId/interface/EcalSubdetector.h"
#include "DataFormats/EcalDetId/interface/EBDetId.h"
#include "DataFormats/EcalDetId/interface/EEDetId.h"
#include "RecoEcal/EgammaCoreTools/interface/EcalClusterTools.h"
#include "RecoEcal/EgammaCoreTools/interface/EcalClusterLazyTools.h"

// ROOT
#include "TTree.h"
#include "TLorentzVector.h"
#include "TPRegexp.h"

typedef std::tuple<int, int, float> triple;
typedef std::vector<triple> triplevec;

inline bool minimizeByZmass(const triple& elpair1, const triple& elpair2){
  return std::get<2>(elpair1)<std::get<2>(elpair2);
}

inline bool sortElectronsByPt(const pat::ElectronRef& el1, const pat::ElectronRef& el2){
  return el1->pt()>el2->pt();
}

class TimingAnalyzer : public edm::one::EDAnalyzer<edm::one::SharedResources,edm::one::WatchRuns> {

public:
  explicit TimingAnalyzer(const edm::ParameterSet&);
  ~TimingAnalyzer();
  
  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);
  
private:
  virtual void beginJob() override;
  virtual void analyze(const edm::Event&, const edm::EventSetup&) override;
  virtual void endJob() override;
  
  virtual void beginRun(edm::Run const&, edm::EventSetup const&) override;
  virtual void endRun(edm::Run const&, edm::EventSetup const&) override;
  
  // Trigger
  const edm::InputTag triggerResultsTag;
  edm::EDGetTokenT<edm::TriggerResults> triggerResultsToken;
  std::vector<std::string>   triggerPathsVector;
  std::map<std::string, int> triggerPathsMap;
  const bool applyHLTFilter;

  // Vertex
  const edm::InputTag verticesTag;
  edm::EDGetTokenT<std::vector<reco::Vertex> > verticesToken;

  // Electrons
  const edm::InputTag vetoelectronsTag;
  const edm::InputTag looseelectronsTag;
  const edm::InputTag mediumelectronsTag;
  const edm::InputTag tightelectronsTag;
  const edm::InputTag heepelectronsTag;
  edm::EDGetTokenT<pat::ElectronRefVector> vetoelectronsToken;
  edm::EDGetTokenT<pat::ElectronRefVector> looseelectronsToken;
  edm::EDGetTokenT<pat::ElectronRefVector> mediumelectronsToken;
  edm::EDGetTokenT<pat::ElectronRefVector> tightelectronsToken;
  edm::EDGetTokenT<pat::ElectronRefVector> heepelectronsToken;
  const bool applyKinematicsFilter;

  // ECAL RecHits
  edm::EDGetTokenT<EcalRecHitCollection> recHitCollectionEBTAG;
  edm::EDGetTokenT<EcalRecHitCollection> recHitCollectionEETAG;

  // Gen Particles and MC info
  const bool isMC;
  edm::EDGetTokenT<std::vector<PileupSummaryInfo> > pileupInfoToken;
  edm::EDGetTokenT<GenEventInfoProduct>             genevtInfoToken;
  edm::EDGetTokenT<edm::View<reco::GenParticle> >   gensToken;

  // ECALELF tools
  EcalClusterLazyTools *clustertools;

  // output ntuple
  // tree
  TTree* tree;

  // event info
  int event, run, lumi;  

  // triggers 
  bool hltdoubleel,hltsingleel,hltelnoiso;

  // vertices
  int nvtx;

  // object counts
  int nvetoelectrons,nlooseelectrons,nmediumelectrons,ntightelectrons,nheepelectrons;

  // electron info
  int   el1pid,el2pid;
  float el1pt,el1eta,el1phi,el1E,el1p;
  float el2pt,el2eta,el2phi,el2E,el2p;

  // timing
  float el1time, el2time;

  // dielectron info
  float zpt,zeta,zphi,zmass,zE,zp;

  // MC Info Only
  // pileup info
  int puobs,putrue; 

  // weights
  float wgt;

  // gen particle info
  int   genzpid,genel1pid,genel2pid;
  float genzpt,genzeta,genzphi,genzmass,genzE,genzp;
  float genel1pt,genel1eta,genel1phi,genel1E,genel1p;
  float genel2pt,genel2eta,genel2phi,genel2E,genel2p;
};


TimingAnalyzer::TimingAnalyzer(const edm::ParameterSet& iConfig): 
  ///////////// TRIGGER and filter info INFO
  triggerResultsTag(iConfig.getParameter<edm::InputTag>("triggerResults")),
  applyHLTFilter(iConfig.existsAs<bool>("applyHLTFilter") ? iConfig.getParameter<bool>("applyHLTFilter") : false),
  
  // vertexes
  verticesTag(iConfig.getParameter<edm::InputTag>("vertices")),
  
  // electrons
  vetoelectronsTag(iConfig.getParameter<edm::InputTag>("vetoelectrons")),
  looseelectronsTag(iConfig.getParameter<edm::InputTag>("looseelectrons")),
  mediumelectronsTag(iConfig.getParameter<edm::InputTag>("mediumelectrons")),
  tightelectronsTag(iConfig.getParameter<edm::InputTag>("tightelectrons")),
  heepelectronsTag(iConfig.getParameter<edm::InputTag>("heepelectrons")),
  applyKinematicsFilter(iConfig.existsAs<bool>("applyKinematicsFilter") ? iConfig.getParameter<bool>("applyKinematicsFilter") : false),  

  //recHits
  recHitCollectionEBTAG(consumes<EcalRecHitCollection>(iConfig.getParameter<edm::InputTag>( "recHitCollectionEB" ))),
  recHitCollectionEETAG(consumes<EcalRecHitCollection>(iConfig.getParameter<edm::InputTag>( "recHitCollectionEE" ))),

  ///////////// GEN INFO
  // isMC or Data --> default Data
  isMC(iConfig.existsAs<bool>("isMC") ? iConfig.getParameter<bool>("isMC") : false)
{
  usesResource();
  usesResource("TFileService");

  // trigger tokens
  triggerResultsToken   = consumes<edm::TriggerResults> (triggerResultsTag);
  
  //vertex
  verticesToken  = consumes<std::vector<reco::Vertex> > (verticesTag);

  // electrons
  vetoelectronsToken   = consumes<pat::ElectronRefVector> (vetoelectronsTag);
  looseelectronsToken  = consumes<pat::ElectronRefVector> (looseelectronsTag);
  mediumelectronsToken = consumes<pat::ElectronRefVector> (mediumelectronsTag);
  tightelectronsToken  = consumes<pat::ElectronRefVector> (tightelectronsTag);
  heepelectronsToken   = consumes<pat::ElectronRefVector> (heepelectronsTag);
   
  // only for simulated samples
  if( isMC ){
    pileupInfoToken = consumes<std::vector<PileupSummaryInfo> > (iConfig.getParameter<edm::InputTag>("pileup"));
    genevtInfoToken = consumes<GenEventInfoProduct> (iConfig.getParameter<edm::InputTag>("genevt"));
    gensToken       = consumes<edm::View<reco::GenParticle> > (iConfig.getParameter<edm::InputTag>("gens"));   
  }
}

TimingAnalyzer::~TimingAnalyzer() {}

void TimingAnalyzer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
  // TRIGGER
  edm::Handle<edm::TriggerResults> triggerResultsH;
  iEvent.getByToken(triggerResultsToken, triggerResultsH);

  // VERTEX
  edm::Handle<std::vector<reco::Vertex> > verticesH;
  iEvent.getByToken(verticesToken, verticesH);

  // ELECTRONS
  edm::Handle<pat::ElectronRefVector> vetoelectronsH;
  iEvent.getByToken(vetoelectronsToken, vetoelectronsH);

  edm::Handle<pat::ElectronRefVector> looseelectronsH;
  iEvent.getByToken(looseelectronsToken, looseelectronsH);

  edm::Handle<pat::ElectronRefVector> mediumelectronsH;
  iEvent.getByToken(mediumelectronsToken, mediumelectronsH);

  edm::Handle<pat::ElectronRefVector> tightelectronsH;
  iEvent.getByToken(tightelectronsToken, tightelectronsH);
  pat::ElectronRefVector tightelectronsvec = *tightelectronsH;

  edm::Handle<pat::ElectronRefVector> heepelectronsH;
  iEvent.getByToken(heepelectronsToken, heepelectronsH);

  // Event, lumi, run info
  event = iEvent.id().event();
  run   = iEvent.id().run();
  lumi  = iEvent.luminosityBlock();
    
  // Trigger info
  hltdoubleel = false;
  hltsingleel = false;
  hltelnoiso  = false;

  // Which triggers fired
  if(triggerResultsH.isValid()){
    for (size_t i = 0; i < triggerPathsVector.size(); i++) {
      if (triggerPathsMap[triggerPathsVector[i]] == -1) continue;	
      if (i == 0 && triggerResultsH->accept(triggerPathsMap[triggerPathsVector[i]])) hltdoubleel = true; // Double electron trigger
      if (i == 1 && triggerResultsH->accept(triggerPathsMap[triggerPathsVector[i]])) hltdoubleel = true; // Double electron trigger
      if (i == 2 && triggerResultsH->accept(triggerPathsMap[triggerPathsVector[i]])) hltsingleel = true; // Single electron trigger
      if (i == 3 && triggerResultsH->accept(triggerPathsMap[triggerPathsVector[i]])) hltsingleel = true; // Single electron trigger
      if (i == 4 && triggerResultsH->accept(triggerPathsMap[triggerPathsVector[i]])) hltsingleel = true; // Single electron trigger
      if (i == 5 && triggerResultsH->accept(triggerPathsMap[triggerPathsVector[i]])) hltsingleel = true; // Single electron trigger
      if (i == 6 && triggerResultsH->accept(triggerPathsMap[triggerPathsVector[i]])) hltelnoiso  = true; // Single electron trigger
      if (i == 7 && triggerResultsH->accept(triggerPathsMap[triggerPathsVector[i]])) hltelnoiso  = true; // Single electron trigger
    }
  }

  // skim on events that pass triggers
  bool triggered = false;
  if      (hltdoubleel) triggered = true;
  else if (hltsingleel) triggered = true;
  else if (hltelnoiso)  triggered = true;
  if (applyHLTFilter && !triggered) return;

  // Vertex info
  nvtx = -99;
  if(verticesH.isValid()) nvtx = verticesH->size();

  // ELECTRON ANALYSIS 
  // nelectrons AFTER PF cleaning (kinematic selection pT > 10, |eta| < 2.5
  nvetoelectrons = -99; nlooseelectrons = -99; nmediumelectrons = -99; ntightelectrons = -99; nheepelectrons = -99;
  if (vetoelectronsH.isValid())   nvetoelectrons   = vetoelectronsH->size();
  if (looseelectronsH.isValid())  nlooseelectrons  = looseelectronsH->size();
  if (mediumelectronsH.isValid()) nmediumelectrons = mediumelectronsH->size();
  if (tightelectronsH.isValid())  ntightelectrons  = tightelectronsH->size();
  if (heepelectronsH.isValid())   nheepelectrons   = heepelectronsH->size();
  
  // tree vars
  zmass   = -99.0; zpt     = -99.0; zeta   = -99.0; zphi   = -99.0; zE = -99.0; zp = -99.0;
  el1pid  = -99;   el1pt   = -99.0; el1eta = -99.0; el1phi = -99.0; 
  el2pid  = -99;   el2pt   = -99.0; el2eta = -99.0; el2phi = -99.0; 
  el1E    = -99.0; el2E    = -99.0; el1p   = -99.0; el2p   = -99.0;
  el1time = -99.0; el2time = -99.0; 
  

  // save only really pure electrons
  std::vector<pat::ElectronRef> tightelectrons;
  for (size_t i = 0; i < tightelectronsvec.size(); i++) {
    if (tightelectronsvec[i]->pt() > 25.){
      tightelectrons.push_back(tightelectronsvec[i]);
    }
  }

  // Z matching + filling of variables
  if (tightelectrons.size()>1){   // need at least two electrons that pass id and pt cuts! 
    // First sort on pT --> should be redudant, as already sorted this way in miniAOD
    std::sort(tightelectrons.begin(), tightelectrons.end(), sortElectronsByPt);
  
    triplevec invmasspairs; // store i-j tight el index + invariant mass
    // only want pair of tight electrons that yield closest zmass diff   
    for (std::size_t i = 0; i < tightelectrons.size(); i++) {
      pat::ElectronRef el1 = tightelectrons[i];
      for (std::size_t j = i+1; j < tightelectrons.size(); j++) {
  	pat::ElectronRef el2 = tightelectrons[j];
  	TLorentzVector el1vec; el1vec.SetPtEtaPhiE(el1->pt(), el1->eta(), el1->phi(), el1->energy());
  	TLorentzVector el2vec; el2vec.SetPtEtaPhiE(el2->pt(), el2->eta(), el2->phi(), el2->energy());
  	el1vec += el2vec;
  	invmasspairs.push_back(std::make_tuple(i,j,std::abs(el1vec.M()-91.1876))); 
      }
    }
    auto best = std::min_element(invmasspairs.begin(),invmasspairs.end(),minimizeByZmass); // keep the lowest! --> returns pointer to lowest element
      
    pat::ElectronRef el1 = tightelectrons[std::get<0>(*best)];
    pat::ElectronRef el2 = tightelectrons[std::get<1>(*best)];

    // set the individual electron variables
    el1pid = el1->pdgId();  el2pid = el2->pdgId();
    el1pt  = el1->pt();     el2pt  = el2->pt();
    el1eta = el1->eta();    el2eta = el2->eta();
    el1phi = el1->phi();    el2phi = el2->phi();
    el1E   = el1->energy(); el2E   = el2->energy();
    el1p   = el1->p();      el2p   = el2->p();

    int nclust = 0;

    // ecal cluster tools --> stolen from ECAL ELF
    clustertools = new EcalClusterLazyTools (iEvent, iSetup, recHitCollectionEBTAG, recHitCollectionEETAG);
    const reco::SuperClusterRef& scel1 = el1->superCluster().isNonnull() ? el1->superCluster() : el1->parentSuperCluster();
    if(el1->ecalDrivenSeed() && scel1.isNonnull()) {
      DetId seedDetId = scel1->seed()->seed();
      const EcalRecHitCollection *recHits = (seedDetId.subdetId() == EcalBarrel) ?  clustertools->getEcalEBRecHitCollection() : clustertools->getEcalEERecHitCollection();
      EcalRecHitCollection::const_iterator seedRecHit = recHits->find(seedDetId) ;
      if(seedRecHit != recHits->end()) {
      	el1time = seedRecHit->time();
      }
      for (reco::CaloCluster_iterator caloiter = scel1->clustersBegin(); caloiter != scel1->clustersEnd(); ++caloiter){ // assume electron candidate is contained in supercluster entirely within EB or EE
	DetId caloDetId = (*caloiter)->seed();
	EcalRecHitCollection::const_iterator caloRecHit = recHits->find(caloDetId) ;
	if(caloRecHit != recHits->end()) {
	  nclust++;
	}
      }
    }

    std::cout << nclust << std::endl;


    const reco::SuperClusterRef& scel2 = el2->superCluster().isNonnull() ? el2->superCluster() : el2->parentSuperCluster();
    if(el2->ecalDrivenSeed() && scel2.isNonnull()) {
      DetId seedDetId = scel2->seed()->seed();
      const EcalRecHitCollection *recHits = (seedDetId.subdetId() == EcalBarrel) ?  clustertools->getEcalEBRecHitCollection() : clustertools->getEcalEERecHitCollection();
      EcalRecHitCollection::const_iterator seedRecHit = recHits->find(seedDetId) ;
      if(seedRecHit != recHits->end()) {
    	el2time = seedRecHit->time();
      }
    }

    TLorentzVector el1vec; el1vec.SetPtEtaPhiE(el1pt, el1eta, el1phi, el1E);
    TLorentzVector el2vec; el2vec.SetPtEtaPhiE(el2pt, el2eta, el2phi, el2E);
      
    TLorentzVector zvec(el1vec);
    zvec += el2vec;
      
    zpt   = zvec.Pt();
    zeta  = zvec.Eta();
    zphi  = zvec.Phi();
    zmass = zvec.M();
    zE    = zvec.Energy();
    zp    = zvec.P();

    delete clustertools;
  } // end section over tight electrons
  else{
    if (applyKinematicsFilter) return;
  }

  // MC INFO    
  if (isMC) {
    edm::Handle<std::vector<PileupSummaryInfo> > pileupInfoH;
    edm::Handle<GenEventInfoProduct>             genevtInfoH;
    edm::Handle<edm::View<reco::GenParticle> >         gensH;
    iEvent.getByToken(pileupInfoToken, pileupInfoH);
    iEvent.getByToken(genevtInfoToken, genevtInfoH);
    iEvent.getByToken(gensToken, gensH);

    // Pileup info
    puobs  = 0;
    putrue = 0;
    if (pileupInfoH.isValid()) {
      for (auto pileupInfo_iter = pileupInfoH->begin(); pileupInfo_iter != pileupInfoH->end(); ++pileupInfo_iter) {
	if (pileupInfo_iter->getBunchCrossing() == 0) {
	  puobs  = pileupInfo_iter->getPU_NumInteractions();
	  putrue = pileupInfo_iter->getTrueNumInteractions();
	}
      }
    }
    
    // Event weight info
    wgt = 1.0;
    if (genevtInfoH.isValid()) {wgt = genevtInfoH->weight();}
  
    // Gen particles info
    genzpid   = -99;   genzpt   = -99.0; genzeta   = -99.0; genzphi   = -99.0; genzmass = -99.0; genzE = -99.0; genzp = -99.0;
    genel1pid = -99;   genel1pt = -99.0; genel1eta = -99.0; genel1phi = -99.0;
    genel2pid = -99;   genel2pt = -99.0; genel2eta = -99.0; genel2phi = -99.0;
    genel1E   = -99.0; genel2E  = -99.0; genel1p   = -99.0; genel2p   = -99.0;

    if (gensH.isValid()){
      // try to get the final state z
      for (auto gens_iter = gensH->begin(); gens_iter != gensH->end(); ++gens_iter) {
  	if (gens_iter->pdgId() == 23 && gens_iter->numberOfDaughters() == 2 && abs(gens_iter->daughter(0)->pdgId()) == 11) { // Final state Z --> ee
	  // Z info
	  genzpid  = gens_iter->pdgId();
	  genzpt   = gens_iter->pt();
	  genzeta  = gens_iter->eta();
	  genzphi  = gens_iter->phi();
	  genzmass = gens_iter->mass();
	  genzE    = gens_iter->energy();
	  genzp    = gens_iter->p();
	  
	  // electron 1 info
	  genel1pid  = gens_iter->daughter(0)->pdgId();
	  genel1pt   = gens_iter->daughter(0)->pt();
	  genel1eta  = gens_iter->daughter(0)->eta();
	  genel1phi  = gens_iter->daughter(0)->phi();
	  genel1E    = gens_iter->daughter(0)->energy();
	  genel1p    = gens_iter->daughter(0)->p();
	  
	  // electron 2 info
	  genel2pid  = gens_iter->daughter(1)->pdgId();
	  genel2pt   = gens_iter->daughter(1)->pt();
	  genel2eta  = gens_iter->daughter(1)->eta();
	  genel2phi  = gens_iter->daughter(1)->phi();
	  genel2E    = gens_iter->daughter(1)->energy();
	  genel2p    = gens_iter->daughter(1)->p();

	} // end check over decay
      } // end loop over gen particles
    
      // if a Z is not found look for a pair of leptons ... 
      // this way when the pdgId is not guaranteed that you catch a Z boson, you can still recover DY production
      if (genzpid == -99) {
  	for (auto gens_iter = gensH->begin(); gens_iter != gensH->end(); ++gens_iter) {
  	  if (gens_iter->isPromptFinalState() || gens_iter->isPromptDecayed()) {
  	    if (gens_iter->pdgId() == 11) {
  	      genel1pid = gens_iter->pdgId();
  	      genel1pt  = gens_iter->pt();
  	      genel1eta = gens_iter->eta();
  	      genel1phi = gens_iter->phi();
  	      genel1E   = gens_iter->energy();
  	      genel1p   = gens_iter->p();
  	    }
  	    else if (gens_iter->pdgId() == -11) {
  	      genel2pid = gens_iter->pdgId();
  	      genel2pt  = gens_iter->pt();
  	      genel2eta = gens_iter->eta();
  	      genel2phi = gens_iter->phi();
  	      genel2E   = gens_iter->energy();
  	      genel2p   = gens_iter->p();
  	    }
  	  } // end check over final state 
	} // end loop over gen particles
  	if (genel1pid == 11 && genel2pid == -11) {
  	  TLorentzVector el1vec; el1vec.SetPtEtaPhiE(genel1pt, genel1eta, genel1phi, genel1E);
  	  TLorentzVector el2vec; el2vec.SetPtEtaPhiE(genel2pt, genel2eta, genel2phi, genel2E);

  	  TLorentzVector zvec(el1vec);
  	  zvec    += el2vec;

	  genzpid  = 23;
  	  genzpt   = zvec.Pt();
  	  genzeta  = zvec.Eta();
  	  genzphi  = zvec.Phi();
  	  genzmass = zvec.M();
  	  genzE    = zvec.Energy();
  	  genzp    = zvec.P();
  	}
      } // end recovery of DY check
    } // end check over gen particles are valid
  } // end check over isMC

  tree->Fill();    
}    

void TimingAnalyzer::beginJob() {

  edm::Service<TFileService> fs;
  tree = fs->make<TTree>("tree"       , "tree");

  // Run, Lumi, Event info
  tree->Branch("event"                , &event                , "event/I");
  tree->Branch("run"                  , &run                  , "run/I");
  tree->Branch("lumi"                 , &lumi                 , "lumi/I");
  
  // Triggers
  tree->Branch("hltsingleel"          , &hltsingleel          , "hltsingleel/O");
  tree->Branch("hltdoubleel"          , &hltdoubleel          , "hltdoubleel/O");
  tree->Branch("hltelnoiso"           , &hltelnoiso           , "hltelnoiso/O");

  // Vertex info
  tree->Branch("nvtx"                 , &nvtx                 , "nvtx/I");

  // Object counts
  tree->Branch("nvetoelectrons"       , &nvetoelectrons       , "nvetoelectrons/I");
  tree->Branch("nlooseelectrons"      , &nlooseelectrons      , "nlooseelectrons/I");
  tree->Branch("nmediumelectrons"     , &nmediumelectrons     , "nmediumelectrons/I");
  tree->Branch("ntightelectrons"      , &ntightelectrons      , "ntightelectrons/I");
  tree->Branch("nheepelectrons"       , &nheepelectrons       , "nheepelectrons/I");

  // Lepton info
  tree->Branch("el1pid"               , &el1pid               , "el1pid/I");
  tree->Branch("el1pt"                , &el1pt                , "el1pt/F");
  tree->Branch("el1eta"               , &el1eta               , "el1eta/F");
  tree->Branch("el1phi"               , &el1phi               , "el1phi/F");
  tree->Branch("el1E"                 , &el1E                 , "el1E/F");
  tree->Branch("el1p"                 , &el1p                 , "el1p/F");

  tree->Branch("el2pid"               , &el2pid               , "el2pid/I");
  tree->Branch("el2pt"                , &el2pt                , "el2pt/F");
  tree->Branch("el2eta"               , &el2eta               , "el2eta/F");
  tree->Branch("el2phi"               , &el2phi               , "el2phi/F");
  tree->Branch("el2E"                 , &el2E                 , "el2E/F");
  tree->Branch("el2p"                 , &el2p                 , "el2p/F");

  // Time info
  tree->Branch("el1time"              , &el1time              , "el1time/F");
  tree->Branch("el2time"              , &el2time              , "el2time/F");

  // Dilepton info
  tree->Branch("zmass"                , &zmass                , "zmass/F");
  tree->Branch("zpt"                  , &zpt                  , "zpt/F");
  tree->Branch("zeta"                 , &zeta                 , "zeta/F");
  tree->Branch("zphi"                 , &zphi                 , "zphi/F");
  tree->Branch("zE"                   , &zE                   , "zE/F");
  tree->Branch("zp"                   , &zp                   , "zp/F");
  
  // Z gen-level info: leptonic 
  if (isMC) {
    // Pileup info
    tree->Branch("puobs"                , &puobs                , "puobs/I");
    tree->Branch("putrue"               , &putrue               , "putrue/I");

    // Event weights
    tree->Branch("wgt"                  , &wgt                  , "wgt/F");

    //Gen particles info
    tree->Branch("genzpid"              , &genzpid              , "genzpid/I");
    tree->Branch("genzpt"               , &genzpt               , "genzpt/F");
    tree->Branch("genzeta"              , &genzeta              , "genzeta/F");
    tree->Branch("genzphi"              , &genzphi              , "genzphi/F");
    tree->Branch("genzmass"             , &genzmass             , "genzmass/F");
    tree->Branch("genzE"                , &genzE                , "genzE/F");
    tree->Branch("genzp"                , &genzp                , "genzp/F");

    tree->Branch("genel1pid"            , &genel1pid            , "genel1pid/I");
    tree->Branch("genel1pt"             , &genel1pt             , "genel1pt/F");
    tree->Branch("genel1eta"            , &genel1eta            , "genel1eta/F");
    tree->Branch("genel1phi"            , &genel1phi            , "genel1phi/F");
    tree->Branch("genel1E"              , &genel1E              , "genel1E/F");
    tree->Branch("genel1p"              , &genel1p              , "genel1p/F");

    tree->Branch("genel2pid"            , &genel2pid            , "genel2pid/I");
    tree->Branch("genel2pt"             , &genel2pt             , "genel2pt/F");
    tree->Branch("genel2eta"            , &genel2eta            , "genel2eta/F");
    tree->Branch("genel2phi"            , &genel2phi            , "genel2phi/F");
    tree->Branch("genel2E"              , &genel2E              , "genel2E/F");
    tree->Branch("genel2p"              , &genel2p              , "genel2p/F");
  }
}

void TimingAnalyzer::endJob() {}

void TimingAnalyzer::beginRun(edm::Run const& iRun, edm::EventSetup const& iSetup) {
  // triggers for the Analysis
  triggerPathsVector.push_back("HLT_Ele17_Ele12_CaloIdL_TrackIdL_IsoVL_DZ");
  triggerPathsVector.push_back("HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL_DZ");
  triggerPathsVector.push_back("HLT_Ele23_WPLoose_Gsf_v");
  triggerPathsVector.push_back("HLT_Ele27_WPLoose_Gsf_v");
  triggerPathsVector.push_back("HLT_Ele23_CaloIdL_TrackIdL_IsoVL_v");
  triggerPathsVector.push_back("HLT_Ele27_WP85_Gsf_v");
  triggerPathsVector.push_back("HLT_Ele105_CaloIdVT_GsfTrkIdT_v");
  triggerPathsVector.push_back("HLT_Ele115_CaloIdVT_GsfTrkIdT_v");
  
  HLTConfigProvider hltConfig;
  bool changedConfig = false;
  hltConfig.init(iRun, iSetup, triggerResultsTag.process(), changedConfig);
  
  for (size_t i = 0; i < triggerPathsVector.size(); i++) {
    triggerPathsMap[triggerPathsVector[i]] = -1;
  }
  
  for(size_t i = 0; i < triggerPathsVector.size(); i++){
    TPRegexp pattern(triggerPathsVector[i]);
    for(size_t j = 0; j < hltConfig.triggerNames().size(); j++){
      std::string pathName = hltConfig.triggerNames()[j];
      if(TString(pathName).Contains(pattern)){
	triggerPathsMap[triggerPathsVector[i]] = j;
      }
    }
  }
}

void TimingAnalyzer::endRun(edm::Run const&, edm::EventSetup const&) {}

void TimingAnalyzer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

DEFINE_FWK_MODULE(TimingAnalyzer);
