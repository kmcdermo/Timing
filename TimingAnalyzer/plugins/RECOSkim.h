// basic C++ headers
#include <iostream>
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
#include "DataFormats/EcalRecHit/interface/EcalRecHitCollections.h"
#include "DataFormats/EcalRecHit/interface/EcalRecHit.h"
#include "DataFormats/Common/interface/ValueMap.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/EgammaCandidates/interface/Photon.h"
#include "DataFormats/PatCandidates/interface/MET.h"
#include "DataFormats/JetReco/interface/PFJet.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"

// DetIds and Ecal stuff
#include "DataFormats/DetId/interface/DetId.h"
#include "DataFormats/EcalDetId/interface/EBDetId.h"
#include "DataFormats/EcalDetId/interface/EEDetId.h"
#include "DataFormats/EcalDetId/interface/EcalSubdetector.h"
#include "DataFormats/GeometryVector/interface/GlobalPoint.h"

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

// ROOT
#include "TTree.h"
#include "TLorentzVector.h"
#include "TPRegexp.h"

// Common types
#include "CommonTypes.h"

class RECOSkim : public edm::one::EDAnalyzer<edm::one::SharedResources,edm::one::WatchRuns> 
{
 public:
  explicit RECOSkim(const edm::ParameterSet&);
  ~RECOSkim();

  float GetChargedHadronEA(const float eta);
  float GetNeutralHadronEA(const float eta);
  float GetGammaEA        (const float eta);

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
  
  // Vertex
  const edm::InputTag verticesTag;
  edm::EDGetTokenT<std::vector<reco::Vertex> > verticesToken;

  // rhos
  const edm::InputTag rhosTag;
  edm::EDGetTokenT<double> rhosToken;

  // mets
  const edm::InputTag metsTag;
  edm::EDGetTokenT<std::vector<reco::PFMET> > metsToken;

  // jets
  const edm::InputTag jetsTag;
  edm::EDGetTokenT<std::vector<reco::PFJet> > jetsToken;

  // photons
  const edm::InputTag photonsTag;
  edm::EDGetTokenT<std::vector<reco::Photon> > photonsToken;

  // ECAL RecHits
  edm::EDGetTokenT<EcalRecHitCollection> recHitCollectionEBToken;
  edm::EDGetTokenT<EcalRecHitCollection> recHitCollectionEEToken;

  // output event level ntuple
  TTree* tree;

  // event info
  int event, run, lumi;  

  // vertices
  int nvtx;
  float vtxX, vtxY, vtxZ;

  // MET
  float t1pfMETpt, t1pfMETphi, t1pfMETsumEt;

  // jets
  int njets;
  std::vector<float> jetE, jetpt, jetphi, jeteta;

  // photon info
  int nphotons;

  // supercluster info 
  std::vector<float> phscE;

  // photon info + ID-like variables
  std::vector<float> phE, phpt, phphi, pheta; 
  std::vector<float> phHoE, phsieie, phr9, phChgIso, phNeuIso, phIso;
  std::vector<float> phsuisseX, phsmaj, phsmin;

  // all rec hit info
  std::vector<int> phnrh, phnrhEcut, phnrhOOT;
  std::vector<float> phseedphi, phseedeta, phseedE, phseedtime;
  std::vector<int> phseedOOT;
};
