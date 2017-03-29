#ifndef _rechitdumper_
#define _rechitdumper_

#include "TROOT.h"
#include "TFile.h"
#include "TTree.h"
#include "TString.h"

#include <fstream>
#include <vector>
#include <map>
#include <utility>

typedef std::map<TString,Int_t> TStrIntMap;
typedef TStrIntMap::iterator    TStrIntMapIter;

class RecHitDumper 
{
public :
  RecHitDumper(TString filename, TString outdir = "output",
	       TString ph1config = "config/plotter-ph1.txt", TString phanyconfig = "config/plotter-phany.txt", TString photonconfig = "config/plotter-photon.txt");
  ~RecHitDumper();
  void InitTree();
  void InitPh1Config(TString config);
  void InitPhAnyConfig(TString config);
  void InitPhotonConfig(TString config);
  void DoDump(const Bool_t allph = false, const Bool_t leading = false, const Bool_t mostdelayed = false);
  void SetupFiles(const Bool_t allph, const Bool_t leading, const Bool_t mostdelayed);
  void EventLoop(const Bool_t allph, const Bool_t leading, const Bool_t mostdelayed);
  void FillAllPhotons();
  void FillLeadingPhoton();
  void FillMostDelayedPhoton();
  void SaveFiles(const Bool_t allph, const Bool_t leading, const Bool_t mostdelayed)
  Int_t GetLeadingPhoton();
  Int_t GetGoodPhotons(std::vector<Int_t> & goodphotons);
  Int_t GetMostDelayedPhoton();

private :
  // Input vars
  TFile * fInFile; //!pointer to file
  TTree * fInTree; //!pointer to the analyzed TTree

  // In routine vars
  TStrIntMap fPhVIDMap;

  // Output vars
  TString fOutDir;
  std::fstream fSeedDumpAll;
  std::fstream fRHDumpAll;
  std::fstream fSeedDumpLeading;
  std::fstream fRHDumpLeading;
  std::fstream fSeedDumpMostDelayed;
  std::fstream fRHDumpMostDelayed;

  // Declaration of leaf types
  ULong64_t event;
  UInt_t    run;
  UInt_t    lumi;
  Int_t   nphotons;
  std::vector<Float_t> * phpt;
  std::vector<Float_t> * phsceta;
  std::vector<Float_t> * phr9;
  std::vector<Float_t> * phsmaj;
  std::vector<Float_t> * phsmin;
  std::vector<Float_t> * phalpha;
  std::vector<Int_t>   * phVID;
  std::vector<Int_t>   * phnrh;
  std::vector<Int_t>   * phseedpos;
  std::vector<std::vector<Float_t> > * phrhE;
  std::vector<std::vector<Float_t> > * phrhtime;
  std::vector<std::vector<Int_t> >   * phrhID;
  std::vector<std::vector<Int_t> >   * phrhOOT;

  // List of branches
  TBranch * b_event;
  TBranch * b_run;
  TBranch * b_lumi;
  TBranch * b_nphotons;
  TBranch * b_phE;
  TBranch * b_phpt;
  TBranch * b_phsceta;
  TBranch * b_phr9;
  TBranch * b_phsmaj;
  TBranch * b_phsmin;
  TBranch * b_phVID;
  TBranch * b_phnrh;
  TBranch * b_phseedpos;
  TBranch * b_phrhE;
  TBranch * b_phrhtime;
  TBranch * b_phrhID;
  TBranch * b_phrhOOT;
};
#endif
