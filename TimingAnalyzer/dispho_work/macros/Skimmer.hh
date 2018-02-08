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
  Skimmer(const TString & indir, const TString & outdir, const TString & filename, const Float_t sumwgts, const Bool_t redophoid = false);
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

  // helper functions for output
  Int_t GetGEDPhoVID(const Pho & outpho);
  Int_t GetOOTPhoVID(const Pho & outpho);
  Float_t GetChargedHadronEA(const Float_t eta);
  Float_t GetNeutralHadronEA(const Float_t eta); 
  Float_t GetGammaEA(const Float_t eta); 
  Float_t GetEcalPFClEA(const Float_t eta);
  Float_t GetHcalPFClEA(const Float_t eta);
  Float_t GetTrackEA(const Float_t eta);
  Float_t GetNeutralHadronPtScale(const Float_t eta, const Float_t pt);
  Float_t GetGammaPtScale(const Float_t eta, const Float_t pt);
  Float_t GetEcalPFClPtScale(const Float_t eta, const Float_t pt);
  Float_t GetHcalPFClPtScale(const Float_t eta, const Float_t pt);
  Float_t GetTrackPtScale(const Float_t eta, const Float_t pt);

private:
  // I/O
  const TString fInDir;
  const TString fOutDir;
  const TString fFileName;
  const Float_t fSumWgts;
  const Bool_t  fRedoPhoID;
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
  
  GmsbVec fInGMSBs;
  HvdsVec fInHVDSs;
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

  GmsbVec fOutGMSBs;
  HvdsVec fOutHVDSs;
  Event   fOutEvent;
  JetVec  fOutJets;
  PhoVec  fOutPhos;

  Configuration fOutConfig;
};

#endif
