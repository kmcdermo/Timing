// use this to compare data/data or MC/MC

void quickoverplot() {
  gStyle->SetOptStat(0);

  TString label1 = "zmasssorted";
  TString label2 = "ptsorted";
  Bool_t  isData = false;  

  TString hname  = "zmass";
  Bool_t  isLogY = false;
  
  TString indir = isData?"DATA/doubleeg":"MC/dyll";

  TFile * file1 = TFile::Open(Form("%s/%s/plots.root",label1.Data(),indir.Data()));
  TFile * file2 = TFile::Open(Form("%s/%s/plots.root",label2.Data(),indir.Data()));

  TH1F * h1 = (TH1F*)file1->Get(hname.Data());
  TH1F * h2 = (TH1F*)file2->Get(hname.Data());

  TCanvas * c1 = new TCanvas();
  c1->cd();
  c1->SetLogy(isLogY);

  TLegend * leg = new TLegend(0.8,0.8,0.94,0.95);

  TPad * uppad  = new TPad("uppad","",0,0.3,1.0,1.0);
  uppad->SetBottomMargin(0); // Upper and lower plot are joined
  
  TPad * lowpad = new TPad("lowpad","",0,0.05,1.0,0.3);
  lowpad->SetTopMargin(0);
  lowpad->SetBottomMargin(0.3);
  lowpad->SetGridy(1);
  
  c1->cd();
  uppad->Draw();
  uppad->cd();
  
  h1->GetYaxis()->SetTitleSize(.045);
  h1->GetYaxis()->SetTitleOffset(.8);

  h1->SetLineColor(kRed);
  h1->SetMarkerColor(kRed);
  h1->SetMarkerStyle(1);
  h1->Draw("ep");

  h2->SetLineColor(kBlue);
  h2->SetMarkerColor(kBlue);
  h2->SetMarkerStyle(1);
  h2->Draw("ep SAME");

  leg->AddEntry(h1,label1.Data(),"l");
  leg->AddEntry(h2,label2.Data(),"l");
  leg->Draw("same");

  c1->cd();
  lowpad->Draw();
  lowpad->cd();

  TH1F * h1clone = (TH1F*)h1->Clone(Form("%s_clone",label1.Data()));
  TH1F * h2clone = (TH1F*)h2->Clone(Form("%s_clone",label2.Data()));
  
  h1clone->GetXaxis()->SetTitleSize(.11);
  h1clone->GetYaxis()->SetTitleSize(.08);
  h1clone->GetYaxis()->SetTitleOffset(.3);
  h1clone->GetYaxis()->SetTitle(Form("%s/%s",label1.Data(),label2.Data()));
  h1clone->GetXaxis()->SetLabelSize(.11);
  h1clone->GetYaxis()->SetLabelSize(.11);
  h1clone->SetMinimum(-0.1);
  h1clone->SetMaximum( 2.1);
  h1clone->SetStats(0);
  h1clone->Divide(h2clone);
  h1clone->Draw("EP");

  c1->cd();
  c1->SaveAs(Form("%s_%s_vs_%s_%s_%s.png",hname.Data(), label1.Data(), label2.Data(), (isData?"data":"mc"), (isLogY?"log":"lin") ));
}
