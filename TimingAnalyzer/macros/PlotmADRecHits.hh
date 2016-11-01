#ifndef _plotmadrechits_
#define _plotmadrechits_

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

typedef std::map<TString,Int_t> VIDMap;

class PlotmADRecHits 
{
public :
  PlotmADRecHits(TString filename, TString outdir = "output", 
		 Bool_t applhltcut = false, Bool_t applyphptcut = false, Float_t phptcut = 100.f,
		 Bool_t applyrhecut = false, Float_t rhEcut = 1.f, Bool_t applyecalacceptcut = false,
		 Bool_t applyvidcut = false, TString VIDcut = "medium");
  ~PlotmADRecHits();
  void InitTree();
  void DoPlots();
  void SetupPlots();
  void SetupRecoPhotons();
  TH1F * MakeTH1F(TString hname, TString htitle, Int_t nbinsx, Float_t xlow, Float_t xhigh, TString xtitle, TString ytitle, TString subdir);
  TH2F * MakeTH2F(TString hname, TString htitle, Int_t nbinsx, Float_t xlow, Float_t xhigh, TString xtitle, Int_t nbinsy, Float_t ylow, Float_t yhigh, TString ytitle, TString subdir);
  void PhotonLoop();
  void FillPatPhotons();
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
  const Bool_t  fApplyHLTCut;
  const Bool_t  fApplyPhPtCut;
  const Float_t fPhPtCut;
  const Bool_t  fApplyrhECut;
  const Float_t frhECut;
  const Bool_t  fApplyECALAcceptCut;
  const Bool_t  fApplyVIDCut;
  const TString fPhVID;
        VIDMap  fPhVIDMap;

  // Output vars
  TString fOutDir;
  TStrMap fSubDirs;
  TFile * fOutFile;

  Int_t   event;
  Int_t   run;
  Int_t   lumi;
  Bool_t  hltdoubleph60;
  Float_t phE;
  Float_t phpt;
  Float_t phphi;
  Float_t pheta;
  Int_t   phVID;
  Float_t phscE;
  Float_t phscphi;
  Float_t phsceta;
  Int_t   phnrhs;
  Int_t   phnrhs_add;
  vector<float> * phrhEs;
  vector<float> * phrhphis;
  vector<float> * phrhetas;
  vector<float> * phrhdelRs;
  vector<float> * phrhtimes;
  vector<int>   * phrhIDs;
  vector<int>   * phrhOOTs;
  Int_t   phseedpos;

  // List of branches
  TBranch * b_event;   //!
  TBranch * b_run;   //!
  TBranch * b_lumi;   //!
  TBranch * b_hltdoubleph60;   //!
  TBranch * b_phE;   //!
  TBranch * b_phpt;   //!
  TBranch * b_phphi;   //!
  TBranch * b_pheta;   //!
  TBranch * b_phVID;   //!
  TBranch * b_phscE;   //!
  TBranch * b_phscphi;   //!
  TBranch * b_phsceta;   //!
  TBranch * b_phnrhs;   //!
  TBranch * b_phnrhs_add;   //!
  TBranch * b_phrhEs;   //!
  TBranch * b_phrhphis;   //!
  TBranch * b_phrhetas;   //!
  TBranch * b_phrhdelRs;   //!
  TBranch * b_phrhtimes;   //!
  TBranch * b_phrhIDs;   //!
  TBranch * b_phrhOOTs;   //!
  TBranch * b_phseedpos;   //!
};
#endif
