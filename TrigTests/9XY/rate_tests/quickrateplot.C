void quickrateplot()
{
  std::ifstream input;
  input.open("rates_uncs_poisson.txt",std::ios::in);
  
  TString path = "";
  const Int_t n = 4;
  Double_t x [n] = {1.40,1.60,1.80,2.00};
  Double_t ex[n] = {0,0,0,0};

  Double_t y [n];
  Double_t ey[n];

  while (input >> path >> y[0] >> ey[0] >> y[1] >> ey[1] >> y[2] >> ey[2] >> y[3] >> ey[3])
  {
    TCanvas * canv = new TCanvas(); canv->cd();
    
    TGraphErrors * gr = new TGraphErrors(n,x,y,ex,ey);
    gr->SetTitle("Individual Rate vs Luminosity");
    gr->GetXaxis()->SetTitle("Luminosity (10^{34})");
    gr->GetYaxis()->SetTitle("Individual Rate (Hz)");
    gr->GetYaxis()->SetRangeUser(y[0]/1.3,y[3]*1.3);
    gr->SetMarkerStyle(20);
    gr->Draw("AP");

    TFormula form("quad","[0]*x+[1]*x*x");
    TF1 * fit  = new TF1("quad_fit","quad",x[0],x[3]);
    fit->SetParName(0,"b"); fit->SetParameter(0,5.f);
    fit->SetParName(1,"a"); fit->SetParameter(0,0.1f);
    
    gr->Fit(fit->GetName(),"RBQ");

    canv->SaveAs(Form("%s_quad.png",path.Data()));

    delete canv;
  }
}
