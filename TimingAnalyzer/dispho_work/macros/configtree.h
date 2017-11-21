//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Tue Nov 21 00:03:39 2017 by ROOT version 6.10/05
// from TTree configtree/configtree
// found on file: dispho.root
//////////////////////////////////////////////////////////

#ifndef configtree_h
#define configtree_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>

// Header file for the classes stored in the TTree if any.
#include "string"

class configtree {
public :
   TTree          *fChain;   //!pointer to the analyzed TTree or TChain
   Int_t           fCurrent; //!current Tree number in a TChain

// Fixed size dimensions of array or collections stored in the TTree if any.

   // Declaration of leaf types
   UInt_t          blindSF;
   Bool_t          applyBlindSF;
   Float_t         blindMET;
   Bool_t          applyBlindMET;
   Float_t         jetpTmin;
   Int_t           jetIDmin;
   Float_t         rhEmin;
   Float_t         phpTmin;
   string          *phIDmin;
   Float_t         seedTimemin;
   Bool_t          splitPho;
   Bool_t          onlyGED;
   Bool_t          onlyOOT;
   Bool_t          storeRecHits;
   Bool_t          applyTrigger;
   Float_t         minHT;
   Bool_t          applyHT;
   Float_t         phgoodpTmin;
   string          *phgoodIDmin;
   Bool_t          applyPhGood;
   Float_t         dRmin;
   Float_t         pTres;
   Float_t         trackdRmin;
   Float_t         trackpTmin;
   string          *inputPaths;
   string          *inputFilters;
   Bool_t          isGMSB;
   Bool_t          isHVDS;
   Bool_t          isBkgd;
   Float_t         xsec;
   Float_t         filterEff;
   Float_t         BR;

   // List of branches
   TBranch        *b_blindSF;   //!
   TBranch        *b_applyBlindSF;   //!
   TBranch        *b_blindMET;   //!
   TBranch        *b_applyBlindMET;   //!
   TBranch        *b_jetpTmin;   //!
   TBranch        *b_jetIDmin;   //!
   TBranch        *b_rhEmin;   //!
   TBranch        *b_phpTmin;   //!
   TBranch        *b_phIDmin;   //!
   TBranch        *b_seedTimemin;   //!
   TBranch        *b_splitPho;   //!
   TBranch        *b_onlyGED;   //!
   TBranch        *b_onlyOOT;   //!
   TBranch        *b_storeRecHits;   //!
   TBranch        *b_applyTrigger;   //!
   TBranch        *b_minHT;   //!
   TBranch        *b_applyHT;   //!
   TBranch        *b_phgoodpTmin;   //!
   TBranch        *b_phgoodIDmin;   //!
   TBranch        *b_applyPhGood;   //!
   TBranch        *b_dRmin;   //!
   TBranch        *b_pTres;   //!
   TBranch        *b_trackdRmin;   //!
   TBranch        *b_trackpTmin;   //!
   TBranch        *b_inputPaths;   //!
   TBranch        *b_inputFilters;   //!
   TBranch        *b_isGMSB;   //!
   TBranch        *b_isHVDS;   //!
   TBranch        *b_isBkgd;   //!
   TBranch        *b_xsec;   //!
   TBranch        *b_filterEff;   //!
   TBranch        *b_BR;   //!

   configtree(TTree *tree=0);
   virtual ~configtree();
   virtual Int_t    Cut(Long64_t entry);
   virtual Int_t    GetEntry(Long64_t entry);
   virtual Long64_t LoadTree(Long64_t entry);
   virtual void     Init(TTree *tree);
   virtual void     Loop();
   virtual Bool_t   Notify();
   virtual void     Show(Long64_t entry = -1);
};

#endif

#ifdef configtree_cxx
configtree::configtree(TTree *tree) : fChain(0) 
{
// if parameter tree is not specified (or zero), connect the file
// used to generate this class and read the Tree.
   if (tree == 0) {
      TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject("dispho.root");
      if (!f || !f->IsOpen()) {
         f = new TFile("dispho.root");
      }
      TDirectory * dir = (TDirectory*)f->Get("dispho.root:/tree");
      dir->GetObject("configtree",tree);

   }
   Init(tree);
}

configtree::~configtree()
{
   if (!fChain) return;
   delete fChain->GetCurrentFile();
}

Int_t configtree::GetEntry(Long64_t entry)
{
// Read contents of entry.
   if (!fChain) return 0;
   return fChain->GetEntry(entry);
}
Long64_t configtree::LoadTree(Long64_t entry)
{
// Set the environment to read one entry
   if (!fChain) return -5;
   Long64_t centry = fChain->LoadTree(entry);
   if (centry < 0) return centry;
   if (fChain->GetTreeNumber() != fCurrent) {
      fCurrent = fChain->GetTreeNumber();
      Notify();
   }
   return centry;
}

void configtree::Init(TTree *tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the branch addresses and branch
   // pointers of the tree will be set.
   // It is normally not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.
   // Init() will be called many times when running on PROOF
   // (once per file to be processed).

   // Set object pointer
   phIDmin = 0;
   phgoodIDmin = 0;
   inputPaths = 0;
   inputFilters = 0;
   // Set branch addresses and branch pointers
   if (!tree) return;
   fChain = tree;
   fCurrent = -1;
   fChain->SetMakeClass(1);

   fChain->SetBranchAddress("blindSF", &blindSF, &b_blindSF);
   fChain->SetBranchAddress("applyBlindSF", &applyBlindSF, &b_applyBlindSF);
   fChain->SetBranchAddress("blindMET", &blindMET, &b_blindMET);
   fChain->SetBranchAddress("applyBlindMET", &applyBlindMET, &b_applyBlindMET);
   fChain->SetBranchAddress("jetpTmin", &jetpTmin, &b_jetpTmin);
   fChain->SetBranchAddress("jetIDmin", &jetIDmin, &b_jetIDmin);
   fChain->SetBranchAddress("rhEmin", &rhEmin, &b_rhEmin);
   fChain->SetBranchAddress("phpTmin", &phpTmin, &b_phpTmin);
   fChain->SetBranchAddress("phIDmin", &phIDmin, &b_phIDmin);
   fChain->SetBranchAddress("seedTimemin", &seedTimemin, &b_seedTimemin);
   fChain->SetBranchAddress("splitPho", &splitPho, &b_splitPho);
   fChain->SetBranchAddress("onlyGED", &onlyGED, &b_onlyGED);
   fChain->SetBranchAddress("onlyOOT", &onlyOOT, &b_onlyOOT);
   fChain->SetBranchAddress("storeRecHits", &storeRecHits, &b_storeRecHits);
   fChain->SetBranchAddress("applyTrigger", &applyTrigger, &b_applyTrigger);
   fChain->SetBranchAddress("minHT", &minHT, &b_minHT);
   fChain->SetBranchAddress("applyHT", &applyHT, &b_applyHT);
   fChain->SetBranchAddress("phgoodpTmin", &phgoodpTmin, &b_phgoodpTmin);
   fChain->SetBranchAddress("phgoodIDmin", &phgoodIDmin, &b_phgoodIDmin);
   fChain->SetBranchAddress("applyPhGood", &applyPhGood, &b_applyPhGood);
   fChain->SetBranchAddress("dRmin", &dRmin, &b_dRmin);
   fChain->SetBranchAddress("pTres", &pTres, &b_pTres);
   fChain->SetBranchAddress("trackdRmin", &trackdRmin, &b_trackdRmin);
   fChain->SetBranchAddress("trackpTmin", &trackpTmin, &b_trackpTmin);
   fChain->SetBranchAddress("inputPaths", &inputPaths, &b_inputPaths);
   fChain->SetBranchAddress("inputFilters", &inputFilters, &b_inputFilters);
   fChain->SetBranchAddress("isGMSB", &isGMSB, &b_isGMSB);
   fChain->SetBranchAddress("isHVDS", &isHVDS, &b_isHVDS);
   fChain->SetBranchAddress("isBkgd", &isBkgd, &b_isBkgd);
   fChain->SetBranchAddress("xsec", &xsec, &b_xsec);
   fChain->SetBranchAddress("filterEff", &filterEff, &b_filterEff);
   fChain->SetBranchAddress("BR", &BR, &b_BR);
   Notify();
}

Bool_t configtree::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}

void configtree::Show(Long64_t entry)
{
// Print contents of entry.
// If entry is not specified, print current entry
   if (!fChain) return;
   fChain->Show(entry);
}
Int_t configtree::Cut(Long64_t entry)
{
// This function may be called from Loop.
// returns  1 if entry is accepted.
// returns -1 otherwise.
   return 1;
}
#endif // #ifdef configtree_cxx
