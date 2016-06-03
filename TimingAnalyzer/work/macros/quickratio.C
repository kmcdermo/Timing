void quickratio() {

  TString hname = "zmass";
  Bool_t  scale = true;

  TFile * mcfile   = TFile::Open("output/MC/dyll/plots.root");
  TFile * datafile = TFile::Open("output/DATA/doubleeg/plots.root");

  TH1F * hdata = (TH1F*)datafile->Get(hname.Data());
  TH1F * hmc   = (TH1F*)mcfile->Get(hname.Data());

  Int_t mcint   = hmc->Integral();
  Int_t dataint = hdata->Integral();

  if (scale) {
    hmc->Scale(dataint/Float_t(mcint));
  }

  TCanvas * c1 = new TCanvas();
  c1->cd();

  TPad * uppad  = new TPad("uppad","",0,0.3,1.0,1.0);
  uppad->SetBottomMargin(0); // Upper and lower plot are joined
  
  TPad * lowpad = new TPad("lowpad","",0,0.05,1.0,0.3);
  lowpad->SetTopMargin(0);
  lowpad->SetBottomMargin(0.3);
  lowpad->SetGridy(1);
  
  c1->cd();
  uppad->Draw();
  uppad->cd();
  
  hmc->GetYaxis()->SetTitleSize(.045);
  hmc->GetYaxis()->SetTitleOffset(.8);

  hmc->SetFillColor(kCyan);
  hmc->SetLineColor(kCyan);
  hmc->Draw("HIST");

  hdata->SetLineColor(kBlack);
  hdata->Draw("EP SAME");

  c1->cd();
  lowpad->Draw();
  lowpad->cd();

  TH1F * hdataclone = (TH1F*)hdata->Clone(Form("%s_clone",hname.Data()));
  TH1F * hmcclone   = (TH1F*)hmc->Clone(Form("%s_clone",hname.Data()));
  
  hdataclone->GetXaxis()->SetTitleSize(.11);
  hdataclone->GetYaxis()->SetTitleSize(.11);
  hdataclone->GetYaxis()->SetTitleOffset(.3);
  hdataclone->GetYaxis()->SetTitle("Data/MC");
  hdataclone->GetXaxis()->SetLabelSize(.11);
  hdataclone->GetYaxis()->SetLabelSize(.11);
  hdataclone->SetMinimum(-0.1);
  hdataclone->SetMaximum( 2.1);
  hdataclone->SetStats(0);
  hdataclone->Divide(hmcclone);
  hdataclone->Draw("EP");

  c1->cd();
  c1->SaveAs(Form("%s_%s.png",hname.Data(), (scale?"scaled":"unscaled") ));
}
