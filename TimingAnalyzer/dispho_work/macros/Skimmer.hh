#ifndef _skimmer_
#define _skimmer_ 

#include "SkimmerTypes.hh"
#include "Common.hh"

#include "TTree.h"
#include "TFile.h"
#include "TLorentzVector.h"

#include <vector>
#include <map>
#include <cmath>

class Skimmer 
{
public:
  // functions
  Skimmer(const TString & indir, const TString & outdir, const TString & filename, 
	  const Float_t sumwgts, const TString & puwgtfilename = "", const TString & skimtype = "Standard");
  ~Skimmer();

  // setup skim type
  void SetSkim();

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
  void InitOutStructs();
  void InitOutBranches();
  void InitOutCutFlowHists();
  void InitOutCutFlowHist(const TH1F * inh_cutflow, TH1F *& outh_cutflow, const TString & label);

  // skim and fill outputs
  void EventLoop();
  void FillOutGMSBs(const UInt_t entry);
  void FillOutHVDSs(const UInt_t entry);
  void FillOutToys(const UInt_t entry);
  void FillOutEvent(const UInt_t entry, const Float_t evtwgt);
  void FillOutJets(const UInt_t entry);
  void FillOutPhos(const UInt_t entry);

private:
  // I/O
  const TString fInDir;
  const TString fOutDir;
  const TString fFileName;
  const Float_t fSumWgts;
  const TString fPUWgtFileName;
  const TString fSkimType;
  std::map<std::string,int> cutLabels;
  Bool_t fIsMC;

  // Input
  SkimEnum fSkim;
  TFile * fInFile;
  TTree * fInTree; 
  TTree * fInConfigTree;
  TH1F  * fInCutFlow;
  //  TH1F  * fInCutFlowWgt;
  TFile * fInPUWgtFile;
  TH1F  * fInPUWgtHist;
  Float_t fSampleWeight;
  std::vector<Float_t> fPUWeights;
  
  GmsbVec fInGMSBs;
  HvdsVec fInHVDSs;
  ToyVec  fInToys;
  Event   fInEvent;
  RecHits fInRecHits;
  Jet     fInJets;
  PhoVec  fInPhos;

  Configuration fInConfig;

  // list of photon indices
  std::vector<Int_t> fPhoList;
  
  // Output
  TFile * fOutFile;
  TTree * fOutTree; 
  TTree * fOutConfigTree;
  TH1F  * fOutCutFlow;
  // TH1F  * fOutCutFlowWgt;
  // TH1F  * fOutCutFlowScl;

  GmsbVec fOutGMSBs;
  HvdsVec fOutHVDSs;
  ToyVec  fOutToys;
  Event   fOutEvent;
  Jet     fOutJets;
  PhoVec  fOutPhos;

  Configuration fOutConfig;
};

#endif
