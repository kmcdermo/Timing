#include "../interface/Analysis.hh"
#include "../interface/Common.hh"
#include <TH1.h>
#include <TCanvas.h>
#include <iostream>

Analysis::Analysis(TString infilename, TString outdir, TString outtype) : 
  fOutDir(outdir), fOutType(outtype) {
  
  fInFile = TFile::Open(infilename.Data());

  // make output directory if it does not exist
  MakeOutDir(fOutDir);
  
  TTree * fInTree = (TTree*)fInFile->Get("tree/tree");
  InitTree();

  // boilerplate tdr style
  fTDRStyle = new TStyle("TDRStyle","Style for P-TDR");
  SetTDRStyle(fTDRStyle);
  gROOT->ForceStyle();
}

Analysis::~Analysis(){
  delete fTDRStyle;
  delete fInTree;
  delete fInFile;
}

void Analysis::TimeResPlots(){
  for (UInt_t entry = 0; entry < fInTree->GetEntries(); entry++){
    if ( (nelectrons==2) && hltdoubleel && (zeemass>76. && zeemass<105.) ){ // we want di-electron z's
      // standard "validation" plots

      std::cout << "hurray!" << std::endl;
    }
  }
}

void Analysis::InitTree(){
  // Set branch addresses and branch pointers

  fInTree->SetBranchAddress("event", &event, &b_event);
  fInTree->SetBranchAddress("run", &run, &b_run);
  fInTree->SetBranchAddress("lumi", &lumi, &b_lumi);
  fInTree->SetBranchAddress("xsec", &xsec, &b_xsec);
  fInTree->SetBranchAddress("wgt", &wgt, &b_wgt);
  fInTree->SetBranchAddress("pswgt", &pswgt, &b_pswgt);
  fInTree->SetBranchAddress("puwgt", &puwgt, &b_puwgt);
  fInTree->SetBranchAddress("puobs", &puobs, &b_puobs);
  fInTree->SetBranchAddress("putrue", &putrue, &b_putrue);
  fInTree->SetBranchAddress("nvtx", &nvtx, &b_nvtx);
  fInTree->SetBranchAddress("hltphoton165", &hltphoton165, &b_hltphoton165);
  fInTree->SetBranchAddress("hltphoton175", &hltphoton175, &b_hltphoton175);
  fInTree->SetBranchAddress("hltphoton120", &hltphoton120, &b_hltphoton120);
  fInTree->SetBranchAddress("hltdoubleel", &hltdoubleel, &b_hltdoubleel);
  fInTree->SetBranchAddress("hltsingleel", &hltsingleel, &b_hltsingleel);
  fInTree->SetBranchAddress("nelectrons", &nelectrons, &b_nelectrons);
  fInTree->SetBranchAddress("ntightelectrons", &ntightelectrons, &b_ntightelectrons);
  fInTree->SetBranchAddress("nheepelectrons", &nheepelectrons, &b_nheepelectrons);
  fInTree->SetBranchAddress("nphotons", &nphotons, &b_nphotons);
  fInTree->SetBranchAddress("el1pid", &el1pid, &b_el1pid);
  fInTree->SetBranchAddress("el1pt", &el1pt, &b_el1pt);
  fInTree->SetBranchAddress("el1eta", &el1eta, &b_el1eta);
  fInTree->SetBranchAddress("el1phi", &el1phi, &b_el1phi);
  fInTree->SetBranchAddress("el1id", &el1id, &b_el1id);
  fInTree->SetBranchAddress("el1idl", &el1idl, &b_el1idl);
  fInTree->SetBranchAddress("el2pid", &el2pid, &b_el2pid);
  fInTree->SetBranchAddress("el2pt", &el2pt, &b_el2pt);
  fInTree->SetBranchAddress("el2eta", &el2eta, &b_el2eta);
  fInTree->SetBranchAddress("el2phi", &el2phi, &b_el2phi);
  fInTree->SetBranchAddress("el2id", &el2id, &b_el2id);
  fInTree->SetBranchAddress("el2idl", &el2idl, &b_el2idl);
  fInTree->SetBranchAddress("el1time", &el1time, &b_el1time);
  fInTree->SetBranchAddress("el2time", &el2time, &b_el1time);
  fInTree->SetBranchAddress("zeemass", &zeemass, &b_zeemass);
  fInTree->SetBranchAddress("zeept", &zeept, &b_zeeept);
  fInTree->SetBranchAddress("zeeeta", &zeeeta, &b_zeeeta);
  fInTree->SetBranchAddress("zeephi", &zeephi, &b_zeephi);
  fInTree->SetBranchAddress("phidl", &phidl, &b_phidl);
  fInTree->SetBranchAddress("phidm", &phidm, &b_phidm);
  fInTree->SetBranchAddress("phidt", &phidt, &b_phidt);
  fInTree->SetBranchAddress("phidh", &phidh, &b_phidh);
  fInTree->SetBranchAddress("phpt", &phpt, &b_phpt);
  fInTree->SetBranchAddress("pheta", &pheta, &b_pheta);
  fInTree->SetBranchAddress("phphi", &phphi, &b_phphi);
  fInTree->SetBranchAddress("wzid", &wzid, &b_wzid);
  fInTree->SetBranchAddress("wzmass", &wzmass, &b_wzmass);
  fInTree->SetBranchAddress("wzpt", &wzpt, &b_wzpt);
  fInTree->SetBranchAddress("wzeta", &wzeta, &b_wzeta);
  fInTree->SetBranchAddress("wzphi", &wzphi, &b_wzphi);
  fInTree->SetBranchAddress("l1id", &l1id, &b_l1id);
  fInTree->SetBranchAddress("l1pt", &l1pt, &b_l1pt);
  fInTree->SetBranchAddress("l1eta", &l1eta, &b_l1eta);
  fInTree->SetBranchAddress("l1phi", &l1phi, &b_l1phi);
  fInTree->SetBranchAddress("l2id", &l2id, &b_l2id);
  fInTree->SetBranchAddress("l2pt", &l2pt, &b_l2pt);
  fInTree->SetBranchAddress("l2eta", &l2eta, &b_l2eta);
  fInTree->SetBranchAddress("l2phi", &l2phi, &b_l2phi);
  fInTree->SetBranchAddress("parid", &parid, &b_parid);
  fInTree->SetBranchAddress("parpt", &parpt, &b_parpt);
  fInTree->SetBranchAddress("pareta", &pareta, &b_pareta);
  fInTree->SetBranchAddress("parphi", &parphi, &b_parphi);
  fInTree->SetBranchAddress("ancid", &ancid, &b_ancid);
  fInTree->SetBranchAddress("ancpt", &ancpt, &b_ancpt);
  fInTree->SetBranchAddress("anceta", &anceta, &b_anceta);
  fInTree->SetBranchAddress("ancphi", &ancphi, &b_ancphi);
}
