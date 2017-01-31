void effPlot_VID(Int_t ijetpt, Int_t injets)
{
  TFile * file = new TFile(Form("output/recoskim/njets%i/jetpt%i/plots.root",injets,ijetpt),"UPDATE");
  file->cd();

  std::vector<TString> samples = {"deg2016B","ctau100","ctau2000","ctau6000"};
  std::vector<Color_t> colors  = {kBlack,kRed+1,kViolet-1,kBlue+1};

  std::vector<Int_t>   phpts   = {30,50,80};
  std::vector<Style_t> markers = {kFullCircle,kFullSquare,kFullTriangleUp,kFullTriangleDown};

  for (UInt_t isample = 0; isample < samples.size(); isample++)
  {
    for (UInt_t jphpt = 0; jphpt < phpts.size(); jphpt++)
    {
      TH1F * hist = new TH1F(Form("eff_%s_phpt%i",samples[isample].Data(),phpts[jphpt]),Form("Efficiency vs. VID Selection [Jet p_{T} = %i GeV/c, nJets = %i]",ijetpt,injets),6,0,6);
      hist->GetYaxis()->SetTitle("Efficiency");
      hist->GetYaxis()->SetRangeUser(0.f,1.f);
      hist->GetXaxis()->SetTitle("VID Selection");
      hist->GetXaxis()->SetBinLabel(1,"Full");
      hist->GetXaxis()->SetBinLabel(2,"No PhIso");
      hist->GetXaxis()->SetBinLabel(3,"No NeuIso");
      hist->GetXaxis()->SetBinLabel(4,"No ChgIso");
      hist->GetXaxis()->SetBinLabel(5,"No Sieie");
      hist->GetXaxis()->SetBinLabel(6,"No HoE");
      
      hist->SetLineColor(colors[isample]);
      hist->SetMarkerColor(colors[isample]);
      hist->SetMarkerStyle(markers[jphpt]);

      hist->Write();
    }
  }
  delete file;
}
