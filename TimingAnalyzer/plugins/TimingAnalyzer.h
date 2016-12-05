// basic C++ headers
#include <iostream>
#include <memory>
#include <vector>
#include <utility>
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
#include "DataFormats/DetId/interface/DetId.h"
#include "DataFormats/EcalDetId/interface/EBDetId.h"
#include "DataFormats/EcalDetId/interface/EEDetId.h"
#include "DataFormats/EcalDetId/interface/EcalSubdetector.h"

// EGamma Tools
#include "RecoEcal/EgammaCoreTools/interface/EcalClusterTools.h"
#include "RecoEcal/EgammaCoreTools/interface/EcalClusterLazyTools.h"

// Geometry
#include "Geometry/Records/interface/CaloGeometryRecord.h"
#include "Geometry/CaloGeometry/interface/CaloGeometry.h"
#include "Geometry/CaloGeometry/interface/CaloSubdetectorGeometry.h"
#include "Geometry/CaloGeometry/interface/CaloCellGeometry.h"

// ROOT
#include "TTree.h"
#include "TLorentzVector.h"
#include "TPRegexp.h"

typedef std::tuple<int, int, float> triple;
typedef std::vector<triple> triplevec;
typedef std::vector<std::pair<DetId,float> > DetIdPairVec;

inline bool minimizeByZmass(const triple& elpair1, const triple& elpair2)
{
  return std::get<2>(elpair1)<std::get<2>(elpair2);
}

inline bool sortElectronsByPt(const pat::ElectronRef& el1, const pat::ElectronRef& el2)
{
  return el1->pt()>el2->pt();
}

class TimingAnalyzer : public edm::one::EDAnalyzer<edm::one::SharedResources,edm::one::WatchRuns> 
{
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

  // how to select electrons
  const bool doZmassSort;

  // ECALELF tools
  EcalClusterLazyTools *clustertools;

  // output ntuple
  // tree
  TTree* tree;

  // event info
  int event, run, lumi;  

  // triggers 
  bool hltdoubleel33,hltdoubleel37;

  // vertices
  int nvtx; 
  float vtxX, vtxY, vtxZ;

  // object counts
  int nvetoelectrons,nlooseelectrons,nmediumelectrons,ntightelectrons,nheepelectrons;

  // electron info
  int   el1pid,el2pid;
  float el1pt,el1eta,el1phi,el1E,el1p;
  float el2pt,el2eta,el2phi,el2E,el2p;
  
  // supercluster info 
  float el1scX, el1scY, el1scZ, el1scE;
  float el2scX, el2scY, el2scZ, el2scE;
  int   el1nrh, el2nrh;

  // all rec hit info
  std::vector<float> el1rhXs, el1rhYs, el1rhZs, el1rhEs, el1rhtimes;
  std::vector<float> el2rhXs, el2rhYs, el2rhZs, el2rhEs, el2rhtimes;
  std::vector<int> el1rhids, el2rhids;
  std::vector<int> el1rhgain1s, el1rhgain6s, el2rhgain1s, el2rhgain6s;

  // seed info
  float el1seedX, el1seedY, el1seedZ, el1seedE, el1seedtime;
  float el2seedX, el2seedY, el2seedZ, el2seedE, el2seedtime;
  int el1seedid, el2seedid;
  int el1seedgain1, el1seedgain6, el2seedgain1, el2seedgain6;

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
