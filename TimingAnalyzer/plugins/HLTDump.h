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
#include "CommonTypes.h"

inline bool file_exists(const std::string & filename){std::fstream input(filename.c_str()); return (bool)input;}

inline bool sortByTrigObjPt(const pat::TriggerObjectStandAlone & trigobj1, const pat::TriggerObjectStandAlone & trigobj2)
{
  return trigobj1.pt()>trigobj2.pt();
}

inline bool sortByJetPt(const pat::Jet & jet1, const pat::Jet & jet2)
{
  return jet1.pt()>jet2.pt();
}

inline bool sortByPhotonPt(const pat::Photon & ph1, const pat::Photon & ph2)
{
  return ph1.pt()>ph2.pt();
}

class HLTDump : public edm::one::EDAnalyzer<edm::one::SharedResources,edm::one::WatchRuns> 
{
 public:
  explicit HLTDump(const edm::ParameterSet&);
  ~HLTDump();

  void PrepTriggerObjects();
  void PrepJets(const edm::Handle<std::vector<pat::Jet> > & jetsH, std::vector<pat::Jet> & jets);
  void PrepPhotons(const edm::Handle<std::vector<pat::Photon> > & photonsH, std::vector<pat::Photon> & photons);

  void InitializeTriggerBranches();
  void ClearTriggerObjectBranches();

  void ClearJetBranches();
  void InitializeJetBranches();

  void ClearRecoPhotonBranches();
  void HLTToPATPhotonMatching(const int iph);
  void InitializeRecoPhotonBranches();

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);
  
 private:
  virtual void beginJob() override;
  virtual void analyze(const edm::Event&, const edm::EventSetup&) override;
  virtual void endJob() override;
  
  virtual void beginRun(edm::Run const&, edm::EventSetup const&) override;
  virtual void endRun(edm::Run const&, edm::EventSetup const&) override;

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

  // rhos
  const edm::InputTag rhosTag;
  edm::EDGetTokenT<double> rhosToken;

  // jets
  const edm::InputTag jetsTag;
  edm::EDGetTokenT<std::vector<pat::Jet> > jetsToken;

  // photons
  const edm::InputTag photonsTag;
  edm::EDGetTokenT<std::vector<pat::Photon> > photonsToken;

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

  // trigger info
  std::vector<bool> triggerBits;

  // trigger object
  std::vector<std::vector<float> > trigobjE, trigobjeta, trigobjphi, trigobjpt;

  // jets
  int njets;
  std::vector<float> jetE, jetpt, jetphi, jeteta;
  std::vector<bool> jetidL;

  // photon info
  int nphotons;
  std::vector<float> phE, phpt, phphi, pheta;
  std::vector<float> phHoE, phr9;
  std::vector<bool> phPixSeed, phEleVeto;
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
