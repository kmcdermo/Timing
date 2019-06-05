#ifndef __RecHitCounter__
#define __RecHitCounter__

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
#include "DataFormats/PatCandidates/interface/Photon.h"
#include "DataFormats/PatCandidates/interface/MET.h"

// Ecal RecHits
#include "DataFormats/EcalRecHit/interface/EcalRecHit.h"
#include "DataFormats/EcalRecHit/interface/EcalRecHitCollections.h"
#include "DataFormats/EcalDetId/interface/EcalSubdetector.h"

// Geometry
#include "Geometry/Records/interface/CaloGeometryRecord.h"
#include "Geometry/CaloGeometry/interface/CaloGeometry.h"
#include "Geometry/CaloGeometry/interface/CaloSubdetectorGeometry.h"
#include "Geometry/CaloGeometry/interface/CaloCellGeometry.h"
#include "DataFormats/GeometryVector/interface/GlobalPoint.h"

// ROOT
#include "TTree.h"

// standard includes
#include <string>
#include <unordered_set>
#include <algorithm>

// Common Utilities
#include "Timing/TimingAnalyzer/plugins/CommonUtils.hh"

///////////////////////
// Struct Definition //
///////////////////////

struct rhStruct
{
  rhStruct () {}

  std::vector<float> x, y, z, E, T;
  std::vector<unsigned int> ID;
  std::vector<bool> isOOT;
};

//////////////////////
// Class Definition //
//////////////////////

class RecHitCounter : public edm::one::EDAnalyzer<edm::one::SharedResources,edm::one::WatchRuns> 
{
public:

  ////////////////////////
  // Internal Functions //
  ////////////////////////

  explicit RecHitCounter(const edm::ParameterSet & iConfig);
  void ConsumeTokens();
  ~RecHitCounter();
  static void fillDescriptions(edm::ConfigurationDescriptions & descriptions);

  ///////////////////////////
  // Setup TTree Functions //
  ///////////////////////////

  void MakeTree();
  void MakeVectorBranches(const std::string & label, rhStruct & rhInfo);

  //////////////////////////
  // Event Prep Functions //
  //////////////////////////

  bool GetObjects(const edm::Event & iEvent);
  bool GetCalibrationConstants(const edm::EventSetup & iSetup);
  void InitializeObjects();
  void PrepObjects();
  void ResetRecHitMaps();
  void ResetStructs();
  void ResetStruct(rhStruct & rhInfo);

  ////////////////////
  // Main Functions //
  ////////////////////
  
  void SetEventInfo();
  void CountRecHitsPre();
  void CountRecHitsPost();
  
  ////////////////////////
  // Set RecHitCounters //
  ////////////////////////

  void SetRecHitMap(const edm::Handle<std::vector<pat::Photon> > & photonsH, uiiumap & recHitMap, int & nRH);
  void SetRecHitMaps();

  void AppendPhotonRecHits(const pat::Photon & photon, uiiumap & recHitMap);
  void SetSharedHits(int & nRH, rhStruct & rhInfo);
  EcalRecHit GetRecHit(const uint32_t rawID, const EcalRecHitCollection * recHits);

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

  // match config 
  const float dRmin;
  const float phpTmin;
  const std::string phIDmin;

  // rho
  const edm::InputTag rhoTag;
  edm::EDGetTokenT<double> rhoToken;
  edm::Handle<double> rhoH;
  float rho;

  // geometry
  edm::ESHandle<CaloGeometry> caloGeoH;
  const CaloSubdetectorGeometry * barrelGeometry;
  const CaloSubdetectorGeometry * endcapGeometry;

  // mets
  const edm::InputTag metsTag;
  edm::EDGetTokenT<std::vector<pat::MET> > metsToken;
  edm::Handle<std::vector<pat::MET> > metsH;

  // gedPhotons
  const edm::InputTag gedPhotonsTag;
  edm::EDGetTokenT<std::vector<pat::Photon> > gedPhotonsToken;
  edm::Handle<std::vector<pat::Photon> > gedPhotonsH;

  // ootPhotons
  const edm::InputTag ootPhotonsTag;
  edm::EDGetTokenT<std::vector<pat::Photon> > ootPhotonsToken;
  edm::Handle<std::vector<pat::Photon> > ootPhotonsH;

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

  //////////////////////
  // Temp I/O Members //
  //////////////////////

  // output MET
  pat::MET t1pfMET;

  // merged photons
  std::vector<pat::Photon> photons;

  // rechit maps
  uiiumap gedRecHitMap;
  uiiumap ootRecHitMap;

  // vector of rechit structs
  rhStruct rhInfo_Pre;
  rhStruct rhInfo_Post;

  ////////////////////
  // Output Members //
  ////////////////////

  // tree
  TTree * tree;
 
  int nGEDRH_Pre , nOOTRH_Pre , nSharedRH_Pre;
  int nGEDRH_Post, nOOTRH_Post, nSharedRH_Post;
};

#endif
