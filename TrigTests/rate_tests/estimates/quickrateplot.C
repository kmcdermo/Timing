void quickrateplot()
{
  TCanvas * canv = new TCanvas(); canv->cd();

  const Int_t n = 4;
//   Double_t x [n] = {1.15,1.25,1.35,1.45};
//   Double_t y [n] = {5.23,5.41,6.15,6.12};
//   Double_t ex[n] = {0,0,0,0};
//   Double_t ey[n] = {0.32,0.32,0.42,1.12};

  Double_t x [n] = {1.15,1.25,1.35,1.45};
  Double_t y [n] = {9.17,10.92,11.51,10.00};
  Double_t ex[n] = {0,0,0,0};
  Double_t ey[n] = {0.43,0.43,0.57,1.43};

  TGraphErrors * gr = new TGraphErrors(n,x,y,ex,ey);
  gr->SetTitle("Individual Rate vs Luminosity");
  gr->GetXaxis()->SetTitle("Luminosity (10^{34})");
  gr->GetYaxis()->SetTitle("Individual Rate (Hz)");
  gr->SetMarkerStyle(20);
  gr->Draw("AP");

  TFormula form("linear","[0]*x");
  TF1 * fit  = new TF1("linear_fit","linear",x[0],x[3]);
  fit->SetParName(0,"Slope"); fit->SetParameter(0,5.f);

  gr->Fit(fit->GetName(),"RBQ");
}
