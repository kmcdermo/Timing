void treeplot(){
  
  gStyle->SetOptStat(0);

  TFile * file = TFile::Open("tree.root");
  TTree * tree = (TTree*)file->Get("tree/tree");

  TCanvas * c1 = new TCanvas();
  c1->cd();

  TH1F * h1 = new TH1F("h1","",100,0,100);
  TH1F * h2 = new TH1F("h2","",100,0,100);

  tree->Draw("nrhel1>>h1","","goff");
  tree->Draw("nrhel2>>h2","","goff");

  h1->Scale(1.0/h1->Integral());
  h1->GetXaxis()->SetTitle("nRecHits");
  h1->GetYaxis()->SetTitle("Norm. Events");
  h1->SetTitle("nRecHits in electron candidates");
  h2->Scale(1.0/h2->Integral());

  h1->SetLineColor(kRed);
  h2->SetLineColor(kBlue);
  h1->Draw();
  h2->Draw("same");

  TLegend * leg = new TLegend(0.7,0.7,0.9,0.9);
  leg->AddEntry(h1,"el1","l");
  leg->AddEntry(h2,"el2","l");
  leg->Draw("same");
  
  c1->SetLogy(1);
  c1->SaveAs("nrechits.png");

  std::cout << "h1 mean: " << h1->GetMean() << std::endl;
  std::cout << "h2 mean: " << h2->GetMean() << std::endl;
}

