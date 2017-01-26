#ifndef _skimreco_
#define _skimreco_

#include "TROOT.h"
#include "TFile.h"
#include "TTree.h"
#include "TString.h"
#include "TEfficiency.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TLorentzVector.h"
#include "TVector3.h"

#include <fstream>
#include <vector>
#include <map>

inline Float_t rad2 (const Float_t x, const Float_t y){return x*x + y*y;}
inline Float_t rad2 (const Float_t x, const Float_t y, const Float_t z){return x*x + y*y + z*z;}
inline void    semiR(const Float_t insmaj, const Float_t insmin, Float_t & smaj, Float_t & smin)
{
  smaj = 1.f/std::sqrt(insmin);
  smin = 1.f/std::sqrt(insmaj);
}

typedef std::map<TString,TEfficiency*> TEffMap;
typedef TEffMap::iterator              TEffMapIter;

typedef std::map<TString,TH1F*> TH1Map;
typedef TH1Map::iterator        TH1MapIter;

typedef std::map<TString,TH2F*> TH2Map;
typedef TH2Map::iterator        TH2MapIter;

typedef std::map<TString,TString> TStrMap;
typedef TStrMap::iterator         TStrMapIter;

typedef std::map<TString,Int_t> TStrIntMap;
typedef TStrIntMap::iterator    TStrIntMapIter;

typedef std::map<Int_t,Int_t> IntMap;
typedef IntMap::iterator      IntMapIter;

class SkimRECO 
{
public :
  SkimRECO(TString filename, TString outdir = "output/skim", TString subdir = "", Bool_t dump = false, Bool_t batch = false,
	   Float_t jetptcut = 0.f, Int_t njetcut = 0,
	   Float_t phptcut = 0.f,
	   Int_t phhoecut = 0, Int_t phsieiecut = 0, Int_t phchgisocut = 0, Int_t phneuisocut = 0, Int_t phisocut = 0,
	   Float_t phsmajEBcut = 1.f, Float_t phsmajEEcut = 1.f, Float_t phsminEBcut = 1.f, Float_t phsminEEcut = 1.f, 
	   Int_t nphscut = 0,
	   Bool_t applyEBonly = false, Bool_t applyEEonly = false);
  ~SkimRECO();
  void InitTree();
  void DoSkim(std::ofstream & effdump);
  void SetupPlots();
  void SetupNminus1();
  void SetupTEffs();
  void SetupAnalysis();
  TH1F * MakeTH1F(TString hname, TString htitle, Int_t nbinsx, Float_t xlow, Float_t xhigh, TString xtitle, TString ytitle);
  TH2F * MakeTH2F(TString hname, TString htitle, Int_t nbinsx, Float_t xlow, Float_t xhigh, TString xtitle, Int_t nbinsy, Float_t ylow, Float_t yhigh, TString ytitle);
  TEfficiency * MakeTEff(TString hname, TString htitle, Int_t nbinsx, Float_t xlow, Float_t xhigh, TString xtitle, TString ytitle);
  void EventLoop();
  void DoEfficiency(Bool_t & passed);
  void FillNminus1(const Int_t njetscut, const std::vector<Int_t> & phs);
  void FillPhsPt(const std::vector<Int_t> & phspt);
  void FillPhsHoE(const std::vector<Int_t> & phshoe);
  void FillPhsSieie(const std::vector<Int_t> & phssieie);
  void FillPhsChgIso(const std::vector<Int_t> & phschgiso);
  void FillPhsNeuIso(const std::vector<Int_t> & phsneuiso);
  void FillPhsIso(const std::vector<Int_t> & phsiso);
  void FillPhsVID(const std::vector<Int_t> & phsvid);
  void FillPhsSmaj(const std::vector<Int_t> & phssmaj);
  void FillPhsSmin(const std::vector<Int_t> & phssmin);
  void FillTEffs(const Bool_t passed, const std::vector<Int_t> & phs);
  void FillAnalysis(const std::vector<Int_t> & phs);
  Int_t GetNJetsPt();
  void GetPhs(std::vector<Int_t> & phs);
  void GetPhsPt(std::vector<Int_t> & phspt);
  void GetPhsHoE(std::vector<Int_t> & phshoe);
  void GetPhsSieie(std::vector<Int_t> & phssieie);
  void GetPhsChgIso(std::vector<Int_t> & phschgiso);
  void GetPhsNeuIso(std::vector<Int_t> & phsneuiso);
  void GetPhsIso(std::vector<Int_t> & phsiso);
  void GetPhsVID(std::vector<Int_t> & phsvid);
  void GetPhsSmaj(std::vector<Int_t> & phssmaj);
  void GetPhsSmin(std::vector<Int_t> & phssmin);
  Int_t GetVID(const Int_t iph);
  void OutputTH1Fs();
  void OutputTH2Fs();
  void OutputTEffs();
  void DumpEfficiency(std::ofstream & effdump);

private :
  // Input vars
  TFile * fInFile; //!pointer to file
  TTree * fInTree; //!pointer to the analyzed TTree

  // In routine vars
  UInt_t  fNEvCheck;
  TH1Map  fPlots;
  TH2Map  fPlots2D;
  TEffMap fEffs;
  Int_t   fNPassed;

  // Config
  const Float_t fJetPtCut;
  const Int_t   fNJetsCut;
  const Float_t fPhPtCut;
  const Int_t   fPhHoECut;
  const Int_t   fPhSieieCut;
  const Int_t   fPhChgIsoCut;
  const Int_t   fPhNeuIsoCut;
  const Int_t   fPhIsoCut;
  const Float_t fPhSmajEBCut;
  const Float_t fPhSmajEECut;
  const Float_t fPhSminEBCut;
  const Float_t fPhSminEECut;
  const Int_t   fNPhsCut;
  const Bool_t  fApplyEBOnly;
  const Bool_t  fApplyEEOnly;

  // Output vars
  const TString fOutDir;
  const TString fSubDir;
        TFile * fOutFile;
  const Bool_t  fDump;
  const Bool_t  fBatch;
  std::ofstream fHistDump;

  // Declaration of leaf types
  Int_t                  event;
  Int_t                  run;
  Int_t                  lumi;
  Int_t                  nvtx;
  Float_t                vtxX;
  Float_t                vtxY;
  Float_t                vtxZ;
  Bool_t                 hltdoubleph;
  Float_t                t1pfMETpt;
  Float_t                t1pfMETphi;
  Float_t                t1pfMETsumEt;
  Int_t                  njets;
  std::vector<Float_t> * jetE;
  std::vector<Float_t> * jetpt;
  std::vector<Float_t> * jetphi;
  std::vector<Float_t> * jeteta;
  Int_t                  nphotons;
  std::vector<Float_t> * phE;
  std::vector<Float_t> * phpt;
  std::vector<Float_t> * phphi;
  std::vector<Float_t> * pheta;
  std::vector<Float_t> * phscE;
  std::vector<Float_t> * phscphi;
  std::vector<Float_t> * phsceta;
  std::vector<Float_t> * phsieie;
  std::vector<Float_t> * phsipip;
  std::vector<Float_t> * phsieip;
  std::vector<Float_t> * phsmaj;
  std::vector<Float_t> * phsmin;
  std::vector<Float_t> * phHoE;
  std::vector<Float_t> * phChgIso;
  std::vector<Float_t> * phNeuIso;
  std::vector<Float_t> * phPhIso;
  std::vector<Float_t> * phr9;
  std::vector<Float_t> * phsuisseX;
  std::vector<Int_t>   * phHoE_b;
  std::vector<Int_t>   * phsieie_b;
  std::vector<Int_t>   * phChgIso_b;
  std::vector<Int_t>   * phNeuIso_b;
  std::vector<Int_t>   * phPhIso_b;
  std::vector<Int_t>   * phnrh;
  std::vector<Int_t>   * phnrhEcut;
  std::vector<Int_t>   * phnrhOOT;
  std::vector<Float_t> * phseedE;
  std::vector<Float_t> * phseedphi;
  std::vector<Float_t> * phseedeta;
  std::vector<Float_t> * phseedtime;
  std::vector<Int_t>   * phseedOOT;

  // List of branches
  TBranch * b_event;
  TBranch * b_run;
  TBranch * b_lumi;
  TBranch * b_nvtx;
  TBranch * b_vtxX;
  TBranch * b_vtxY;
  TBranch * b_vtxZ;
  TBranch * b_hltdoubleph;
  TBranch * b_t1pfMETpt;
  TBranch * b_t1pfMETphi;
  TBranch * b_t1pfMETsumEt;
  TBranch * b_njets;
  TBranch * b_jetE;
  TBranch * b_jetpt;
  TBranch * b_jetphi;
  TBranch * b_jeteta;
  TBranch * b_nphotons;
  TBranch * b_phE;
  TBranch * b_phpt;
  TBranch * b_phphi;
  TBranch * b_pheta;
  TBranch * b_phscE;
  TBranch * b_phscphi;
  TBranch * b_phsceta;
  TBranch * b_phsieie;
  TBranch * b_phsipip;
  TBranch * b_phsieip;
  TBranch * b_phsmaj;
  TBranch * b_phsmin;
  TBranch * b_phHoE;
  TBranch * b_phChgIso;
  TBranch * b_phNeuIso;
  TBranch * b_phPhIso;
  TBranch * b_phr9;
  TBranch * b_phsuisseX;
  TBranch * b_phHoE_b;
  TBranch * b_phsieie_b;
  TBranch * b_phChgIso_b;
  TBranch * b_phNeuIso_b;
  TBranch * b_phPhIso_b;
  TBranch * b_phnrh;
  TBranch * b_phnrhEcut;
  TBranch * b_phnrhOOT;
  TBranch * b_phseedE;
  TBranch * b_phseedphi;
  TBranch * b_phseedeta;
  TBranch * b_phseedtime;
  TBranch * b_phseedOOT;
};
#endif
