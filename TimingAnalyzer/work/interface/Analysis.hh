#ifndef _analysis_
#define _analysis_ 

#include "Config.hh"
#include "Common.hh"

#include "TH2F.h"
#include "TF1.h"

#include <utility>

typedef std::map<TString,TH1F*> TH1Map;
typedef TH1Map::iterator        TH1MapIter;

typedef std::map<TString,Int_t> TStrIntMap; // to be used as TH1BinMaps

typedef std::map<TString,TH2F*> TH2Map;
typedef TH2Map::iterator        TH2MapIter;

class Analysis {
public:
  // functions
  Analysis(TString sample, Bool_t isMC);
  ~Analysis();
  void InitTree();
  void StandardPlots();
  void TimeResPlots();
  void TriggerEffs();
  void Project2Dto1D(TH2F *& hist2d, TStrMap & subdir2dmap, TH1Map & th1map, TStrMap & subdir1dmap, TStrIntMap & th1binmap);
  void ProduceMeanSigma(TH1Map & th1map, TStrIntMap & th1binmap, TString name, TString xtitle, const DblVec vxbins, Float_t fitrange, TString subdir);
  TH1F * MakeTH1Plot(TString hname, TString htitle, Int_t nbins, Double_t xlow, Double_t xhigh, TString xtitle, TString ytitle, TStrMap& subdirmap, TString subdir);
  TH2F * MakeTH2Plot(TString hname, TString htitle, const DblVec vxbins, Int_t nbinsy, Double_t ylow, Double_t yhigh, TString xtitle, TString ytitle, TStrMap& subdirmap, TString subdir);
  void SaveTH1s(TH1Map & th1map, TStrMap & subdirmap);
  void SaveTH1andFit(TH1F *& hist, TString subdir, TF1 *& fit);
  void SaveTH2s(TH2Map & th2map, TStrMap & subdirmap);
  void DumpTH1Names(TH1Map & th1map, TStrMap & subdirmap);
  void DeleteTH1s(TH1Map & th1map);
  void DeleteTH2s(TH2Map & th2map);

private:
  // Input
  TFile * fInFile;
  TTree * fInTree;
  TString fSample;
  Bool_t  fIsMC;

  // PU weight input
  FltVec fPUweights;

  // Output
  TString fOutDir;
  TFile*  fOutFile;
  std::ofstream fTH1Dump; 
  
  // Output colors
  Color_t fColor;
  
public:
  // Declaration of leaf types
  Int_t           event;
  Int_t           run;
  Int_t           lumi;
  Bool_t          hltsingleel;
  Bool_t          hltdoubleel;
  Bool_t          hltelnoiso;
  Int_t           nvtx;
  Int_t           nvetoelectrons;
  Int_t           nlooseelectrons;
  Int_t           nmediumelectrons;
  Int_t           ntightelectrons;
  Int_t           nheepelectrons;
  Int_t           el1pid;
  Float_t         el1pt;
  Float_t         el1eta;
  Float_t         el1phi;
  Int_t           el2pid;
  Float_t         el2pt;
  Float_t         el2eta;
  Float_t         el2phi;
  Float_t         el1time;
  Float_t         el2time;
  Float_t         zmass;
  Float_t         zpt;
  Float_t         zeta;
  Float_t         zphi;
  Int_t           puobs;
  Int_t           putrue;
  Float_t         xsec;
  Float_t         wgt;
  Int_t           genzpid;
  Float_t         genzpt;
  Float_t         genzeta;
  Float_t         genzphi;
  Float_t         genzmass;
  Int_t           genel1pid;
  Float_t         genel1pt;
  Float_t         genel1eta;
  Float_t         genel1phi;
  Int_t           genel2pid;
  Float_t         genel2pt;
  Float_t         genel2eta;
  Float_t         genel2phi;
  Float_t         wgtsum;

  // List of branches
  TBranch        *b_event;   //!
  TBranch        *b_run;   //!
  TBranch        *b_lumi;   //!
  TBranch        *b_hltsingleel;   //!
  TBranch        *b_hltdoubleel;   //!
  TBranch        *b_hltelnoiso;   //!
  TBranch        *b_nvtx;   //!
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
  TBranch        *b_zmass;   //!
  TBranch        *b_zpt;   //!
  TBranch        *b_zeta;   //!
  TBranch        *b_zphi;   //!
  TBranch        *b_puobs;   //!
  TBranch        *b_putrue;   //!
  TBranch        *b_xsec;   //!
  TBranch        *b_wgt;   //!
  TBranch        *b_genzpid;   //!
  TBranch        *b_genzpt;   //!
  TBranch        *b_genzeta;   //!
  TBranch        *b_genzphi;   //!
  TBranch        *b_genzmass;   //!
  TBranch        *b_genel1pid;   //!
  TBranch        *b_genel1pt;   //!
  TBranch        *b_genel1eta;   //!
  TBranch        *b_genel1phi;   //!
  TBranch        *b_genel2pid;   //!
  TBranch        *b_genel2pt;   //!
  TBranch        *b_genel2eta;   //!
  TBranch        *b_genel2phi;   //!
  TBranch        *b_wgtsum;   //!
};

#endif
