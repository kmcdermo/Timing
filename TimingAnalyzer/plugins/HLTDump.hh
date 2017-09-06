#ifndef __HLTDump__
#define __HLTDump__

// basic C++ headers
#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <vector>
#include <tuple>
#include <utility>
#include <map>
#include <unordered_map>
#include <cmath>
#include <algorithm>

// FWCore
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h" 

// HLT + Trigger info
#include "FWCore/Common/interface/TriggerNames.h"
#include "HLTrigger/HLTcore/interface/HLTConfigProvider.h"
#include "DataFormats/Common/interface/TriggerResults.h"
#include "DataFormats/HLTReco/interface/TriggerEvent.h"
#include "DataFormats/HLTReco/interface/TriggerObject.h"

// DataFormats
#include "DataFormats/Common/interface/ValueMap.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/PatCandidates/interface/Photon.h"
#include "DataFormats/PatCandidates/interface/MET.h"
#include "DataFormats/PatCandidates/interface/Jet.h"

// DetIds 
#include "DataFormats/DetId/interface/DetId.h"
#include "DataFormats/EcalDetId/interface/EBDetId.h"
#include "DataFormats/EcalDetId/interface/EEDetId.h"
#include "DataFormats/EcalDetId/interface/EcalSubdetector.h"

// Ecal RecHits
#include "DataFormats/EcalRecHit/interface/EcalRecHit.h"
#include "DataFormats/EcalRecHit/interface/EcalRecHitCollections.h"

// Supercluster info
#include "DataFormats/EgammaReco/interface/SuperCluster.h"
#include "DataFormats/CaloRecHit/interface/CaloCluster.h"

// EGamma Tools
#include "RecoEcal/EgammaCoreTools/interface/EcalTools.h"
#include "RecoEcal/EgammaCoreTools/interface/EcalClusterTools.h"
#include "RecoEcal/EgammaCoreTools/interface/EcalClusterLazyTools.h"

// Geometry
#include "Geometry/Records/interface/CaloGeometryRecord.h"
#include "Geometry/CaloGeometry/interface/CaloGeometry.h"
#include "Geometry/CaloGeometry/interface/CaloSubdetectorGeometry.h"
#include "Geometry/CaloGeometry/interface/CaloCellGeometry.h"
#include "DataFormats/GeometryVector/interface/GlobalPoint.h"

// ROOT
#include "TTree.h"
#include "TLorentzVector.h"
#include "TString.h"
#include "TPRegexp.h"

// Common types
#include "Timing/TimingAnalyzer/plugins/CommonUtils.hh"

class HLTDump : public edm::one::EDAnalyzer<edm::one::SharedResources,edm::one::WatchRuns> 
{
 public:
  explicit HLTDump(const edm::ParameterSet&);
  ~HLTDump();

  void InitializeTriggerBranches();
  void ClearTriggerObjectBranches();
  void InitializePVBranches();
  void InitializeMETBranches();
  void ClearJetBranches();
  void InitializeJetBranches();
  void ClearRecoPhotonBranches();
  void InitializeRecoPhotonBranches();

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);
  
 private:
  virtual void beginJob() override;
  virtual void analyze(const edm::Event&, const edm::EventSetup&) override;
  virtual void endJob() override;
  
  virtual void beginRun(edm::Run const&, edm::EventSetup const&) override;
  virtual void endRun(edm::Run const&, edm::EventSetup const&) override;

  const float phpTmin;

  // dR matching criteria
  const float jetpTmin;
  const float dRmin;
  const float pTres;
  const bool  saveTrigObjs;

  // triggers
  const std::string inputPaths;
  std::vector<std::string> pathNames;
  const std::string inputFilters;
  std::vector<std::string> filterNames;
  const edm::InputTag triggerResultsTag;
  edm::EDGetTokenT<edm::TriggerResults> triggerResultsToken;
  const edm::InputTag triggerObjectsTag;
  edm::EDGetTokenT<std::vector<pat::TriggerObjectStandAlone> > triggerObjectsToken;
  std::vector<std::vector<pat::TriggerObjectStandAlone> > triggerObjectsByFilter; // first index is filter label, second is trigger objects

  // vertices
  const edm::InputTag verticesTag;
  edm::EDGetTokenT<std::vector<reco::Vertex> > verticesToken;

  // rhos
  const edm::InputTag rhosTag;
  edm::EDGetTokenT<double> rhosToken;

  // mets
  const edm::InputTag metsTag;
  edm::EDGetTokenT<std::vector<pat::MET> > metsToken;

  // jets
  const edm::InputTag jetsTag;
  edm::EDGetTokenT<std::vector<pat::Jet> > jetsToken;

  // photons
  const edm::InputTag photonsTag;
  edm::EDGetTokenT<std::vector<pat::Photon> > photonsToken;

  const edm::InputTag ootPhotonsTag;
  edm::EDGetTokenT<std::vector<pat::Photon> > ootPhotonsToken;

  // ECAL RecHits
  const edm::InputTag recHitsEBTag;
  edm::EDGetTokenT<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > recHitsEBToken;
  const edm::InputTag recHitsEETag;
  edm::EDGetTokenT<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > recHitsEEToken;

  // output event level ntuple
  TTree* tree;

  // event info
  unsigned long int event;
  unsigned int run, lumi;  

  // vertices
  int nvtx;
  float vtxX, vtxY, vtxZ;

  // trigger info
  std::vector<bool> triggerBits;

  // trigger object
  std::vector<std::vector<float> > trigobjE, trigobjeta, trigobjphi, trigobjpt;

  // MET
  float t1pfMETpt, t1pfMETphi, t1pfMETsumEt;

  // jets
  int njets;
  std::vector<float> jetE, jetpt, jetphi, jeteta;
  std::vector<bool> jetidL;

  // photon info
  int nphotons;
  std::vector<int> phisOOT;
  std::vector<float> phE, phpt, phphi, pheta;
  std::vector<float> phHOvE, phHTowOvE, phr9;
  std::vector<bool> phPixSeed, phEleVeto;
  std::vector<float> phChgIso, phNeuIso, phIso;
  std::vector<float> phPFClEcalIso, phPFClHcalIso, phHollowTkIso;
  std::vector<float> phsieie, phsipip, phsieip, phsmaj, phsmin, phalpha;
  std::vector<std::vector<int> > phIsHLTMatched; // first index is iph, second is for filter, true/false

  // supercluster info 
  std::vector<float> phscE, phsceta, phscphi;

  // seed info
  std::vector<float> phseedeta, phseedphi, phseedE, phseedtime;
  std::vector<int> phseedID;
  std::vector<int> phseedOOT;
  
  // all rec hit info
  std::vector<int> phnrh;
};

#endif
