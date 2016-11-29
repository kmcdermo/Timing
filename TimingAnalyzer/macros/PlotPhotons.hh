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

#include <vector>
#include <map>

static const Float_t sol = 2.99792458e8; // cm/s

inline Float_t rad2  (const Float_t x, const Float_t y){return x*x + y*y;}
inline Float_t rad2  (const Float_t x, const Float_t y, const Float_t z){return x*x + y*y + z*z;}
inline Float_t gamma (const Float_t p, const Float_t m){return std::sqrt(1.f+std::pow(p/m,2));}
inline Float_t beta  (const Float_t p, const Float_t m){return std::sqrt(1.f/(1.f+std::pow(m/p,2)));}
inline Float_t bg    (const Float_t p, const Float_t m){return std::abs(p/m);}

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
  PlotPhotons(TString filename, Bool_t isMC, Bool_t applyevnocut = false, Bool_t applyevcut = false, TString outdir = "output", 
	      Bool_t applyjetptcut = false, Float_t jetptcut = 35.f, Bool_t applyphptcut = false, Float_t phptcut = 100.f,
	      Bool_t applyphvidcut = false, TString phvid = "medium", Bool_t applyrhecut = false, Float_t rhEcut = 1.f,
	      Bool_t applyecalacceptcut = false);
  ~PlotPhotons();
  void InitTree();
  void DoPlots(Bool_t generic, Bool_t eff, Bool_t analysis);
  void SetupPlots(Bool_t generic, Bool_t eff, Bool_t analysis);
  void SetupEffs();
  void SetupGenInfo();
  void SetupGenParticles();
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
  void CountEvents(Bool_t & event_b);
  void FillEffs();
  void FillGenInfo();
  void FillGenParticles();
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
  const Bool_t  fIsMC;

  // In routine vars
  UInt_t  fNEvCheck;
  TEffMap fEffs;
  TH1Map  fPlots;
  TH2Map  fPlots2D;
  TStrIntMap fEfficiency;
  IntMap  fEvents;
  Float_t fCTau;

  // Config
  const Bool_t  fApplyEvNoCut;
  const Bool_t  fApplyEvCut;
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
  Float_t gengr2E;
  Float_t gengr2pt;
  Float_t gengr2phi;
  Float_t gengr2eta;
  Int_t   ngenjets;
  vector<int>   * genjetmatch;
  vector<float> * genjetE;
  vector<float> * genjetpt;
  vector<float> * genjetphi;
  vector<float> * genjeteta;
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
  vector<int>   * jetmatch;
  vector<float> * jetE;
  vector<float> * jetpt;
  vector<float> * jetphi;
  vector<float> * jeteta;
  Int_t   nphotons;
  vector<int>   * phmatch;
  vector<int>   * phVID;
  vector<float> * phE;
  vector<float> * phpt;
  vector<float> * phphi;
  vector<float> * pheta;
  vector<float> * phscX;
  vector<float> * phscY;
  vector<float> * phscZ;
  vector<float> * phscE;
  vector<int>   * phnrhs;
  vector<int>   * phseedpos;
  vector<vector<float> > * phrhXs;
  vector<vector<float> > * phrhYs;
  vector<vector<float> > * phrhZs;
  vector<vector<float> > * phrhEs;
  vector<vector<float> > * phrhdelRs;
  vector<vector<float> > * phrhtimes;
  vector<vector<int> > * phrhIDs;
  vector<vector<int> > * phrhOOTs;

  // List of branches
  TBranch * b_event;   //!
  TBranch * b_run;   //!
  TBranch * b_lumi;   //!
  TBranch * b_genwgt;   //!
  TBranch * b_genpuobs;   //!
  TBranch * b_genputrue;   //!
  TBranch * b_nNeutralino;
  TBranch * b_nNeutoPhGr;
  TBranch * b_genN1mass;   //!
  TBranch * b_genN1E;   //!
  TBranch * b_genN1pt;   //!
  TBranch * b_genN1phi;   //!
  TBranch * b_genN1eta;   //!
  TBranch * b_genN1prodvx;   //!
  TBranch * b_genN1prodvy;   //!
  TBranch * b_genN1prodvz;   //!
  TBranch * b_genN1decayvx;   //!
  TBranch * b_genN1decayvy;   //!
  TBranch * b_genN1decayvz;   //!
  TBranch * b_genph1E;   //!
  TBranch * b_genph1pt;   //!
  TBranch * b_genph1phi;   //!
  TBranch * b_genph1eta;   //!
  TBranch * b_genph1match;   //!
  TBranch * b_gengr1E;   //!
  TBranch * b_gengr1pt;   //!
  TBranch * b_gengr1phi;   //!
  TBranch * b_gengr1eta;   //!
  TBranch * b_genN2mass;   //!
  TBranch * b_genN2E;   //!
  TBranch * b_genN2pt;   //!
  TBranch * b_genN2phi;   //!
  TBranch * b_genN2eta;   //!
  TBranch * b_genN2prodvx;   //!
  TBranch * b_genN2prodvy;   //!
  TBranch * b_genN2prodvz;   //!
  TBranch * b_genN2decayvx;   //!
  TBranch * b_genN2decayvy;   //!
  TBranch * b_genN2decayvz;   //!
  TBranch * b_genph2E;   //!
  TBranch * b_genph2pt;   //!
  TBranch * b_genph2phi;   //!
  TBranch * b_genph2eta;   //!
  TBranch * b_genph2match;   //!
  TBranch * b_gengr2E;   //!
  TBranch * b_gengr2pt;   //!
  TBranch * b_gengr2phi;   //!
  TBranch * b_gengr2eta;   //!
  TBranch * b_ngenjets;   //!
  TBranch * b_genjetmatch;   //!
  TBranch * b_genjetE;   //!
  TBranch * b_genjetpt;   //!
  TBranch * b_genjetphi;   //!
  TBranch * b_genjeteta;   //!
  TBranch * b_nvtx;   //!
  TBranch * b_vtxX;   //!
  TBranch * b_vtxY;   //!
  TBranch * b_vtxZ;   //!
  TBranch * b_t1pfMETpt;   //!
  TBranch * b_t1pfMETphi;   //!
  TBranch * b_t1pfMETsumEt;   //!
  TBranch * b_t1pfMETuncorpt;   //!
  TBranch * b_t1pfMETuncorphi;   //!
  TBranch * b_t1pfMETuncorsumEt;   //!
  TBranch * b_t1pfMETcalopt;   //!
  TBranch * b_t1pfMETcalophi;   //!
  TBranch * b_t1pfMETcalosumEt;   //!
  TBranch * b_t1pfMETgenMETpt;   //!
  TBranch * b_t1pfMETgenMETphi;   //!
  TBranch * b_t1pfMETgenMETsumEt;   //!
  TBranch * b_njets;   //!
  TBranch * b_jetmatch;   //!
  TBranch * b_jetE;   //!
  TBranch * b_jetpt;   //!
  TBranch * b_jetphi;   //!
  TBranch * b_jeteta;   //!
  TBranch * b_nhotons;   //!
  TBranch * b_phmatch;   //!
  TBranch * b_phVID;   //!
  TBranch * b_phE;   //!
  TBranch * b_phpt;   //!
  TBranch * b_phphi;   //!
  TBranch * b_pheta;   //!
  TBranch * b_phscX;   //!
  TBranch * b_phscY;   //!
  TBranch * b_phscZ;   //!
  TBranch * b_phscE;   //!
  TBranch * b_phnrhs;   //!
  TBranch * b_phseedpos;   //!
  TBranch * b_phrhXs;   //!
  TBranch * b_phrhYs;   //!
  TBranch * b_phrhZs;   //!
  TBranch * b_phrhEs;   //!
  TBranch * b_phrhdelRs;   //!
  TBranch * b_phrhtimes;   //!
  TBranch * b_phrhIDs;   //!
  TBranch * b_phrhOOTs;   //!
};
#endif
