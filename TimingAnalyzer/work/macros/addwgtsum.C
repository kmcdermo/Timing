#include <iostream>

void addwgtsum(TString dir) {

  TFile * file = TFile::Open(Form("%s/tree.root",dir.Data()),"UPDATE");
  TTree * tree = (TTree*)file->Get("tree/tree");

  Float_t   wgt;
  TBranch * b_wgt;
  tree->SetBranchAddress("wgt", &wgt, &b_wgt);

  std::cout << "Adding up weights to get sum" << std::endl;
  Float_t wgtsum = 0.;
  for (Long64_t i = 0; i < tree->GetEntries(); i++) {
    b_wgt  -> GetEvent(i);
    wgtsum += wgt;
  }

  std::cout << "Copy wgtsum branch into tree" << std::endl;
  TBranch * b_wgtsum = tree->Branch("wgtsum", &wgtsum, "wgtsum/F");
  for (Long64_t i = 0; i < tree->GetEntries(); i++) {
    b_wgtsum->Fill();
  }

  file->Write();
  delete tree;
  delete file;
  
  // moving name to with wgtsum
  std::cout << "Moving file from tree.root to treewgtsum.root" << std::endl;
  gSystem->Exec(Form("mv %s/tree.root %s/treewgtsum.root",dir.Data(),dir.Data()));
  std::cout << "Finished macro" << std::endl;
}

