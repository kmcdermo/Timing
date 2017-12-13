#ifndef _skimmer_
#define _skimmer_ 

#include "SkimmerTypes.hh"
#include "common/Common.hh"

#include "TTree.h"
#include "TFile.h"

#include <vector>
#include <map>

class Skimmer 
{
public:
  // functions
  Skimmer(const TString & indir, const TString & outdir, const TString & filename, const Float_t sumwgts);
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

  // setup gen inputs
  void GetSampleWeight();
  void GetPUWeights();

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
  const TString fInDir;
  const TString fOutDir;
  const TString fFileName;
  const Float_t fSumWgts;
  std::map<std::string,int> cutLabels;
  Bool_t fIsMC;
  
  // Input
  TFile * fInFile;
  TTree * fInTree; 
  TTree * fInConfigTree;
  TH1F  * fInCutFlow;
  TFile * fInPUWgtFile;
  TH1F  * fInPUWgtHist;
  Float_t fSampleWeight;
  std::vector<Float_t> fPUWeights;
  
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
