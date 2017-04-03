#ifndef _plotphotons_
#define _plotphotons_

#include "TROOT.h"
#include "TFile.h"
#include "TTree.h"
#include "TString.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TLorentzVector.h"
#include "TVector3.h"

#include <fstream>
#include <vector>
#include <map>
#include <utility>

static const Float_t sol = 2.99792458e8; // cm/s

inline Float_t rad2 (const Float_t x, const Float_t y){return x*x + y*y;}
inline Float_t rad2 (const Float_t x, const Float_t y, const Float_t z){return x*x + y*y + z*z;}
inline Float_t gamma(const Float_t p, const Float_t m){return std::sqrt(1.f+std::pow(p/m,2));}
inline Float_t bg   (const Float_t p, const Float_t m){return std::abs(p/m);}
inline void    semiR(const Float_t insmaj, const Float_t insmin, Float_t & smaj, Float_t & smin)
{
  smaj = 1.f/std::sqrt(insmin);
  smin = 1.f/std::sqrt(insmaj);
}

typedef std::pair<TH1F*,TH1F*>     TH1Pair;
typedef std::map<TString, TH1Pair> TH1PairMap;
typedef TH1PairMap::iterator       TH1PairMapIter;

typedef std::pair<TH2F*,TH2F*>     TH2Pair;
typedef std::map<TString, TH2Pair> TH2PairMap;
typedef TH2PairMap::iterator       TH2PairMapIter;

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
  PlotPhotons(TString filename, Bool_t isGMSB = false, Bool_t isHVDS = false, Bool_t isBkg = false, 
	      TString outdir = "output", Bool_t savehists = false, Bool_t savesub = false,
	      TString genericconfig = "config/plotter-generic.txt", TString hltconfig = "config/plotter-hlt.txt", TString jetconfig = "config/plotter-jet.txt",
	      TString ph1config = "config/plotter-ph1.txt", TString phanyconfig = "config/plotter-phany.txt", TString photonconfig = "config/plotter-photon.txt");
  ~PlotPhotons();
  void InitTree();
  void InitGenericConfig(TString config);
  void InitHLTConfig(TString config);
  void InitJetConfig(TString config);
  void InitPh1Config(TString config);
  void InitPhAnyConfig(TString config);
  void InitPhotonConfig(TString config);
  void DoPlots(Bool_t geninfo = false, Bool_t vtxs = false, Bool_t met = false, Bool_t jets = false, 
	       Bool_t photons = false, Bool_t ph1 = false, Bool_t phdelay = false, Bool_t trigger = false, Bool_t analysis = false);
  void SetupPlots(Bool_t geninfo, Bool_t vtxs, Bool_t met, Bool_t jets, Bool_t photons, Bool_t ph1, Bool_t phdelay, Bool_t trigger, Bool_t analysis);
  void SetupGenInfo();
  void SetupGMSB();
  void SetupGenJets();
  void SetupHVDS();
  void SetupVertices();
  void SetupMET();
  void SetupJets();
  void SetupRecoPhotons();
  void SetupLeading();
  void SetupMostDelayed();
  void SetupTrigger();
  void SetupAnalysis();
  TH1F * MakeTH1F(TString hname, TString htitle, Int_t nbinsx, Float_t xlow, Float_t xhigh, TString xtitle, TString ytitle, TString subdir);
  std::pair<TH1F*,TH1F*> MakeTrigTH1Fs(TString hname, TString htitle, Int_t nbinsx, Float_t xlow, Float_t xhigh, TString xtitle, TString ytitle, TString path, TString subdir);
  void MakeEffPlot(TH1F *& eff, TString hname, TH1F *& denom, TH1F *& numer);
  TH2F * MakeTH2F(TString hname, TString htitle, Int_t nbinsx, Float_t xlow, Float_t xhigh, TString xtitle, Int_t nbinsy, Float_t ylow, Float_t yhigh, TString ytitle, TString subdir);
  std::pair<TH2F*,TH2F*> MakeTrigTH2Fs(TString hname, TString htitle, Int_t nbinsx, Float_t xlow, Float_t xhigh, TString xtitle, Int_t nbinsy, Float_t ylow, Float_t yhigh, TString ytitle, TString path, TString subdir);
  void MakeEffPlot2D(TH2F *& eff, TString hname, TH2F *& denom, TH2F *& numer);
  void EventLoop(Bool_t geninfo, Bool_t vtxs, Bool_t met, Bool_t jets, Bool_t photons, Bool_t ph1, Bool_t phdelay, Bool_t trigger, Bool_t analysis);
  Bool_t CountEvents();
  Int_t GetLeadingPhoton();
  Int_t GetGoodPhotons(std::vector<Int_t> & goodphotons);
  Int_t GetMostDelayedPhoton();
  Float_t GetJetHTAbovePt();
  Int_t GetNJetsAbovePt();
  void FillGenInfo();
  void FillGMSB();
  void FillGenJets();
  void FillHVDS();
  void FillVertices();
  void FillMET();
  void FillJets();
  void FillRecoPhotons();
  void FillLeading();
  void FillMostDelayed();
  void FillTrigger();
  void FillTriggerPlot(TH1Pair & th1pair, const Bool_t passed, const Float_t value);
  void FillTriggerPlot2D(TH2Pair & th2pair, const Bool_t passed, const Float_t xvalue, const Float_t yvalue);
  void FillAnalysis(const Bool_t passed);
  void DumpEventCounts();
  void MakeSubDirs();
  void OutputTH1Fs();
  void OutputTrigTH1Fs();
  void OutputTH2Fs();
  void OutputTrigTH2Fs();

private :
  // Input vars
  TFile * fInFile; //!pointer to file
  TTree * fInTree; //!pointer to the analyzed TTree
  const Bool_t fIsGMSB;
  const Bool_t fIsHVDS;
  const Bool_t fIsBkg;
        Bool_t fIsMC;
  const Bool_t fSaveHists;
  const Bool_t fSaveSub;

  // In routine vars
  TH1Map  fPlots;
  TH2Map  fPlots2D;
  TH1PairMap fTrigPlots;
  TH2PairMap fTrigPlots2D;
  TStrIntMap fEfficiency;
  Float_t    fCTau;
  TStrIntMap fPhVIDMap;

  // Output vars
  TString fOutDir;
  TString fOutDump;
  TStrMap fSubDirs;
  TFile * fOutFile;

  // Declaration of leaf types
  ULong64_t event;
  UInt_t    run;
  UInt_t    lumi;
  std::vector<Bool_t> * triggerBits;
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
  Int_t   nvPions;   
  std::vector<Float_t> * genvPionprodvx;
  std::vector<Float_t> * genvPionprodvy;
  std::vector<Float_t> * genvPionprodvz;
  std::vector<Float_t> * genvPiondecayvx;
  std::vector<Float_t> * genvPiondecayvy;
  std::vector<Float_t> * genvPiondecayvz;
  std::vector<Float_t> * genvPionmass;
  std::vector<Float_t> * genvPionE;
  std::vector<Float_t> * genvPionpt;
  std::vector<Float_t> * genvPionphi;
  std::vector<Float_t> * genvPioneta;
  std::vector<Float_t> * genHVph1E;
  std::vector<Float_t> * genHVph1pt;
  std::vector<Float_t> * genHVph1phi;
  std::vector<Float_t> * genHVph1eta;
  std::vector<Float_t> * genHVph2E;
  std::vector<Float_t> * genHVph2pt;
  std::vector<Float_t> * genHVph2phi;
  std::vector<Float_t> * genHVph2eta;
  std::vector<Int_t>   * genHVph1match;
  std::vector<Int_t>   * genHVph2match;
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
  std::vector<Bool_t>  * phIsGenMatched;
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
  std::vector<Int_t>   * phVID;
  std::vector<Int_t>   * phHoE_b;
  std::vector<Int_t>   * phsieie_b;
  std::vector<Int_t>   * phChgIso_b;
  std::vector<Int_t>   * phNeuIso_b;
  std::vector<Int_t>   * phIso_b;
  std::vector<std::vector<Int_t> >   * phIsHLTMatched;
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
  TBranch * b_triggerBits;
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
  TBranch * b_nvPions;
  TBranch * b_genvPionprodvx;
  TBranch * b_genvPionprodvy;
  TBranch * b_genvPionprodvz;
  TBranch * b_genvPiondecayvx;
  TBranch * b_genvPiondecayvy;
  TBranch * b_genvPiondecayvz;
  TBranch * b_genvPionmass;
  TBranch * b_genvPionE;
  TBranch * b_genvPionpt;
  TBranch * b_genvPionphi;
  TBranch * b_genvPioneta;
  TBranch * b_genHVph1E;
  TBranch * b_genHVph1pt;
  TBranch * b_genHVph1phi;
  TBranch * b_genHVph1eta;
  TBranch * b_genHVph2E;
  TBranch * b_genHVph2pt;
  TBranch * b_genHVph2phi;
  TBranch * b_genHVph2eta;
  TBranch * b_genHVph1match;
  TBranch * b_genHVph2match;
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
  TBranch * b_phIsGenMatched;
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
  TBranch * b_phVID;
  TBranch * b_phHoE_b;
  TBranch * b_phsieie_b;
  TBranch * b_phChgIso_b;
  TBranch * b_phNeuIso_b;
  TBranch * b_phIso_b;
  TBranch * b_phIsHLTMatched;
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
