#include <iostream>

void addwgtsum(TString sample) {

  TFile * file = TFile::Open(Form("input/MC/%s/tree.root",sample.Data()),"UPDATE");
  TTree * tree = (TTree*)file->Get("tree/tree");

  Float_t   wgt;
  TBranch * b_wgt;
  tree->SetBranchAddress("wgt", &wgt, &b_wgt);

  Float_t wgtsum = 0.;
  for (Long64_t i = 0; i < tree->GetEntries(); i++) {
    b_wgt  -> GetEvent(i);
    wgtsum += wgt;
  }

  std::cout << "Weight sum for sample: " << sample.Data() << " " << wgtsum << std::endl;
  
  delete tree;
  delete file;
}

