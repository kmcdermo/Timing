#ifndef _plotrecophotons_
#define _plotrecophotons_

#include "TROOT.h"
#include "TFile.h"
#include "TTree.h"
#include "TString.h"
#include "TH1F.h"
#include "TH2F.h"

#include <fstream>
#include <vector>
#include <map>

typedef std::map<TString,TH1F*> TH1Map;
typedef TH1Map::iterator        TH1MapIter;

typedef std::map<TString,TH2F*> TH2Map;
typedef TH2Map::iterator        TH2MapIter;

typedef std::map<TString,TString> TStrMap;
typedef TStrMap::iterator         TStrMapIter;

typedef std::vector<TString> TStrVec;

class PlotRECOPhotons 
{
public :
  PlotRECOPhotons(TString filename, TString outdir = "output", 
		  Bool_t applyptcut, Float_t ptcut);
  ~PlotRECOPhotons();
  void InitTree();
  void DoPlots();
  void SetupPlots();
  void SetupRecoPhotons();
  TH1F * MakeTH1F(TString hname, TString htitle, Int_t nbinsx, Float_t xlow, Float_t xhigh, TString xtitle, TString ytitle, TString subdir);
  TH2F * MakeTH2F(TString hname, TString htitle, Int_t nbinsx, Float_t xlow, Float_t xhigh, TString xtitle, Int_t nbinsy, Float_t ylow, Float_t yhigh, TString ytitle, TString subdir);
  void EventLoop();
  void FillRecoPhotons();
  void MakeSubDirs();
  void OutputTH1Fs();
  void OutputTH2Fs();

private :
  // Input vars
  TFile * fInFile; //!pointer to file
  TTree * fInTree; //!pointer to the analyzed TTree

  // In routine vars
  UInt_t  fNEvCheck;
  TH1Map  fPlots;
  TH2Map  fPlots2D;
  
  // Photon cuts
  const Bool_t  fApplyPtCut;
  const Float_t fPtCut;

  // Output vars
  TString fOutDir;
  TStrMap fSubDirs;
  TFile * fOutFile;

  // Declaration of leaf types
  ULong64_t       event;
  UInt_t          run;
  UInt_t          lumi;
  Int_t           nphotons;
  std::vector<Float_t> * phE;
  std::vector<Float_t> * phpt;
  std::vector<Float_t> * phphi;
  std::vector<Float_t> * pheta;
  std::vector<Float_t> * phscE;
  std::vector<Float_t> * phsceta;
  std::vector<Float_t> * phscphi;
  std::vector<Float_t> * phHoE;
  std::vector<Float_t> * phr9;
  std::vector<Float_t> * phChgIso;
  std::vector<Float_t> * phNeuIso;
  std::vector<Float_t> * phIso;
  std::vector<Float_t> * phsuisseX;
  std::vector<Float_t> * phsieie;
  std::vector<Float_t> * phsipip;
  std::vector<Float_t> * phsieip;
  std::vector<Float_t> * phsmaj;
  std::vector<Float_t> * phsmin;
  std::vector<Float_t> * phalpha;
  std::vector<Int_t>   * phnrh;
  std::vector<Int_t>   * phseedpos;
  std::vector<std::vector<Float_t> > * phrheta;
  std::vector<std::vector<Float_t> > * phrhphi;
  std::vector<std::vector<Float_t> > * phrhE;
  std::vector<std::vector<Float_t> > * phrhtime;
  std::vector<std::vector<Int_t> > * phrhID;
  std::vector<std::vector<Int_t> > * phrhOOT;

  // List of branches
  TBranch * b_event;
  TBranch * b_run;
  TBranch * b_lumi;
  TBranch * b_nphotons;
  TBranch * b_phE;
  TBranch * b_phpt;
  TBranch * b_phphi;
  TBranch * b_pheta;
  TBranch * b_phscE;
  TBranch * b_phsceta;
  TBranch * b_phscphi;
  TBranch * b_phHoE;
  TBranch * b_phr9;
  TBranch * b_phChgIso;
  TBranch * b_phNeuIso;
  TBranch * b_phIso;
  TBranch * b_phsuisseX;
  TBranch * b_phsieie;
  TBranch * b_phsipip;
  TBranch * b_phsieip;
  TBranch * b_phsmaj;
  TBranch * b_phsmin;
  TBranch * b_phalpha;
  TBranch * b_phnrh;
  TBranch * b_phseedpos;
  TBranch * b_phrheta;
  TBranch * b_phrhphi;
  TBranch * b_phrhE;
  TBranch * b_phrhtime;
  TBranch * b_phrhID;
  TBranch * b_phrhOOT;
};
#endif
