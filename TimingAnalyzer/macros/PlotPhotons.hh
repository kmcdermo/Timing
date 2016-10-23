#ifndef _plotphotons_
#define _plotphotons_

#include "TROOT.h"
#include "TFile.h"
#include "TTree.h"
#include "TString.h"
#include "TH1F.h"

#include <vector>
#include <map>

typedef std::map<TString,TH1F*> TH1Map;
typedef TH1Map::iterator        TH1MapIter;

class PlotPhotons 
{
public :
  PlotPhotons(TString filename, TString outdir);
  ~PlotPhotons();
  void InitTree();
  void DoPlots();
  void SetupTH1Fs();
  void SetupGenInfoTH1Fs();
  void SetupGenParticlesTH1Fs();
  void SetupObjectCountsTH1Fs();
  void SetupMETTH1Fs();
  void SetupJetsTH1Fs();
  void SetupRecoPhotonsTH1Fs();
  TH1F * MakeTH1F(TString hname, TString htitle, Int_t nbinsx, Float_t xlow, Float_t xhigh, TString xtitle, TString ytitle);
  void EventLoop();
  void FillGenInfo();
  void FillGenParticles();
  void FillObjectCounts();
  void FillMET();
  void FillJets();
  void FillRecoPhotons();
  void OutputTH1Fs();

private :
  // Input vars
  TFile * fInFile; //!pointer to file
  TTree * fInTree; //!pointer to the analyzed TTree

  // In routine vars
  UInt_t  fNEvCheck;
  TH1Map  fPlots;

  // Output vars
  TString fOutDir;
  TFile * fOutFile;

  // Declaration of leaf types
  Int_t   event;
  Int_t   run;
  Int_t   lumi;
  Float_t genwgt;
  Int_t   genpuobs;
  Int_t   genputrue;
  Float_t genN1mass;
  Float_t genN1E;
  Float_t genN1pt;
  Float_t genN1phi;
  Float_t genN1eta;
  Float_t genph1E;
  Float_t genph1pt;
  Float_t genph1phi;
  Float_t genph1eta;
  Int_t   genph1match;
  Float_t gengl1E;
  Float_t gengl1pt;
  Float_t gengl1phi;
  Float_t gengl1eta;
  Float_t genN2mass;
  Float_t genN2E;
  Float_t genN2pt;
  Float_t genN2phi;
  Float_t genN2eta;
  Float_t genph2E;
  Float_t genph2pt;
  Float_t genph2phi;
  Float_t genph2eta;
  Int_t   genph2match;
  Float_t gengl2E;
  Float_t gengl2pt;
  Float_t gengl2phi;
  Float_t gengl2eta;
  Int_t   nvtx;
  Float_t vtxX;
  Float_t vtxY;
  Float_t vtxZ;
  Float_t t1pfmet;
  Float_t t1pfmetphi;
  Float_t t1pfmeteta;
  Float_t t1pfmetsumEt;
  Int_t   njets;
  Int_t   nphotons;
  vector<float> * jetE;
  vector<float> * jetpt;
  vector<float> * jetphi;
  vector<float> * jeteta;
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
  TBranch * b_genN1mass;   //!
  TBranch * b_genN1E;   //!
  TBranch * b_genN1pt;   //!
  TBranch * b_genN1phi;   //!
  TBranch * b_genN1eta;   //!
  TBranch * b_genph1E;   //!
  TBranch * b_genph1pt;   //!
  TBranch * b_genph1phi;   //!
  TBranch * b_genph1eta;   //!
  TBranch * b_genph1match;   //!
  TBranch * b_gengl1E;   //!
  TBranch * b_gengl1pt;   //!
  TBranch * b_gengl1phi;   //!
  TBranch * b_gengl1eta;   //!
  TBranch * b_genN2mass;   //!
  TBranch * b_genN2E;   //!
  TBranch * b_genN2pt;   //!
  TBranch * b_genN2phi;   //!
  TBranch * b_genN2eta;   //!
  TBranch * b_genph2E;   //!
  TBranch * b_genph2pt;   //!
  TBranch * b_genph2phi;   //!
  TBranch * b_genph2eta;   //!
  TBranch * b_genph2match;   //!
  TBranch * b_gengl2E;   //!
  TBranch * b_gengl2pt;   //!
  TBranch * b_gengl2phi;   //!
  TBranch * b_gengl2eta;   //!
  TBranch * b_nvtx;   //!
  TBranch * b_vtxX;   //!
  TBranch * b_vtxY;   //!
  TBranch * b_vtxZ;   //!
  TBranch * b_t1pfmet;   //!
  TBranch * b_t1pfmetphi;   //!
  TBranch * b_t1pfmeteta;   //!
  TBranch * b_t1pfmetsumEt;   //!
  TBranch * b_njets;   //!
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
  TBranch * b_phrhtimes;   //!
  TBranch * b_phrhIDs;   //!
  TBranch * b_phrhOOTs;   //!
};
#endif
