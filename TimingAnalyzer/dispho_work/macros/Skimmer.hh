#ifndef _skimmer_
#define _skimmer_ 

#include "SkimmerTypes.hh"
#include "CommonTypes.hh"
#include "Config.hh"
#include "CommonUtils.hh"

#include "TTree.h"
#include "TBranch.h"

class Skimmer 
{
public:
  // functions
  Skimmer(const TString & dir);
  ~Skimmer();
  void InitTree();
  void InitStructs();
  void InitBranchVecs();
  void InitBranches();
  void InitAndReadConfigTree();
  void InitConfigStrings();
  void InitConfigBranches();
  void EventLoop();

private:
  // I/O
  const TString fDir;

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

  Configuration fInconfig;

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

  Configuration fOutconfig;
};

#endif
