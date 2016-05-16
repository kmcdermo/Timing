#include "TimeRes.hh"
#include "Common.h"
#include <TH1.h>
#include <TStyle.h>
#include <TCanvas.h>

TimeRes::TimeRes(TString filename, TString outdir, Bool_t mvinput, TString outtype) : 
  fChain(0), inFileName(filename), outDir(outdir), mvInput(mvinput), outType(outtype) {
  file = TFile::Open(inFileName.Data());

  // make output directory if it does not exist
  MakeOutDir(outDir);
  
  TTree * tree = (TTree*)file->Get("tree/tree");
  Init(tree);
}

TimeRes::~TimeRes(){
  if (!fChain) return;
  delete fChain->GetCurrentFile();
}

Int_t TimeRes::GetEntry(Long64_t entry){
  // Read contents of entry.
  if (!fChain) return 0;
  return fChain->GetEntry(entry);
}

Long64_t TimeRes::LoadTree(Long64_t entry){
  // Set the environment to read one entry
  if (!fChain) return -5;
  Long64_t centry = fChain->LoadTree(entry);
  if (centry < 0) return centry;
  if (fChain->GetTreeNumber() != fCurrent) {
    fCurrent = fChain->GetTreeNumber();
  }
  return centry;
}

void TimeRes::MakePlots(){

  // This is the loop skeleton where:
  // jentry is the global entry number in the chain
  // ientry is the entry number in the current Tree
  // Note that the argument to GetEntry must be:
  // jentry for TChain::GetEntry
  // ientry for TTree::GetEntry and TBranch::GetEntry

  if (fChain == 0) return;
  Long64_t nentries = fChain->GetEntriesFast();
  Long64_t nbytes = 0, nb = 0;
  for (Long64_t jentry=0; jentry<nentries;jentry++) {
    Long64_t ientry = LoadTree(jentry);
    if (ientry < 0) break;
    nb = fChain->GetEntry(jentry);   nbytes += nb;
    // if (Cut(ientry) < 0) continue;
  }

  if (mvInput) {MoveInput(inFileName,outDir);}
}

void TimeRes::Init(TTree *tree){
  // Set branch addresses and branch pointers
  if (!tree) return;
  fChain = tree;
  fCurrent = -1;
  fChain->SetMakeClass(1);

  fChain->SetBranchAddress("event", &event, &b_event);
  fChain->SetBranchAddress("run", &run, &b_run);
  fChain->SetBranchAddress("lumi", &lumi, &b_lumi);
  fChain->SetBranchAddress("xsec", &xsec, &b_xsec);
  fChain->SetBranchAddress("wgt", &wgt, &b_wgt);
  fChain->SetBranchAddress("pswgt", &pswgt, &b_pswgt);
  fChain->SetBranchAddress("puwgt", &puwgt, &b_puwgt);
  fChain->SetBranchAddress("puobs", &puobs, &b_puobs);
  fChain->SetBranchAddress("putrue", &putrue, &b_putrue);
  fChain->SetBranchAddress("nvtx", &nvtx, &b_nvtx);
  fChain->SetBranchAddress("hltphoton165", &hltphoton165, &b_hltphoton165);
  fChain->SetBranchAddress("hltphoton175", &hltphoton175, &b_hltphoton175);
  fChain->SetBranchAddress("hltphoton120", &hltphoton120, &b_hltphoton120);
  fChain->SetBranchAddress("hltdoubleel", &hltdoubleel, &b_hltdoubleel);
  fChain->SetBranchAddress("hltsingleel", &hltsingleel, &b_hltsingleel);
  fChain->SetBranchAddress("nelectrons", &nelectrons, &b_nelectrons);
  fChain->SetBranchAddress("ntightelectrons", &ntightelectrons, &b_ntightelectrons);
  fChain->SetBranchAddress("nheepelectrons", &nheepelectrons, &b_nheepelectrons);
  fChain->SetBranchAddress("nphotons", &nphotons, &b_nphotons);
  fChain->SetBranchAddress("el1pid", &el1pid, &b_el1pid);
  fChain->SetBranchAddress("el1pt", &el1pt, &b_el1pt);
  fChain->SetBranchAddress("el1eta", &el1eta, &b_el1eta);
  fChain->SetBranchAddress("el1phi", &el1phi, &b_el1phi);
  fChain->SetBranchAddress("el1id", &el1id, &b_el1id);
  fChain->SetBranchAddress("el1idl", &el1idl, &b_el1idl);
  fChain->SetBranchAddress("el2pid", &el2pid, &b_el2pid);
  fChain->SetBranchAddress("el2pt", &el2pt, &b_el2pt);
  fChain->SetBranchAddress("el2eta", &el2eta, &b_el2eta);
  fChain->SetBranchAddress("el2phi", &el2phi, &b_el2phi);
  fChain->SetBranchAddress("el2id", &el2id, &b_el2id);
  fChain->SetBranchAddress("el2idl", &el2idl, &b_el2idl);
  fChain->SetBranchAddress("el1time", &el1time, &b_el1time);
  fChain->SetBranchAddress("el2time", &el2time, &b_el1time);
  fChain->SetBranchAddress("zeemass", &zeemass, &b_zeemass);
  fChain->SetBranchAddress("zeept", &zeept, &b_zeeept);
  fChain->SetBranchAddress("zeeeta", &zeeeta, &b_zeeeta);
  fChain->SetBranchAddress("zeephi", &zeephi, &b_zeephi);
  fChain->SetBranchAddress("phidl", &phidl, &b_phidl);
  fChain->SetBranchAddress("phidm", &phidm, &b_phidm);
  fChain->SetBranchAddress("phidt", &phidt, &b_phidt);
  fChain->SetBranchAddress("phidh", &phidh, &b_phidh);
  fChain->SetBranchAddress("phpt", &phpt, &b_phpt);
  fChain->SetBranchAddress("pheta", &pheta, &b_pheta);
  fChain->SetBranchAddress("phphi", &phphi, &b_phphi);
  fChain->SetBranchAddress("wzid", &wzid, &b_wzid);
  fChain->SetBranchAddress("wzmass", &wzmass, &b_wzmass);
  fChain->SetBranchAddress("wzpt", &wzpt, &b_wzpt);
  fChain->SetBranchAddress("wzeta", &wzeta, &b_wzeta);
  fChain->SetBranchAddress("wzphi", &wzphi, &b_wzphi);
  fChain->SetBranchAddress("l1id", &l1id, &b_l1id);
  fChain->SetBranchAddress("l1pt", &l1pt, &b_l1pt);
  fChain->SetBranchAddress("l1eta", &l1eta, &b_l1eta);
  fChain->SetBranchAddress("l1phi", &l1phi, &b_l1phi);
  fChain->SetBranchAddress("l2id", &l2id, &b_l2id);
  fChain->SetBranchAddress("l2pt", &l2pt, &b_l2pt);
  fChain->SetBranchAddress("l2eta", &l2eta, &b_l2eta);
  fChain->SetBranchAddress("l2phi", &l2phi, &b_l2phi);
  fChain->SetBranchAddress("parid", &parid, &b_parid);
  fChain->SetBranchAddress("parpt", &parpt, &b_parpt);
  fChain->SetBranchAddress("pareta", &pareta, &b_pareta);
  fChain->SetBranchAddress("parphi", &parphi, &b_parphi);
  fChain->SetBranchAddress("ancid", &ancid, &b_ancid);
  fChain->SetBranchAddress("ancpt", &ancpt, &b_ancpt);
  fChain->SetBranchAddress("anceta", &anceta, &b_anceta);
  fChain->SetBranchAddress("ancphi", &ancphi, &b_ancphi);
}

