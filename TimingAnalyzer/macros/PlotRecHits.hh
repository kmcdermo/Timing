#ifndef _plotrechits_
#define _plotrechits_

#include "TROOT.h"
#include "TFile.h"
#include "TTree.h"
#include "TString.h"
#include "TH1F.h"
#include "TH2F.h"

#include <vector>
#include <map>

typedef std::map<TString,TH1F*> TH1Map;
typedef TH1Map::iterator        TH1MapIter;

typedef std::map<TString,TH2F*> TH2Map;
typedef TH2Map::iterator        TH2MapIter;

typedef std::map<TString,TString> TStrMap;
typedef TStrMap::iterator         TStrMapIter;

typedef std::vector<TString> TStrVec;

class PlotRecHits 
{
public :
  PlotRecHits(TString filename, TString outdir = "output", Bool_t applyphptcut = false, Float_t phptcut = 100.f,
	      Bool_t applyrhecut = false, Float_t rhEcut = 1.f, Bool_t applyecalacceptcut = false);
  ~PlotRecHits();
  void InitTree();
  void DoPlots();
  void SetupPlots();
  void SetupRecoPhotons();
  TH1F * MakeTH1F(TString hname, TString htitle, Int_t nbinsx, Float_t xlow, Float_t xhigh, TString xtitle, TString ytitle, TString subdir);
  TH2F * MakeTH2F(TString hname, TString htitle, Int_t nbinsx, Float_t xlow, Float_t xhigh, TString xtitle, Int_t nbinsy, Float_t ylow, Float_t yhigh, TString ytitle, TString subdir);
  void PhotonLoop();
  void FillRecoPhotons();
  void MakeSubDirs();
  void OutputTH1Fs();
  void OutputTotalTH1Fs();
  void OutputTH2Fs();
  void ClearTH1Map();
  void ClearTH2Map();

private :
  // Input vars
  TFile * fInFile; //!pointer to file
  TTree * fInTree; //!pointer to the analyzed TTree

  // In routine vars
  UInt_t  fNPhCheck;
  TH1Map  fPlots;
  TH2Map  fPlots2D;
  TStrVec fTotalNames;

  // Config
  const Bool_t  fApplyPhPtCut;
  const Float_t fPhPtCut;
  const Bool_t  fApplyrhECut;
  const Float_t frhECut;
  const Bool_t  fApplyECALAcceptCut;

  // Output vars
  TString fOutDir;
  TString fOutDump;
  TStrMap fSubDirs;
  TFile * fOutFile;

  Int_t   event;
  Int_t   run;
  Int_t   lumi;
  Float_t phE;
  Float_t phpt;
  Float_t phphi;
  Float_t pheta;
  Float_t phscE;
  Float_t phscphi;
  Float_t phsceta;
  Int_t   phnfrhs;
  Int_t   phnfrhs_add;
  vector<float> * phfrhEs;
  vector<float> * phfrhphis;
  vector<float> * phfrhetas;
  vector<float> * phfrhdelRs;
  vector<float> * phfrhtimes;
  vector<int>   * phfrhIDs;
  vector<int>   * phfrhOOTs;
  Int_t   phfseedpos;
  Int_t   phnrrhs;
  Int_t   phnrrhs_add;
  vector<float> * phrrhEs;
  vector<float> * phrrhphis;
  vector<float> * phrrhetas;
  vector<float> * phrrhdelRs;
  vector<float> * phrrhtimes;
  vector<int>   * phrrhIDs;
  vector<int>   * phrrhOOTs;
  Int_t   phrseedpos;

  // List of branches
  TBranch * b_event;   //!
  TBranch * b_run;   //!
  TBranch * b_lumi;   //!
  TBranch * b_phE;   //!
  TBranch * b_phpt;   //!
  TBranch * b_phphi;   //!
  TBranch * b_pheta;   //!
  TBranch * b_phscE;   //!
  TBranch * b_phscphi;   //!
  TBranch * b_phsceta;   //!
  TBranch * b_phnfrhs;   //!
  TBranch * b_phnfrhs_add;   //!
  TBranch * b_phfrhEs;   //!
  TBranch * b_phfrhphis;   //!
  TBranch * b_phfrhetas;   //!
  TBranch * b_phfrhdelRs;   //!
  TBranch * b_phfrhtimes;   //!
  TBranch * b_phfrhIDs;   //!
  TBranch * b_phfrhOOTs;   //!
  TBranch * b_phfseedpos;   //!
  TBranch * b_phnrrhs;   //!
  TBranch * b_phnrrhs_add;   //!
  TBranch * b_phrrhEs;   //!
  TBranch * b_phrrhphis;   //!
  TBranch * b_phrrhetas;   //!
  TBranch * b_phrrhdelRs;   //!
  TBranch * b_phrrhtimes;   //!
  TBranch * b_phrrhIDs;   //!
  TBranch * b_phrrhOOTs;   //!
  TBranch * b_phrseedpos;   //!
};
#endif
