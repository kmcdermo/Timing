void plot_ic_times_lc()
{
  gStyle->SetOptStat("eRMou");
  gStyle->SetStatH(0.12);
  gStyle->SetStatW(0.4);
  gStyle->SetStatX(0.88);
  gStyle->SetStatY(0.88);

  auto file = TFile::Open("skims/v4p1/dixtal/dixtal.root");
  auto tree = (TTree*)file->Get("Data_Tree");

  auto hist = new TH1F("hist","LC * IC (2017);LC * IC;Fraction of Events",60,0,6);
  hist->Sumw2();
  hist->SetLineColor(kBlue);
  hist->SetMarkerColor(kBlue);
  hist->SetLineWidth(2);
  hist->SetMarkerStyle(20);
  hist->SetMarkerSize(0.6);

  tree->Draw("(phoseedLaser_0*phoseedInterCalib_0)>>hist","","goff");
  hist->Scale(1.0/hist->Integral());

  auto canv = new TCanvas();
  canv->cd();
  canv->SetLogy();
  canv->SetGridx();
  canv->SetGridy();

  hist->Draw("ep");
  canv->SaveAs("lc_ic.png");
}
