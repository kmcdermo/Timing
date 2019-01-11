#ifndef __HLTPlots__
#define __HLTPlots__

// basic C++ headers
#include <iostream>
#include <fstream>
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

// HLT + Trigger info
#include "FWCore/Common/interface/TriggerNames.h"
#include "HLTrigger/HLTcore/interface/HLTConfigProvider.h"
#include "DataFormats/Common/interface/TriggerResults.h"
#include "DataFormats/HLTReco/interface/TriggerEvent.h"
#include "DataFormats/HLTReco/interface/TriggerObject.h"

// DataFormats
#include "DataFormats/Common/interface/ValueMap.h"
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

// Tracks
#include "DataFormats/TrackReco/interface/Track.h"

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
#include "TEfficiency.h"
#include "TLorentzVector.h"
#include "TString.h"
#include "TPRegexp.h"

// Common types
#include "Timing/TimingAnalyzer/plugins/CommonUtils.hh"

///////////////////////
// Local Struct Defs //
///////////////////////

struct TestOptions
{
  std::string denomName;
  std::string numerName;
  
  int inumer;
  int idenom;
};

struct TestResults
{
  bool passed;
  int  goodph;
};

struct TestStruct
{
  TestOptions options;
  TestResults results;
};

//////////////////////
// Class Definition //
//////////////////////

class HLTPlots : public edm::one::EDAnalyzer<edm::one::SharedResources,edm::one::WatchRuns> 
{
public:

  ////////////////////////
  // Internal Functions //
  ////////////////////////

  explicit HLTPlots(const edm::ParameterSet & iConfig);
  ~HLTPlots();
  static void fillDescriptions(edm::ConfigurationDescriptions & descriptions);

  //////////////////////////
  // Event Prep Functions //
  //////////////////////////
  
  bool GetObjects(const edm::Event & iEvent, const edm::EventSetup & iSetup);
  void InitializeObjects();
  void PrepObjects(const edm::Event & iEvent);

  ////////////////////
  // Main Functions //
  ////////////////////

  inline bool ApplyPreSelection();

  void SetTriggerInfo();
  void InitializeTriggerBranches();

  void SetJetInfo();
  void ClearJetBranches();
  void InitializeJetBranches();

  void SetPhotonInfo();
  void ClearRecoPhotonBranches();
  void InitializeRecoPhotonBranches();

  void SetGoodPhotons();

  void PerformTests();
  void ResetTestResults();

  //////////////////////
  // Helper Functions //
  //////////////////////

  void GetDenomPhs(const std::vector<int> & goodphs, const int idenom, std::vector<int> & denomphs);
  void GetFirstLegResult(const std::vector<int> & goodphs, const std::string & label);
  void GetStandardLegResult(const std::vector<int> & goodphs, const std::string & label, const bool sortByTime);
  void GetLastLegResult(const std::vector<int> & goodphs, const std::string & label);

private:

  ////////////////////////
  // Internal functions //
  ////////////////////////

  virtual void beginJob() override;
  virtual void analyze(const edm::Event & iEvent, const edm::EventSetup & iSetup) override;
  virtual void endJob() override;
  
  virtual void beginRun(const edm::Run & iRun, const edm::EventSetup & iSetup) override;
  virtual void endRun(const edm::Run & iRun, const edm::EventSetup & iSetup) override;

  ///////////////////
  // Input Members //
  ///////////////////

  // pre-selection options
  const bool applyTriggerPS;
  const std::string psPath; 

  // cuts on objects
  const float jetpTmin;
  const int   jetIDmin;
  const float jetEtamax;
  const float phpTmin;
  const std::string phIDmin;
  
  // trig dR matching criteria
  const float dRmin;
  const float pTres;

  // track dR matching criteria
  const float trackpTmin;
  const float trackdRmin;

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

  // trigger I/O
  std::map<std::string,std::vector<pat::TriggerObjectStandAlone> > triggerObjectsByFilterMap; // first index is filter label, second is trigger objects
  std::map<std::string,TestStruct> effTestMap;

  // rho
  const edm::InputTag rhoTag;
  edm::EDGetTokenT<double> rhoToken;
  edm::Handle<double> rhoH;
  float rho;

  // mets
  const edm::InputTag metsTag;
  edm::EDGetTokenT<std::vector<pat::MET> > metsToken;
  edm::Handle<std::vector<pat::MET> > metsH;

  // output met
  pat::MET t1pfMET;

  // jets
  const edm::InputTag jetsTag;
  edm::EDGetTokenT<std::vector<pat::Jet> > jetsToken;
  edm::Handle<std::vector<pat::Jet> > jetsH;

  // output jets
  std::vector<pat::Jet> jets;

  // ged photons
  const edm::InputTag gedPhotonsTag;
  edm::EDGetTokenT<std::vector<pat::Photon> > gedPhotonsToken;
  edm::Handle<std::vector<pat::Photon> > gedPhotonsH;

  // oot photons
  const edm::InputTag ootPhotonsTag;
  edm::EDGetTokenT<std::vector<pat::Photon> > ootPhotonsToken;
  edm::Handle<std::vector<pat::Photon> > ootPhotonsH;
  
  // output photons
  std::vector<pat::Photon> photons;
  std::vector<int> goodphs;

  // EB RecHits
  const edm::InputTag recHitsEBTag;
  edm::EDGetTokenT<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > recHitsEBToken;
  edm::Handle<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > recHitsEBH;
  const edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > * recHitsEB;

  // EE RecHits
  const edm::InputTag recHitsEETag;
  edm::EDGetTokenT<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > recHitsEEToken;
  edm::Handle<edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > > recHitsEEH;
  const edm::SortedCollection<EcalRecHit,edm::StrictWeakOrdering<EcalRecHit> > * recHitsEE;

  // Tracks
  const edm::InputTag tracksTag;
  edm::EDGetTokenT<std::vector<reco::Track> > tracksToken;
  edm::Handle<std::vector<reco::Track> > tracksH;

  // geometry (from ECAL ELF)
  edm::ESHandle<CaloGeometry> caloGeoH;
  const CaloSubdetectorGeometry * barrelGeometry;
  const CaloSubdetectorGeometry * endcapGeometry;

  ///////////////////////////
  // Temp Internal Members //
  ///////////////////////////

  // trigger info
  std::vector<bool> triggerBits;

  // jets
  int njets;
  std::vector<float> jetE, jetpt, jetphi, jeteta;
  float pfJetHT;

  // photon info
  int nphotons;
  std::vector<int> phisOOT;
  std::vector<float> phE, phpt, phphi, pheta;
  std::vector<float> phHoE, phr9;
  std::vector<bool> phPixSeed, phEleVeto;
  std::vector<float> phChgIso, phNeuIso, phIso;
  std::vector<float> phPFClEcalIso, phPFClHcalIso, phHollowTkIso;
  std::vector<float> phsieie, phsipip, phsieip, phsmaj, phsmin, phalpha;
  std::vector<std::vector<int> > phIsHLTMatched; // first index is iph, second is for filter, true/false
  std::vector<int> phIsTrack;

  // supercluster info 
  std::vector<float> phscE, phsceta, phscphi;

  // seed info
  std::vector<float> phseedeta, phseedphi, phseedE, phseedtime;
  std::vector<int> phseedID;
  std::vector<int> phseedOOT;
  
  // all rec hit info
  std::vector<int> phnrh;

  ////////////////////
  // Output Members //
  ////////////////////

  // output hists
  std::map<std::string,TEfficiency*> effETEBs;
  std::map<std::string,TEfficiency*> effETEEs;
  std::map<std::string,TEfficiency*> effetas;
  std::map<std::string,TEfficiency*> effphis;
  std::map<std::string,TEfficiency*> efftimes;
  std::map<std::string,TEfficiency*> effHTs;
};

#endif
