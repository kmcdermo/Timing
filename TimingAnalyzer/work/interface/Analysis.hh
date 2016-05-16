#ifndef _analysis_
#define _analysis_ 

#include <TROOT.h>
#include <TChain.h>
#include <TTree.h>
#include <TFile.h>
#include <TString.h>
#include <TStyle.h>

class Analysis {
public:
  // functions
  Analysis(TString filename, TString outdir, TString outtype);
  ~Analysis();
  void     InitTree();
  void     TimeResPlots();

private:
  // I/O
  TFile* fInFile;
  TTree* fInTree;

  TString fOutDir;
  TFile*  fOutFile;
  TString fOutType;
  
  // CMS demands this...
  TStyle* fTDRStyle;

public:
  // Declaration of leaf types
  UInt_t          event;
  UInt_t          run;
  UInt_t          lumi;
  Double_t        xsec;
  Double_t        wgt;
  Double_t        pswgt;
  Double_t        puwgt;
  Int_t           puobs;
  Int_t           putrue;
  UInt_t          nvtx;
  UChar_t         hltphoton165;
  UChar_t         hltphoton175;
  UChar_t         hltphoton120;
  UChar_t         hltdoubleel;
  UChar_t         hltsingleel;
  UInt_t          nelectrons;
  UInt_t          ntightelectrons;
  UInt_t          nheepelectrons;
  UInt_t          nphotons;
  Int_t           el1pid;
  Double_t        el1pt;
  Double_t        el1eta;
  Double_t        el1phi;
  Int_t           el1id;
  Int_t           el1idl;
  Int_t           el2pid;
  Double_t        el2pt;
  Double_t        el2eta;
  Double_t        el2phi;
  Int_t           el2id;
  Int_t           el2idl;
  Double_t        el1time;
  Double_t        el2time;
  Double_t        zeemass;
  Double_t        zeept;
  Double_t        zeeeta;
  Double_t        zeephi;
  Int_t           phidl;
  Int_t           phidm;
  Int_t           phidt;
  Int_t           phidh;
  Double_t        phpt;
  Double_t        pheta;
  Double_t        phphi;
  Int_t           wzid;
  Double_t        wzmass;
  Double_t        wzpt;
  Double_t        wzeta;
  Double_t        wzphi;
  Int_t           l1id;
  Double_t        l1pt;
  Double_t        l1eta;
  Double_t        l1phi;
  Int_t           l2id;
  Double_t        l2pt;
  Double_t        l2eta;
  Double_t        l2phi;
  Int_t           parid;
  Double_t        parpt;
  Double_t        pareta;
  Double_t        parphi;
  Int_t           ancid;
  Double_t        ancpt;
  Double_t        anceta;
  Double_t        ancphi;

  // List of branches
  TBranch        *b_event;   //!
  TBranch        *b_run;   //!
  TBranch        *b_lumi;   //!
  TBranch        *b_xsec;   //!
  TBranch        *b_wgt;   //!
  TBranch        *b_pswgt;   //!
  TBranch        *b_puwgt;   //!
  TBranch        *b_puobs;   //!
  TBranch        *b_putrue;   //!
  TBranch        *b_nvtx;   //!
  TBranch        *b_hltphoton165;   //!
  TBranch        *b_hltphoton175;   //!
  TBranch        *b_hltphoton120;   //!
  TBranch        *b_hltdoubleel;   //!
  TBranch        *b_hltsingleel;   //!
  TBranch        *b_nelectrons;   //!
  TBranch        *b_ntightelectrons;   //!
  TBranch        *b_nheepelectrons;   //!
  TBranch        *b_nphotons;   //!
  TBranch        *b_el1pid;   //!
  TBranch        *b_el1pt;   //!
  TBranch        *b_el1eta;   //!
  TBranch        *b_el1phi;   //!
  TBranch        *b_el1id;   //!
  TBranch        *b_el1idl;   //!
  TBranch        *b_el2pid;   //!
  TBranch        *b_el2pt;   //!
  TBranch        *b_el2eta;   //!
  TBranch        *b_el2phi;   //!
  TBranch        *b_el2id;   //!
  TBranch        *b_el2idl;   //!
  TBranch        *b_el1time;   //!
  TBranch        *b_el2time;   //!
  TBranch        *b_zeemass;   //!
  TBranch        *b_zeeept;   //!
  TBranch        *b_zeeeta;   //!
  TBranch        *b_zeephi;   //!
  TBranch        *b_phidl;   //!
  TBranch        *b_phidm;   //!
  TBranch        *b_phidt;   //!
  TBranch        *b_phidh;   //!
  TBranch        *b_phpt;   //!
  TBranch        *b_pheta;   //!
  TBranch        *b_phphi;   //!
  TBranch        *b_wzid;   //!
  TBranch        *b_wzmass;   //!
  TBranch        *b_wzpt;   //!
  TBranch        *b_wzeta;   //!
  TBranch        *b_wzphi;   //!
  TBranch        *b_l1id;   //!
  TBranch        *b_l1pt;   //!
  TBranch        *b_l1eta;   //!
  TBranch        *b_l1phi;   //!
  TBranch        *b_l2id;   //!
  TBranch        *b_l2pt;   //!
  TBranch        *b_l2eta;   //!
  TBranch        *b_l2phi;   //!
  TBranch        *b_parid;   //!
  TBranch        *b_parpt;   //!
  TBranch        *b_pareta;   //!
  TBranch        *b_parphi;   //!
  TBranch        *b_ancid;   //!
  TBranch        *b_ancpt;   //!
  TBranch        *b_anceta;   //!
  TBranch        *b_ancphi;   //!
};

#endif
