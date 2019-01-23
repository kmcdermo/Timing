#ifndef __Counter__
#define __Counter__

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

// ROOT
#include "TH1F.h"
#include "TTree.h"

// standard includes
#include <cmath>
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <unordered_set>

// Common Utilities
#include "Timing/TimingAnalyzer/plugins/CommonUtils.hh"

///////////////////////////
// Enum of Matching Type //
///////////////////////////

enum IDType {N,L,T};

///////////////////////
// Struct of indices //
///////////////////////

struct MatchingInfo
{
  MatchingInfo() {}

  // Self info
  std::string label;
  
  // VID info
  std::string baseVID;
  std::string testVID;

  // counters
  int              npho;
  std::vector<int> nbase_to_tests_GT;
  std::vector<int> nbase_to_tests_LT;
  int              nbase_unmatched;
  std::vector<int> nbase_to_bases;
  int              nbase_to_keep;
  int              nbase_to_drop;
  int              nbase_to_both; // should be an error!

  // indices
  std::vector<std::vector<int> > bases_to_tests_GT;
  std::vector<std::vector<int> > bases_to_tests_LT;
  std::vector<int>               bases_unmatched;
  std::vector<std::vector<int> > bases_to_bases;

  // sets for defining keeps/drops
  std::unordered_set<int> bases_to_keep;
  std::unordered_set<int> bases_to_drop;
};

/////////////////
// Photon info //
/////////////////

struct PhotonInfo
{
  PhotonInfo() {}

  std::string label;
  std::vector<pat::Photon> photons;
  std::map<IDType,MatchingInfo> matchingInfoMap;
};

//////////////////////
// Class Definition //
//////////////////////

class Counter : public edm::one::EDAnalyzer<edm::one::SharedResources,edm::one::WatchRuns> 
{
public:

  ////////////////////////
  // Internal Functions //
  ////////////////////////

  explicit Counter(const edm::ParameterSet & iConfig);
  ~Counter();
  static void fillDescriptions(edm::ConfigurationDescriptions & descriptions);

  //////////////////////////////
  // Internal Setup Functions //
  //////////////////////////////

  void SetInternalInfo();
  void SetLabels(PhotonInfo & photonInfo, const std::string & label);

  ///////////////////////////
  // Setup TTree Functions //
  ///////////////////////////

  void MakeTree();
  void InitBranches();
  void InitBranches(PhotonInfo & photonInfo, const std::string & base, const std::string & test);

  //////////////////////////
  // Event Prep Functions //
  //////////////////////////

  bool GetObjects(const edm::Event & iEvent);
  void InitializeObjects();
  void PrepObjects();

  ///////////////////
  // Main Function //
  ///////////////////
  
  void SetEventInfo();

  ////////////////////////
  // Set Basic Counters //
  ////////////////////////

  void SetBasicCounters();
  void SetBasicCounters(PhotonInfo & photonInfo);

  /////////////////////////
  // Set Overlap Indices //
  /////////////////////////

  void SetOverlapIndices();
  void SetBaseToTestIndices(PhotonInfo & basePhotonInfo, const PhotonInfo & testPhotonInfo);
  void SetBaseToBaseIndices(PhotonInfo & basePhotonInfo);

  //////////////////////////
  // Set Overlap Counters //
  //////////////////////////

  void SetOverlapCounters();
  void SetOverlapCounters(PhotonInfo & photonInfo);
  void SetOverlapCounter(const std::vector<std::vector<int> > & indicesvec, std::vector<int> & counter);
  void SetOverlapCounter(const std::vector<int> & indices, int & counter);

  /////////////////////
  // Set Photon Sets //
  /////////////////////
  
  void SetPhotonSets();
  void SetPhotonSets(PhotonInfo & basePhotonInfo, const PhotonInfo & testPhotonInfo);

  /////////////////////////////
  // Set Photon Set Counters //
  /////////////////////////////

  void SetPhotonSetCounters();
  void SetPhotonSetCounters(PhotonInfo & photonInfo);

  //////////////////
  // Set MET Info //
  //////////////////

  void SetMETInfo();
  void SetCorrectedMET(const IDType ID, float & ootMETpt, float & ootMETphi);

  ////////////////////
  // Reset Counters //
  ////////////////////

  void ResetCounters();
  void ResetCounters(PhotonInfo & photonInfo);
  void ResetCounter(std::vector<int> & counter);
  void ResetCounter(int & counter);

  ///////////////////
  // Reset Indices //
  ///////////////////

  void ResetOverlapIndices();
  void ResetOverlapIndices(PhotonInfo & photonInfo);
  void ResetOverlapIndices(std::vector<std::vector<int> > & indices, const int size);
  void ResetOverlapIndices(std::vector<int> & indices, const int size);

  ///////////////////////
  // Reset Photon Sets //
  ///////////////////////

  void ResetPhotonSets();
  void ResetPhotonSets(PhotonInfo & photonInfo);
  void ResetPhotonSet(std::unordered_set<int> & indices);

  /////////////////////
  // DEBUG FUNCTIONS //
  /////////////////////

  void DumpPhotons(const edm::Event & iEvent);
  void DumpPhotons(const PhotonInfo & basePhotonInfo, const std::vector<pat::Photon> & testPhotons);
  void DumpPhotons(const int ibase, const pat::Photon & basePhoton,
		   const std::vector<pat::Photon> & basePhotons,
		   const std::vector<pat::Photon> & testPhotons,
		   const MatchingInfo & baseMatchingInfo);
  void DumpPhotons(const pat::Photon & basePhoton, const std::vector<pat::Photon> & testPhotons, 
		   const std::vector<int> & indices, const std::string & text);
  void DumpPhoton(const int i, const pat::Photon & photon, const std::string & prefix, const std::string & suffix);

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
  const float pTmin;

  // useOOTVID
  const bool useGEDVID;
  const bool useOOTVID;

  // debug config
  const bool debug;

  // MC config
  const bool isMC;

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

  //////////////////////
  // Temp I/O Members //
  //////////////////////

  PhotonInfo gedInfo;
  PhotonInfo ootInfo;

  ////////////////////
  // Output Members //
  ////////////////////

  // tree
  TTree * tree;
 
  // MET
  float t1pfMETpt, t1pfMETphi;
  float genMETpt, genMETphi;
  float ootMETpt_N, ootMETphi_N;
  float ootMETpt_L, ootMETphi_L;
  float ootMETpt_T, ootMETphi_T;
};

#endif
