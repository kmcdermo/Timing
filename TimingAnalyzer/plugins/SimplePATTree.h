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

// DataFormats
#include "DataFormats/PatCandidates/interface/Photon.h"
#include "DataFormats/EgammaReco/interface/SuperCluster.h"
#include "DataFormats/CaloRecHit/interface/CaloCluster.h"
#include "DataFormats/DetId/interface/DetId.h"
#include "DataFormats/EcalDetId/interface/EBDetId.h"
#include "DataFormats/EcalDetId/interface/EEDetId.h"
#include "DataFormats/EcalDetId/interface/EcalSubdetector.h"
#include "DataFormats/EcalRecHit/interface/EcalRecHit.h"
#include "DataFormats/EcalRecHit/interface/EcalRecHitCollections.h"

// EGamma Tools
#include "RecoEcal/EgammaCoreTools/interface/EcalTools.h"
#include "RecoEcal/EgammaCoreTools/interface/EcalClusterTools.h"

// Geometry
#include "Geometry/CaloTopology/interface/CaloTopology.h"
#include "Geometry/Records/interface/CaloTopologyRecord.h"
#include "Geometry/CaloGeometry/interface/CaloGeometry.h"
#include "Geometry/Records/interface/CaloGeometryRecord.h"
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

struct PatPhoton
{
  pat::Photon photon;
  bool isOOT_;
};

inline bool sortByPhotonPt(const PatPhoton & ph1, const PatPhoton & ph2)
{
  return ph1.photon.pt()>ph2.photon.pt();
}

class SimplePATTree : public edm::one::EDAnalyzer<edm::one::SharedResources,edm::one::WatchRuns> 
{
 public:
  explicit SimplePATTree(const edm::ParameterSet&);
  ~SimplePATTree();

  void PrepPhotons(const edm::Handle<std::vector<pat::Photon> > & photonsH, std::vector<PatPhoton> & photons, const bool isOOT);

  void ClearRecoPhotonBranches();
  void InitializeRecoPhotonBranches();

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);
  
 private:
  virtual void beginJob() override;
  virtual void analyze(const edm::Event&, const edm::EventSetup&) override;
  virtual void endJob() override;
  
  virtual void beginRun(edm::Run const&, edm::EventSetup const&) override;
  virtual void endRun(edm::Run const&, edm::EventSetup const&) override;

  // rhos
  const edm::InputTag rhosTag;
  edm::EDGetTokenT<double> rhosToken;

  // photons
  const edm::InputTag photonsTag;
  edm::EDGetTokenT<std::vector<pat::Photon> > photonsToken;
  const edm::InputTag ootphotonsTag;
  edm::EDGetTokenT<std::vector<pat::Photon> > ootphotonsToken;

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

  // photon info
  int nphotons;
  std::vector<int>   phIsOOT, phIsEB;
  std::vector<float> phE, phpt, phphi, pheta;
  std::vector<float> phHoE, phr9, phsieie;
  std::vector<float> phsmaj, phsmin, phalpha;
  std::vector<float> phPFClEcalIso, phPFClHcalIso, phHollowTkIso;

  // all rec hit info
  std::vector<int>   phnrh;
  std::vector<float> phseedE, phseedtime;
  std::vector<int>   phseedOOT;
};
