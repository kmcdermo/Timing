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

////////////////////
// Unique typedef //
////////////////////

typedef ROOT::Math::PositionVector3D<ROOT::Math::Cartesian3D<float>,ROOT::Math::DefaultCoordinateSystemTag> Point3D;

//////////////////////
// Class Definition //
//////////////////////

class DisPho : public edm::one::EDAnalyzer<edm::one::SharedResources,edm::one::WatchRuns> 
{
public:

  ////////////////////////
  // Internal Functions //
  ////////////////////////

  explicit DisPho(const edm::ParameterSet & iConfig);
  void ConsumeTokens();
  void SetupCutFlowLabels();
  ~DisPho();
  static void fillDescriptions(edm::ConfigurationDescriptions & descriptions);  

  /////////////////////////////
  // Analyzer Prep Functions //
  /////////////////////////////

  void MakeHists();
  void MakeAndFillConfigTree();
  void MakeEventTree();

  ///////////////////////////
  // Object Prep Functions //
  ///////////////////////////

  bool GetObjects(const edm::Event & iEvent, const edm::EventSetup & iSetup);
  bool GetStandardObjects(const edm::Event & iEvent);
  bool GetCalibrationConstants(const  edm::EventSetup & iSetup);
  bool GetMCObjects(const edm::Event & iEvent);
  void InitializeObjects(const edm::Event & iEvent);

  void GetWeights();
  void InitializeGenPUBranches();
  void SetGenPUBranches();
  void AlwaysFillHists();

  void PrepObjects(const edm::Event & iEvent);

  ////////////////////////////////
  // Blinding and Pre-Selection //
  ////////////////////////////////

  inline bool ApplyBlindSF();
  void FillBlindSF();

  inline bool ApplyBlindMET();
  void FillBlindMET();

  bool ApplyPreSelectionTrigger();
  void FillPreSelectionTrigger();

  bool ApplyPreSelectionHT();
  void FillPreSelectionHT();

  bool ApplyPreSelectionGoodPhoton();
  void FillPreSelectionGoodPhoton();

  //////////////////////////////////////
  // Fill Tree from Objects Functions //
  //////////////////////////////////////

  void FillTreeFromObjects(const edm::Event & iEvent);

  void SetMCInfo();
  void InitializeGenPointBranches();
  void SetGenT0Branches();
  void SetGenXYZ0Branches();
  void InitializeGMSBBranches();
  void SetGMSBBranches();
  void InitializeHVDSBranches();
  void SetHVDSBranches();
  void InitializeToyBranches();
  void SetToyBranches();

  void SetRecordInfo(const edm::Event & iEvent);
  void SetTriggerBranches();
  void SetMETFilterBranches();

  void InitializePVBranches();
  void SetPVBranches();

  void InitializeMETBranches();
  void SetMETBranches();
  void InitializeMETBranchesMC();
  void SetMETBranchesMC();

  void InitializeJetBranches();
  void SetJetBranches();
  void InitializeJetBranchesMC();
  void SetJetBranchesMC();
  int GenJetMatcher(const pat::Jet & jet, const edm::Handle<std::vector<reco::GenJet> > & genJetsH, const float jer);
  void GetStochasticSmear(std::mt19937 & mt_rand, const float jer, const float jer_sf, float & jet_smear);
  void CheckJetSmear(const float energy, float & jet_smear);
  
  void InitializeElectronBranches();
  void SetElectronBranches();
  void InitializeMuonBranches();
  void SetMuonBranches();

  void InitializeRecHitBranches();
  void SetRecHitBranches();
  void SetRecHitBranches(const EcalRecHitCollection * recHits, const CaloSubdetectorGeometry * geometry, const float adcToGeV);

  void InitializePhoBranches();
  void SetPhoBranches();
  void InitializePhoBranchesMC();
  void SetPhoBranchesMC();
  int  CheckMatchHVDS(const int iphoton, const hvdsStruct & hvdsBranch);

private:

  ////////////////////////
  // Internal Functions //
  ////////////////////////

  virtual void beginJob() override;
  virtual void analyze(const edm::Event & iEvent, const edm::EventSetup & iSetup) override;
  virtual void endJob() override;
  
  virtual void beginRun(const edm::Run & iRun, const edm::EventSetup & iSetup) override;
  virtual void endRun(const edm::Run & iRun, const edm::EventSetup & iSetup) override;

  ///////////////////
  // Input Members //
  ///////////////////

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
  const int nPhosmax;

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

  // trigger inputs
  const std::string inputPaths;
  std::vector<std::string> pathNames;
  strBitMap triggerBitMap;
  const std::string inputFilters;
  std::vector<std::string> filterNames;

  // trigger results
  const edm::InputTag triggerResultsTag;
  edm::EDGetTokenT<edm::TriggerResults> triggerResultsToken;
  edm::Handle<edm::TriggerResults> triggerResultsH;

  // trigger objects
  const edm::InputTag triggerObjectsTag;
  edm::EDGetTokenT<std::vector<pat::TriggerObjectStandAlone> > triggerObjectsToken;
  edm::Handle<std::vector<pat::TriggerObjectStandAlone> > triggerObjectsH;

  // output triggers
  std::map<std::string,std::vector<pat::TriggerObjectStandAlone> > triggerObjectsByFilterMap; // first index is filter label, second is trigger objects

  // met filter inputs
  const std::string inputFlags;
  std::vector<std::string> flagNames;
  strBitMap triggerFlagMap;

  // met filters
  const edm::InputTag triggerFlagsTag;
  edm::EDGetTokenT<edm::TriggerResults> triggerFlagsToken;
  edm::Handle<edm::TriggerResults> triggerFlagsH;

  const edm::InputTag ecalBadCalibFlagTag;
  edm::EDGetTokenT<bool> ecalBadCalibFlagToken;
  edm::Handle<bool> ecalBadCalibFlagH;

  // Tracks
  const edm::InputTag tracksTag;
  edm::EDGetTokenT<std::vector<reco::Track> > tracksToken;
  edm::Handle<std::vector<reco::Track> > tracksH;

  // vertices
  const edm::InputTag verticesTag;
  edm::EDGetTokenT<std::vector<reco::Vertex> > verticesToken;
  edm::Handle<std::vector<reco::Vertex> > verticesH;

  // rho
  const edm::InputTag rhoTag;
  edm::EDGetTokenT<double> rhoToken;
  edm::Handle<double> rhoH;
  float rho;

  // mets
  const edm::InputTag metsTag;
  edm::EDGetTokenT<std::vector<pat::MET> > metsToken;
  edm::Handle<std::vector<pat::MET> > metsH;

  // output MET
  pat::MET t1pfMET;

  // jets
  const edm::InputTag jetsTag;
  edm::EDGetTokenT<std::vector<pat::Jet> > jetsToken;
  edm::Handle<std::vector<pat::Jet> > jetsH;
  std::vector<pat::Jet> jets;

  // electrons
  const edm::InputTag electronsTag;
  edm::EDGetTokenT<std::vector<pat::Electron> > electronsToken;
  edm::Handle<std::vector<pat::Electron> > electronsH;
  std::vector<pat::Electron> electrons;
  
  // muons
  const edm::InputTag muonsTag;
  edm::EDGetTokenT<std::vector<pat::Muon> > muonsToken;
  edm::Handle<std::vector<pat::Muon> > muonsH;
  std::vector<pat::Muon> muons;
  
  // RecHits EB
  const edm::InputTag recHitsEBTag;
  edm::EDGetTokenT<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > recHitsEBToken;
  edm::Handle<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > recHitsEBH;
  const edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > * recHitsEB;

  // RecHits EE
  const edm::InputTag recHitsEETag;
  edm::EDGetTokenT<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > recHitsEEToken;
  edm::Handle<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > recHitsEEH;
  const edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > * recHitsEE;

  // Output rechit map
  uiiumap recHitMap;

  // gedPhotons
  const edm::InputTag gedPhotonsTag;
  edm::EDGetTokenT<std::vector<pat::Photon> > gedPhotonsToken;
  edm::Handle<std::vector<pat::Photon> > gedPhotonsH;

  // ootPhotons
  const edm::InputTag ootPhotonsTag;
  edm::EDGetTokenT<std::vector<pat::Photon> > ootPhotonsToken;
  edm::Handle<std::vector<pat::Photon> > ootPhotonsH;

  // output photons
  std::vector<pat::Photon> photons;

  // geometry
  edm::ESHandle<CaloGeometry> caloGeoH;
  const CaloSubdetectorGeometry * barrelGeometry;
  const CaloSubdetectorGeometry * endcapGeometry;

  // lasers
  edm::ESHandle<EcalLaserDbService> laserH;
  edm::Timestamp evTime;

  // inter calibration
  edm::ESHandle<EcalIntercalibConstants> interCalibH;
  const EcalIntercalibConstantMap *      interCalibMap;

  // ADCToGeV
  edm::ESHandle<EcalADCToGeVConstant> adcToGeVH;
  float adcToGeVEB;
  float adcToGeVEE;

  // pedestals
  edm::ESHandle<EcalPedestals> pedestalsH;

  // JECs
  edm::ESHandle<JetCorrectorParametersCollection> jetCorrH;
  
  // JERs
  JME::JetResolution jetRes;
  JME::JetResolutionScaleFactor jetRes_sf; 

  // Gen config and MC info
  const bool isGMSB;
  const bool isHVDS;
  const bool isBkgd;
  const bool isToy;
  const bool isADD;
  const float xsec;
  const float filterEff;
  const float BR;
  bool isMC;

  // genEvtInfo
  const edm::InputTag genEvtInfoTag;
  edm::EDGetTokenT<GenEventInfoProduct> genEvtInfoToken;
  edm::Handle<GenEventInfoProduct> genEvtInfoH;

  // gen time
  const edm::InputTag gent0Tag;
  edm::EDGetTokenT<float> gent0Token;
  edm::Handle<float> gent0H;
  
  // gen vertex
  const edm::InputTag genxyz0Tag;
  edm::EDGetTokenT<Point3D> genxyz0Token;
  edm::Handle<Point3D> genxyz0H;
  
  // pileups
  const edm::InputTag pileupInfosTag;
  edm::EDGetTokenT<std::vector<PileupSummaryInfo> > pileupInfosToken;
  edm::Handle<std::vector<PileupSummaryInfo> > pileupInfosH;
  
  // genParticles
  const edm::InputTag genParticlesTag;
  edm::EDGetTokenT<std::vector<reco::GenParticle> > genParticlesToken;
  edm::Handle<std::vector<reco::GenParticle> > genParticlesH;

  // genJets
  const edm::InputTag genJetsTag;
  edm::EDGetTokenT<std::vector<reco::GenJet> > genJetsToken;
  edm::Handle<std::vector<reco::GenJet> > genJetsH;

  // output gen particles
  std::vector<reco::GenParticle> neutralinos;
  std::vector<reco::GenParticle> vPions;
  std::vector<reco::GenParticle> toys;

  ///////////////////////////
  // Temp Internal Members //
  ///////////////////////////

  float wgt;
  int nJets, nRecHits, nPhotons;

  ////////////////////
  // Output Members //
  ////////////////////

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
  float gent0,genx0,geny0,genz0;
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

  // Type-1 PF MET
  float t1pfMETpt, t1pfMETphi, t1pfMETsumEt;
  float t1pfMETptUncorr, t1pfMETphiUncorr, t1pfMETsumEtUncorr;

  // GEN MET
  float genMETpt, genMETphi;

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
