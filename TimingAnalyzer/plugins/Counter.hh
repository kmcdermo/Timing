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

class Counter : public edm::one::EDAnalyzer<edm::one::SharedResources,edm::one::WatchRuns> 
{
 public:
  explicit Counter(const edm::ParameterSet&);
  ~Counter();

  void SetMCInfo();

  void SetBasicCounters();
  void ResetCounters();
  bool isOOT_GT_GED(const pat::Photon & gedPhoton, const pat::Photon & ootPhoton);
  void CountPhotons(const std::string & gedVID, const std::string & ootVID,
		    int & matchedGTGED, int & matchedLTGED,
		    int & unmatchedGED, int & matchedCands);

  void SetMETInfo();
  void METCorrection(const std::string & gedVID, const std::string & ootVID,
		     float & newMETpt, float & newMETphi);

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);
  
 private:
  virtual void beginJob() override;
  virtual void analyze(const edm::Event&, const edm::EventSetup&) override;
  virtual void endJob() override;
  
  virtual void beginRun(edm::Run const&, edm::EventSetup const&) override;
  virtual void endRun(edm::Run const&, edm::EventSetup const&) override;

  const float dRmin;
  const float pTmin;
  const float pTres;

  // cands
  const edm::InputTag candsTag;
  edm::EDGetTokenT<std::vector<pat::PackedCandidate> > candsToken;
  edm::Handle<std::vector<pat::PackedCandidate> > candsH;

  // mets
  const edm::InputTag metsTag;
  edm::EDGetTokenT<std::vector<pat::MET> > metsToken;
  edm::Handle<std::vector<pat::MET> > metsH;

  // gedPhotons + ids
  const edm::InputTag gedPhotonsTag;
  edm::EDGetTokenT<std::vector<pat::Photon> > gedPhotonsToken;
  edm::Handle<std::vector<pat::Photon> > gedPhotonsH;

  // ootPhotons + ids
  const edm::InputTag ootPhotonsTag;
  edm::EDGetTokenT<std::vector<pat::Photon> > ootPhotonsToken;
  edm::Handle<std::vector<pat::Photon> > ootPhotonsH;

  // Gen Particles and MC info
  const edm::InputTag genevtInfoTag;
  edm::EDGetTokenT<GenEventInfoProduct> genevtInfoToken;
  edm::Handle<GenEventInfoProduct> genevtInfoH;

  const edm::InputTag pileupInfoTag;
  edm::EDGetTokenT<std::vector<PileupSummaryInfo> > pileupInfoToken;
  edm::Handle<std::vector<PileupSummaryInfo> > pileupInfoH;

  const edm::InputTag genpartsTag;
  edm::EDGetTokenT<std::vector<reco::GenParticle> > genpartsToken;
  edm::Handle<std::vector<reco::GenParticle> > genparticlesH;

  // MC config
  const bool isMC;
  float xsec;
  float BR;

  // output event level ntuple
  TTree * tree;
 
  // MC info
  float genwgt;
  int genputrue;

  // counters
  int nGED;
  int nGED_T;
  int nGED_L;

  int nOOT;
  int nOOT_L;
  int nOOT_T;

  int nOOT_matchedGTGED;
  int nOOT_L_matchedGTGED;
  int nOOT_T_matchedGTGED;

  int nOOT_matchedLTGED;
  int nOOT_L_matchedLTGED;
  int nOOT_T_matchedLTGED;

  int nOOT_unmatchedGED;
  int nOOT_L_unmatchedGED;
  int nOOT_T_unmatchedGED;

  int nOOT_matchedCands;
  int nOOT_L_matchedCands;
  int nOOT_T_matchedCands;

  // MET
  float t1pfMETpt, t1pfMETphi;
  float genMETpt, genMETphi;
  float ootMETpt, ootMETphi;
  float ootMETpt_L, ootMETphi_L;
  float ootMETpt_T, ootMETphi_T;
};

#endif
