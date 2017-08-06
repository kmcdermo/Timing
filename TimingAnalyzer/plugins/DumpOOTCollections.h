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
#include "DataFormats/EgammaCandidates/interface/Photon.h"
#include "DataFormats/EgammaReco/interface/SuperCluster.h"
#include "DataFormats/CaloRecHit/interface/CaloCluster.h"
#include "DataFormats/DetId/interface/DetId.h"
#include "DataFormats/EcalDetId/interface/EBDetId.h"
#include "DataFormats/EcalDetId/interface/EEDetId.h"
#include "DataFormats/EcalDetId/interface/EcalSubdetector.h"
#include "DataFormats/EcalRecHit/interface/EcalRecHit.h"
#include "DataFormats/EcalRecHit/interface/EcalRecHitCollections.h"

// ROOT
#include "TTree.h"
#include "TLorentzVector.h"
#include "TString.h"
#include "TPRegexp.h"

// Common types
#include "CommonTypes.h"

class DumpOOTCollections : public edm::one::EDAnalyzer<edm::one::SharedResources,edm::one::WatchRuns> 
{
 public:
  explicit DumpOOTCollections(const edm::ParameterSet&);
  ~DumpOOTCollections();

  int GetNRecHits(const DetIdPairVec & hitsAndFractions, const EcalRecHitCollection *& recHits);

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);
  
 private:
  virtual void beginJob() override;
  virtual void analyze(const edm::Event&, const edm::EventSetup&) override;
  virtual void endJob() override;
  
  virtual void beginRun(edm::Run const&, edm::EventSetup const&) override;
  virtual void endRun(edm::Run const&, edm::EventSetup const&) override;

  // photons
  const edm::InputTag photonsTag;
  edm::EDGetTokenT<std::vector<reco::Photon> > photonsToken;

  // photonCores
  const edm::InputTag photonCoresTag;
  edm::EDGetTokenT<std::vector<reco::PhotonCore> > photonCoresToken;

  // superClusters
  const edm::InputTag superClustersEBTag;
  edm::EDGetTokenT<std::vector<reco::SuperCluster> > superClustersEBToken;
  const edm::InputTag superClustersEETag;
  edm::EDGetTokenT<std::vector<reco::SuperCluster> > superClustersEEToken;

  // clusters
  const edm::InputTag clustersEBTag;
  edm::EDGetTokenT<std::vector<reco::CaloCluster> > clustersEBToken;
  const edm::InputTag clustersEETag;
  edm::EDGetTokenT<std::vector<reco::CaloCluster> > clustersEEToken;
  const edm::InputTag clustersESTag;
  edm::EDGetTokenT<std::vector<reco::CaloCluster> > clustersESToken;

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

  int nPhotons, nPhotonCores, nSuperClustersEB, nSuperClustersEE, nClustersEB, nClustersEE, nClustersES;
  std::vector<int> nPhClusters, nPhRecHits;
};
