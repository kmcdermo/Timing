// basic C++ headers
#include <iostream>
#include <string>
#include <vector>
#include <utility>
#include <map>
#include <cmath>

// Common types
#include "CommonTypes.h"

// FWCore
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h" 

// HLT info
#include "HLTrigger/HLTcore/interface/HLTConfigProvider.h"
#include "DataFormats/Common/interface/TriggerResults.h"

// Gen Info
#include "SimDataFormats/PileupSummaryInfo/interface/PileupSummaryInfo.h"
#include "SimDataFormats/GeneratorProducts/interface/GenEventInfoProduct.h"

// DataFormats
#include "DataFormats/Common/interface/ValueMap.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/PatCandidates/interface/Photon.h"
#include "DataFormats/PatCandidates/interface/MET.h"
#include "DataFormats/PatCandidates/interface/Jet.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"

// DetIds and Ecal stuff
#include "DataFormats/EcalRecHit/interface/EcalRecHitCollections.h"
#include "DataFormats/EcalRecHit/interface/EcalRecHit.h"
#include "DataFormats/DetId/interface/DetId.h"
#include "DataFormats/EcalDetId/interface/EBDetId.h"
#include "DataFormats/EcalDetId/interface/EEDetId.h"
#include "DataFormats/EcalDetId/interface/EcalSubdetector.h"
#include "DataFormats/GeometryVector/interface/GlobalPoint.h"

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

class OOTRecHits_mAD : public edm::one::EDAnalyzer<edm::one::SharedResources,edm::one::WatchRuns> 
{
public:
  explicit OOTRecHits_mAD(const edm::ParameterSet&);
  ~OOTRecHits_mAD();
  
  void InitializePhotonBranches();
  void InitializeRecHitBranches();

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);
  
private:
  virtual void beginJob() override;
  virtual void analyze(const edm::Event&, const edm::EventSetup&) override;
  virtual void endJob() override;
  
  virtual void beginRun(edm::Run const&, edm::EventSetup const&) override;
  virtual void endRun(edm::Run const&, edm::EventSetup const&) override;
  
  // delta R cuts
  const bool addrhsInDelR;
  const double delRcut;

  // Trigger
  const edm::InputTag triggerResultsTag;
  edm::EDGetTokenT<edm::TriggerResults> triggerResultsToken;
  std::vector<std::string>   triggerPathsVector;
  std::map<std::string, int> triggerPathsMap;

  // Photons + IDS
  const edm::InputTag photonLooseIdMapTag;
  edm::EDGetTokenT<edm::ValueMap<bool> > photonLooseIdMapToken;
  const edm::InputTag photonMediumIdMapTag;
  edm::EDGetTokenT<edm::ValueMap<bool> > photonMediumIdMapToken;
  const edm::InputTag photonTightIdMapTag;
  edm::EDGetTokenT<edm::ValueMap<bool> > photonTightIdMapToken;

  const edm::InputTag photonsTag;
  edm::EDGetTokenT<std::vector<pat::Photon> > photonsToken;

  // ECAL RecHits
  edm::EDGetTokenT<EcalRecHitCollection> recHitsReducedEBTAG;
  edm::EDGetTokenT<EcalRecHitCollection> recHitsReducedEETAG;

  // event info
  int event, run, lumi;  
  bool hltdoubleph60;

  // photon ntuple
  // phrhtree;
  TTree * phrhtree;
  float phE, phpt, phphi, pheta;
  float phscE, phscphi, phsceta;
  int   phVID;

  // reduced hits
  int phnrhs, phnrhs_add;
  std::vector<float> phrhEs, phrhphis, phrhetas, phrhdelRs, phrhtimes;
  std::vector<int>   phrhIDs, phrhOOTs;
  int phseedpos;
};
