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
  void PrepPhotonP4();
  void PrepPhotonP4(std::vector<pat::Photon> & photons);

  ////////////////////
  // Main Functions //
  ////////////////////

  void SetMCInfo();

  void SetBasicCounters();

  void SetPhotonIndices();
  void SetPhotonIndices(const std::string & gedVID, const std::string & ootVID,
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

  //////////////////////
  // Helper Functions //
  //////////////////////

  inline bool isOOT_GT_GED(const pat::Photon & gedPhoton, const pat::Photon & ootPhoton);

  void ResetCounters();
  void ResetCounter(int & counter);

  void ResetPhotonIndices();
  void ResetPhotonIndices(std::vector<int> & indices);

  void ResetPhotonPhis();
  void ResetPhotonPhis(std::vector<float> & phis);

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

  // cands
  const edm::InputTag candsTag;
  edm::EDGetTokenT<std::vector<pat::PackedCandidate> > candsToken;
  edm::Handle<std::vector<pat::PackedCandidate> > candsH;
  std::vector<pat::PackedCandidate> cands;

  // mets
  const edm::InputTag metsTag;
  edm::EDGetTokenT<std::vector<pat::MET> > metsToken;
  edm::Handle<std::vector<pat::MET> > metsH;
  std::vector<pat::MET> mets;

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

  // gen evt record
  const edm::InputTag genevtInfoTag;
  edm::EDGetTokenT<GenEventInfoProduct> genevtInfoToken;
  edm::Handle<GenEventInfoProduct> genevtInfoH;
  GenEventInfoProduct genevtInfo;

  // pileup info
  const edm::InputTag pileupInfosTag;
  edm::EDGetTokenT<std::vector<PileupSummaryInfo> > pileupInfosToken;
  edm::Handle<std::vector<PileupSummaryInfo> > pileupInfosH;
  std::vector<PileupSummaryInfo> pileupInfos;

  // gen particles
  const edm::InputTag genpartsTag;
  edm::EDGetTokenT<std::vector<reco::GenParticle> > genpartsToken;
  edm::Handle<std::vector<reco::GenParticle> > genparticlesH;
  std::vector<reco::GenParticle> genparticles;

  // MC config --> also output!
  const bool isMC;
  float xsec;
  float BR;

  ///////////////////////////
  // Temp Internal Members //
  ///////////////////////////

  // photon indices
  std::vector<int> matchedGTGED_N, matchedLTGED_N, unmatchedGED_N, matchedCands_N;
  std::vector<int> matchedGTGED_L, matchedLTGED_L, unmatchedGED_L, matchedCands_L;
  std::vector<int> matchedGTGED_T, matchedLTGED_T, unmatchedGED_T, matchedCands_T;

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
};

#endif
