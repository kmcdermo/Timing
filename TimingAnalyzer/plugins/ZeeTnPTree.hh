#ifndef __ZeeTnPTree__
#define __ZeeTnPTree__

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

class ZeeTnPTree : public edm::one::EDAnalyzer<edm::one::SharedResources,edm::one::WatchRuns> 
{
public:
  explicit ZeeTnPTree(const edm::ParameterSet&);
  ~ZeeTnPTree();
  
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

  // ECAL RecHits
  const edm::InputTag recHitsEBTag;
  edm::EDGetTokenT<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > recHitsEBToken;
  const edm::InputTag recHitsEETag;
  edm::EDGetTokenT<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > recHitsEEToken;

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
  int   el1pid,el1vid,el2pid,el2vid;
  float el1E,el1p,el1pt,el1eta,el1phi;
  float el2E,el2p,el2pt,el2eta,el2phi;
  
  // supercluster info 
  float el1scX, el1scY, el1scZ, el1scE;
  float el2scX, el2scY, el2scZ, el2scE;
  
  // all rec hit info
  float el1seedX,el1seedY,el1seedZ,el1seedE,el1seedtime;
  int   el1seedid,el1seedOOT,el1seedgain1,el1seedgain6;
  float el2seedX,el2seedY,el2seedZ,el2seedE,el2seedtime;
  int   el2seedid,el2seedOOT,el2seedgain1,el2seedgain6;

  // dielectron info
  float zpt,zeta,zphi,zmass,zE,zp;
};

#endif
