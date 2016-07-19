#include <cmath>

void tof(){

  TFile * file = TFile::Open("input/DATA/doubleeg/skimmedtree.root");
  TTree * tree = (TTree*)file->Get("tree/tree");

  Float_t time;
  tree->SetBranchAddress("el1seedtime",&time);

  Float_t x, y, z;
  tree->SetBranchAddress("el1seedX",&x);
  tree->SetBranchAddress("el1seedY",&y);
  tree->SetBranchAddress("el1seedZ",&z);

  Float_t vx, vy, vz;
  tree->SetBranchAddress("vtxX",&vx);
  tree->SetBranchAddress("vtxY",&vy);
  tree->SetBranchAddress("vtxZ",&vz);

  const Float_t sol = 29.9792;

  TH1F * hist1 = new TH1F("hist1","corr",100,-10,10);
  TH1F * hist2 = new TH1F("hist2","uncorr",100,-10,10);

  TH1F * histz = new TH1F("histz","vz",100,-20,20);

  Float_t best = -99.0;

  for (int i = 0; i < 10000; i++) {
    tree->GetEntry(i);
    hist2->Fill(time);
    
    Float_t tp  = std::sqrt(z*z + x*x + y*y)/sol;
    Float_t tpv = std::sqrt((z-vz)*(z-vz) + (x-vx)*(x-vx) + (y-vy)*(y-vy))/sol;

    z += 10;

    if (std::abs(vz) > best) {best = std::abs(vz);}

    hist1->Fill(time+tp-tpv);
    

    histz->Fill(vz);

  //   if (time+tpv-tpv<-10){
      
//       std::cout << time << " " << tpv << " " << tp << std::endl;
//     }
  }
  

  //  std::cout << best << std::endl;

  TCanvas *c1 = new TCanvas();
  c1->cd();
  c1->SetLogy(1);

  histz->Draw();

//   hist1->SetLineColor(kRed);
//   hist2->SetLineColor(kBlack);

//   hist1->Scale(1.0/hist1->Integral());
//   hist2->Scale(1.0/hist2->Integral());
  
//   hist1->Draw();
//   hist2->Draw("same");

  
//   TCanvas *c2 = new TCanvas();
//   c2->cd();
//   c2->SetLogy(1);

//   hist1->Add(hist2,-1.0);
  
//   hist1->Draw();
  

  

}
