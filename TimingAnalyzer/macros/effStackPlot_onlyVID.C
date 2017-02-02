void effStackPlot_onlyVID(Int_t ijetpt, Int_t injets)
{
  gStyle->SetOptStat(0);

  TFile * file = TFile::Open(Form("output/recoskim/onlyvid-sph1/njets%i/jetpt%i/plots.root",injets,ijetpt));
  file->cd();

//   std::vector<TString> samples  = {"sph2016B","ctau100","ctau2000","ctau6000"};
//   std::vector<TString> tsamples = {"2016B SinglePh","c#tau = 36.5 mm","c#tau = 730.5 mm","c#tau = 2192 mm"};
  std::vector<TString> samples  = {"sph2016B","ctau100","ctau6000"};
  std::vector<TString> tsamples = {"2016B SinglePh","c#tau = 36.5 mm","c#tau = 2192 mm"};
  std::vector<Int_t>   phpts    = {30,40,50};

  TCanvas * canv = new TCanvas(); canv->cd();
  TLegend * leg  = new TLegend(0.0,0.0,1.0,1.0);
  for (UInt_t isample = 0; isample < samples.size(); isample++)
  {
    for (UInt_t jphpt = 0; jphpt < phpts.size(); jphpt++)
    {
      TH1F * hist = (TH1F*)file->Get(Form("eff_%s_phpt%i",samples[isample].Data(),phpts[jphpt]));

      if (isample == 0) hist->Draw((phpts[jphpt] == 0)?"ep":"ep same");
      else              hist->Draw("hist ep same");

      leg->AddEntry(hist,Form("%s photon p_{T}: %i",tsamples[isample].Data(),phpts[jphpt]),"epl");
    }
  }
  canv->SaveAs(Form("eff_onlyvid_jetpt%i_njets%i.png",ijetpt,injets));

  TCanvas * canvL = new TCanvas(); canvL->cd();
  leg->Draw();
  if (ijetpt == 0 && injets == 0) canvL->SaveAs("eff_leg_onlyVID.png");
  
  delete canvL;
  delete leg;
  delete canv;
  delete file;
}
