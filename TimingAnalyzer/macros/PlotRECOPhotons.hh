#ifndef _plotrecophotons_
#define _plotrecophotons_

#include "TFile.h"
#include "TTree.h"
#include "TString.h"
#include "TH1F.h"
#include "TH2F.h"

#include <vector>
#include <map>

namespace ECAL
{
  static const Float_t etaEB = 1.4442;
  static const Float_t etaEEmin = 1.566;
  static const Float_t etaEEmax = 2.5;
};

typedef std::vector<TString> TStrVec;

typedef std::map<TString,TH1F*> TH1Map;
typedef TH1Map::iterator        TH1MapIter;

typedef std::map<TString,TString> TStrMap;
typedef TStrMap::iterator         TStrMapIter;

class PlotRECOPhotons 
{
public :
  PlotRECOPhotons(TString filename, TString outdir = "output", Bool_t applyrhEcut = false, Float_t rhEcut = 1.f,
		  Bool_t applyptcut = false, Float_t ptcut = 10.f, Bool_t applyhoecut = false, Float_t hoecut = 0.3f, Bool_t applyr9cut = false, Float_t r9cut = 0.5f,
		  Bool_t applysieiecut = false, Float_t sieieEBcut = 0.03f, Float_t sieieEEcut = 0.07f);
  ~PlotRECOPhotons();
  void InitTree();
  void DoPlots();
  void SetupPlots();
  void SetupRecoPhotons();
  TH1F * MakeTH1F(TString hname, TString htitle, Int_t nbinsx, Float_t xlow, Float_t xhigh, TString xtitle, TString ytitle);
  void EventLoop();
  void FillRecoPhotons();
  void OutputTH1Fs();

private :
  // Input vars
  TFile * fInFile; //!pointer to file
  TTree * fInTree; //!pointer to the analyzed TTree

  // In routine vars
  UInt_t  fNEvCheck;
  TH1Map  fPlots;
  
  // Photon cuts
  const Bool_t  fApplyrhECut;
  const Float_t frhECut;
  const Bool_t  fApplyPtCut;
  const Float_t fPtCut;
  const Bool_t  fApplyHoECut;
  const Float_t fHoECut;
  const Bool_t  fApplyR9Cut;
  const Float_t fR9Cut;
  const Bool_t  fApplySieieCut;
  const Float_t fSieieEBCut;
  const Float_t fSieieEECut;

  // Output vars
  TString fOutDir;
  TStrMap fSubDirs;
  TFile * fOutFile;

  // Declaration of leaf types
  ULong64_t event;
  UInt_t    run;
  UInt_t    lumi;
  Int_t     nphotons;
  std::vector<Int_t>   * phisOOT;
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
  std::vector<Float_t> * phEcalIso;
  std::vector<Float_t> * phHcalIso;
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
  TBranch * b_phisOOT;
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
  TBranch * b_phEcalIso;
  TBranch * b_phHcalIso;
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
