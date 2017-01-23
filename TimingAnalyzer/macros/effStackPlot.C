void effStackPlot(Int_t iphpt, Int_t ivid)
{
  TFile * file = TFile::Open(Form("output/phpt%i/phvid%i/plots.root",iphpt,ivid));
  file->cd();

  std::vector<TString> samples  = {"data","ctau100","ctau2000","ctau6000"};
  std::vector<TString> tsamples = {"Data","c#tau = 36.5 mm","c#tau = 730.5 mm","c#tau = 2192 mm"};
  std::vector<Int_t>   njets    = {0,1,2,3};

  TCanvas * canv = new TCanvas(); canv->cd();
  TLegend * leg  = new TLegend(0.0,0.0,1.0,1.0);
  for (UInt_t isample = 0; isample < samples.size(); isample++)
  {
    for (UInt_t jnjet = 0; jnjet < njets.size(); jnjet++)
    {
      TH1F * hist = (TH1F*)file->Get(Form("eff_%s_njets%i",samples[isample].Data(),njets[jnjet]));
      //      hist->SetDirectory(0);

      if (isample == 0) hist->Draw((njets[jnjet] == 0)?"ep":"ep same");
      else              hist->Draw("hist same");

      leg->AddEntry(hist,Form("%s njets: %i",tsamples[isample].Data(),njets[jnjet]),"epl");

      //      delete hist;
    }
  }
  canv->SaveAs(Form("eff_jet_phpt%i_phvid%i.png",iphpt,ivid));

  TCanvas * canvL = new TCanvas(); canvL->cd();
  leg->Draw();
  if (iphpt == 20 && ivid == 1) canvL->SaveAs("eff_leg.png");
  
  delete canvL;
  delete leg;
  delete canv;
  delete file;
}
