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

// Gen Info
#include "SimDataFormats/PileupSummaryInfo/interface/PileupSummaryInfo.h"
#include "SimDataFormats/GeneratorProducts/interface/GenEventInfoProduct.h"

// DataFormats
#include "DataFormats/EcalRecHit/interface/EcalRecHitCollections.h"
#include "DataFormats/EcalRecHit/interface/EcalRecHit.h"
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

inline bool sortByPhotonPt(const pat::Photon & ph1, const pat::Photon & ph2)
{
  return ph1.pt()>ph2.pt();
}

class PhotonDump : public edm::one::EDAnalyzer<edm::one::SharedResources,edm::one::WatchRuns> 
{
 public:
  explicit PhotonDump(const edm::ParameterSet&);
  ~PhotonDump();

  void PrepPhotons(const edm::Handle<std::vector<pat::Photon> > & photonsH, 
		   const edm::ValueMap<bool> & photonLooseIdMap, 
		   const edm::ValueMap<bool> & photonMediumIdMap, 
		   const edm::ValueMap<bool> & photonTightIdMap, 
		   std::vector<pat::Photon> & photons);

  bool PhotonMatching(const pat::Photon & photon, const edm::Handle<std::vector<reco::GenParticle> > & genparticlesH);

  float GetChargedHadronEA(const float);
  float GetNeutralHadronEA(const float);
  float GetGammaEA        (const float);

  int PassHoE   (const float eta, const float HoE);
  int PassSieie (const float eta, const float Sieie);
  int PassChgIso(const float eta, const float ChgIso);
  int PassNeuIso(const float eta, const float NeuIso, const float pt);
  int PassPhIso (const float eta, const float PhIso,  const float pt);

  void InitializeGenEvtBranches();

  void InitializeGenPUBranches();

  void DumpGenIds(const edm::Handle<std::vector<reco::GenParticle> > &);
  void InitializeGMSBBranches();

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

  // rhos
  const edm::InputTag rhosTag;
  edm::EDGetTokenT<double> rhosToken;

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
  const bool isGMSB;
  const bool isBkg;
  const bool dumpIds;
  bool isMC;
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
  float genN1prodvx, genN1prodvy, genN1prodvz;
  float genN1decayvx, genN1decayvy, genN1decayvz;
  float genph1E, genph1pt, genph1phi, genph1eta;
  int genph1match;
  float gengr1mass, gengr1E, gengr1pt, gengr1phi, gengr1eta;

  float genN2mass, genN2E, genN2pt, genN2phi, genN2eta;
  float genN2prodvx, genN2prodvy, genN2prodvz;
  float genN2decayvx, genN2decayvy, genN2decayvz;
  float genph2E, genph2pt, genph2phi, genph2eta;
  int genph2match;
  float gengr2mass, gengr2E, gengr2pt, gengr2phi, gengr2eta;

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
  std::vector<float> phE, phpt, phphi, pheta;
  std::vector<int>   phmatch;
  std::vector<bool>  phisMatched;
  std::vector<float> phHoE, phr9, phChgIso, phNeuIso, phIso, phsuisseX;
  std::vector<float> phsieie, phsipip, phsieip, phsmaj, phsmin;
  std::vector<int>   phVID, phHoE_b, phsieie_b, phChgIso_b, phNeuIso_b, phIso_b;

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
