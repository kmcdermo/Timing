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

inline bool sortByPhotonPt(const pat::Photon & ph1, const pat::Photon & ph2)
{
  return ph1.pt()>ph2.pt();
}

class SimplePATTree : public edm::one::EDAnalyzer<edm::one::SharedResources,edm::one::WatchRuns> 
{
 public:
  explicit SimplePATTree(const edm::ParameterSet&);
  ~SimplePATTree();

  void PrepPhotons(const edm::Handle<std::vector<pat::Photon> > & photonsH, std::vector<pat::Photon> & photons);

  float GetChargedHadronEA(const float);
  float GetNeutralHadronEA(const float);
  float GetGammaEA        (const float);

  void ClearRecoPhotonBranches();
  void InitializeRecoPhotonBranches();
  void InitializeRecoRecHitBranches(const int iph);

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

  // photons + ids
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

  // photon info
  int nphotons;
  std::vector<float> phE, phpt, phphi, pheta;
  std::vector<int>   phmatch;
  std::vector<float> phHoE, phr9, phChgIso, phNeuIso, phIso, phsuisseX;
  std::vector<float> phsieie, phsipip, phsieip, phsmaj, phsmin, phalpha;
  std::vector<float> phPFClEcalIso, phPFClHcalIso;
  std::vector<float> phHollowTkIso;

  // supercluster info 
  std::vector<float> phscE, phsceta, phscphi;

  // seed info
  std::vector<int> phseedpos;
  
  // all rec hit info
  std::vector<int> phnrh;
  std::vector<std::vector<float> > phrheta, phrhphi, phrhE, phrhtime;
  std::vector<std::vector<int> > phrhID;
  std::vector<std::vector<int> > phrhOOT;
};
