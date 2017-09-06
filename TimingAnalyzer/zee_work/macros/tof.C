#include <cmath>

void tof()
{
  gStyle->SetOptStat(0);

  TFile * file = TFile::Open("input/DATA/2016/doubleeg/skimmedtree.root");
  TTree * tree = (TTree*)file->Get("tree/tree");

  TH1F * histseed1 = new TH1F("histseed1","el1seed time",100,-10,10);
  TH1F * histseed2 = new TH1F("histseed2","el2seed time",100,-10,10);

  std::cout << "Begin of seedtime filling" << std::endl;

  tree->Draw("el1seedtime>>histseed1","","goff");
  tree->Draw("el2seedtime>>histseed2","","goff");

  std::cout << "End of seedtime filling" << std::endl;

  const Float_t sol = 29.9792;
  const Float_t el1shift = -0.520395; //-0.518782;
  const Float_t el2shift = -0.521207; //-0.519583;

  TH1F * histtof1 = new TH1F("histtof1","el1seed TOF",100,-10,10);
  TH1F * histtof2 = new TH1F("histtof2","el2seed TOF",100,-10,10);

  std::cout << "Begin of TOF filling" << std::endl;

  tree->Draw(Form("el1seedtime+sqrt((el1seedX*el1seedX)+(el1seedY*el1seedY)+(el1seedZ*el1seedZ))/%f-sqrt((vtxX-el1seedX)*(vtxX-el1seedX)+(vtxY-el1seedY)*(vtxY-el1seedY)+(vtxZ-el1seedZ)*(vtxZ-el1seedZ))/%f-%f>>histtof1",sol,sol,el1shift));
  tree->Draw(Form("el2seedtime+sqrt((el2seedX*el2seedX)+(el2seedY*el2seedY)+(el2seedZ*el2seedZ))/%f-sqrt((vtxX-el2seedX)*(vtxX-el2seedX)+(vtxY-el2seedY)*(vtxY-el2seedY)+(vtxZ-el2seedZ)*(vtxZ-el2seedZ))/%f-%f>>histtof2",sol,sol,el2shift));

  std::cout << "End of TOF filling" << std::endl;

  TCanvas *c1 = new TCanvas();
  c1->cd();
  //  c1->SetLogy(1);

  histseed1->SetLineColor(kRed);
  histtof1 ->SetLineColor(kBlue);

  histseed1->SetMaximum((histseed1->GetMaximum()>histtof1->GetMaximum()?histseed1->GetMaximum():histtof1->GetMaximum())*1.05);
  
  histseed1->Draw();
  histtof1 ->Draw("same");

  std::cout << "el1seedtime: " << histseed1->GetMean() << "+/-" << histseed1->GetStdDev() << std::endl;
  std::cout << "el1TOFtime:  " << histtof1 ->GetMean() << "+/-" << histtof1 ->GetStdDev() << std::endl;

  TLegend * leg1 = new TLegend(0.7,0.85,1.0,1.0);
  leg1->AddEntry(histseed1,"el1 seed","l");
  leg1->AddEntry(histtof1 ,"el1 tof", "l");
  leg1->Draw("same");

  c1->SaveAs("el1_seedtime_vs_tof_noprompt.png");

  TCanvas *c2 = new TCanvas();
  c2->cd();
  //  c2->SetLogy(1);

  histseed2->SetLineColor(kRed);
  histtof2 ->SetLineColor(kBlue);

  histseed2->SetMaximum((histseed2->GetMaximum()>histtof2->GetMaximum()?histseed2->GetMaximum():histtof2->GetMaximum())*1.05);

  histseed2->Draw();
  histtof2 ->Draw("same");

  std::cout << "el2seedtime: " << histseed2->GetMean() << "+/-" << histseed2->GetStdDev() << std::endl;
  std::cout << "el2TOFtime:  " << histtof2 ->GetMean() << "+/-" << histtof2 ->GetStdDev() << std::endl;

  TLegend * leg2 = new TLegend(0.7,0.85,1.0,1.0);
  leg2->AddEntry(histseed2,"el2 seed","l");
  leg2->AddEntry(histtof2 ,"el2 tof", "l");
  leg2->Draw("same");

  c2->SaveAs("el2_seedtime_vs_tof_noprompt.png");
}
