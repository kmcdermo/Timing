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

typedef std::vector<std::pair<DetId,float> > DetIdPairVec;
typedef std::unordered_map<uint32_t,int> uiiumap;

inline float rad2  (const float x, const float y){return x*x + y*y;}
inline float phi   (const float x, const float y){return std::atan2(y,x);}
inline float theta (const float r, const float z){return std::atan2(r,z);}
inline float eta   (const float x, const float y, const float z)
{
  return -1.0f*std::log(std::tan(theta(std::sqrt(rad2(x,y)),z)/2.f));
}
inline float deltaR(const float phi1, const float eta1, const float phi2, const float eta2)
{
  return std::sqrt(rad2(eta2-eta1,phi2-phi1));
}

class PhotonDump : public edm::one::EDAnalyzer<edm::one::SharedResources,edm::one::WatchRuns> 
{
 public:
  explicit PhotonDump(const edm::ParameterSet&);
  ~PhotonDump();

  void InitializeGenEvtBranches();

  void InitializeGenPUBranches();

  void DumpGenIds(const edm::Handle<std::vector<reco::GenParticle> > &);
  void InitializeGenParticleBranches();

  void InitializePVBranches();

  void InitializeMETBranches();

  void ClearGenJetBranches();
  void InitializeGenJetBranches();

  void ClearJetBranches();
  void InitializeJetBranches();

  void ClearRecoPhotonBranches();
  void InitializeRecoPhotonBranches();
  void InitializeRecoRecHitBranches(int iph);
  void DumpRecHitInfo(int, const DetIdPairVec &, const EcalRecHitCollection *&);

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
  const bool dumpRHs;
  edm::EDGetTokenT<EcalRecHitCollection> recHitCollectionEBTAG;
  edm::EDGetTokenT<EcalRecHitCollection> recHitCollectionEETAG;

  // Gen Particles and MC info
  const bool isMC;
  const bool dumpIds;
  edm::EDGetTokenT<GenEventInfoProduct>             genevtInfoToken;
  edm::EDGetTokenT<std::vector<PileupSummaryInfo> > pileupInfoToken;
  edm::EDGetTokenT<std::vector<reco::GenParticle> > genpartsToken;
  edm::EDGetTokenT<std::vector<reco::GenJet> >      genjetsToken;

  // output event level ntuple
  TTree* tree;

  // event info
  int event, run, lumi;  

  // Generator level info
  float genwgt;
  int genpuobs, genputrue;

  // Gen particle info
  int nNeutralino, nNeutoPhGr;

  float genN1mass, genN1E, genN1pt, genN1phi, genN1eta;
  float genph1E, genph1pt, genph1phi, genph1eta;
  int genph1match;
  float gengr1E, gengr1pt, gengr1phi, gengr1eta;

  float genN2mass, genN2E, genN2pt, genN2phi, genN2eta;
  float genph2E, genph2pt, genph2phi, genph2eta;
  int genph2match;
  float gengr2E, gengr2pt, gengr2phi, gengr2eta;

  // gen jets
  int ngenjets;
  std::vector<int> genjetmatch;
  std::vector<float> genjetE, genjetpt, genjetphi, genjeteta;

  // vertices
  int nvtx;
  float vtxX, vtxY, vtxZ;

  // MET
  float t1pfMETpt,       t1pfMETphi,       t1pfMETsumEt;
  float t1pfMETuncorpt,  t1pfMETuncorphi,  t1pfMETuncorsumEt;
  float t1pfMETcalopt,   t1pfMETcalophi,   t1pfMETcalosumEt;
  float t1pfMETgenMETpt, t1pfMETgenMETphi, t1pfMETgenMETsumEt;

  // jets
  int njets;
  std::vector<int> jetmatch;
  std::vector<float> jetE, jetpt, jetphi, jeteta;

  // photon info
  int nphotons;
  std::vector<int> phmatch, phVID;
  std::vector<float> phE, phpt, phphi, pheta;

  // supercluster info 
  std::vector<float> phscX, phscY, phscZ, phscE;

  // seed info
  std::vector<int> phseedpos;
  
  // all rec hit info
  std::vector<int> phnrhs;
  std::vector<std::vector<float> > phrhXs, phrhYs, phrhZs, phrhEs, phrhdelRs, phrhtimes;
  std::vector<std::vector<int> > phrhIDs;
  std::vector<std::vector<int> > phrhOOTs;
};
