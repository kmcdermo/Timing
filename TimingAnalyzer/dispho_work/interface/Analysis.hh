#ifndef _analysis_
#define _analysis_ 

#include "AnalysisTypes.hh"
#include "CommonTypes.hh"
#include "Config.hh"
#include "CommonUtils.hh"

#include "TTree.h"
#include "TBranch.h"

class Analysis 
{
public:
  // functions
  Analysis(const TString & sample, const Bool_t isMC);
  ~Analysis();
  void InitTree();
  void InitStructs();
  void InitBranchVecs();
  void InitBranches();
  void InitAndReadConfigTree();
  void InitConfigStrings();
  void InitConfigBranches();
  void EventLoop();
  void SetupEventStandardPlots();
  void SetupPhotonStandardPlots();
  void SetupIsoPlots();
  void SetupIsoNvtxPlots();
  void SetupIsoPtPlots();
  void FillEventStandardPlots(const Float_t weight);
  void FillPhotonStandardPlots(const Int_t Nphotons, const Float_t weight);
  void FillIsoPlots(const Int_t Nphotons, const Float_t weight);
  void FillIsoNvtxPlots(const Int_t Nphotons, const Float_t weight);
  void FillIsoPtPlots(const Int_t Nphotons, const Float_t weight);
  void OutputEventStandardPlots();
  void OutputPhotonStandardPlots();
  void OutputIsoPlots();
  void OutputIsoNvtxPlots();
  void OutputIsoPtPlots();
  void MakeInclusiveTH1s(TH1Map & th1map, TStrMap & subdirmap);
  void MakeInclusiveNphoTH1s(TH1Map & th1map, TStrMap & subdirmap);
  void MakeInclusiveSplitTH1s(TH1Map & th1map, TStrMap & subdirmap);
  void MakeInclusiveRegionTH1s(TH1Map & th1map, TStrMap & subdirmap);
  void MakeInclusiveTH2s(TH2Map & th2map, TStrMap & subdirmap);
  void MakeInclusiveNphoTH2s(TH2Map & th2map, TStrMap & subdirmap);
  void MakeInclusiveSplitTH2s(TH2Map & th2map, TStrMap & subdirmap);
  void MakeInclusiveRegionTH2s(TH2Map & th2map, TStrMap & subdirmap);
  void Make1DFrom2DPlots(const TH2F * hist2d, const TString & subdir2d, const TString & name);
  void Project2Dto1D(const TH2F * hist2d, const TString & subdir2d, TH1Map & th1dmap, TStrMap & subdir1dmap, TStrIntMap & th1dbinmap);
  void ProduceQuantile(const TH2F * hist2d, const TString & subdir2d, TH1Map & th1dmap, TStrIntMap & th1dbinmap);
  void ProduceMeanHist(const TH2F * hist2d, const TString & subdir2d, TH1Map & th1dmap, TStrIntMap & th1dbinmap);
  void GetQuantileX(const TH1F * hist, Float_t & x, Float_t & dx_dn, Float_t & dx_up);
  Float_t FluctuateX(const FltVec & eff, const FltVec & eff_err, const Float_t qprob, const FltVec & centers, const Float_t x, const Bool_t isUp);
  TH1F * MakeTH1Plot(const TString & hname, const TString & htitle, const Int_t nbinsx, const Double_t xlow, const Double_t xhigh, 
		     const TString & xtitle, const TString & ytitle, TStrMap& subdirmap, const TString & subdir);
  TH1F * MakeTH1PlotFromTH2(const TH2F * hist2d, const TString & name, const TString & ytitle);
  TH2F * MakeTH2Plot(const TString & hname, const TString & htitle, const Int_t nbinsx, const Double_t xlow, const Double_t xhigh, const TString & xtitle,
		     const Int_t nbinsy, const Double_t ylow, const Double_t yhigh, const TString & ytitle, TStrMap& subdirmap, const TString & subdir);
  TH2F * MakeTH2Plot(const TString & hname, const TString & htitle, const DblVec& vxbins, const TString & xtitle, 
		     const Int_t nbinsy, const Double_t ylow, const Double_t yhigh, const TString & ytitle, TStrMap& subdirmap, const TString & subdir);
  void SaveTH1s(TH1Map & th1map, TStrMap & subdirmap);
  void SaveProjectedTH1(TH1F * hist, const TString & subdir2d);
  void SaveTH2s(TH2Map & th2map, TStrMap & subdirmap);
  void DumpTH1Names(TH1Map & th1map, TStrMap & subdirmap);
  void DumpTH1PhoNames(TH1Map & th1map, TStrMap & subdirmap);
  void DeleteTH1s(TH1Map & th1map);
  void DeleteTH2s(TH2Map & th2map);

private:
  // Input
  const TString fSample;
  const Bool_t  fIsMC;
  Bool_t fIsGMSB;
  Bool_t fIsHVDS;
  TFile * fInFile;
  TTree * fInTree; 
  TTree * fConfigTree;
  TH1F  * fCutFlow;

  // MC weight input
  Float_t fWgtSum;
  DblVec  fPUweights;

  // Output
  TString fOutDir;
  TFile*  fOutFile;
  std::ofstream fTH1Dump; 
  std::ofstream fTH1PhoDump; 
  
  // Output colors
  Color_t fColor;

  ///////////////
  // Plot maps //
  ///////////////
  TH1Map stdevTH1Map; TStrMap stdevTH1SubMap;
  TH2Map stdevTH2Map; TStrMap stdevTH2SubMap;
  TH1Map stdphoTH1Map; TStrMap stdphoTH1SubMap;
  TH1Map isoTH1Map; TStrMap isoTH1SubMap;
  TH2Map isonvtxTH2Map; TStrMap isonvtxTH2SubMap;
  TH2Map isoptTH2Map; TStrMap isoptTH2SubMap;

  ///////////////////////////////////////////
  // Declaration of leaf types for fInTree //
  ///////////////////////////////////////////
  Float_t   genwgt;
  Int_t     genpuobs;
  Int_t     genputrue;
  Int_t     nNeutoPhGr;
  GMSBVec   gmsbs;
  Int_t     nvPions;
  HVDSVec   hvdss;

  UInt_t    run;
  UInt_t    lumi;
  ULong64_t event;
  Bool_t    hltDisPho;
  Int_t     nvtx;
  Float_t   vtxX;
  Float_t   vtxY;
  Float_t   vtxZ;
  Float_t   rho;
  Float_t   t1pfMETpt;
  Float_t   t1pfMETphi;
  Float_t   t1pfMETsumEt;
  Float_t   jetHT;
  Int_t     njets;
  JetVec    jets;
  Int_t     nrechits;
  std::vector<Float_t> * rheta;
  std::vector<Float_t> * rhphi;
  std::vector<Float_t> * rhE;
  std::vector<Float_t> * rhtime;
  std::vector<Int_t>   * rhOOT;
  std::vector<UInt_t>  * rhID;
  Int_t     nphotons;
  PhoVec    phos;

  //////////////////////////////////
  // List of branches for fInTree //
  //////////////////////////////////
  TBranch * b_genwgt;
  TBranch * b_genpuobs;
  TBranch * b_genputrue;
  TBranch * b_nNeutoPhGr;
  TBranch * b_nvPions;
  TBranch * b_run;
  TBranch * b_lumi;
  TBranch * b_event;
  TBranch * b_hltDisPho;
  TBranch * b_nvtx;
  TBranch * b_vtxX;
  TBranch * b_vtxY;
  TBranch * b_vtxZ;
  TBranch * b_rho;
  TBranch * b_t1pfMETpt;
  TBranch * b_t1pfMETphi;
  TBranch * b_t1pfMETsumEt;
  TBranch * b_jetHT;
  TBranch * b_njets;
  TBranch * b_nrechits;
  TBranch * b_rheta;
  TBranch * b_rhphi;
  TBranch * b_rhE;
  TBranch * b_rhtime;
  TBranch * b_rhOOT;
  TBranch * b_rhID;
  TBranch * b_nphotons;

  ///////////////////////////////////////////////
  // Declaration of leaf types for fConfigTree //
  ///////////////////////////////////////////////
  UInt_t  blindSF;
  Bool_t  applyBlindSF;
  Float_t blindMET;
  Bool_t  applyBlindMET;
  Float_t jetpTmin;
  Int_t   jetIDmin;
  Float_t rhEmin;
  Float_t phpTmin;
  std::string * phIDmin;
  Float_t seedTimemin;
  Bool_t  splitPho;
  Bool_t  onlyGED;
  Bool_t  onlyOOT;
  Bool_t  applyTrigger;
  Float_t minHT;
  Bool_t  applyHT;
  Float_t phgoodpTmin;
  std::string * phgoodIDmin;
  Bool_t  applyPhGood;
  Float_t dRmin;
  Float_t pTres;
  Float_t trackdRmin;
  Float_t trackpTmin;
  std::string * inputPaths;
  std::string * inputFilters;
  Bool_t  isGMSB;
  Bool_t  isHVDS;
  Bool_t  isBkgd;
  Float_t xsec;
  Float_t filterEff;

  //////////////////////////////////////
  // List of branches for fConfigTree //
  //////////////////////////////////////
  TBranch * b_blindSF;
  TBranch * b_applyBlindSF;
  TBranch * b_blindMET;
  TBranch * b_applyBlindMET;
  TBranch * b_jetpTmin;
  TBranch * b_jetIDmin;
  TBranch * b_rhEmin;
  TBranch * b_phpTmin;
  TBranch * b_phIDmin;
  TBranch * b_seedTimemin;
  TBranch * b_splitPho;
  TBranch * b_onlyGED;
  TBranch * b_onlyOOT;
  TBranch * b_applyTrigger;
  TBranch * b_minHT;
  TBranch * b_applyHT;
  TBranch * b_phgoodpTmin;
  TBranch * b_phgoodIDmin;
  TBranch * b_applyPhGood;
  TBranch * b_dRmin;
  TBranch * b_pTres;
  TBranch * b_trackdRmin;
  TBranch * b_trackpTmin;
  TBranch * b_inputPaths;
  TBranch * b_inputFilters;
  TBranch * b_isGMSB;
  TBranch * b_isHVDS;
  TBranch * b_isBkgd;
  TBranch * b_xsec;
  TBranch * b_filterEff;
};

#endif
