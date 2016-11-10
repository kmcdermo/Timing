void expfit()
{
  TFile * file = TFile::Open("output/moreplots/cuts_jetpt35.0_phpt100.0_phVIDmedium_rhE1.0_ecalaccept/plots.root");
  TH1F * hist = (TH1F*)file->Get("genN1ctau");
  hist->Scale(1.0/hist->Integral());
  hist->GetYaxis()->SetTitle("Neutralinos/nNeutralinos");

  TFormula form("texp","[0]*exp(-x/[1])");
  TF1 * fit = new TF1("texp_fit",form.GetName(),0.f,100.f);
  fit->SetParName(0,"Norm"); fit->SetParameter(0,hist->GetMaximum());
  fit->SetParName(1,"d");    fit->SetParameter(1,hist->GetMean());
  hist->Fit(fit->GetName());
}
