#ifndef _analysis_
#define _analysis_ 

#include "AnalysisTypes.hh"
#include "CommonTypes.hh"
#include "Config.hh"
#include "CommonUtils.hh"

#include "TH2F.h"
#include "TF1.h"

typedef std::map<TString,TH1F*> TH1Map;
typedef TH1Map::iterator        TH1MapIter;

typedef std::map<TString,TH2F*> TH2Map;
typedef TH2Map::iterator        TH2MapIter;

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
  void EventLoop();
  void SetupEventStandardPlots();
  void SetupPhotonStandardPlots();
  void SetupIsoPlots();
  void SetupIsoNvtxPlots();
  void FillEventStandardPlots(const Float_t weight);
  void FillPhotonStandardPlots(const Int_t Nphotons, const Float_t weight);
  void FillIsoPlots(const Int_t Nphotons, const Float_t weight);
  void FillIsoNvtxPlots(const Int_t Nphotons, const Float_t weight);
  void OutputEventStandardPlots();
  void OutputPhotonStandardPlots();
  void OutputIsoPlots();
  void OutputIsoNvtxPlots();
  void MakeInclusiveTH1s(TH1Map & th1map, TStrMap & subdirmap);
  void MakeInclusiveTH2s(TH2Map & th2map, TStrMap & subdirmap);
  void Make1DIsoPlots(const TH2F * hist2d, const TString & subdir2d, const TString & name);
  void Project2Dto1D(const TH2F * hist2d, const TString & subdir2d, TH1Map & th1dmap, TStrMap & subdir1dmap, TStrIntMap & th1dbinmap);
  void ProduceMeanHist(const TH2F * hist2d, const TString & subdir2d, TH1Map & th1dmap, TStrIntMap & th1dbinmap);
  TH1F * MakeTH1Plot(const TString & hname, const TString & htitle, const Int_t nbinsx, const Double_t xlow, const Double_t xhigh, 
		     const TString & xtitle, const TString & ytitle, TStrMap& subdirmap, const TString & subdir);
  TH2F * MakeTH2Plot(const TString & hname, const TString & htitle, const Int_t nbinsx, const Double_t xlow, const Double_t xhigh, const TString & xtitle,
		     const Int_t nbinsy, const Double_t ylow, const Double_t yhigh, const TString & ytitle, TStrMap& subdirmap, const TString & subdir);
  TH2F * MakeTH2Plot(const TString & hname, const TString & htitle, const DblVec& vxbins, const TString & xtitle, 
		     const Int_t nbinsy, const Double_t ylow, const Double_t yhigh, const TString & ytitle, TStrMap& subdirmap, const TString & subdir);
  void SaveTH1s(TH1Map & th1map, TStrMap & subdirmap);
  void SaveTH2s(TH2Map & th2map, TStrMap & subdirmap);
  void DumpTH1Names(TH1Map & th1map, TStrMap & subdirmap);
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
  
  // MC weight input
  //  FltVec  fPUweights;
  DblVec  fPUweights;
  Float_t fXsec;
  Float_t fWgtsum;

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
  TH1Map stdphoTH1Map; TStrMap stdphoTH1SubMap;
  TH1Map isoTH1Map; TStrMap isoTH1SubMap;
  TH2Map isonvtxTH2Map; TStrMap isonvtxTH2SubMap;

  ///////////////////////////////
  // Declaration of leaf types //
  ///////////////////////////////
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

  // List of branches
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
};

#endif
