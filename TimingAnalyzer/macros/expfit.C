void expfit()
{
  TFile * file = TFile::Open("output/testdump/cuts_jetpt35.0_phpt100.0_phVIDmedium_rhE1.0_ecalaccept/plots.root");
  TH1F * hist = (TH1F*)file->Get("genN1ctau");
  hist->Scale(1.0/hist->Integral());

  TFormula form("texp","[0]*exp(-[1]*x)");
  TF1 * fit = new TF1("texp_fit",form.GetName(),0.f,100.f);
  hist->Fit(fit->GetName());
}
