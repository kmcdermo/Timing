void lifetime_SF()
{
  TCanvas * canv = new TCanvas();
  canv->cd();
  TGraph * lifetime = new TGraph(3);
  lifetime->SetTitle("miniAOD c#tau vs Pythia c#tau");

  lifetime->SetPoint(0,36.5,62.5); // ctau100
  lifetime->SetPoint(1,730.5,1242); // ctau2000
  lifetime->SetPoint(2,2192,3772); // ctau6000

  lifetime->GetXaxis()->SetTitle("Pythia c#tau [mm]");
  lifetime->GetYaxis()->SetTitle("miniAOD c#tau [mm]");
  lifetime->GetYaxis()->SetTitleOffset(1.2);
  lifetime->SetMarkerStyle(21);

  lifetime->Draw("AP");

  TFormula form("tlin","[0]+[1]*x");
  TF1 * fit = new TF1("tlin_fit",form.GetName(),0.f,5000.f);
  fit->SetParName(0,"Intercept"); fit->SetParameter(0,0.f);
  fit->SetParName(1,"Slope");     fit->SetParameter(1,1.7);
  lifetime->Fit(fit->GetName());
}
