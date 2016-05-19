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

typedef std::tuple<int, int, double> triple;
typedef std::vector<triple> triplevec;

inline bool minimizeByZmass(const triple& elpair1, const triple& elpair2){
  return std::get<2>(elpair1)<std::get<2>(elpair2);
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
  
  //   void findMother(const reco::Candidate*, int &, double &, double &, double &);
  //   void findFirstNonPhotonMother(const reco::Candidate*, int &, double &, double &, double &);

  // Gen Particles and MC info
  const bool isMC;
  const bool isSignalSample;
  const bool addGenParticles;
  edm::EDGetTokenT<std::vector<PileupSummaryInfo> > pileupInfoToken;
  edm::EDGetTokenT<GenEventInfoProduct>             genevtInfoToken;
  edm::EDGetTokenT<edm::View<reco::GenParticle> >   gensToken;
  double xsec;

  // Trigger
  const edm::InputTag triggerResultsTag;
  edm::EDGetTokenT<edm::TriggerResults> triggerResultsToken;
  std::vector<std::string>   triggerPathsVector;
  std::map<std::string, int> triggerPathsMap;
  const bool applyHLTFilter;

  // Vertex
  const edm::InputTag verticesTag;
  edm::EDGetTokenT<std::vector<reco::Vertex> > verticesToken;

  // electrons
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

  // ECAL RecHits
  edm::EDGetTokenT<EcalRecHitCollection> recHitCollectionEBTAG;
  edm::EDGetTokenT<EcalRecHitCollection> recHitCollectionEETAG;

  // output ntuple
  // tree
  TTree* tree;

  // pileup info
  int32_t puobs,putrue; 
  //  int32_t wzid,l1id,l2id;
  int32_t el1pid,el2pid;
  //   int32_t phidl,phidm,phidt,phidh,parid,ancid; 

  // event info
  uint32_t event, run, lumi;  
  uint32_t nvtx;
  uint32_t nvetoelectrons,nlooseelectrons,nmediumelectrons,ntightelectrons,nheepelectrons;

  // trigger and met filters flags 
  uint8_t hltdoubleel,hltsingleel,hltelnoiso;

  // muon, ele, dilepton info
  double el1pt,el1eta,el1phi,ele1e,el2pt,ele2e,el2eta,el2phi;
  double zeemass,zeept,zeeeta,zeephi;
  // gen info leptoni W/Z boson (1 per event)
  //  double wzmass,wzpt,wzeta,wzphi,l1pt,l1eta,l1phi,l2pt,l2eta,l2phi;

  // timing
  double el1time, el2time;

  // weights
  double wgt,puwgt;

  // Stolen from ECALELF
  EcalClusterLazyTools *clustertools;

  // sorting objects
  template<typename T> 
  class PatPtSorter{
  public:
    bool operator ()(const T & i, const T & j) const {
      return (i->pt() > j->pt());
    }
  };
  PatPtSorter<pat::ElectronRef> electronSorter;
};


TimingAnalyzer::TimingAnalyzer(const edm::ParameterSet& iConfig): 
  ///////////// GEN INFO
  // isMC or Data --> default Data
  isMC(iConfig.existsAs<bool>("isMC") ? iConfig.getParameter<bool>("isMC") : false),
  
  // is signal sample or not
  isSignalSample(iConfig.existsAs<bool>("isSignalSample") ? iConfig.getParameter<bool>("isSignalSample") : false),
  addGenParticles(iConfig.existsAs<bool>("addGenParticles") ? iConfig.getParameter<bool>("addGenParticles") : false),
  // xsec
  xsec(iConfig.existsAs<double>("xsec") ? iConfig.getParameter<double>("xsec") * 1000.0 : -1000.),
  
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
  
  //recHits
  recHitCollectionEBTAG(consumes<EcalRecHitCollection>(iConfig.getParameter<edm::InputTag>( "recHitCollectionEB" ))),
  recHitCollectionEETAG(consumes<EcalRecHitCollection>(iConfig.getParameter<edm::InputTag>( "recHitCollectionEE" )))
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
  // Get handles to all the requisite collections
  // TRIGGER and FILTERS
  edm::Handle<edm::TriggerResults> triggerResultsH;
  iEvent.getByToken(triggerResultsToken, triggerResultsH);

  // GEN INFO    
  edm::Handle<std::vector<PileupSummaryInfo> > pileupInfoH;
  edm::Handle<GenEventInfoProduct>             genevtInfoH;
  edm::Handle<edm::View<reco::GenParticle> >         gensH;

  if(isMC){
    iEvent.getByToken(pileupInfoToken, pileupInfoH);
    if (addGenParticles or isSignalSample)
      iEvent.getByToken(gensToken, gensH);
  }

  // VERTEX
  edm::Handle<std::vector<reco::Vertex> > verticesH;
  iEvent.getByToken(verticesToken, verticesH);

  // ELECTRONS
  edm::Handle<pat::ElectronRefVector> vetoelectronsH;
  iEvent.getByToken(vetoelectronsToken, vetoelectronsH);
  pat::ElectronRefVector vetoelectrons = *vetoelectronsH;

  edm::Handle<pat::ElectronRefVector> looseelectronsH;
  iEvent.getByToken(looseelectronsToken, looseelectronsH);
  pat::ElectronRefVector looseelectrons = *looseelectronsH;

  edm::Handle<pat::ElectronRefVector> mediumelectronsH;
  iEvent.getByToken(mediumelectronsToken, mediumelectronsH);
  pat::ElectronRefVector mediumelectrons = *mediumelectronsH;

  edm::Handle<pat::ElectronRefVector> tightelectronsH;
  iEvent.getByToken(tightelectronsToken, tightelectronsH);
  pat::ElectronRefVector tightelectrons = *tightelectronsH;

  edm::Handle<pat::ElectronRefVector> heepelectronsH;
  iEvent.getByToken(heepelectronsToken, heepelectronsH);
  pat::ElectronRefVector heepelectrons = *heepelectronsH;

  // Event, lumi, run info
  event = iEvent.id().event();
  run   = iEvent.id().run();
  lumi  = iEvent.luminosityBlock();
    
  // Trigger info
  hltdoubleel     = 0;
  hltsingleel     = 0;
  hltelnoiso      = 0;

  // Which triggers fired
  if(triggerResultsH.isValid()){
    for (size_t i = 0; i < triggerPathsVector.size(); i++) {
      if (triggerPathsMap[triggerPathsVector[i]] == -1) continue;	
      if (i == 0 && triggerResultsH->accept(triggerPathsMap[triggerPathsVector[i]])) hltdoubleel     = 1; // Double electron trigger
      if (i == 1 && triggerResultsH->accept(triggerPathsMap[triggerPathsVector[i]])) hltdoubleel     = 1; // Double electron trigger
      if (i == 2 && triggerResultsH->accept(triggerPathsMap[triggerPathsVector[i]])) hltsingleel     = 1; // Single electron trigger
      if (i == 3 && triggerResultsH->accept(triggerPathsMap[triggerPathsVector[i]])) hltsingleel     = 1; // Single electron trigger
      if (i == 4 && triggerResultsH->accept(triggerPathsMap[triggerPathsVector[i]])) hltsingleel     = 1; // Single electron trigger
      if (i == 5 && triggerResultsH->accept(triggerPathsMap[triggerPathsVector[i]])) hltsingleel     = 1; // Single electron trigger
      if (i == 6 && triggerResultsH->accept(triggerPathsMap[triggerPathsVector[i]])) hltelnoiso      = 1; // Single electron trigger
      if (i == 7 && triggerResultsH->accept(triggerPathsMap[triggerPathsVector[i]])) hltelnoiso      = 1; // Single electron trigger
    }
  }

  // skim on events that pass triggers
  bool triggered = false;
  if (hltdoubleel     == 1) triggered = true;
  if (hltsingleel     == 1) triggered = true;
  if (hltelnoiso      == 1) triggered = true;
  if (applyHLTFilter && !triggered) return;

  // Vertex + PU info
  if(verticesH.isValid()) nvtx = verticesH->size();
  else nvtx = 0;

  puobs  = 0;
  putrue = 0;
  puwgt  = 1.;
  wgt = 1.0;  // in case the cross section is not set from outside --> fix to 1 as dummy value
  if (pileupInfoH.isValid()) {
    for (auto pileupInfo_iter = pileupInfoH->begin(); pileupInfo_iter != pileupInfoH->end(); ++pileupInfo_iter) {
      if (pileupInfo_iter->getBunchCrossing() == 0) {
	puobs  = pileupInfo_iter->getPU_NumInteractions();
	putrue = pileupInfo_iter->getTrueNumInteractions();
      }
    }
  }

  // ELECTRON ANALYSIS
  if (vetoelectronsH.isValid())   nvetoelectrons   = vetoelectronsH->size();
  if (looseelectronsH.isValid())  nlooseelectrons  = looseelectronsH->size();
  if (mediumelectronsH.isValid()) nmediumelectrons = mediumelectronsH->size();
  if (tightelectronsH.isValid())  ntightelectrons  = tightelectronsH->size();
  if (heepelectronsH.isValid())   nheepelectrons   = heepelectronsH->size();

  // tree vars
  zeemass     = 0.0;  zeept       = 0.0;  zeeeta      = 0.0; zeephi      = 0.0;
  el1pid      = 0;    el1pt       = 0.0;  el1eta      = 0.0; el1phi      = 0.0; 
  el2pid      = 0;    el2pt       = 0.0;  el2eta      = 0.0; el2phi      = 0.0; 
  el1time     = -99.; el2time     = -99.; 

  if (tightelectrons.size()>1){ // need at least two tight electrons!
    triplevec invmasspairs; // store i-j tight el index + invariant mass

    // only want pair of tight electrons that yield closest zmass diff   
    for (std::size_t i = 0; i < tightelectrons.size(); i++) {
      pat::ElectronRef el1 = tightelectrons[i];
      for (std::size_t j = i+1; j < tightelectrons.size(); j++) {
  	pat::ElectronRef el2 = tightelectrons[j];
  	TLorentzVector el1vec; el1vec.SetPtEtaPhiE(el1->pt(), el1->eta(), el1->phi(), el1->p());
  	TLorentzVector el2vec; el2vec.SetPtEtaPhiE(el2->pt(), el2->eta(), el2->phi(), el2->p());
  	el1vec += el2vec;
  	invmasspairs.push_back(std::make_tuple(i,j,std::abs(el1vec.M()-91.1876))); 
      }
    }
    auto best = std::min_element(invmasspairs.begin(),invmasspairs.end(),minimizeByZmass); // keep the lowest! --> returns pointer to lowest element
      
    pat::ElectronRef el1 = tightelectrons[std::get<0>(*best)];
    pat::ElectronRef el2 = tightelectrons[std::get<1>(*best)];
      
    el1pid = el1->pdgId(); el2pid = el2->pdgId();
    el1pt  = el1->pt();    el2pt  = el2->pt();
    el1eta = el1->eta();   el2eta = el2->eta();
    el1phi = el1->phi();   el2phi = el2->phi();

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
    }
    const reco::SuperClusterRef& scel2 = el2->superCluster().isNonnull() ? el2->superCluster() : el2->parentSuperCluster();
    if(el2->ecalDrivenSeed() && scel2.isNonnull()) {
      DetId seedDetId = scel2->seed()->seed();
      const EcalRecHitCollection *recHits = (seedDetId.subdetId() == EcalBarrel) ?  clustertools->getEcalEBRecHitCollection() : clustertools->getEcalEERecHitCollection();
      EcalRecHitCollection::const_iterator seedRecHit = recHits->find(seedDetId) ;
      if(seedRecHit != recHits->end()) {
    	el2time = seedRecHit->time();
      }
    }

    TLorentzVector el1vec; el1vec.SetPtEtaPhiE(el1pt, el1eta, el1phi, el1->p());
    TLorentzVector el2vec; el2vec.SetPtEtaPhiE(el2pt, el2eta, el2phi, el2->p());
      
    TLorentzVector zvec(el1vec);
    zvec += el2vec;
      
    zeemass = zvec.M();
    zeept   = zvec.Pt();
    zeeeta  = zvec.Eta();
    zeephi  = zvec.Phi();

    delete clustertools;
  } // end section over tight electrons

    // Generator-level information
  //     wzid          = 0; wzmass        = 0.0; wzpt          = 0.0; wzeta         = 0.0; wzphi         = 0.0;
  //     l1id          = 0; l1pt          = 0.0; l1eta         = 0.0; l1phi         = 0.0;
  //     l2id          = 0; l2pt          = 0.0; l2eta         = 0.0; l2phi         = 0.0;
  //     parid         = 0; parpt         = 0.0; pareta        = 0.0; parphi        = 0.0;
  //     ancid         = 0; ancpt         = 0.0; anceta        = 0.0; ancphi        = 0.0;

  // dump inportant gen particles
  //   if(addGenParticles and gensH.isValid()){
      
  //       // loop on genParticles (prunedGenParticles) trying to find W/Z decying leptonically or hadronically, top and anti-top quarks
  //       for (auto gens_iter = gensH->begin(); gens_iter != gensH->end(); ++gens_iter) {
  // 	if ( (gens_iter->pdgId() == 23 || abs(gens_iter->pdgId()) == 24) && // Z or W-boson
  // 	     gens_iter->numberOfDaughters() > 1 && // before the decay (more than one daughter)
  // 	     abs(gens_iter->daughter(0)->pdgId()) > 10 && 
  // 	     abs(gens_iter->daughter(0)->pdgId()) < 17)  { // decays into leptons, neutrinos 
	  
  // 	  wzid   = gens_iter->pdgId();
  // 	  wzmass = gens_iter->mass();
  // 	  wzpt   = gens_iter->pt();
  // 	  wzeta  = gens_iter->eta();
  // 	  wzphi  = gens_iter->phi();
	  
  // 	  l1id   = gens_iter->daughter(0)->pdgId();
  // 	  l1pt   = gens_iter->daughter(0)->pt();
  // 	  l1eta  = gens_iter->daughter(0)->eta();
  // 	  l1phi  = gens_iter->daughter(0)->phi();
	
  // 	  l2id   = gens_iter->daughter(1)->pdgId();
  // 	  l2pt   = gens_iter->daughter(1)->pt();
  // 	  l2eta  = gens_iter->daughter(1)->eta();
  // 	  l2phi  = gens_iter->daughter(1)->phi();
	
  // 	}
  //       }
    
  // if a Z/W is not found look for a pair of lepton .. this way with the pdgId is not guaranteed that you catch a Z/W boson and also recover DY production
  //       if (wzid == 0) {
  // 	for (auto gens_iter = gensH->begin(); gens_iter != gensH->end(); ++gens_iter) {
  // 	  if (gens_iter->isPromptFinalState() || gens_iter->isPromptDecayed()) {
  // 	    if (gens_iter->pdgId() >  10 && gens_iter->pdgId() <  17) {
  // 	      l1id   = gens_iter->pdgId();
  // 	      l1pt   = gens_iter->pt();
  // 	      l1eta  = gens_iter->eta();
  // 	      l1phi  = gens_iter->phi();
  // 	    }
  // 	    if (gens_iter->pdgId() < -10 && gens_iter->pdgId() > -17) {
  // 	      l2id   = gens_iter->pdgId();
  // 	      l2pt   = gens_iter->pt();
  // 	      l2eta  = gens_iter->eta();
  // 	      l2phi  = gens_iter->phi();
  // 	    }
  // 	  }
  // 	}
  // 	if (l1id > 0) {
  // 	  TLorentzVector l1vec;
  // 	  TLorentzVector l2vec;
  // 	  l1vec.SetPtEtaPhiM(l1pt, l1eta, l1phi, 0.);
  // 	  l2vec.SetPtEtaPhiM(l2pt, l2eta, l2phi, 0.);
  // 	  TLorentzVector wzvec(l1vec);
  // 	  wzvec += l2vec;
  // 	  wzmass = wzvec.M();
  // 	  wzpt   = wzvec.Pt();
  // 	  wzeta  = wzvec.Eta();
  // 	  wzphi  = wzvec.Phi();
  // 	  if (l1id+l2id == 0) wzid = 23;
  // 	  else                wzid = 24;
  // 	}
  //       }
      
  //       // no W or Z decay leptonically
  //       if (wzid == 0) {
  // 	for (auto gens_iter = gensH->begin(); gens_iter != gensH->end(); ++gens_iter) { // loop on prunedGenParticles
  // 	  if (gens_iter->pdgId() == 22 && // photons
  // 	      gens_iter->status() == 1 && // final state
  // 	      gens_iter->isPromptFinalState() &&
  // 	      gens_iter->pt() > wzpt) {
	  
  // 	    wzid   = gens_iter->pdgId();
  // 	    wzpt   = gens_iter->pt();
  // 	    wzeta  = gens_iter->eta();
  // 	    wzphi  = gens_iter->phi();
	  
  // 	    findFirstNonPhotonMother(&(*gens_iter), ancid, ancpt, anceta, ancphi);
  // 	    findMother(&(*gens_iter), parid, parpt, pareta, parphi);
  // 	  }
  // 	}          
  //       }
      
  //    }
  tree->Fill();    
}    

void TimingAnalyzer::beginJob() {

  edm::Service<TFileService> fs;
  tree = fs->make<TTree>("tree"       , "tree");

  // Run, Lumi, Event info
  tree->Branch("event"                , &event                , "event/i");
  tree->Branch("run"                  , &run                  , "run/i");
  tree->Branch("lumi"                 , &lumi                 , "lumi/i");
  // Event weights
  tree->Branch("xsec"                 , &xsec                 , "xsec/D");
  tree->Branch("wgt"                  , &wgt                  , "wgt/D");
  // Pileup info
  tree->Branch("puwgt"                , &puwgt                , "puwgt/D");
  tree->Branch("puobs"                , &puobs                , "puobs/I");
  tree->Branch("putrue"               , &putrue               , "putrue/I");
  tree->Branch("nvtx"                 , &nvtx                 , "nvtx/i");
  
  // Triggers
  tree->Branch("hltsingleel"          , &hltsingleel          , "hltsingleel/b");
  tree->Branch("hltdoubleel"          , &hltdoubleel          , "hltdoubleel/b");
  tree->Branch("hltelnoiso"           , &hltelnoiso           , "hltelnoiso/b");

  // Object counts
  tree->Branch("nvetoelectrons"       , &nvetoelectrons       , "nvetoelectrons/i");
  tree->Branch("nlooseelectrons"      , &nlooseelectrons      , "nlooseelectrons/i");
  tree->Branch("nmediumelectrons"     , &nmediumelectrons     , "nmediumelectrons/i");
  tree->Branch("ntightelectrons"      , &ntightelectrons      , "ntightelectrons/i");
  tree->Branch("nheepelectrons"       , &nheepelectrons       , "nheepelectrons/i");

  // Lepton info
  tree->Branch("el1pid"               , &el1pid               , "el1pid/I");
  tree->Branch("el1pt"                , &el1pt                , "el1pt/D");
  tree->Branch("el1eta"               , &el1eta               , "el1eta/D");
  tree->Branch("el1phi"               , &el1phi               , "el1phi/D");
  tree->Branch("el2pid"               , &el2pid               , "el2pid/I");
  tree->Branch("el2pt"                , &el2pt                , "el2pt/D");
  tree->Branch("el2eta"               , &el2eta               , "el2eta/D");
  tree->Branch("el2phi"               , &el2phi               , "el2phi/D");

  // Time info
  tree->Branch("el1time"              , &el1time              , "el1time/D");
  tree->Branch("el2time"              , &el2time              , "el2time/D");

  // Dilepton info
  tree->Branch("zeemass"              , &zeemass              , "zeemass/D");
  tree->Branch("zeept"                , &zeept                , "zeept/D");
  tree->Branch("zeeeta"               , &zeeeta               , "zeeeta/D");
  tree->Branch("zeephi"               , &zeephi               , "zeephi/D");
  
  //   // Z gen-level info: leptonic 
  //   tree->Branch("zid"                 , &wid                 , "zid/I");
  //   tree->Branch("wzmass"               , &wzmass               , "wzmass/D");
  //   tree->Branch("wzpt"                 , &wzpt                 , "wzpt/D");
  //   tree->Branch("wzeta"                , &wzeta                , "wzeta/D");
  //   tree->Branch("wzphi"                , &wzphi                , "wzphi/D");
  //   tree->Branch("l1id"                 , &l1id                 , "l1id/I");
  //   tree->Branch("l1pt"                 , &l1pt                 , "l1pt/D");
  //   tree->Branch("l1eta"                , &l1eta                , "l1eta/D");
  //   tree->Branch("l1phi"                , &l1phi                , "l1phi/D");
  //   tree->Branch("l2id"                 , &l2id                 , "l2id/I");
  //   tree->Branch("l2pt"                 , &l2pt                 , "l2pt/D");
  //   tree->Branch("l2eta"                , &l2eta                , "l2eta/D");
  //   tree->Branch("l2phi"                , &l2phi                , "l2phi/D");
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

// // for photons
// void TimingAnalyzer::findFirstNonPhotonMother(const reco::Candidate *particle, int& ancestorid, double& ancestorpt, double& ancestoreta, double& ancestorphi) {

//   if (particle == 0)
//     return;
  
//   if (abs(particle->pdgId()) == 22) 
//     findFirstNonPhotonMother(particle->mother(0), ancestorid, ancestorpt, ancestoreta, ancestorphi);
//   else {
//     ancestorid  = particle->pdgId();
//     ancestorpt  = particle->pt();
//     ancestoreta = particle->eta();
//     ancestorphi = particle->phi();
//   }
//   return;
// }

// void TimingAnalyzer::findMother(const reco::Candidate *particle, int& ancestorid, double& ancestorpt, double& ancestoreta, double& ancestorphi) {
  
//   if (particle == 0) 
//     return;

//   if (abs(particle->pdgId()) == 22) {
//     ancestorid  = particle->pdgId();
//     ancestorpt  = particle->pt();
//     ancestoreta = particle->eta();
//     ancestorphi = particle->phi();
//   }
//   return;
// }

DEFINE_FWK_MODULE(TimingAnalyzer);
