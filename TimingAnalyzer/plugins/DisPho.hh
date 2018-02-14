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
#include "DataFormats/TrackReco/interface/Track.h"
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
#include "TH1F.h"
#include "TTree.h"
#include "Math/PositionVector3D.h"

// Common Utilities
#include "Timing/TimingAnalyzer/plugins/CommonUtils.hh"

// Unique structs
#include "Timing/TimingAnalyzer/plugins/DisPhoTypes.hh"

// Unique typedef
typedef ROOT::Math::PositionVector3D<ROOT::Math::Cartesian3D<float>,ROOT::Math::DefaultCoordinateSystemTag> Point3D;

class DisPho : public edm::one::EDAnalyzer<edm::one::SharedResources,edm::one::WatchRuns> 
{
 public:
  explicit DisPho(const edm::ParameterSet&);
  ~DisPho();

  void MakeHists();
  void MakeAndFillConfigTree();
  void MakeEventTree();
  void MakeGMSBBranch(const int i, gmsbStruct& gmsbBranch);
  void MakeHVDSBranch(const int i, hvdsStruct& hvdsBranch);
  void MakeJetBranch(const int i, jetStruct& jetBranch);
  void MakePhoBranch(const int i, phoStruct& phoBranch);
  void MakePhoBranchMC(const int i, phoStruct& phoBranch);

  void InitializeGenEvtBranches();
  void InitializeGenPointBranches();
  void InitializeGenPUBranches();
  void InitializeGMSBBranches();
  void InitializeGMSBBranch(gmsbStruct& gmsbBranch);
  void SetGMSBBranch(const reco::GenParticle & neutralino, gmsbStruct & gmsbBranch, const std::vector<oot::Photon> & photons);
  void InitializeHVDSBranches();
  void InitializeHVDSBranch(hvdsStruct& hvdsBranch);
  void SetHVDSBranch(const reco::GenParticle & vPion, hvdsStruct & hvdsBranch, const std::vector<oot::Photon> & photons);
  void SetTriggerBits(edm::Handle<edm::TriggerResults> & triggerResultsH, const edm::Event & iEvent);
  void InitializePVBranches();
  void InitializeTrackBranches();
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
		    const EcalRecHitCollection * recHitsEB, const EcalRecHitCollection * recHitsEE,
		    const edm::Handle<std::vector<reco::Track> > & tracksH);
  void InitializePhoBranchesMC();
  void InitializePhoBranchMC(phoStruct & phoBranch);
  void SetPhoBranchMC(const int iph, const oot::Photon& photon, phoStruct& phoBranch, 
		      const edm::Handle<std::vector<reco::GenParticle> > & genparticlesH);
  int  CheckMatchHVDS(const int iph, const hvdsStruct& hvdsBranch);

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);
  
 private:
  virtual void beginJob() override;
  virtual void analyze(const edm::Event&, const edm::EventSetup&) override;
  virtual void endJob() override;
  
  virtual void beginRun(edm::Run const&, edm::EventSetup const&) override;
  virtual void endRun(edm::Run const&, edm::EventSetup const&) override;

  // blinding
  const unsigned int blindSF;
  const bool applyBlindSF;
  const float blindMET;
  const bool applyBlindMET;

  // object prep
  const float jetpTmin;
  const float jetEtamax;
  const int jetIDmin;
  const float rhEmin;
  const float phpTmin;
  const std::string phIDmin;

  // object extra pruning
  const float seedTimemin;
  const int jetIDStoremin;

  // photon storing options
  const bool splitPho;
  const bool onlyGED;
  const bool onlyOOT;

  // rechit storing options
  const bool storeRecHits;

  // pre-selection vars
  const bool applyTrigger;
  const float minHT;
  const bool applyHT;
  const float phgoodpTmin;
  const std::string phgoodIDmin;
  const bool applyPhGood;

  // dR matching criteria
  const float dRmin;
  const float pTres;
  const float genpTres;
  const float trackdRmin;
  const float trackpTmin;

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

  // met filters
  const std::string inputFlags;
  std::vector<std::string> flagNames;
  strBitMap triggerFlagMap;
  const edm::InputTag triggerFlagsTag;
  edm::EDGetTokenT<edm::TriggerResults> triggerFlagsToken;

  // Tracks
  const edm::InputTag tracksTag;
  edm::EDGetTokenT<std::vector<reco::Track> > tracksToken;

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

  // ECAL RecHits
  const edm::InputTag recHitsEBTag;
  edm::EDGetTokenT<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > recHitsEBToken;
  const edm::InputTag recHitsEETag;
  edm::EDGetTokenT<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > recHitsEEToken;

  // photons + ids
  const edm::InputTag photonsTag;
  edm::EDGetTokenT<std::vector<pat::Photon> > photonsToken;

  // ootPhotons + ids
  const edm::InputTag ootPhotonsTag;
  edm::EDGetTokenT<std::vector<pat::Photon> > ootPhotonsToken;

  // Gen Particles and MC info
  const bool isGMSB;
  const bool isHVDS;
  const bool isBkgd;
  const bool isToy;
  const float xsec;
  const float filterEff;
  const float BR;
  bool isMC;
  edm::EDGetTokenT<GenEventInfoProduct>             genevtInfoToken;
  edm::EDGetTokenT<Point3D>                         genxyz0Token;
  edm::EDGetTokenT<float>                           gent0Token;
  edm::EDGetTokenT<std::vector<PileupSummaryInfo> > pileupInfoToken;
  edm::EDGetTokenT<std::vector<reco::GenParticle> > genpartsToken;
  edm::EDGetTokenT<std::vector<reco::GenJet> >      genjetsToken;

  // output histograms
  TH1F * h_cutflow;
  std::map<std::string,int> cutflowLabelMap;

  // output metadeta configs
  TTree * configtree;

  // output event level ntuple
  TTree * disphotree;
 
  // MC info
  float genwgt;
  float genx0,geny0,genz0,gent0;
  int genpuobs, genputrue;

  // gmsb
  int nNeutoPhGr;
  gmsbStruct gmsbBranch0, gmsbBranch1;

  // hvds
  int nvPions;
  hvdsStruct hvdsBranch0, hvdsBranch1, hvdsBranch2, hvdsBranch3;

  // event info
  unsigned long int event; // technically unsigned long long in Event.h...
  unsigned int run, lumi;  

  // trigger info
  bool hltSignal;
  bool hltRefPhoID;
  bool hltRefDispID;
  bool hltRefHT;
  bool hltPho50;
  bool hltPho200;
  bool hltDiPho70;
  bool hltDiPho3022M90;
  bool hltDiPho30PV18PV;
  bool hltDiEle33MW;
  bool hltDiEle27WPT;
  bool hltJet500;

  // met filter info
  bool metPV;
  bool metBeamHalo;
  bool metHBHENoise;
  bool metHBHEisoNoise;
  bool metECALTP;
  bool metPFMuon;
  bool metPFChgHad;
  bool metEESC;
  bool metECALCalib;

  // vertices
  int nvtx;
  float vtxX, vtxY, vtxZ;

  // extra track info
  std::vector<float> vtx_z;
  std::vector<int> vtx_nTks, vtx_nTks_pt09_B, vtx_nTks_p09_E, vtx_nTks_p2_E, vtx_nTks_pt09_E, vtx_nTks_pt2_E;
  int sum_nTks, sum_nTks_pt09_B, sum_nTks_p09_E, sum_nTks_p2_E, sum_nTks_pt09_E, sum_nTks_pt2_E;

  // rho
  float rho;

  // MET
  float t1pfMETpt, t1pfMETphi, t1pfMETsumEt;

  // HT + njets
  float jetHT, jetHTpt15, jetHTeta3, jetHTidL, jetHTnopho, jetHTidT;
  int   njets, njetspt15, njetseta3, njetsidL, njetsnopho, njetsidT; 

  // jets
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
