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

// Gen Info
#include "SimDataFormats/PileupSummaryInfo/interface/PileupSummaryInfo.h"
#include "SimDataFormats/GeneratorProducts/interface/GenEventInfoProduct.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"

// DataFormats
#include "DataFormats/PatCandidates/interface/PackedCandidate.h"
#include "DataFormats/PatCandidates/interface/Photon.h"
#include "DataFormats/PatCandidates/interface/MET.h"

// ROOT
#include "TH1F.h"
#include "TTree.h"

// standard includes
#include <cmath>
#include <iostream>
#include <string>
#include <vector>

// Common Utilities
#include "Timing/TimingAnalyzer/plugins/CommonUtils.hh"

////////////////////////
// GED/OOT Photon Idx //
////////////////////////

struct ReducedPhoton
{
  ReducedPhoton() {}
  ReducedPhoton(const int idx, const bool isGED) : idx(idx), isGED(isGED) {}

  int idx;
  bool isGED;
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

  //////////////////////////
  // Event Prep Functions //
  //////////////////////////

  void GetObjects(const edm::Event & iEvent);
  void PrepObjects();
  void InitializeObjects();

  void PrepPhotonCollections();
  void PrepPhotonCollection(std::vector<ReducedPhoton> & reducedPhotons, const std::vector<int> & matchedOOT, const std::vector<int> & matchedLTGED);

  ////////////////////
  // Main Functions //
  ////////////////////
  
  void SetEventInfo();

  void SetBasicCounters();

  void SetPhotonIndices();
  void SetPhotonIndices(const std::string & gedVID, const std::string & ootVID, std::vector<int> & matchedOOT,
			std::vector<int> & matchedGTGED, std::vector<int> & matchedLTGED,
			std::vector<int> & unmatchedGED, std::vector<int> & matchedCands);

  void SetPhotonCounters();
  void SetPhotonCounter(const std::vector<int> & indices, int & counter);

  void SetPhotonPhis(const std::vector<int> & matchedGTGED, const std::vector<int> & matchedLTGED, const std::vector<int> & unmatchedGED, 
		     std::vector<float> & matchedGTGEDphi, std::vector<float> & unmatchedGEDphi);
  void SetPhotonPhis();

  void SetMETInfo();
  void SetCorrectedMET(const std::vector<int> & matchedGTGED, const std::vector<int> & matchedLTGED,
		       const std::vector<int> & unmatchedGED, float & ootMETpt, float & ootMETphi);

  void SetMCInfo();

  void SetEResiduals();
  void SetEResiduals(const std::vector<ReducedPhoton> & reducedPhotons,
		     std::vector<float> & beforeGEDEres, std::vector<float> & afterGEDEres,
		     std::vector<float> & beforeOOTEres, std::vector<float> & afterOOTEres);

  //////////////////////
  // Helper Functions //
  //////////////////////

  void ResetCounters();
  void ResetCounter(int & counter);

  void ResetPhotonIndices();
  void ResetPhotonIndices(std::vector<int> & indices, const int size);

  void ResetPhotonPhis();
  void ResetEResiduals();
  void ResetPhotonVars(std::vector<float> & vars);

  void ResetPhotonCollections();
  void ResetPhotonCollection(std::vector<ReducedPhoton> & reducedPhotons);

  /////////////////////
  // DEBUG FUNCTIONS //
  /////////////////////

  void DumpPhotons(const edm::Event & iEvent);
  void DumpPhotons(const std::string & group, const std::vector<int> & matchedOOT,
		   const std::vector<int> & matchedGTGED, const std::vector<int> & matchedLTGED,
		   const std::vector<int> & unmatchedGED);
  void DumpPhotons(const std::vector<pat::Photon> & photons, const int size, 
		   const std::vector<int> & indices, const bool check, const std::string & label,
		   const std::vector<pat::Photon> & refPhotons);
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
  const float pTres;

  // useGEDVID
  const bool useGEDVID;

  // debug config
  const bool debug;

  // cands
  const edm::InputTag candsTag;
  edm::EDGetTokenT<std::vector<pat::PackedCandidate> > candsToken;
  edm::Handle<std::vector<pat::PackedCandidate> > candsH;

  // mets
  const edm::InputTag metsTag;
  edm::EDGetTokenT<std::vector<pat::MET> > metsToken;
  edm::Handle<std::vector<pat::MET> > metsH;

  // gedPhotons
  const edm::InputTag gedPhotonsTag;
  edm::EDGetTokenT<std::vector<pat::Photon> > gedPhotonsToken;
  edm::Handle<std::vector<pat::Photon> > gedPhotonsH;
  std::vector<pat::Photon> gedPhotons;

  // ootPhotons
  const edm::InputTag ootPhotonsTag;
  edm::EDGetTokenT<std::vector<pat::Photon> > ootPhotonsToken;
  edm::Handle<std::vector<pat::Photon> > ootPhotonsH;
  std::vector<pat::Photon> ootPhotons;

  // MC config --> also output!
  const bool isMC;
  float xsec;
  float BR;

  // gen evt record
  const edm::InputTag genEvtInfoTag;
  edm::EDGetTokenT<GenEventInfoProduct> genEvtInfoToken;
  edm::Handle<GenEventInfoProduct> genEvtInfoH;

  // pileup info
  const edm::InputTag pileupInfosTag;
  edm::EDGetTokenT<std::vector<PileupSummaryInfo> > pileupInfosToken;
  edm::Handle<std::vector<PileupSummaryInfo> > pileupInfosH;

  // gen particles
  const edm::InputTag genParticlesTag;
  edm::EDGetTokenT<std::vector<reco::GenParticle> > genParticlesToken;
  edm::Handle<std::vector<reco::GenParticle> > genParticlesH;

  ///////////////////////////
  // Temp Internal Members //
  ///////////////////////////

  // photon indices
  std::vector<int> matchedOOT_N, matchedGTGED_N, matchedLTGED_N, unmatchedGED_N, matchedCands_N;
  std::vector<int> matchedOOT_L, matchedGTGED_L, matchedLTGED_L, unmatchedGED_L, matchedCands_L;
  std::vector<int> matchedOOT_T, matchedGTGED_T, matchedLTGED_T, unmatchedGED_T, matchedCands_T;

  // neutralinos
  std::vector<reco::GenParticle> neutralinos;

  // reduced photon index collections
  std::vector<ReducedPhoton> reducedPhotons_N;
  std::vector<ReducedPhoton> reducedPhotons_L;
  std::vector<ReducedPhoton> reducedPhotons_T;

  ////////////////////
  // Output Members //
  ////////////////////

  // tree
  TTree * tree;
 
  // counters
  int nGED_N, nOOT_N, nOOT_matchedGTGED_N, nOOT_matchedLTGED_N, nOOT_unmatchedGED_N, nOOT_matchedCands_N;
  int nGED_L, nOOT_L, nOOT_matchedGTGED_L, nOOT_matchedLTGED_L, nOOT_unmatchedGED_L, nOOT_matchedCands_L;
  int nGED_T, nOOT_T, nOOT_matchedGTGED_T, nOOT_matchedLTGED_T, nOOT_unmatchedGED_T, nOOT_matchedCands_T;

  // photon phis
  std::vector<float> matchedGTGEDphi_N, unmatchedGEDphi_N;
  std::vector<float> matchedGTGEDphi_L, unmatchedGEDphi_L;
  std::vector<float> matchedGTGEDphi_T, unmatchedGEDphi_T;

  // MET
  float t1pfMETpt, t1pfMETphi;
  float genMETpt, genMETphi;
  float ootMETpt_N, ootMETphi_N;
  float ootMETpt_L, ootMETphi_L;
  float ootMETpt_T, ootMETphi_T;

  // MC info
  float genwgt;
  int genputrue;

  // E residuals
  std::vector<float> beforeGEDEres_N, afterGEDEres_N, beforeOOTEres_N, afterOOTEres_N;
  std::vector<float> beforeGEDEres_L, afterGEDEres_L, beforeOOTEres_L, afterOOTEres_L;
  std::vector<float> beforeGEDEres_T, afterGEDEres_T, beforeOOTEres_T, afterOOTEres_T;
};

#endif
