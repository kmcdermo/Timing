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

inline bool sortByJetPt(const reco::PFJet & jet1, const reco::PFJet & jet2)
{
  return jet1.pt()>jet2.pt();
}

inline bool sortByPhotonPt(const reco::Photon & ph1, const reco::Photon & ph2)
{
  return ph1.pt()>ph2.pt();
}

class RECOSkim : public edm::one::EDAnalyzer<edm::one::SharedResources,edm::one::WatchRuns> 
{
 public:
  explicit RECOSkim(const edm::ParameterSet&);
  ~RECOSkim();

  void PrepJets(const edm::Handle<std::vector<reco::PFJet> > & jetsH, std::vector<reco::PFJet> & jets);
  void PrepPhotons(const edm::Handle<std::vector<reco::Photon> > & photonsH, std::vector<reco::Photon> & photons);

  float GetChargedHadronEA(const float eta);
  float GetNeutralHadronEA(const float eta);
  float GetGammaEA        (const float eta);

  int PassHoE   (const float eta, const float HoE);
  int PassSieie (const float eta, const float Sieie);
  int PassChgIso(const float eta, const float ChgIso);
  int PassNeuIso(const float eta, const float NeuIso, const float pt);
  int PassPhIso (const float eta, const float PhIso , const float pt);

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

  // photon info
  std::vector<float> phE, phpt, phphi, pheta; 

  // supercluster info 
  std::vector<float> phscE, phscphi, phsceta;

  // cluster shape info
  std::vector<float> phsieie, phsipip, phsieip, phsmaj, phsmin;

  // ID-like variables
  std::vector<float> phHoE, phChgIso, phNeuIso, phPhIso;
  std::vector<float> phr9, phsuisseX;
  std::vector<int> phHoE_b, phsieie_b, phChgIso_b, phNeuIso_b, phPhIso_b;

  // all rec hit info
  std::vector<int> phnrh, phnrhEcut, phnrhOOT;
  std::vector<float> phseedE, phseedphi, phseedeta, phseedtime;
  std::vector<int> phseedOOT;
};
