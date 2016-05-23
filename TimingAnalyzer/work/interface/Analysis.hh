#ifndef _analysis_
#define _analysis_ 

#include "TROOT.h"
#include "TChain.h"
#include "TTree.h"
#include "TFile.h"
#include "TString.h"
#include "TStyle.h"
#include "TH1F.h"
#include "TH2F.h"

#include <map>
#include <vector>

typedef std::map<TString,TH1F*> TH1Map;
typedef TH1Map::iterator TH1MapIter;

typedef std::map<TString,TH2F*> TH2Map;
typedef TH2Map::iterator TH2MapIter;

typedef std::map<TString,TString> TStrMap;
typedef TStrMap::iterator TStrMapIter;

class Analysis {
public:
  // functions
  Analysis(TString filename, TString outdir, TString outtype, Float_t lumi);
  ~Analysis();
  void InitTree();
  void StandardPlots();
  void TimeResPlots();
  void TriggerEffs();
  void ComputeRatioPlot(const TH1F * numer, const TH1F * denom, TH1F *& ratioPlot);
  TH1F * MakeTH1Plot(TString hname, TString htitle, Int_t nbins, Double_t xlow, Double_t xhigh, TString xtitle, TString ytitle, TStrMap& subdirmap, TString subdir);
  TH2F * MakeTH2Plot(TString hname, TString htitle, Int_t nbinsx, Double_t xlow, Double_t xhigh, Int_t nbinsy, Double_t ylow, Double_t yhigh, TString xtitle, TString ytitle, TStrMap& subdirmap, TString subdir);
  TH2F * MakeTH2Plot(TString hname, TString htitle, std::vector<Double_t> vxbins, Int_t nbinsy, Double_t ylow, Double_t yhigh, TString xtitle, TString ytitle, TStrMap& subdirmap, TString subdir);
  void MakeSubDirs(const TStrMap & subdirmap);
  void SaveTH1s(TH1Map & th1map, TStrMap & subdirmap);
  void SaveTH2s(TH2Map & th2map, TStrMap & subdirmap);
  void DeleteTH1s(TH1Map & th1map);
  void DeleteTH2s(TH2Map & th2map);

private:
  // I/O
  TFile* fInFile;
  TTree* fInTree;

  TString fOutDir;
  TFile*  fOutFile;
  TString fOutType;
  
  // CMS demands this...
  TStyle* fTDRStyle;
  Float_t fLumi;

public:
  // Declaration of leaf types
  UInt_t          event;
  UInt_t          run;
  UInt_t          lumi;
  Double_t        xsec;
  Double_t        wgt;
  Double_t        puwgt;
  Int_t           puobs;
  Int_t           putrue;
  UInt_t          nvtx;
  UChar_t         hltsingleel;
  UChar_t         hltdoubleel;
  UChar_t         hltelnoiso;
  UInt_t          nvetoelectrons;
  UInt_t          nlooseelectrons;
  UInt_t          nmediumelectrons;
  UInt_t          ntightelectrons;
  UInt_t          nheepelectrons;
  Int_t           el1pid;
  Double_t        el1pt;
  Double_t        el1eta;
  Double_t        el1phi;
  Int_t           el2pid;
  Double_t        el2pt;
  Double_t        el2eta;
  Double_t        el2phi;
  Double_t        el1time;
  Double_t        el2time;
  Double_t        zeemass;
  Double_t        zeept;
  Double_t        zeeeta;
  Double_t        zeephi;

  // List of branches
  TBranch        *b_event;   //!
  TBranch        *b_run;   //!
  TBranch        *b_lumi;   //!
  TBranch        *b_xsec;   //!
  TBranch        *b_wgt;   //!
  TBranch        *b_puwgt;   //!
  TBranch        *b_puobs;   //!
  TBranch        *b_putrue;   //!
  TBranch        *b_nvtx;   //!
  TBranch        *b_hltsingleel;   //!
  TBranch        *b_hltdoubleel;   //!
  TBranch        *b_hltelnoiso;   //!
  TBranch        *b_nvetoelectrons;   //!
  TBranch        *b_nlooseelectrons;   //!
  TBranch        *b_nmediumelectrons;   //!
  TBranch        *b_ntightelectrons;   //!
  TBranch        *b_nheepelectrons;   //!
  TBranch        *b_el1pid;   //!
  TBranch        *b_el1pt;   //!
  TBranch        *b_el1eta;   //!
  TBranch        *b_el1phi;   //!
  TBranch        *b_el2pid;   //!
  TBranch        *b_el2pt;   //!
  TBranch        *b_el2eta;   //!
  TBranch        *b_el2phi;   //!
  TBranch        *b_el1time;   //!
  TBranch        *b_el2time;   //!
  TBranch        *b_zeemass;   //!
  TBranch        *b_zeept;   //!
  TBranch        *b_zeeeta;   //!
  TBranch        *b_zeephi;   //!
};

#endif
