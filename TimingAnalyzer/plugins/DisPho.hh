#ifndef __DisPho__
#define __DisPho__

// FWCore
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h" 

// HLT + Trigger info
#include "FWCore/Common/interface/TriggerNames.h"
#include "DataFormats/Common/interface/TriggerResults.h"
#include "DataFormats/HLTReco/interface/TriggerObject.h"

// Gen Info
#include "SimDataFormats/PileupSummaryInfo/interface/PileupSummaryInfo.h"
#include "SimDataFormats/GeneratorProducts/interface/GenEventInfoProduct.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"

// DataFormats
#include "DataFormats/Common/interface/ValueMap.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/PatCandidates/interface/Photon.h"
#include "DataFormats/PatCandidates/interface/MET.h"
#include "DataFormats/PatCandidates/interface/Jet.h"

// DetIds 
#include "DataFormats/DetId/interface/DetId.h"
#include "DataFormats/EcalDetId/interface/EBDetId.h"
#include "DataFormats/EcalDetId/interface/EEDetId.h"
#include "DataFormats/EcalDetId/interface/EcalSubdetector.h"

// Ecal RecHits
#include "DataFormats/EcalRecHit/interface/EcalRecHit.h"
#include "DataFormats/EcalRecHit/interface/EcalRecHitCollections.h"

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
#include "DataFormats/GeometryVector/interface/GlobalPoint.h"

// ROOT
#include "TTree.h"
#include "TLorentzVector.h"
#include "TString.h"
#include "TPRegexp.h"

// Common Utilities
#include "Timing/TimingAnalyzer/plugins/CommonUtils.hh"

// Unique structs
struct gmsbStruct
{
  float genNmass_;
  float genNE_;
  float genNpt_;
  float genNphi_;
  float genNeta_;

  float genNprodvx_;
  float genNprodvy_;
  float genNprodvz_;

  float genNdecayvx_;
  float genNdecayvy_;
  float genNdecayvz_;
  
  float genphE_;
  float genphpt_;
  float genphphi_;
  float genpheta_;
  float genphmatch_;

  float gengrmass_;
  float gengrE_;
  float gengrpt_;
  float gengrphi_;
  float gengreta_;
};

struct jetStruct
{
  float E_;
  float Pt_;
  float Phi_;
  float Eta_;
};

struct phoStruct
{
  float E_;
  float Pt_;
  float Phi_;
  float Eta_;

  float scE_;
  float scPhi_;
  float scEta_;

  float HoE_;
  float r9_;
  float ChgHadIso_;
  float NeuHadIso_;
  float PhoIso_;

  float EcalPFClIso_;
  float HcalPFClIso_;
  float TrkIso_;

  float Sieie_; 
  float Sipip_;
  float Sieip_;

  float Smaj_;
  float Smin_;
  float alpha_;

  int seed_;
  std::vector<int> recHits_;

  bool isOOT_;
  bool isEB_;
  bool isHLT_;
  int  ID_;
};

class DisPho : public edm::one::EDAnalyzer<edm::one::SharedResources,edm::one::WatchRuns> 
{
 public:
  explicit DisPho(const edm::ParameterSet&);
  ~DisPho();

  void MakeGMSBBranch(const int i, gmsbStruct& gmsbBranch);
  void MakeJetBranch(const int i, jetStruct& jetBranch);
  void MakePhoBranch(const int i, phoStruct& phoBranch);

  void InitializeGenEvtBranches();
  void InitializeGenPUBranches();
  void InitializeGMSBBranches();
  void InitializeGMSBBranch(gmsbStruct& gmsbBranch);
  void SetGMSBBranch(const reco::GenParticle & neutralino, gmsbStruct & gmsbBranch, const std::vector<oot::Photon> & photons);
  void SetTriggerBits(edm::Handle<edm::TriggerResults> & triggerResultsH, const edm::Event & iEvent);
  void InitializePVBranches();
  void InitializeMETBranches();
  void InitializeJetBranches();
  void InitializeJetBranch(jetStruct & jetBranch);
  void SetJetBranch(const pat::Jet & jet, jetStruct & jetBranch);
  void InitializeRecHitBranches();
  void SetRecHitBranches(const EcalRecHitCollection * recHits, const CaloSubdetectorGeometry * geometry,
			 const uiiumap& recHitMap);
  void InitializePhoBranches();
  void InitializePhoBranch(phoStruct & phoBranch);
  void SetPhoBranch(const oot::Photon& photon, phoStruct & phoBranch, const uiiumap & recHitMap,
		    const EcalRecHitCollection * recHitsEB, const EcalRecHitCollection * recHitsEE);

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);
  
 private:
  virtual void beginJob() override;
  virtual void analyze(const edm::Event&, const edm::EventSetup&) override;
  virtual void endJob() override;
  
  virtual void beginRun(edm::Run const&, edm::EventSetup const&) override;
  virtual void endRun(edm::Run const&, edm::EventSetup const&) override;
  
  // object prep
  const float jetpTmin;
  const int jetIDmin;
  const float rhEmin;
  const float phpTmin;
  const std::string phIDmin;

  // pre-selection vars
  const bool applyTrigger;
  const float minHT;
  const bool applyHT;
  const float ph1pTmin;
  const std::string ph1IDmin;
  const bool applyPh1;

  // dR matching criteria
  const float dRmin;
  const float pTres;

  // triggers
  const std::string inputPaths;
  std::vector<std::string> pathNames;
  strBitMap triggerBitMap;
  const std::string inputFilters;
  std::vector<std::string> filterNames;
  const edm::InputTag triggerResultsTag;
  edm::EDGetTokenT<edm::TriggerResults> triggerResultsToken;
  const edm::InputTag triggerObjectsTag;
  edm::EDGetTokenT<std::vector<pat::TriggerObjectStandAlone> > triggerObjectsToken;
  trigObjVecMap triggerObjectsByFilterMap; // first index is filter label, second is trigger objects

  // vertices
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
  const edm::InputTag photonsTag;
  edm::EDGetTokenT<std::vector<pat::Photon> > photonsToken;
  const edm::InputTag photonLooseIdMapTag;
  edm::EDGetTokenT<edm::ValueMap<bool> > photonLooseIdMapToken;
  const edm::InputTag photonMediumIdMapTag;
  edm::EDGetTokenT<edm::ValueMap<bool> > photonMediumIdMapToken;
  const edm::InputTag photonTightIdMapTag;
  edm::EDGetTokenT<edm::ValueMap<bool> > photonTightIdMapToken;

  // ootPhotons + ids
  const edm::InputTag ootPhotonsTag;
  edm::EDGetTokenT<std::vector<pat::Photon> > ootPhotonsToken;
  const edm::InputTag ootPhotonLooseIdMapTag;
  edm::EDGetTokenT<edm::ValueMap<bool> > ootPhotonLooseIdMapToken;
  const edm::InputTag ootPhotonMediumIdMapTag;
  edm::EDGetTokenT<edm::ValueMap<bool> > ootPhotonMediumIdMapToken;
  const edm::InputTag ootPhotonTightIdMapTag;
  edm::EDGetTokenT<edm::ValueMap<bool> > ootPhotonTightIdMapToken;

  // ECAL RecHits
  const edm::InputTag recHitsEBTag;
  edm::EDGetTokenT<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > recHitsEBToken;
  const edm::InputTag recHitsEETag;
  edm::EDGetTokenT<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > recHitsEEToken;

  // Gen Particles and MC info
  const bool isGMSB;
  const bool isHVDS;
  const bool isBkg;
  bool isMC;
  edm::EDGetTokenT<GenEventInfoProduct>             genevtInfoToken;
  edm::EDGetTokenT<std::vector<PileupSummaryInfo> > pileupInfoToken;
  edm::EDGetTokenT<std::vector<reco::GenParticle> > genpartsToken;
  edm::EDGetTokenT<std::vector<reco::GenJet> >      genjetsToken;

  // output event level ntuple
  TTree* tree;
 
  // MC info
  float genwgt;
  int genpuobs, genputrue;

  // gmsb
  int nNeutoPhGr;
  gmsbStruct gmsbBranch0, gmsbBranch1;

  // event info
  unsigned long int event;
  unsigned int run, lumi;  

  // trigger info
  bool hltDisPho;

  // vertices
  int nvtx;
  float vtxX, vtxY, vtxZ;

  // rho
  float rho;

  // MET
  float t1pfMETpt, t1pfMETphi, t1pfMETsumEt;

  // HT
  float jetHT;

  // jets
  int njets;
  jetStruct jetBranch0, jetBranch1, jetBranch2, jetBranch3;

  // RecHits
  int nrechits;
  std::vector<float> rheta, rhphi, rhE, rhtime;
  std::vector<int> rhOOT;
  std::vector<unsigned int> rhID;

  // photon info
  int nphotons;
  phoStruct phoBranch0, phoBranch1, phoBranch2, phoBranch3;
};

#endif
