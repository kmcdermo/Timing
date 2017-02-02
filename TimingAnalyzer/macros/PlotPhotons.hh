#ifndef _plotphotons_
#define _plotphotons_

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

static const Float_t sol = 2.99792458e8; // cm/s

inline Float_t rad2 (const Float_t x, const Float_t y){return x*x + y*y;}
inline Float_t rad2 (const Float_t x, const Float_t y, const Float_t z){return x*x + y*y + z*z;}
inline Float_t gamma(const Float_t p, const Float_t m){return std::sqrt(1.f+std::pow(p/m,2));}
inline Float_t beta (const Float_t p, const Float_t m){return std::sqrt(1.f/(1.f+std::pow(m/p,2)));}
inline Float_t bg   (const Float_t p, const Float_t m){return std::abs(p/m);}
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

class PlotPhotons 
{
public :
  PlotPhotons(TString filename, Bool_t isGMSB = false, Bool_t isBkg = false, Bool_t applyevcut = false, Bool_t rhdump = false,
	      TString outdir = "output", Bool_t savehists = false, 
	      Bool_t applyjetptcut = false, Float_t jetptcut = 35.f, Bool_t applyphptcut = false, Float_t phptcut = 100.f,
	      Bool_t applyphvidcut = false, TString phvid = "medium", Bool_t applyrhecut = false, Float_t rhEcut = 1.f,
	      Bool_t applyecalacceptcut = false, Bool_t applyEBonly = false, Bool_t applyEEonly = false);
  ~PlotPhotons();
  void InitTree();
  void DoPlots(Bool_t generic, Bool_t eff, Bool_t analysis);
  void SetupPlots(Bool_t generic, Bool_t eff, Bool_t analysis);
  void SetupEffs();
  void SetupGenInfo();
  void SetupGMSB();
  void SetupGenJets();
  void SetupObjectCounts();
  void SetupMET();
  void SetupJets();
  void SetupRecoPhotons();
  void SetupAnalysis();
  TEfficiency * MakeTEff(TString hname, TString htitle, Int_t nbinsx, Float_t xlow, Float_t xhigh, TString xtitle, TString ytitle, TString subdir);
  TH1F * MakeTH1F(TString hname, TString htitle, Int_t nbinsx, Float_t xlow, Float_t xhigh, TString xtitle, TString ytitle, TString subdir);
  TH2F * MakeTH2F(TString hname, TString htitle, Int_t nbinsx, Float_t xlow, Float_t xhigh, TString xtitle, Int_t nbinsy, Float_t ylow, Float_t yhigh, TString ytitle, TString subdir);
  void EventLoop(Bool_t generic, Bool_t eff, Bool_t analysis);
  void RecHitDumper();
  void CountEvents(Bool_t & event_b);
  void FillEffs();
  void FillGenInfo();
  void FillGMSB();
  void FillGenJets();
  void FillObjectCounts();
  void FillMET();
  void FillJets();
  void FillRecoPhotons();
  void FillAnalysis();
  void DumpEventCounts();
  void MakeSubDirs();
  void OutputTEffs();
  void OutputTH1Fs();
  void OutputTH2Fs();

private :
  // Input vars
  TFile * fInFile; //!pointer to file
  TTree * fInTree; //!pointer to the analyzed TTree
  const Bool_t fIsGMSB;
  const Bool_t fIsBkg;
        Bool_t fIsMC;

  // In routine vars
  UInt_t  fNEvCheck;
  TEffMap fEffs;
  TH1Map  fPlots;
  TH2Map  fPlots2D;
  TStrIntMap fEfficiency;
  Float_t fCTau;
  std::ofstream fSeedDump;
  std::ofstream fAllRHDump;

  // Config
  const Bool_t  fRHDump;
  const Bool_t  fApplyEvCut;
  const Bool_t  fSaveHists;
  const Bool_t  fApplyJetPtCut;
  const Float_t fJetPtCut;
  const Bool_t  fApplyPhPtCut;
  const Float_t fPhPtCut;
  const Bool_t  fApplyPhVIDCut;
  const TString fPhVID;
  TStrIntMap    fPhVIDMap;
  const Bool_t  fApplyrhECut;
  const Float_t frhECut;
  const Bool_t  fApplyECALAcceptCut;
  const Bool_t  fApplyEBOnly;
  const Bool_t  fApplyEEOnly;

  // Output vars
  TString fOutDir;
  TString fOutDump;
  TStrMap fSubDirs;
  TFile * fOutFile;

  // Declaration of leaf types
  Int_t   event;
  Int_t   run;
  Int_t   lumi;
  Float_t genwgt;
  Int_t   genpuobs;
  Int_t   genputrue;
  Int_t   nNeutralino;
  Int_t   nNeutoPhGr;
  Float_t genN1mass;
  Float_t genN1E;
  Float_t genN1pt;
  Float_t genN1phi;
  Float_t genN1eta;
  Float_t genN1prodvx;
  Float_t genN1prodvy;
  Float_t genN1prodvz;
  Float_t genN1decayvx;
  Float_t genN1decayvy;
  Float_t genN1decayvz;
  Float_t genph1E;
  Float_t genph1pt;
  Float_t genph1phi;
  Float_t genph1eta;
  Int_t   genph1match;
  Float_t gengr1mass;
  Float_t gengr1E;
  Float_t gengr1pt;
  Float_t gengr1phi;
  Float_t gengr1eta;
  Float_t genN2mass;
  Float_t genN2E;
  Float_t genN2pt;
  Float_t genN2phi;
  Float_t genN2eta;
  Float_t genN2prodvx;
  Float_t genN2prodvy;
  Float_t genN2prodvz;
  Float_t genN2decayvx;
  Float_t genN2decayvy;
  Float_t genN2decayvz;
  Float_t genph2E;
  Float_t genph2pt;
  Float_t genph2phi;
  Float_t genph2eta;
  Int_t   genph2match;
  Float_t gengr2mass;
  Float_t gengr2E;
  Float_t gengr2pt;
  Float_t gengr2phi;
  Float_t gengr2eta;
  Int_t   ngenjets;
  std::vector<Int_t>   * genjetmatch;
  std::vector<Float_t> * genjetE;
  std::vector<Float_t> * genjetpt;
  std::vector<Float_t> * genjetphi;
  std::vector<Float_t> * genjeteta;
  Int_t   nvtx;
  Float_t vtxX;
  Float_t vtxY;
  Float_t vtxZ;
  Float_t t1pfMETpt;
  Float_t t1pfMETphi;
  Float_t t1pfMETsumEt;
  Float_t t1pfMETuncorpt;
  Float_t t1pfMETuncorphi;
  Float_t t1pfMETuncorsumEt;
  Float_t t1pfMETcalopt;
  Float_t t1pfMETcalophi;
  Float_t t1pfMETcalosumEt;
  Float_t t1pfMETgenMETpt;
  Float_t t1pfMETgenMETphi;
  Float_t t1pfMETgenMETsumEt;
  Int_t   njets;
  std::vector<Int_t>   * jetmatch;
  std::vector<Float_t> * jetE;
  std::vector<Float_t> * jetpt;
  std::vector<Float_t> * jetphi;
  std::vector<Float_t> * jeteta;
  Int_t   nphotons;
  std::vector<Float_t> * phE;
  std::vector<Float_t> * phpt;
  std::vector<Float_t> * phphi;
  std::vector<Float_t> * pheta;
  std::vector<Int_t>   * phmatch;
  std::vector<Bool_t>  * phisMatched;
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
  std::vector<Int_t>   * phVID;
  std::vector<Int_t>   * phHoE_b;
  std::vector<Int_t>   * phsieie_b;
  std::vector<Int_t>   * phChgIso_b;
  std::vector<Int_t>   * phNeuIso_b;
  std::vector<Int_t>   * phIso_b;
  std::vector<Int_t>   * phnrh;
  std::vector<Int_t>   * phseedpos;
  std::vector<std::vector<Float_t> > * phrheta;
  std::vector<std::vector<Float_t> > * phrhphi;
  std::vector<std::vector<Float_t> > * phrhE;
  std::vector<std::vector<Float_t> > * phrhtime;
  std::vector<std::vector<Int_t> >   * phrhID;
  std::vector<std::vector<Int_t> >   * phrhOOT;

  // List of branches
  TBranch * b_event;
  TBranch * b_run;
  TBranch * b_lumi;
  TBranch * b_genwgt;
  TBranch * b_genpuobs;
  TBranch * b_genputrue;
  TBranch * b_nNeutralino;
  TBranch * b_nNeutoPhGr;
  TBranch * b_genN1mass;
  TBranch * b_genN1E;
  TBranch * b_genN1pt;
  TBranch * b_genN1phi;
  TBranch * b_genN1eta;
  TBranch * b_genN1prodvx;
  TBranch * b_genN1prodvy;
  TBranch * b_genN1prodvz;
  TBranch * b_genN1decayvx;
  TBranch * b_genN1decayvy;
  TBranch * b_genN1decayvz;
  TBranch * b_genph1E;
  TBranch * b_genph1pt;
  TBranch * b_genph1phi;
  TBranch * b_genph1eta;
  TBranch * b_genph1match;
  TBranch * b_gengr1mass;
  TBranch * b_gengr1E;
  TBranch * b_gengr1pt;
  TBranch * b_gengr1phi;
  TBranch * b_gengr1eta;
  TBranch * b_genN2mass;
  TBranch * b_genN2E;
  TBranch * b_genN2pt;
  TBranch * b_genN2phi;
  TBranch * b_genN2eta;
  TBranch * b_genN2prodvx;
  TBranch * b_genN2prodvy;
  TBranch * b_genN2prodvz;
  TBranch * b_genN2decayvx;
  TBranch * b_genN2decayvy;
  TBranch * b_genN2decayvz;
  TBranch * b_genph2E;
  TBranch * b_genph2pt;
  TBranch * b_genph2phi;
  TBranch * b_genph2eta;
  TBranch * b_genph2match;
  TBranch * b_gengr2mass;
  TBranch * b_gengr2E;
  TBranch * b_gengr2pt;
  TBranch * b_gengr2phi;
  TBranch * b_gengr2eta;
  TBranch * b_ngenjets;
  TBranch * b_genjetmatch;
  TBranch * b_genjetE;
  TBranch * b_genjetpt;
  TBranch * b_genjetphi;
  TBranch * b_genjeteta;
  TBranch * b_nvtx;
  TBranch * b_vtxX;
  TBranch * b_vtxY;
  TBranch * b_vtxZ;
  TBranch * b_t1pfMETpt;
  TBranch * b_t1pfMETphi;
  TBranch * b_t1pfMETsumEt;
  TBranch * b_t1pfMETuncorpt;
  TBranch * b_t1pfMETuncorphi;
  TBranch * b_t1pfMETuncorsumEt;
  TBranch * b_t1pfMETcalopt;
  TBranch * b_t1pfMETcalophi;
  TBranch * b_t1pfMETcalosumEt;
  TBranch * b_t1pfMETgenMETpt;
  TBranch * b_t1pfMETgenMETphi;
  TBranch * b_t1pfMETgenMETsumEt;
  TBranch * b_njets;
  TBranch * b_jetmatch;
  TBranch * b_jetE;
  TBranch * b_jetpt;
  TBranch * b_jetphi;
  TBranch * b_jeteta;
  TBranch * b_nphotons;
  TBranch * b_phE;
  TBranch * b_phpt;
  TBranch * b_phphi;
  TBranch * b_pheta;
  TBranch * b_phmatch;
  TBranch * b_phisMatched;
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
  TBranch * b_phVID;
  TBranch * b_phHoE_b;
  TBranch * b_phsieie_b;
  TBranch * b_phChgIso_b;
  TBranch * b_phNeuIso_b;
  TBranch * b_phIso_b;
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
