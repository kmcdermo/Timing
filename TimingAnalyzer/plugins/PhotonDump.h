// basic C++ headers
#include <iostream>
#include <memory>
#include <vector>
#include <utility>
#include <map>
#include <unordered_map>
#include <string>
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
#include "DataFormats/DetId/interface/DetId.h"
#include "DataFormats/EcalDetId/interface/EBDetId.h"
#include "DataFormats/EcalDetId/interface/EEDetId.h"
#include "DataFormats/EcalDetId/interface/EcalSubdetector.h"

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

typedef std::vector<std::pair<DetId,float> > DetIdPairVec;
typedef std::unordered_map<uint32_t,int> uiiumap;

inline bool sortJetsByPt (const pat::Jet& jet1, const pat::Jet& jet2)  
{
  return jet1.pt()>jet2.pt();
}

inline bool sortPhotonsByPt (const pat::Photon& ph1, const pat::Photon& ph2)  
{
  return ph1.pt()>ph2.pt();
}

class PhotonDump : public edm::one::EDAnalyzer<edm::one::SharedResources,edm::one::WatchRuns> 
{
public:
  explicit PhotonDump(const edm::ParameterSet&);
  ~PhotonDump();

  void ClearJetBranches();
  void InitializeJetBranches();

  void ClearRecoPhotonBranches();
  void InitializeRecoPhotonBranches();
  void InitializeRecoRecHitBranches(int iph);
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

  // mets
  const edm::InputTag metsTag;
  edm::EDGetTokenT<std::vector<pat::MET> > metsToken;

  // jets
  const edm::InputTag jetsTag;
  edm::EDGetTokenT<std::vector<pat::Jet> > jetsToken;

  // photons + ids
  const edm::InputTag photonLooseIdMapTag;
  edm::EDGetTokenT<edm::ValueMap<bool> > photonLooseIdMapToken;
  const edm::InputTag photonMediumIdMapTag;
  edm::EDGetTokenT<edm::ValueMap<bool> > photonMediumIdMapToken;
  const edm::InputTag photonTightIdMapTag;
  edm::EDGetTokenT<edm::ValueMap<bool> > photonTightIdMapToken;

  const edm::InputTag photonsTag;
  edm::EDGetTokenT<std::vector<pat::Photon> > photonsToken;

  // ECAL RecHits
  edm::EDGetTokenT<EcalRecHitCollection> recHitCollectionEBTAG;
  edm::EDGetTokenT<EcalRecHitCollection> recHitCollectionEETAG;

  // Gen Particles and MC info
  const bool isMC;
  edm::EDGetTokenT<GenEventInfoProduct>             genevtInfoToken;
  edm::EDGetTokenT<std::vector<PileupSummaryInfo> > pileupInfoToken;
  edm::EDGetTokenT<std::vector<reco::GenParticle> > genpartsToken;
  edm::EDGetTokenT<std::vector<reco::GenJet> >      genjetsToken;

  // output event level ntuple
  TTree* tree;

  // event info
  int event, run, lumi;  

  // MC Info
  float genwgt;
  int genpuobs, genputrue;

  // object counts
  int nvtx, njets, nphotons;

  // vertices
  float vtxX, vtxY, vtxZ;

  // MET
  float t1pfmet, t1pfmetphi, t1pfmeteta, t1pfmetsumEt;

  // jets
  std::vector<float> jetE, jetpt, jetphi, jeteta;

  // photon info
  std::vector<float> phE, phpt, phphi, pheta;

  // supercluster info 
  std::vector<float> phscX, phscY, phscZ, phscE;

  // seed info
  std::vector<int> phseedpos;
  
  // all rec hit info
  std::vector<int> phnrhs;
  std::vector<std::vector<float> > phrhXs, phrhYs, phrhZs, phrhEs, phrhtimes;
  std::vector<std::vector<int> > phrhIDs;
  std::vector<std::vector<int> > phrhOOTs;
};
