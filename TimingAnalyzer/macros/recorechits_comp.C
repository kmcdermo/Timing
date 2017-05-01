void recophoton_comp()
{
  gStyle->SetOptStat(0);

  std::vector<TString> filenames = 
    {"output/rechits/reco/deg_2016B/plots.root","output/rechits/rereco/deg_2016B/plots.root",
     "output/rechits/reco/sph_2016C/plots.root","output/rechits/rereco/sph_2016C/plots.root"};

  std::vector<TString> labels = {"RECO DEG2016B","ReRECO DEG2016B","RECO SPH2016C","ReRECO SPH2016C"};
  std::vector<Color_t> colors = {kBlack,kBlue,kGreen+1,kMagenta};

  std::vector<TFile*> files(filenames.size());
  for (UInt_t ifile = 0; ifile < filenames.size(); ifile++)
  {
    files[ifile] = TFile::Open(filenames[ifile].Data());
  }

  std::vector<TString> hists = {"nphotons","phE","phseedtime","phseedOOT","phnrh","phnrhOOT"};
  for (UInt_t ihist = 0; ihist < hists.size(); ihist++)
  { 
    TCanvas * canv = new TCanvas(); canv->cd(); canv->SetLogy(1);
    TLegend * leg  = new TLegend(0.65,0.65,0.95,0.95);

    std::vector<TH1F*> th1fs(filenames.size());
    Float_t max = -1e7;
    for (UInt_t ith1f = 0; ith1f < th1fs.size(); ith1f++)
    {
      th1fs[ith1f] = (TH1F*)files[ith1f]->Get(hists[ihist]);

      th1fs[ith1f]->SetLineColor(colors[ith1f]);
      th1fs[ith1f]->SetMarkerColor(colors[ith1f]);

      Float_t tmpmax = th1fs[ith1f]->GetMaximum();

      if (tmpmax > max) max = tmpmax;
    }

    for (UInt_t ith1f = 0; ith1f < th1fs.size(); ith1f++)
    {
      th1fs[ith1f]->SetMaximum(max*1.1);
      th1fs[ith1f]->Draw(ith1f>0?"same hist":"hist");
      leg->AddEntry(th1fs[ith1f],Form("%s: %5.3f",labels[ith1f].Data(),th1fs[ith1f]->GetMean()),"l");
    }

    leg->Draw("same");    
    canv->SaveAs(Form("output/recophotons/%s.png",hists[ihist].Data()));

    for (UInt_t ith1f = 0; ith1f < th1fs.size(); ith1f++)   
    {
      delete th1fs[ith1f];
    }

    delete leg;
    delete canv;
  }
}
