void efficiency_SF()
{
  TCanvas * canv = new TCanvas();
  canv->cd();
  TGraph * lifetime = new TGraph(3);
  lifetime->SetTitle("Efficiency vs c#tau");

  lifetime->SetPoint(0,36.5,(19724./48900.)); // ctau100
  lifetime->SetPoint(1,730.5,(16948./38399.)); // ctau2000
  lifetime->SetPoint(2,2192,(19182./49199.)); // ctau6000

  lifetime->GetXaxis()->SetTitle("Pythia c#tau [mm]");
  lifetime->GetYaxis()->SetTitle("Efficiency");
  lifetime->SetMarkerStyle(21);

  lifetime->Draw("AP");

  TFormula form("tlin","[0]+[1]*x");
  TF1 * fit = new TF1("tlin_fit",form.GetName(),0.f,3000.f);
  fit->SetParName(0,"Intercept"); fit->SetParameter(0,0.f);
  fit->SetParName(1,"Slope");     fit->SetParameter(1,1.7);
  lifetime->Fit(fit->GetName());
}
