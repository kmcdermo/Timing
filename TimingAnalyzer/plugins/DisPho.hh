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
#include "DataFormats/JetReco/interface/GenJet.h"

// DataFormats
#include "DataFormats/Common/interface/ValueMap.h"
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/PatCandidates/interface/Photon.h"
#include "DataFormats/PatCandidates/interface/MET.h"
#include "DataFormats/PatCandidates/interface/Jet.h"
#include "DataFormats/PatCandidates/interface/Electron.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/MuonReco/interface/MuonSelectors.h"

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

// ECAL Record info (Laser Constants)
#include "CalibCalorimetry/EcalLaserCorrection/interface/EcalLaserDbService.h"
#include "CalibCalorimetry/EcalLaserCorrection/interface/EcalLaserDbRecord.h"

// ECAL Record info (Intercalibration Constants)
#include "CondFormats/EcalObjects/interface/EcalIntercalibConstants.h"
#include "CondFormats/DataRecord/interface/EcalIntercalibConstantsRcd.h"

// ECAL Record info (ADCToGeV)
#include "CondFormats/EcalObjects/interface/EcalADCToGeVConstant.h"
#include "CondFormats/DataRecord/interface/EcalADCToGeVConstantRcd.h"

// ECAL Record info (ADCToGeV)
#include "CondFormats/EcalObjects/interface/EcalADCToGeVConstant.h"
#include "CondFormats/DataRecord/interface/EcalADCToGeVConstantRcd.h"

// ECAL Record info (Pedestals)
#include "CondFormats/EcalObjects/interface/EcalPedestals.h"
#include "CondFormats/DataRecord/interface/EcalPedestalsRcd.h"

// JECS
#include "CondFormats/JetMETObjects/interface/JetCorrectorParameters.h"
#include "JetMETCorrections/Objects/interface/JetCorrectionsRecord.h"
#include "CondFormats/JetMETObjects/interface/JetCorrectionUncertainty.h"

// JERs
#include "CondFormats/JetMETObjects/interface/JetResolutionObject.h"
#include "JetMETCorrections/Modules/interface/JetResolution.h"

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

  void InitializeRhoBranches();
  void SetRhoBranches(const edm::Handle<double> & rhosH);

  void InitializeGenEvtBranches();
  void SetGenEvtBranches(const edm::Handle<GenEventInfoProduct> & genevtInfoH);

  void InitializeGenPointBranches();
  void SetGenXYZ0Branches(const edm::Handle<Point3D> & genxyz0H);
  void SetGenT0Branches(const edm::Handle<float> & gent0H);

  void InitializeGenPUBranches();
  void SetGenPUBranches(const edm::Handle<std::vector<PileupSummaryInfo> > & pileupInfoH);

  void InitializeGMSBBranches();
  void SetGMSBBranches(const std::vector<reco::GenParticle> & neutralinos, const std::vector<oot::Photon> & photons, const int nPhotons);

  void InitializeHVDSBranches();
  void SetHVDSBranches(const std::vector<reco::GenParticle> & vPions, const std::vector<oot::Photon> & photons, const int nPhotons);
 
  void InitializeToyBranches();
  void SetToyBranches(const std::vector<reco::GenParticle> & toys, const std::vector<oot::Photon> & photons, const int nPhotons);

  void SetRecordInfo(const edm::Event& iEvent);
  void SetTriggerBranches();
  void SetMETFilterBranches(const edm::Handle<bool> & ecalBadCalibFlagH);

  void InitializePVBranches();
  void SetPVBranches(const edm::Handle<std::vector<reco::Vertex> > & verticesH);

  void InitializeMETBranches();
  void SetMETBranches(const pat::MET & t1pfMET);

  void InitializeJetBranches(const int nJets);
  void SetJetBranches(const std::vector<pat::Jet> & jet, const int nJets);

  void InitializeJetBranchesMC(const int nJets);
  void SetJetBranchesMC(const std::vector<pat::Jet> & jet, const int nJets, const edm::Handle<std::vector<reco::GenJet> > & genjetsH,
			JetCorrectionUncertainty & jetCorrUnc, const JME::JetResolution & jetRes, const JME::JetResolutionScaleFactor & jetRes_sf);
  int GenJetMatcher(const pat::Jet & jet, const std::vector<reco::GenJet> & genjets, const float jer);
  void GetStochasticSmear(std::mt19937 & mt_rand, const float jer, const float jer_sf, float & jet_smear);
  void CheckJetSmear(const float energy, float & jet_smear);
  
  void InitializeElectronBranches();
  void SetElectronBranches(const std::vector<pat::Electron> & electrons);

  void InitializeMuonBranches();
  void SetMuonBranches(const std::vector<pat::Muon> & muons);  

  void InitializeRecHitBranches(const int nRecHits);
  void SetRecHitBranches(const EcalRecHitCollection * recHitsEB, const CaloSubdetectorGeometry * barrelGeometry,
			 const EcalRecHitCollection * recHitsEE, const CaloSubdetectorGeometry * endcapGeometry,
			 const uiiumap & recHitMap, const edm::Event & iEvent,
			 const edm::ESHandle<EcalLaserDbService> & laserH, const EcalIntercalibConstantMap * interCalibMap,
			 const edm::ESHandle<EcalADCToGeVConstant> & adcToGeVH, const edm::ESHandle<EcalPedestals> & pedestalsH);
  void SetRecHitBranches(const EcalRecHitCollection * recHits, const CaloSubdetectorGeometry * geometry,
			 const uiiumap & recHitMap, const edm::Event & iEvent, 
			 const edm::ESHandle<EcalLaserDbService> & laserH, const EcalIntercalibConstantMap * interCalibMap,
			 const float adcToGeV, const edm::ESHandle<EcalPedestals> & pedestalsH);

  void InitializePhoBranches();
  void SetPhoBranches(const std::vector<oot::Photon> photons, const int nPhotons, const uiiumap & recHitMap,
		      const EcalRecHitCollection * recHitsEB, const EcalRecHitCollection * recHitsEE,
		      const edm::Handle<std::vector<reco::Track> > & tracksH);

  void InitializePhoBranchesMC();
  void SetPhoBranchesMC(const std::vector<oot::Photon> photons, const int nPhotons, 
			const edm::Handle<std::vector<reco::GenParticle> > & genparticlesH);
  int  CheckMatchHVDS(const int iphoton, const hvdsStruct& hvdsBranch);

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

  // photon storing options
  const bool splitPho;
  const bool onlyGED;
  const bool onlyOOT;

  // lepton prep cuts
  const float ellowpTmin;
  const float elhighpTmin;
  const float mulowpTmin;
  const float muhighpTmin;

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
  const float gendRmin;
  const float genpTres;
  const float trackdRmin;
  const float trackpTmin;
  const float genjetdRmin;
  const float genjetpTfactor;
  const float leptondRmin;

  // JER extra info
  const float smearjetEmin;

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
  const edm::InputTag ecalBadCalibFlagTag;
  edm::EDGetTokenT<bool> ecalBadCalibFlagToken;

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

  // electrons
  const edm::InputTag electronsTag;
  edm::EDGetTokenT<std::vector<pat::Electron> > electronsToken;

  // muons
  const edm::InputTag muonsTag;
  edm::EDGetTokenT<std::vector<pat::Muon> > muonsToken;

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
  const bool isADD;
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
  TH1F * h_cutflow_wgt;
  std::map<std::string,int> cutflowLabelMap;
  TH1F * h_genpuobs;
  TH1F * h_genpuobs_wgt;
  TH1F * h_genputrue;
  TH1F * h_genputrue_wgt;

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
  std::vector<gmsbStruct> gmsbBranches;

  // hvds
  int nvPions;
  std::vector<hvdsStruct> hvdsBranches;

  // toyMC
  int nToyPhs;
  std::vector<toyStruct> toyBranches;

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
  bool hltEle32WPT;
  bool hltDiEle33MW;
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
  bool metECALBadCalib;

  // vertices
  int nvtx;
  float vtxX, vtxY, vtxZ;

  // rho
  float rho;

  // MET
  float t1pfMETpt, t1pfMETphi, t1pfMETsumEt;

  // jets
  int njets;
  std::vector<float> jetE, jetpt, jetphi, jeteta;
  std::vector<int>   jetID;
  std::vector<float> jetNHF, jetNEMF, jetCHF, jetCEMF, jetMUF, jetNHM, jetCHM;

  std::vector<float> jetscaleRel, jetsmearSF, jetsmearDownSF, jetsmearUpSF;
  std::vector<int>   jetisGen;

  // electrons
  int nelLowL, nelLowM, nelLowT, nelHighL, nelHighM, nelHighT;

  // muons
  int nmuLowL, nmuLowM, nmuLowT, nmuHighL, nmuHighM, nmuHighT;

  // RecHits
  int nrechits;
  std::vector<float> rhX, rhY, rhZ, rhE, rhtime, rhtimeErr, rhTOF;
  std::vector<unsigned int> rhID;
  std::vector<bool> rhisOOT, rhisGS6, rhisGS1;
  std::vector<float> rhadcToGeV;
  std::vector<float> rhped12, rhped6, rhped1;
  std::vector<float> rhpedrms12, rhpedrms6, rhpedrms1;

  // photon info
  int nphotons;
  std::vector<phoStruct> phoBranches;
};

#endif
