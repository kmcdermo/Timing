#ifndef __ZeeTree__
#define __ZeeTree__

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
#include "DataFormats/EcalRecHit/interface/EcalRecHit.h"
#include "DataFormats/EcalRecHit/interface/EcalRecHitCollections.h"

// Geometry
#include "Geometry/Records/interface/CaloGeometryRecord.h"
#include "Geometry/CaloGeometry/interface/CaloGeometry.h"
#include "Geometry/CaloGeometry/interface/CaloSubdetectorGeometry.h"
#include "Geometry/CaloGeometry/interface/CaloCellGeometry.h"

// ROOT
#include "TTree.h"
#include "TLorentzVector.h"
#include "TPRegexp.h"

#include "Timing/TimingAnalyzer/plugins/CommonUtils.hh"

class ZeeTree : public edm::one::EDAnalyzer<edm::one::SharedResources,edm::one::WatchRuns> 
{
public:
  explicit ZeeTree(const edm::ParameterSet&);
  ~ZeeTree();
  
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

  // Electrons + ID
  const edm::InputTag electronVetoIdMapTag;
  edm::EDGetTokenT<edm::ValueMap<bool> > electronVetoIdMapToken;
  const edm::InputTag electronLooseIdMapTag;
  edm::EDGetTokenT<edm::ValueMap<bool> > electronLooseIdMapToken;
  const edm::InputTag electronMediumIdMapTag;
  edm::EDGetTokenT<edm::ValueMap<bool> > electronMediumIdMapToken;
  const edm::InputTag electronTightIdMapTag;
  edm::EDGetTokenT<edm::ValueMap<bool> > electronTightIdMapToken;
  const edm::InputTag electronsTag;
  edm::EDGetTokenT<std::vector<pat::Electron> > electronsToken;
  const bool applyKinematicsFilter;

  // ECAL RecHits
  const edm::InputTag recHitsEBTag;
  edm::EDGetTokenT<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > recHitsEBToken;
  const edm::InputTag recHitsEETag;
  edm::EDGetTokenT<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > recHitsEEToken;

  // Gen Particles and MC info
  const bool isMC;
  edm::EDGetTokenT<std::vector<PileupSummaryInfo> > pileupInfoToken;
  edm::EDGetTokenT<GenEventInfoProduct>             genevtInfoToken;
  edm::EDGetTokenT<edm::View<reco::GenParticle> >   gensToken;

  // output ntuple
  // tree
  TTree * tree;

  // event info
  unsigned int run, lumi;
  unsigned long int event;
  
  // triggers 
  bool hltdoubleel23_12,hltdoubleel33_33,hltdoubleel37_27;

  // vertices
  int nvtx; 
  float vtxX, vtxY, vtxZ;

  // electron info
  int el1pid,el2pid;
  float el1E,el1p,el1pt,el1eta,el1phi;
  float el2E,el2p,el2pt,el2eta,el2phi;
  
  // supercluster info 
  float el1scX, el1scY, el1scZ, el1scE;
  float el2scX, el2scY, el2scZ, el2scE;
  
  // global rechit info
  int el1nrh, el1seedpos;
  int el2nrh, el2seedpos;
  
  // all rec hit info
  std::vector<float> el1rhXs, el1rhYs, el1rhZs, el1rhEs, el1rhtimes;
  std::vector<float> el2rhXs, el2rhYs, el2rhZs, el2rhEs, el2rhtimes;
  std::vector<int> el1rhids, el2rhids;
  std::vector<bool> el1rhOOTs, el1rhgain1s, el1rhgain6s;
  std::vector<bool> el2rhOOTs, el2rhgain1s, el2rhgain6s;

  // dielectron info
  float zE,zp,zpt,zeta,zphi,zmass;

  // MC Info Only
  // pileup info
  int puobs,putrue; 

  // weights
  float wgt;

  // gen particle info
  int genzpid,genel1pid,genel2pid;
  float genzE,genzp,genzpt,genzeta,genzphi,genzmass;
  float genel1E,genel1p,genel1pt,genel1eta,genel1phi;
  float genel2E,genel2p,genel2pt,genel2eta,genel2phi;
};

#endif
