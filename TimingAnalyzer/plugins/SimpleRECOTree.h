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
#include "DataFormats/Common/interface/ValueMap.h"
#include "DataFormats/EgammaCandidates/interface/Photon.h"
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

struct RecoPhoton
{
  reco::Photon photon;
  bool  isOOT;
  float ecalIso;
  float hcalIso;
};

inline bool sortByPhotonPt(const RecoPhoton & ph1, const RecoPhoton & ph2)
{
  return ph1.photon.pt()>ph2.photon.pt();
}

class SimpleRECOTree : public edm::one::EDAnalyzer<edm::one::SharedResources,edm::one::WatchRuns> 
{
 public:
  explicit SimpleRECOTree(const edm::ParameterSet&);
  ~SimpleRECOTree();

  void PrepPhotons(const edm::Handle<std::vector<reco::Photon> > & photonsH, std::vector<RecoPhoton> & photons,
		   const edm::ValueMap<float> & ecalIso, const edm::ValueMap<float> & hcalIso, const bool isOOT);

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

  // photons
  const edm::InputTag photonsTag;
  edm::EDGetTokenT<std::vector<reco::Photon> > photonsToken;
  const edm::InputTag ecalIsoTag;
  edm::EDGetTokenT<edm::ValueMap<float> > ecalIsoToken;
  const edm::InputTag hcalIsoTag;
  edm::EDGetTokenT<edm::ValueMap<float> > hcalIsoToken;

  // ootphotons
  const edm::InputTag ootphotonsTag;
  edm::EDGetTokenT<std::vector<reco::Photon> > ootphotonsToken;
  const edm::InputTag ootecalIsoTag;
  edm::EDGetTokenT<edm::ValueMap<float> > ootecalIsoToken;
  const edm::InputTag oothcalIsoTag;
  edm::EDGetTokenT<edm::ValueMap<float> > oothcalIsoToken;

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
  std::vector<int>   phisOOT;
  std::vector<float> phE, phpt, phphi, pheta;
  std::vector<float> phHoE, phr9, phChgIso, phNeuIso, phIso, phsuisseX;
  std::vector<float> phsieie, phsipip, phsieip, phsmaj, phsmin, phalpha;
  std::vector<float> phEcalIso, phHcalIso;

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
