#ifndef _analysis_
#define _analysis_ 

#include "Config.hh"
#include "Common.hh"

#include "TH2F.h"
#include "TF1.h"

#include <utility>

typedef std::map<TString,TH1F*> TH1Map;
typedef TH1Map::iterator        TH1MapIter;

typedef std::map<TString,TH2F*> TH2Map;
typedef TH2Map::iterator        TH2MapIter;

class Analysis {
public:
  // functions
  Analysis(TString sample, Bool_t isMC);
  ~Analysis();
  void InitTree();
  void EventLoop();
  void SetupStandardPlots();
  void SetupZPlots();
  void SetupEffEPlots();
  void SetupNvtxPlots();
  void SetupEtaPlots();
  void SetupVtxZPlots();
  void SetupSingleEPlots();
  void SetupRunPlots();
  void SetupTrigEffPlots();
  void FillStandardPlots(const Float_t weight, const Float_t timediff, const Float_t effE, const Float_t effseedE, 
			 const Float_t el1time, const Float_t el1seedeta, Bool_t el1eb, Bool_t el1ee, Bool_t el1ep, Bool_t el1em, const FFPairVec & el1rhetpairs,
			 const Float_t el2time, const Float_t el2seedeta, Bool_t el2eb, Bool_t el2ee, Bool_t el2ep, Bool_t el2em, const FFPairVec & el2rhetpairs);
  void FillZPlots(const Float_t weight, const Float_t timediff);
  void FillEffEPlots(const Float_t weight, const Float_t timediff, const Float_t effE, const Float_t effseedE, 
		     Bool_t el1eb, Bool_t el1ee, Bool_t el1ep, Bool_t el1em, Bool_t el2eb, Bool_t el2ee, Bool_t el2ep, Bool_t el2em);
  void FillNvtxPlots(const Float_t weight, const Float_t timediff, const Float_t el1time, const Float_t el2time,
		     Bool_t el1eb, Bool_t el1ee, Bool_t el1ep, Bool_t el1em, Bool_t el2eb, Bool_t el2ee, Bool_t el2ep, Bool_t el2em);
  void FillEtaPlots(const Float_t weight, const Float_t timediff, const Float_t el1time, const Float_t el2time, const Float_t el1seedeta, const Float_t el2seedeta,
		    Bool_t el1eb, Bool_t el1ee, Bool_t el1ep, Bool_t el1em, Bool_t el2eb, Bool_t el2ee, Bool_t el2ep, Bool_t el2em);
  void FillVtxZPlots(const Float_t weight, const Float_t timediff, const Float_t el1time, const Float_t el2time);
  void FillSingleEPlots(const Float_t weight, const Float_t el1time, const Float_t el2time,
			Bool_t el1eb, Bool_t el1ee, Bool_t el1ep, Bool_t el1em, Bool_t el2eb, Bool_t el2ee, Bool_t el2ep, Bool_t el2em);
  void FillRunPlots(const Float_t weight, const Float_t timediff, Bool_t el1eb, Bool_t el1ee, Bool_t el2eb, Bool_t el2ee);
  void FillTrigEffPlots(const Float_t weight);
  void OutputStandardPlots();
  void OutputZPlots();
  void OutputEffEPlots();
  void OutputNvtxPlots();
  void OutputEtaPlots();
  void OutputVtxZPlots();
  void OutputSingleEPlots();
  void OutputRunPlots();
  void OutputTrigEffPlots();
  void Make1DTimingPlots(TH2F *& hist2D, const TString subdir2D, const DblVec& bins2D, TString name);
  void Project2Dto1D(TH2F *& hist2d, TString subdir2d, TH1Map & th1map, TStrMap & subdir1dmap, TStrIntMap & th1binmap);
  void ProduceMeanSigma(TH1Map & th1map, TStrIntMap & th1binmap, TString name, TString xtitle, const DblVec vxbins, TString subdir);
  void PrepFit(TF1 *& fit, TH1F *& hist);
  void GetMeanSigma(TF1 *& fit, Float_t & mean, Float_t & emean, Float_t & sigma, Float_t & esigma); 
  void DrawSubComp(TF1 *& fit, TCanvas *& canv, TF1 *& sub1, TF1 *& sub2);
  TH1F * MakeTH1Plot(TString hname, TString htitle, Int_t nbins, Double_t xlow, Double_t xhigh, TString xtitle, TString ytitle, TStrMap& subdirmap, TString subdir);
  TH2F * MakeTH2Plot(TString hname, TString htitle, const DblVec& vxbins, Int_t nbinsy, Double_t ylow, Double_t yhigh, TString xtitle, TString ytitle, TStrMap& subdirmap, TString subdir);
  void FillHistFromPairVecFirst (TH1F *& hist, const FFPairVec & pairvec);
  void FillHistFromPairVecSecond(TH1F *& hist, const FFPairVec & pairvec);
  void SaveTH1s(TH1Map & th1map, TStrMap & subdirmap);
  void SaveTH1andFit(TH1F *& hist, TString subdir, TF1 *& fit);
  void SaveTH2s(TH2Map & th2map, TStrMap & subdirmap);
  void DumpTH1Names(TH1Map & th1map, TStrMap & subdirmap);
  void DeleteFit(TF1 *& fit, TF1 *& sub1, TF1 *& sub2);
  void DeleteTH1s(TH1Map & th1map);
  void DeleteTH2s(TH2Map & th2map);

private:
  // Input
  TFile * fInFile;
  TTree * fInTree;
  TString fSample;
  Bool_t  fIsMC;

  // MC weight input
  //  FltVec  fPUweights;
  DblVec  fPUweights;
  Float_t fXsec;
  Float_t fWgtsum;

  // Output
  TString fOutDir;
  TFile*  fOutFile;
  std::ofstream fTH1Dump; 
  
  // Output colors
  Color_t fColor;

  ////////////////////////
  // Standard plot maps //
  ////////////////////////
  TH1Map standardTH1Map; TStrMap standardTH1SubMap;
  TH1Map timingMap;      TStrMap timingSubMap;

  ///////////////////
  // time res maps //
  ///////////////////
  // z variables
  TH2Map z2DMap; TStrMap z2DSubMap; TStrDblVMap zbins;

  // effective energy plots
  TH2Map effE2DMap;     TStrMap effE2DSubMap;     TStrDblVMap effEbins;
  TH2Map effseedE2DMap; TStrMap effseedE2DSubMap; TStrDblVMap effseedEbins;

  // nvtx plots
  TH2Map nvtx2DMap; TStrMap nvtx2DSubMap; DblVec nvtxbins;

  // delta eta plots
  TH2Map deta2DMap; TStrMap deta2DSubMap; DblVec detabins;
  
  // single el eta plots
  TH2Map eleta2DMap; TStrMap eleta2DSubMap; DblVec eletabins;

  // vtxZ plots
  TH2Map vtxZ2DMap; TStrMap vtxZ2DSubMap; DblVec vtxZbins;

  // single el energy plots
  TH2Map el1E2DMap;     TStrMap el1E2DSubMap;     TStrDblVMap el1Ebins;
  TH2Map el1seedE2DMap; TStrMap el1seedE2DSubMap; TStrDblVMap el1seedEbins;

  TH2Map el2E2DMap;     TStrMap el2E2DSubMap;     TStrDblVMap el2Ebins;
  TH2Map el2seedE2DMap; TStrMap el2seedE2DSubMap; TStrDblVMap el2seedEbins;

  // run numbers (data only)
  TH2Map runs2DMap; TStrMap runs2DSubMap; DblVec dRunNos; 

  //////////////////////
  // trigger eff maps //
  //////////////////////
  TH1Map  trTH1Map; TStrMap trTH1SubMap;
  TH1F * n_hltdoubleel_el1pt; TH1F * d_hltdoubleel_el1pt;
  TH1F * n_hltdoubleel_el2pt; TH1F * d_hltdoubleel_el2pt;

public:
  // Declaration of leaf types
  Int_t           event;
  Int_t           run;
  Int_t           lumi;
  Bool_t          hltsingleel;
  Bool_t          hltdoubleel;
  Bool_t          hltelnoiso;
  Int_t           nvtx;
  Float_t         vtxX;
  Float_t         vtxY;
  Float_t         vtxZ;
  Int_t           nvetoelectrons;
  Int_t           nlooseelectrons;
  Int_t           nmediumelectrons;
  Int_t           ntightelectrons;
  Int_t           nheepelectrons;
  Int_t           el1pid;
  Float_t         el1pt;
  Float_t         el1eta;
  Float_t         el1phi;
  Float_t         el1E;
  Float_t         el1p;
  Int_t           el2pid;
  Float_t         el2pt;
  Float_t         el2eta;
  Float_t         el2phi;
  Float_t         el2E;
  Float_t         el2p;
  Float_t         el1scX;
  Float_t         el1scY;
  Float_t         el1scZ;
  Float_t         el1scE;
  Float_t         el2scX;
  Float_t         el2scY;
  Float_t         el2scZ;
  Float_t         el2scE;
  std::vector<float>   *el1rhXs;
  std::vector<float>   *el1rhYs;
  std::vector<float>   *el1rhZs;
  std::vector<float>   *el1rhEs;
  std::vector<float>   *el1rhtimes;
  std::vector<int>     *el1rhids;
  std::vector<float>   *el2rhXs;
  std::vector<float>   *el2rhYs;
  std::vector<float>   *el2rhZs;
  std::vector<float>   *el2rhEs;
  std::vector<float>   *el2rhtimes;
  std::vector<int>     *el2rhids;
  Float_t         el1seedX;
  Float_t         el1seedY;
  Float_t         el1seedZ;
  Float_t         el1seedE;
  Float_t         el1seedtime;
  Int_t           el1seedid;
  Float_t         el2seedX;
  Float_t         el2seedY;
  Float_t         el2seedZ;
  Float_t         el2seedE;
  Float_t         el2seedtime;
  Int_t           el2seedid;
  Int_t           el1nrh;
  Int_t           el2nrh;
  Float_t         zmass;
  Float_t         zpt;
  Float_t         zeta;
  Float_t         zphi;
  Float_t         zE;
  Float_t         zp;
  Int_t           puobs;
  Int_t           putrue;

  // MC
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

  // List of branches
  TBranch        *b_event;   //!
  TBranch        *b_run;   //!
  TBranch        *b_lumi;   //!
  TBranch        *b_hltsingleel;   //!
  TBranch        *b_hltdoubleel;   //!
  TBranch        *b_hltelnoiso;   //!
  TBranch        *b_nvtx;   //!
  TBranch        *b_vtxX;   //!
  TBranch        *b_vtxY;   //!
  TBranch        *b_vtxZ;   //!
  TBranch        *b_nvetoelectrons;   //!
  TBranch        *b_nlooseelectrons;   //!
  TBranch        *b_nmediumelectrons;   //!
  TBranch        *b_ntightelectrons;   //!
  TBranch        *b_nheepelectrons;   //!
  TBranch        *b_el1pid;   //!
  TBranch        *b_el1pt;   //!
  TBranch        *b_el1eta;   //!
  TBranch        *b_el1phi;   //!
  TBranch        *b_el1E;   //!
  TBranch        *b_el1p;   //!
  TBranch        *b_el2pid;   //!
  TBranch        *b_el2pt;   //!
  TBranch        *b_el2eta;   //!
  TBranch        *b_el2phi;   //!
  TBranch        *b_el2E;   //!
  TBranch        *b_el2p;   //!
  TBranch        *b_el1scX;   //!
  TBranch        *b_el1scY;   //!
  TBranch        *b_el1scZ;   //!
  TBranch        *b_el1scE;   //!
  TBranch        *b_el2scX;   //!
  TBranch        *b_el2scY;   //!
  TBranch        *b_el2scZ;   //!
  TBranch        *b_el2scE;   //!
  TBranch        *b_el1rhXs;   //!
  TBranch        *b_el1rhYs;   //!
  TBranch        *b_el1rhZs;   //!
  TBranch        *b_el1rhEs;   //!
  TBranch        *b_el1rhtimes;   //!
  TBranch        *b_el1rhids;   //!
  TBranch        *b_el2rhXs;   //!
  TBranch        *b_el2rhYs;   //!
  TBranch        *b_el2rhZs;   //!
  TBranch        *b_el2rhEs;   //!
  TBranch        *b_el2rhtimes;   //!
  TBranch        *b_el2rhids;   //!
  TBranch        *b_el1seedX;   //!
  TBranch        *b_el1seedY;   //!
  TBranch        *b_el1seedZ;   //!
  TBranch        *b_el1seedE;   //!
  TBranch        *b_el1seedtime;   //!
  TBranch        *b_el1seedid;   //!
  TBranch        *b_el2seedX;   //!
  TBranch        *b_el2seedY;   //!
  TBranch        *b_el2seedZ;   //!
  TBranch        *b_el2seedE;   //!
  TBranch        *b_el2seedtime;   //!
  TBranch        *b_el2seedid;   //!
  TBranch        *b_el1nrh;   //!
  TBranch        *b_el2nrh;   //!
  TBranch        *b_zmass;   //!
  TBranch        *b_zpt;   //!
  TBranch        *b_zeta;   //!
  TBranch        *b_zphi;   //!
  TBranch        *b_zE;   //!
  TBranch        *b_zp;   //!

  // MC
  TBranch        *b_puobs;   //!
  TBranch        *b_putrue;   //!
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
};

#endif
