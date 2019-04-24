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
#include <numeric>

class Skimmer 
{
public:
  // functions
  Skimmer(const TString & indir, const TString & outdir, const TString & filename, const TString & skimconfig);
  ~Skimmer();

  // skim config
  void SetupDefaults();
  void SetupSkimConfig();
  void SetupSkimType(const TString & skim_type);
  void SetupEnergyCorrection(const TString & str, ECorr & ecorr, const TString & text);

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
  void InitOutCutFlowHist(const TH1F * inh_cutflow, TH1F *& outh_cutflow, const TString & name);

  // skim and fill outputs
  void EventLoop();
  void FillOutGMSBs(const UInt_t entry);
  void FillOutHVDSs(const UInt_t entry);
  void FillOutToys(const UInt_t entry);
  void FillOutEvent(const UInt_t entry, const Float_t evtwgt);
  void FillOutJets(const UInt_t entry);
  void FillOutPhos(const UInt_t entry);

  // Correct MET
  void CorrectMET();

  // Reordering functions
  void ReorderJets();

  // Signal VID SFs
  void FillOutSFs();

  // helper functions
  void FillPhoListStandard();

private:
  // I/O
  const TString fInDir;
  const TString fOutDir;
  const TString fFileName;
  const TString fSkimConfig;

  // Config
  SkimType fSkim;
  Float_t fSumWgts;
  TString fPUWgtFileName;
  ECorr fJEC;
  ECorr fJER;
  ECorr fPhoSc;
  ECorr fPhoSm;

  std::map<std::string,int> cutLabels;
  Bool_t fIsMC;
  Float_t fNOutPhos;

  // Input
  TFile * fInFile;
  TTree * fInTree; 
  TTree * fInConfigTree;
  TH1F  * fInCutFlow;
  TH1F  * fInCutFlowWgt;
  TFile * fInPUWgtFile;
  TH1F  * fInPUWgtHist;
  Float_t fSampleWeight;
  std::vector<Float_t> fPUWeights;
  TFile * fInGEDSFFile;
  TH2F  * fInGEDSFHist;
  TFile * fInOOTSFFile;
  TH2F  * fInOOTSFHist;
  TFile * fInIsTrkSFFile;
  TH2F  * fInIsTrkSFHist;
  
  GmsbVec fInGMSBs;
  HvdsVec fInHVDSs;
  ToyVec  fInToys;
  Event   fInEvent;
  RecHits fInRecHits;
  Jet     fInJets;
  PhoVec  fInPhos;

  Configuration fInConfig;

  // timefit weight constants
  Float_t fTimeFitN;
  Float_t fTimeFitC;

  // list of photon indices
  std::vector<Int_t> fPhoList;
  
  // Output
  TFile * fOutFile;
  TTree * fOutTree; 
  TTree * fOutConfigTree;
  TH1F  * fOutCutFlow;
  TH1F  * fOutCutFlowWgt;
  TH1F  * fOutCutFlowScl;

  GmsbVec fOutGMSBs;
  HvdsVec fOutHVDSs;
  ToyVec  fOutToys;
  Event   fOutEvent;
  Jet     fOutJets;
  PhoVec  fOutPhos;

  Configuration fOutConfig;
};

#endif
