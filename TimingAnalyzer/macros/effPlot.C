void effPlot(Int_t iphpt, Int_t ivid)
{
  TFile * file = new TFile(Form("output/phpt%i/phvid%i/plots.root",iphpt,ivid),"UPDATE");
  file->cd();

  std::vector<TString> samples = {"data","ctau100","ctau2000","ctau6000"};
  std::vector<Color_t> colors  = {kBlack,kRed+1,kViolet-1,kBlue+1};

  std::vector<Int_t>   njets   = {0,1,2,3};
  std::vector<Style_t> markers = {kFullCircle,kFullSquare,kFullTriangleUp,kFullTriangleDown};

  for (UInt_t isample = 0; isample < samples.size(); isample++)
  {
    for (UInt_t jnjet = 0; jnjet < njets.size(); jnjet++)
    {
      TH1F * hist = new TH1F(Form("eff_%s_njets%i",samples[isample].Data(),jnjet),Form("Efficiency vs. Jet p_{T} [Photon p_{T} = %i GeV/c, VID = %s]",iphpt,(ivid==1?"loose":"medium")),4,0,4);
      hist->GetYaxis()->SetTitle("Efficiency");
      hist->GetXaxis()->SetTitle("Jet p_{T} [GeV/c]");
      hist->GetXaxis()->SetBinLabel(1,"20");
      hist->GetXaxis()->SetBinLabel(2,"35");
      hist->GetXaxis()->SetBinLabel(3,"50");
      hist->GetXaxis()->SetBinLabel(4,"80");
      
      hist->SetLineColor(colors[isample]);
      hist->SetMarkerColor(colors[isample]);
      hist->SetMarkerStyle(markers[jnjet]);

      hist->Write();
    }
  }
}
