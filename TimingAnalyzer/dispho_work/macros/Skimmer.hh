#ifndef _skimmer_
#define _skimmer_ 

#include "SkimmerTypes.hh"
#include "CommonTypes.hh"
#include "Config.hh"
#include "CommonUtils.hh"

#include "TTree.h"
#include "TBranch.h"

namespace Config
{
  constexpr Int_t nEvCheck = 10000;
  constexpr Int_t nGMSBs = 2;
  constexpr Int_t nHVDSs = 4;
  constexpr Int_t nJets = 4;
  constexpr Int_t nPhotons = 4;
};

class Skimmer 
{
public:
  // functions
  Skimmer(const TString & dir);
  ~Skimmer();

  // setup config inputs
  void GetInConfig();
  void InitInConfigStrings();
  void InitInConfigBranches();

  // setup tree inputs
  void InitInTree();
  void InitInStructs();
  void InitInBranchVecs();
  void InitInBranches();

  // setup outputs
  void InitAndSetOutConfig();
  void InitOutTree();
  void InitOutCutFlow();

  // skim and fill outputs
  void EventLoop();
  void FillOutGMSBs();
  void FillOutHVDSs();
  void FillOutEvent();
  void FillOutJets();
  void FillOutPhos();

private:
  // I/O
  const TString fFile;
  std::map<std::string> cutLabels;
  Bool_t fIsMC;

  // Input
  TFile * fInFile;
  TTree * fInTree; 
  TTree * fInConfigTree;
  TH1F  * fCutFlow;

  GMSBVec fInGMSBs;
  HVDSVec fInHVDSs;
  Event   fInEvent;
  RecHits fInRecHits;
  JetVec  fInJets;
  PhoVec  fInPhos;

  Configuration fInConfig;
  
  // Output
  TFile * fOutFile;
  TTree * fOutTree; 
  TTree * fOutConfigTree;
  TH1F  * fOutCutFlow;

  GMSBVec fOutGMSBs;
  HVDSVec fOutHVDSs;
  Event   fOutEvent;
  JetVec  fOutJets;
  PhoVec  fOutPhos;

  Configuration fOutConfig;
};

#endif
