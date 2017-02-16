// basic C++ headers
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <utility>
#include <map>
#include <unordered_map>
#include <cmath>

// FWCore
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h" 

// Photons + Clusters
#include "DataFormats/EgammaCandidates/interface/Photon.h"
#include "DataFormats/EgammaReco/interface/SuperCluster.h"
#include "DataFormats/CaloRecHit/interface/CaloCluster.h"

// DetIds and Ecal stuff
#include "DataFormats/EcalRecHit/interface/EcalRecHitCollections.h"
#include "DataFormats/EcalRecHit/interface/EcalRecHit.h"
#include "DataFormats/DetId/interface/DetId.h"
#include "DataFormats/EcalDetId/interface/EBDetId.h"
#include "DataFormats/EcalDetId/interface/EEDetId.h"
#include "DataFormats/EcalDetId/interface/EcalSubdetector.h"

// Geometry
#include "Geometry/Records/interface/CaloGeometryRecord.h"
#include "Geometry/CaloGeometry/interface/CaloGeometry.h"
#include "Geometry/CaloGeometry/interface/CaloSubdetectorGeometry.h"
#include "Geometry/CaloGeometry/interface/CaloCellGeometry.h"

// ROOT
#include "TTree.h"
#include "TLorentzVector.h"
#include "TPRegexp.h"

struct RHInfo
{
  RHInfo(){}
  RHInfo(int rawid, bool oot, float energy, float time) : rawid_(rawid), oot_(oot), energy_(energy), time_(time) {}
  int   rawid_;
  bool  oot_;
  float energy_;
  float time_;
};

class AODRecHitsCheck : public edm::one::EDAnalyzer<edm::one::SharedResources,edm::one::WatchRuns> 
{
public:
  explicit AODRecHitsCheck(const edm::ParameterSet&);
  ~AODRecHitsCheck();
  
  void InitializeBranches();

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
  const edm::InputTag gedPhotonsTag;
  edm::EDGetTokenT<std::vector<reco::Photon> > gedPhotonsToken;

  // ECAL RecHits
  const edm::InputTag recHitsEBTag;
  edm::EDGetTokenT<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > recHitsEBToken;
  const edm::InputTag recHitsEETag;
  edm::EDGetTokenT<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > recHitsEEToken;

  // input seeds
  std::unordered_map<int,RHInfo>               evseedmap;
  std::unordered_map<int,std::vector<RHInfo> > evrhvecmap;

  // tree
  TTree * tree;

  int   event;
  bool  isEvent;

  int   seedID;
  bool  isEB;
  bool  isPhoton;
  bool  isGedPhoton;
  bool  isCluster;
  float seedE;
  float seedtime;
  bool  seedOOT;
  bool  isSeed;

  int   nRHs;
  std::vector<int>   rhIDs;
  std::vector<float> rhEs;
  std::vector<float> rhtimes;
  std::vector<bool>  rhOOTs;
  std::vector<bool>  isRHs;
};
