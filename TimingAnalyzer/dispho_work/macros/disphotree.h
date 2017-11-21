o//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Tue Nov 21 00:03:48 2017 by ROOT version 6.10/05
// from TTree disphotree/disphotree
// found on file: dispho.root
//////////////////////////////////////////////////////////

#ifndef disphotree_h
#define disphotree_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>

// Header file for the classes stored in the TTree if any.
#include "vector"
#include "vector"
#include "vector"

class disphotree {
public :
   TTree          *fChain;   //!pointer to the analyzed TTree or TChain
   Int_t           fCurrent; //!current Tree number in a TChain

// Fixed size dimensions of array or collections stored in the TTree if any.

   // Declaration of leaf types

   vector<float>   *rheta;
   vector<float>   *rhphi;
   vector<float>   *rhE;
   vector<float>   *rhtime;
   vector<int>     *rhOOT;
   vector<unsigned int> *rhID;

   Float_t         phoE_0;
   Float_t         phopt_0;
   Float_t         phoeta_0;
   Float_t         phophi_0;
   Float_t         phoscE_0;
   Float_t         phosceta_0;
   Float_t         phoscphi_0;
   Float_t         phoHoE_0;
   Float_t         phor9_0;
   Float_t         phoChgHadIso_0;
   Float_t         phoNeuHadIso_0;
   Float_t         phoPhoIso_0;
   Float_t         phoEcalPFClIso_0;
   Float_t         phoHcalPFClIso_0;
   Float_t         phoTrkIso_0;
   Float_t         phosieie_0;
   Float_t         phosipip_0;
   Float_t         phosieip_0;
   Float_t         phosmaj_0;
   Float_t         phosmin_0;
   Float_t         phoalpha_0;
   Int_t           phoseed_0;
   vector<int>     *phorecHits_0;
   Bool_t          phoisOOT_0;
   Bool_t          phoisEB_0;
   Bool_t          phoisHLT_0;
   Bool_t          phoisTrk_0;
   Int_t           phoID_0;
   Float_t         phoE_1;
   Float_t         phopt_1;
   Float_t         phoeta_1;
   Float_t         phophi_1;
   Float_t         phoscE_1;
   Float_t         phosceta_1;
   Float_t         phoscphi_1;
   Float_t         phoHoE_1;
   Float_t         phor9_1;
   Float_t         phoChgHadIso_1;
   Float_t         phoNeuHadIso_1;
   Float_t         phoPhoIso_1;
   Float_t         phoEcalPFClIso_1;
   Float_t         phoHcalPFClIso_1;
   Float_t         phoTrkIso_1;
   Float_t         phosieie_1;
   Float_t         phosipip_1;
   Float_t         phosieip_1;
   Float_t         phosmaj_1;
   Float_t         phosmin_1;
   Float_t         phoalpha_1;
   Int_t           phoseed_1;
   vector<int>     *phorecHits_1;
   Bool_t          phoisOOT_1;
   Bool_t          phoisEB_1;
   Bool_t          phoisHLT_1;
   Bool_t          phoisTrk_1;
   Int_t           phoID_1;
   Float_t         phoE_2;
   Float_t         phopt_2;
   Float_t         phoeta_2;
   Float_t         phophi_2;
   Float_t         phoscE_2;
   Float_t         phosceta_2;
   Float_t         phoscphi_2;
   Float_t         phoHoE_2;
   Float_t         phor9_2;
   Float_t         phoChgHadIso_2;
   Float_t         phoNeuHadIso_2;
   Float_t         phoPhoIso_2;
   Float_t         phoEcalPFClIso_2;
   Float_t         phoHcalPFClIso_2;
   Float_t         phoTrkIso_2;
   Float_t         phosieie_2;
   Float_t         phosipip_2;
   Float_t         phosieip_2;
   Float_t         phosmaj_2;
   Float_t         phosmin_2;
   Float_t         phoalpha_2;
   Int_t           phoseed_2;
   vector<int>     *phorecHits_2;
   Bool_t          phoisOOT_2;
   Bool_t          phoisEB_2;
   Bool_t          phoisHLT_2;
   Bool_t          phoisTrk_2;
   Int_t           phoID_2;
   Float_t         phoE_3;
   Float_t         phopt_3;
   Float_t         phoeta_3;
   Float_t         phophi_3;
   Float_t         phoscE_3;
   Float_t         phosceta_3;
   Float_t         phoscphi_3;
   Float_t         phoHoE_3;
   Float_t         phor9_3;
   Float_t         phoChgHadIso_3;
   Float_t         phoNeuHadIso_3;
   Float_t         phoPhoIso_3;
   Float_t         phoEcalPFClIso_3;
   Float_t         phoHcalPFClIso_3;
   Float_t         phoTrkIso_3;
   Float_t         phosieie_3;
   Float_t         phosipip_3;
   Float_t         phosieip_3;
   Float_t         phosmaj_3;
   Float_t         phosmin_3;
   Float_t         phoalpha_3;
   Int_t           phoseed_3;
   vector<int>     *phorecHits_3;
   Bool_t          phoisOOT_3;
   Bool_t          phoisEB_3;
   Bool_t          phoisHLT_3;
   Bool_t          phoisTrk_3;
   Int_t           phoID_3;
   Int_t           phoisSignal_0;
   Bool_t          phoisGen_0;
   Int_t           phoisSignal_1;
   Bool_t          phoisGen_1;
   Int_t           phoisSignal_2;
   Bool_t          phoisGen_2;
   Int_t           phoisSignal_3;
   Bool_t          phoisGen_3;

   // List of branches
 
   TBranch        *b_rheta;   //!
   TBranch        *b_rhphi;   //!
   TBranch        *b_rhE;   //!
   TBranch        *b_rhtime;   //!
   TBranch        *b_rhOOT;   //!
   TBranch        *b_rhID;   //!
 
   TBranch        *b_phoE_0;   //!
   TBranch        *b_phopt_0;   //!
   TBranch        *b_phoeta_0;   //!
   TBranch        *b_phophi_0;   //!
   TBranch        *b_phoscE_0;   //!
   TBranch        *b_phosceta_0;   //!
   TBranch        *b_phoscphi_0;   //!
   TBranch        *b_phoHoE_0;   //!
   TBranch        *b_phor9_0;   //!
   TBranch        *b_phoChgHadIso_0;   //!
   TBranch        *b_phoNeuHadIso_0;   //!
   TBranch        *b_phoPhoIso_0;   //!
   TBranch        *b_phoEcalPFClIso_0;   //!
   TBranch        *b_phoHcalPFClIso_0;   //!
   TBranch        *b_phoTrkIso_0;   //!
   TBranch        *b_phosieie_0;   //!
   TBranch        *b_phosipip_0;   //!
   TBranch        *b_phosieip_0;   //!
   TBranch        *b_phosmaj_0;   //!
   TBranch        *b_phosmin_0;   //!
   TBranch        *b_phoalpha_0;   //!
   TBranch        *b_phoseed_0;   //!
   TBranch        *b_phorecHits_0;   //!
   TBranch        *b_phoisOOT_0;   //!
   TBranch        *b_phoisEB_0;   //!
   TBranch        *b_phoisHLT_0;   //!
   TBranch        *b_phoisTrk_0;   //!
   TBranch        *b_phoID_0;   //!
   TBranch        *b_phoE_1;   //!
   TBranch        *b_phopt_1;   //!
   TBranch        *b_phoeta_1;   //!
   TBranch        *b_phophi_1;   //!
   TBranch        *b_phoscE_1;   //!
   TBranch        *b_phosceta_1;   //!
   TBranch        *b_phoscphi_1;   //!
   TBranch        *b_phoHoE_1;   //!
   TBranch        *b_phor9_1;   //!
   TBranch        *b_phoChgHadIso_1;   //!
   TBranch        *b_phoNeuHadIso_1;   //!
   TBranch        *b_phoPhoIso_1;   //!
   TBranch        *b_phoEcalPFClIso_1;   //!
   TBranch        *b_phoHcalPFClIso_1;   //!
   TBranch        *b_phoTrkIso_1;   //!
   TBranch        *b_phosieie_1;   //!
   TBranch        *b_phosipip_1;   //!
   TBranch        *b_phosieip_1;   //!
   TBranch        *b_phosmaj_1;   //!
   TBranch        *b_phosmin_1;   //!
   TBranch        *b_phoalpha_1;   //!
   TBranch        *b_phoseed_1;   //!
   TBranch        *b_phorecHits_1;   //!
   TBranch        *b_phoisOOT_1;   //!
   TBranch        *b_phoisEB_1;   //!
   TBranch        *b_phoisHLT_1;   //!
   TBranch        *b_phoisTrk_1;   //!
   TBranch        *b_phoID_1;   //!
   TBranch        *b_phoE_2;   //!
   TBranch        *b_phopt_2;   //!
   TBranch        *b_phoeta_2;   //!
   TBranch        *b_phophi_2;   //!
   TBranch        *b_phoscE_2;   //!
   TBranch        *b_phosceta_2;   //!
   TBranch        *b_phoscphi_2;   //!
   TBranch        *b_phoHoE_2;   //!
   TBranch        *b_phor9_2;   //!
   TBranch        *b_phoChgHadIso_2;   //!
   TBranch        *b_phoNeuHadIso_2;   //!
   TBranch        *b_phoPhoIso_2;   //!
   TBranch        *b_phoEcalPFClIso_2;   //!
   TBranch        *b_phoHcalPFClIso_2;   //!
   TBranch        *b_phoTrkIso_2;   //!
   TBranch        *b_phosieie_2;   //!
   TBranch        *b_phosipip_2;   //!
   TBranch        *b_phosieip_2;   //!
   TBranch        *b_phosmaj_2;   //!
   TBranch        *b_phosmin_2;   //!
   TBranch        *b_phoalpha_2;   //!
   TBranch        *b_phoseed_2;   //!
   TBranch        *b_phorecHits_2;   //!
   TBranch        *b_phoisOOT_2;   //!
   TBranch        *b_phoisEB_2;   //!
   TBranch        *b_phoisHLT_2;   //!
   TBranch        *b_phoisTrk_2;   //!
   TBranch        *b_phoID_2;   //!
   TBranch        *b_phoE_3;   //!
   TBranch        *b_phopt_3;   //!
   TBranch        *b_phoeta_3;   //!
   TBranch        *b_phophi_3;   //!
   TBranch        *b_phoscE_3;   //!
   TBranch        *b_phosceta_3;   //!
   TBranch        *b_phoscphi_3;   //!
   TBranch        *b_phoHoE_3;   //!
   TBranch        *b_phor9_3;   //!
   TBranch        *b_phoChgHadIso_3;   //!
   TBranch        *b_phoNeuHadIso_3;   //!
   TBranch        *b_phoPhoIso_3;   //!
   TBranch        *b_phoEcalPFClIso_3;   //!
   TBranch        *b_phoHcalPFClIso_3;   //!
   TBranch        *b_phoTrkIso_3;   //!
   TBranch        *b_phosieie_3;   //!
   TBranch        *b_phosipip_3;   //!
   TBranch        *b_phosieip_3;   //!
   TBranch        *b_phosmaj_3;   //!
   TBranch        *b_phosmin_3;   //!
   TBranch        *b_phoalpha_3;   //!
   TBranch        *b_phoseed_3;   //!
   TBranch        *b_phorecHits_3;   //!
   TBranch        *b_phoisOOT_3;   //!
   TBranch        *b_phoisEB_3;   //!
   TBranch        *b_phoisHLT_3;   //!
   TBranch        *b_phoisTrk_3;   //!
   TBranch        *b_phoID_3;   //!
   TBranch        *b_phoisSignal_0;   //!
   TBranch        *b_phoisGen_0;   //!
   TBranch        *b_phoisSignal_1;   //!
   TBranch        *b_phoisGen_1;   //!
   TBranch        *b_phoisSignal_2;   //!
   TBranch        *b_phoisGen_2;   //!
   TBranch        *b_phoisSignal_3;   //!
   TBranch        *b_phoisGen_3;   //!

   disphotree(TTree *tree=0);
   virtual ~disphotree();
   virtual Int_t    Cut(Long64_t entry);
   virtual Int_t    GetEntry(Long64_t entry);
   virtual Long64_t LoadTree(Long64_t entry);
   virtual void     Init(TTree *tree);
   virtual void     Loop();
   virtual Bool_t   Notify();
   virtual void     Show(Long64_t entry = -1);
};

#endif

#ifdef disphotree_cxx
disphotree::disphotree(TTree *tree) : fChain(0) 
{
// if parameter tree is not specified (or zero), connect the file
// used to generate this class and read the Tree.
   if (tree == 0) {
      TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject("dispho.root");
      if (!f || !f->IsOpen()) {
         f = new TFile("dispho.root");
      }
      TDirectory * dir = (TDirectory*)f->Get("dispho.root:/tree");
      dir->GetObject("disphotree",tree);

   }
   Init(tree);
}

disphotree::~disphotree()
{
   if (!fChain) return;
   delete fChain->GetCurrentFile();
}

Int_t disphotree::GetEntry(Long64_t entry)
{
// Read contents of entry.
   if (!fChain) return 0;
   return fChain->GetEntry(entry);
}
Long64_t disphotree::LoadTree(Long64_t entry)
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

void disphotree::Init(TTree *tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the branch addresses and branch
   // pointers of the tree will be set.
   // It is normally not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.
   // Init() will be called many times when running on PROOF
   // (once per file to be processed).

   // Set object pointer
   rheta = 0;
   rhphi = 0;
   rhE = 0;
   rhtime = 0;
   rhOOT = 0;
   rhID = 0;
   phorecHits_0 = 0;
   phorecHits_1 = 0;
   phorecHits_2 = 0;
   phorecHits_3 = 0;
   // Set branch addresses and branch pointers
   if (!tree) return;
   fChain = tree;
   fCurrent = -1;
   fChain->SetMakeClass(1);

   fChain->SetBranchAddress("genwgt", &genwgt, &b_genwgt);
   fChain->SetBranchAddress("genpuobs", &genpuobs, &b_genpuobs);
   fChain->SetBranchAddress("genputrue", &genputrue, &b_genputrue);
   fChain->SetBranchAddress("nNeutoPhGr", &nNeutoPhGr, &b_nNeutoPhGr);
   fChain->SetBranchAddress("genNmass_0", &genNmass_0, &b_genNmass_0);
   fChain->SetBranchAddress("genNE_0", &genNE_0, &b_genNE_0);
   fChain->SetBranchAddress("genNpt_0", &genNpt_0, &b_genNpt_0);
   fChain->SetBranchAddress("genNphi_0", &genNphi_0, &b_genNphi_0);
   fChain->SetBranchAddress("genNeta_0", &genNeta_0, &b_genNeta_0);
   fChain->SetBranchAddress("genNprodvx_0", &genNprodvx_0, &b_genNprodvx_0);
   fChain->SetBranchAddress("genNprodvy_0", &genNprodvy_0, &b_genNprodvy_0);
   fChain->SetBranchAddress("genNprodvz_0", &genNprodvz_0, &b_genNprodvz_0);
   fChain->SetBranchAddress("genNdecayvx_0", &genNdecayvx_0, &b_genNdecayvx_0);
   fChain->SetBranchAddress("genNdecayvy_0", &genNdecayvy_0, &b_genNdecayvy_0);
   fChain->SetBranchAddress("genNdecayvz_0", &genNdecayvz_0, &b_genNdecayvz_0);
   fChain->SetBranchAddress("genphE_0", &genphE_0, &b_genphE_0);
   fChain->SetBranchAddress("genphpt_0", &genphpt_0, &b_genphpt_0);
   fChain->SetBranchAddress("genphphi_0", &genphphi_0, &b_genphphi_0);
   fChain->SetBranchAddress("genpheta_0", &genpheta_0, &b_genpheta_0);
   fChain->SetBranchAddress("genphmatch_0", &genphmatch_0, &b_genphmatch_0);
   fChain->SetBranchAddress("gengrmass_0", &gengrmass_0, &b_gengrmass_0);
   fChain->SetBranchAddress("gengrE_0", &gengrE_0, &b_gengrE_0);
   fChain->SetBranchAddress("gengrpt_0", &gengrpt_0, &b_gengrpt_0);
   fChain->SetBranchAddress("gengrphi_0", &gengrphi_0, &b_gengrphi_0);
   fChain->SetBranchAddress("gengreta_0", &gengreta_0, &b_gengreta_0);
   fChain->SetBranchAddress("genNmass_1", &genNmass_1, &b_genNmass_1);
   fChain->SetBranchAddress("genNE_1", &genNE_1, &b_genNE_1);
   fChain->SetBranchAddress("genNpt_1", &genNpt_1, &b_genNpt_1);
   fChain->SetBranchAddress("genNphi_1", &genNphi_1, &b_genNphi_1);
   fChain->SetBranchAddress("genNeta_1", &genNeta_1, &b_genNeta_1);
   fChain->SetBranchAddress("genNprodvx_1", &genNprodvx_1, &b_genNprodvx_1);
   fChain->SetBranchAddress("genNprodvy_1", &genNprodvy_1, &b_genNprodvy_1);
   fChain->SetBranchAddress("genNprodvz_1", &genNprodvz_1, &b_genNprodvz_1);
   fChain->SetBranchAddress("genNdecayvx_1", &genNdecayvx_1, &b_genNdecayvx_1);
   fChain->SetBranchAddress("genNdecayvy_1", &genNdecayvy_1, &b_genNdecayvy_1);
   fChain->SetBranchAddress("genNdecayvz_1", &genNdecayvz_1, &b_genNdecayvz_1);
   fChain->SetBranchAddress("genphE_1", &genphE_1, &b_genphE_1);
   fChain->SetBranchAddress("genphpt_1", &genphpt_1, &b_genphpt_1);
   fChain->SetBranchAddress("genphphi_1", &genphphi_1, &b_genphphi_1);
   fChain->SetBranchAddress("genpheta_1", &genpheta_1, &b_genpheta_1);
   fChain->SetBranchAddress("genphmatch_1", &genphmatch_1, &b_genphmatch_1);
   fChain->SetBranchAddress("gengrmass_1", &gengrmass_1, &b_gengrmass_1);
   fChain->SetBranchAddress("gengrE_1", &gengrE_1, &b_gengrE_1);
   fChain->SetBranchAddress("gengrpt_1", &gengrpt_1, &b_gengrpt_1);
   fChain->SetBranchAddress("gengrphi_1", &gengrphi_1, &b_gengrphi_1);
   fChain->SetBranchAddress("gengreta_1", &gengreta_1, &b_gengreta_1);
   fChain->SetBranchAddress("run", &run, &b_run);
   fChain->SetBranchAddress("lumi", &lumi, &b_lumi);
   fChain->SetBranchAddress("event", &event, &b_event);
   fChain->SetBranchAddress("hltSignal", &hltSignal, &b_hltSignal);
   fChain->SetBranchAddress("hltRefPhoID", &hltRefPhoID, &b_hltRefPhoID);
   fChain->SetBranchAddress("hltRefDispID", &hltRefDispID, &b_hltRefDispID);
   fChain->SetBranchAddress("hltRefHT", &hltRefHT, &b_hltRefHT);
   fChain->SetBranchAddress("hltPho50", &hltPho50, &b_hltPho50);
   fChain->SetBranchAddress("nvtx", &nvtx, &b_nvtx);
   fChain->SetBranchAddress("vtxX", &vtxX, &b_vtxX);
   fChain->SetBranchAddress("vtxY", &vtxY, &b_vtxY);
   fChain->SetBranchAddress("vtxZ", &vtxZ, &b_vtxZ);
   fChain->SetBranchAddress("rho", &rho, &b_rho);
   fChain->SetBranchAddress("t1pfMETpt", &t1pfMETpt, &b_t1pfMETpt);
   fChain->SetBranchAddress("t1pfMETphi", &t1pfMETphi, &b_t1pfMETphi);
   fChain->SetBranchAddress("t1pfMETsumEt", &t1pfMETsumEt, &b_t1pfMETsumEt);
   fChain->SetBranchAddress("jetHT", &jetHT, &b_jetHT);
   fChain->SetBranchAddress("njets", &njets, &b_njets);
   fChain->SetBranchAddress("jetE_0", &jetE_0, &b_jetE_0);
   fChain->SetBranchAddress("jetpt_0", &jetpt_0, &b_jetpt_0);
   fChain->SetBranchAddress("jeteta_0", &jeteta_0, &b_jeteta_0);
   fChain->SetBranchAddress("jetphi_0", &jetphi_0, &b_jetphi_0);
   fChain->SetBranchAddress("jetE_1", &jetE_1, &b_jetE_1);
   fChain->SetBranchAddress("jetpt_1", &jetpt_1, &b_jetpt_1);
   fChain->SetBranchAddress("jeteta_1", &jeteta_1, &b_jeteta_1);
   fChain->SetBranchAddress("jetphi_1", &jetphi_1, &b_jetphi_1);
   fChain->SetBranchAddress("jetE_2", &jetE_2, &b_jetE_2);
   fChain->SetBranchAddress("jetpt_2", &jetpt_2, &b_jetpt_2);
   fChain->SetBranchAddress("jeteta_2", &jeteta_2, &b_jeteta_2);
   fChain->SetBranchAddress("jetphi_2", &jetphi_2, &b_jetphi_2);
   fChain->SetBranchAddress("jetE_3", &jetE_3, &b_jetE_3);
   fChain->SetBranchAddress("jetpt_3", &jetpt_3, &b_jetpt_3);
   fChain->SetBranchAddress("jeteta_3", &jeteta_3, &b_jeteta_3);
   fChain->SetBranchAddress("jetphi_3", &jetphi_3, &b_jetphi_3);
   fChain->SetBranchAddress("nrechits", &nrechits, &b_nrechits);
   fChain->SetBranchAddress("rheta", &rheta, &b_rheta);
   fChain->SetBranchAddress("rhphi", &rhphi, &b_rhphi);
   fChain->SetBranchAddress("rhE", &rhE, &b_rhE);
   fChain->SetBranchAddress("rhtime", &rhtime, &b_rhtime);
   fChain->SetBranchAddress("rhOOT", &rhOOT, &b_rhOOT);
   fChain->SetBranchAddress("rhID", &rhID, &b_rhID);
   fChain->SetBranchAddress("nphotons", &nphotons, &b_nphotons);
   fChain->SetBranchAddress("phoE_0", &phoE_0, &b_phoE_0);
   fChain->SetBranchAddress("phopt_0", &phopt_0, &b_phopt_0);
   fChain->SetBranchAddress("phoeta_0", &phoeta_0, &b_phoeta_0);
   fChain->SetBranchAddress("phophi_0", &phophi_0, &b_phophi_0);
   fChain->SetBranchAddress("phoscE_0", &phoscE_0, &b_phoscE_0);
   fChain->SetBranchAddress("phosceta_0", &phosceta_0, &b_phosceta_0);
   fChain->SetBranchAddress("phoscphi_0", &phoscphi_0, &b_phoscphi_0);
   fChain->SetBranchAddress("phoHoE_0", &phoHoE_0, &b_phoHoE_0);
   fChain->SetBranchAddress("phor9_0", &phor9_0, &b_phor9_0);
   fChain->SetBranchAddress("phoChgHadIso_0", &phoChgHadIso_0, &b_phoChgHadIso_0);
   fChain->SetBranchAddress("phoNeuHadIso_0", &phoNeuHadIso_0, &b_phoNeuHadIso_0);
   fChain->SetBranchAddress("phoPhoIso_0", &phoPhoIso_0, &b_phoPhoIso_0);
   fChain->SetBranchAddress("phoEcalPFClIso_0", &phoEcalPFClIso_0, &b_phoEcalPFClIso_0);
   fChain->SetBranchAddress("phoHcalPFClIso_0", &phoHcalPFClIso_0, &b_phoHcalPFClIso_0);
   fChain->SetBranchAddress("phoTrkIso_0", &phoTrkIso_0, &b_phoTrkIso_0);
   fChain->SetBranchAddress("phosieie_0", &phosieie_0, &b_phosieie_0);
   fChain->SetBranchAddress("phosipip_0", &phosipip_0, &b_phosipip_0);
   fChain->SetBranchAddress("phosieip_0", &phosieip_0, &b_phosieip_0);
   fChain->SetBranchAddress("phosmaj_0", &phosmaj_0, &b_phosmaj_0);
   fChain->SetBranchAddress("phosmin_0", &phosmin_0, &b_phosmin_0);
   fChain->SetBranchAddress("phoalpha_0", &phoalpha_0, &b_phoalpha_0);
   fChain->SetBranchAddress("phoseed_0", &phoseed_0, &b_phoseed_0);
   fChain->SetBranchAddress("phorecHits_0", &phorecHits_0, &b_phorecHits_0);
   fChain->SetBranchAddress("phoisOOT_0", &phoisOOT_0, &b_phoisOOT_0);
   fChain->SetBranchAddress("phoisEB_0", &phoisEB_0, &b_phoisEB_0);
   fChain->SetBranchAddress("phoisHLT_0", &phoisHLT_0, &b_phoisHLT_0);
   fChain->SetBranchAddress("phoisTrk_0", &phoisTrk_0, &b_phoisTrk_0);
   fChain->SetBranchAddress("phoID_0", &phoID_0, &b_phoID_0);
   fChain->SetBranchAddress("phoE_1", &phoE_1, &b_phoE_1);
   fChain->SetBranchAddress("phopt_1", &phopt_1, &b_phopt_1);
   fChain->SetBranchAddress("phoeta_1", &phoeta_1, &b_phoeta_1);
   fChain->SetBranchAddress("phophi_1", &phophi_1, &b_phophi_1);
   fChain->SetBranchAddress("phoscE_1", &phoscE_1, &b_phoscE_1);
   fChain->SetBranchAddress("phosceta_1", &phosceta_1, &b_phosceta_1);
   fChain->SetBranchAddress("phoscphi_1", &phoscphi_1, &b_phoscphi_1);
   fChain->SetBranchAddress("phoHoE_1", &phoHoE_1, &b_phoHoE_1);
   fChain->SetBranchAddress("phor9_1", &phor9_1, &b_phor9_1);
   fChain->SetBranchAddress("phoChgHadIso_1", &phoChgHadIso_1, &b_phoChgHadIso_1);
   fChain->SetBranchAddress("phoNeuHadIso_1", &phoNeuHadIso_1, &b_phoNeuHadIso_1);
   fChain->SetBranchAddress("phoPhoIso_1", &phoPhoIso_1, &b_phoPhoIso_1);
   fChain->SetBranchAddress("phoEcalPFClIso_1", &phoEcalPFClIso_1, &b_phoEcalPFClIso_1);
   fChain->SetBranchAddress("phoHcalPFClIso_1", &phoHcalPFClIso_1, &b_phoHcalPFClIso_1);
   fChain->SetBranchAddress("phoTrkIso_1", &phoTrkIso_1, &b_phoTrkIso_1);
   fChain->SetBranchAddress("phosieie_1", &phosieie_1, &b_phosieie_1);
   fChain->SetBranchAddress("phosipip_1", &phosipip_1, &b_phosipip_1);
   fChain->SetBranchAddress("phosieip_1", &phosieip_1, &b_phosieip_1);
   fChain->SetBranchAddress("phosmaj_1", &phosmaj_1, &b_phosmaj_1);
   fChain->SetBranchAddress("phosmin_1", &phosmin_1, &b_phosmin_1);
   fChain->SetBranchAddress("phoalpha_1", &phoalpha_1, &b_phoalpha_1);
   fChain->SetBranchAddress("phoseed_1", &phoseed_1, &b_phoseed_1);
   fChain->SetBranchAddress("phorecHits_1", &phorecHits_1, &b_phorecHits_1);
   fChain->SetBranchAddress("phoisOOT_1", &phoisOOT_1, &b_phoisOOT_1);
   fChain->SetBranchAddress("phoisEB_1", &phoisEB_1, &b_phoisEB_1);
   fChain->SetBranchAddress("phoisHLT_1", &phoisHLT_1, &b_phoisHLT_1);
   fChain->SetBranchAddress("phoisTrk_1", &phoisTrk_1, &b_phoisTrk_1);
   fChain->SetBranchAddress("phoID_1", &phoID_1, &b_phoID_1);
   fChain->SetBranchAddress("phoE_2", &phoE_2, &b_phoE_2);
   fChain->SetBranchAddress("phopt_2", &phopt_2, &b_phopt_2);
   fChain->SetBranchAddress("phoeta_2", &phoeta_2, &b_phoeta_2);
   fChain->SetBranchAddress("phophi_2", &phophi_2, &b_phophi_2);
   fChain->SetBranchAddress("phoscE_2", &phoscE_2, &b_phoscE_2);
   fChain->SetBranchAddress("phosceta_2", &phosceta_2, &b_phosceta_2);
   fChain->SetBranchAddress("phoscphi_2", &phoscphi_2, &b_phoscphi_2);
   fChain->SetBranchAddress("phoHoE_2", &phoHoE_2, &b_phoHoE_2);
   fChain->SetBranchAddress("phor9_2", &phor9_2, &b_phor9_2);
   fChain->SetBranchAddress("phoChgHadIso_2", &phoChgHadIso_2, &b_phoChgHadIso_2);
   fChain->SetBranchAddress("phoNeuHadIso_2", &phoNeuHadIso_2, &b_phoNeuHadIso_2);
   fChain->SetBranchAddress("phoPhoIso_2", &phoPhoIso_2, &b_phoPhoIso_2);
   fChain->SetBranchAddress("phoEcalPFClIso_2", &phoEcalPFClIso_2, &b_phoEcalPFClIso_2);
   fChain->SetBranchAddress("phoHcalPFClIso_2", &phoHcalPFClIso_2, &b_phoHcalPFClIso_2);
   fChain->SetBranchAddress("phoTrkIso_2", &phoTrkIso_2, &b_phoTrkIso_2);
   fChain->SetBranchAddress("phosieie_2", &phosieie_2, &b_phosieie_2);
   fChain->SetBranchAddress("phosipip_2", &phosipip_2, &b_phosipip_2);
   fChain->SetBranchAddress("phosieip_2", &phosieip_2, &b_phosieip_2);
   fChain->SetBranchAddress("phosmaj_2", &phosmaj_2, &b_phosmaj_2);
   fChain->SetBranchAddress("phosmin_2", &phosmin_2, &b_phosmin_2);
   fChain->SetBranchAddress("phoalpha_2", &phoalpha_2, &b_phoalpha_2);
   fChain->SetBranchAddress("phoseed_2", &phoseed_2, &b_phoseed_2);
   fChain->SetBranchAddress("phorecHits_2", &phorecHits_2, &b_phorecHits_2);
   fChain->SetBranchAddress("phoisOOT_2", &phoisOOT_2, &b_phoisOOT_2);
   fChain->SetBranchAddress("phoisEB_2", &phoisEB_2, &b_phoisEB_2);
   fChain->SetBranchAddress("phoisHLT_2", &phoisHLT_2, &b_phoisHLT_2);
   fChain->SetBranchAddress("phoisTrk_2", &phoisTrk_2, &b_phoisTrk_2);
   fChain->SetBranchAddress("phoID_2", &phoID_2, &b_phoID_2);
   fChain->SetBranchAddress("phoE_3", &phoE_3, &b_phoE_3);
   fChain->SetBranchAddress("phopt_3", &phopt_3, &b_phopt_3);
   fChain->SetBranchAddress("phoeta_3", &phoeta_3, &b_phoeta_3);
   fChain->SetBranchAddress("phophi_3", &phophi_3, &b_phophi_3);
   fChain->SetBranchAddress("phoscE_3", &phoscE_3, &b_phoscE_3);
   fChain->SetBranchAddress("phosceta_3", &phosceta_3, &b_phosceta_3);
   fChain->SetBranchAddress("phoscphi_3", &phoscphi_3, &b_phoscphi_3);
   fChain->SetBranchAddress("phoHoE_3", &phoHoE_3, &b_phoHoE_3);
   fChain->SetBranchAddress("phor9_3", &phor9_3, &b_phor9_3);
   fChain->SetBranchAddress("phoChgHadIso_3", &phoChgHadIso_3, &b_phoChgHadIso_3);
   fChain->SetBranchAddress("phoNeuHadIso_3", &phoNeuHadIso_3, &b_phoNeuHadIso_3);
   fChain->SetBranchAddress("phoPhoIso_3", &phoPhoIso_3, &b_phoPhoIso_3);
   fChain->SetBranchAddress("phoEcalPFClIso_3", &phoEcalPFClIso_3, &b_phoEcalPFClIso_3);
   fChain->SetBranchAddress("phoHcalPFClIso_3", &phoHcalPFClIso_3, &b_phoHcalPFClIso_3);
   fChain->SetBranchAddress("phoTrkIso_3", &phoTrkIso_3, &b_phoTrkIso_3);
   fChain->SetBranchAddress("phosieie_3", &phosieie_3, &b_phosieie_3);
   fChain->SetBranchAddress("phosipip_3", &phosipip_3, &b_phosipip_3);
   fChain->SetBranchAddress("phosieip_3", &phosieip_3, &b_phosieip_3);
   fChain->SetBranchAddress("phosmaj_3", &phosmaj_3, &b_phosmaj_3);
   fChain->SetBranchAddress("phosmin_3", &phosmin_3, &b_phosmin_3);
   fChain->SetBranchAddress("phoalpha_3", &phoalpha_3, &b_phoalpha_3);
   fChain->SetBranchAddress("phoseed_3", &phoseed_3, &b_phoseed_3);
   fChain->SetBranchAddress("phorecHits_3", &phorecHits_3, &b_phorecHits_3);
   fChain->SetBranchAddress("phoisOOT_3", &phoisOOT_3, &b_phoisOOT_3);
   fChain->SetBranchAddress("phoisEB_3", &phoisEB_3, &b_phoisEB_3);
   fChain->SetBranchAddress("phoisHLT_3", &phoisHLT_3, &b_phoisHLT_3);
   fChain->SetBranchAddress("phoisTrk_3", &phoisTrk_3, &b_phoisTrk_3);
   fChain->SetBranchAddress("phoID_3", &phoID_3, &b_phoID_3);
   fChain->SetBranchAddress("phoisSignal_0", &phoisSignal_0, &b_phoisSignal_0);
   fChain->SetBranchAddress("phoisGen_0", &phoisGen_0, &b_phoisGen_0);
   fChain->SetBranchAddress("phoisSignal_1", &phoisSignal_1, &b_phoisSignal_1);
   fChain->SetBranchAddress("phoisGen_1", &phoisGen_1, &b_phoisGen_1);
   fChain->SetBranchAddress("phoisSignal_2", &phoisSignal_2, &b_phoisSignal_2);
   fChain->SetBranchAddress("phoisGen_2", &phoisGen_2, &b_phoisGen_2);
   fChain->SetBranchAddress("phoisSignal_3", &phoisSignal_3, &b_phoisSignal_3);
   fChain->SetBranchAddress("phoisGen_3", &phoisGen_3, &b_phoisGen_3);
   Notify();
}

Bool_t disphotree::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}

void disphotree::Show(Long64_t entry)
{
// Print contents of entry.
// If entry is not specified, print current entry
   if (!fChain) return;
   fChain->Show(entry);
}
Int_t disphotree::Cut(Long64_t entry)
{
// This function may be called from Loop.
// returns  1 if entry is accepted.
// returns -1 otherwise.
   return 1;
}
#endif // #ifdef disphotree_cxx
