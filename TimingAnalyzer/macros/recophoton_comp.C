void recophoton_comp()
{
  gStyle->SetOptStat(0);

  std::vector<TFile*> files(2);
  files[0] = TFile::Open("output/recophotons/reco/deg_2016B/plots.root");
  files[1] = TFile::Open("output/recophotons/rereco/deg_2016B/plots.root");
  files[2] = TFile::Open("output/recophotons/rereco/deg_2016B_ptcuts/plots.root");
  files[3] = TFile::Open("output/recophotons/reco/sph_2016C/plots.root");
  files[4] = TFile::Open("output/recophotons/rereco/sph_2016C/plots.root");
  files[5] = TFile::Open("output/recophotons/rereco/sph_2016C_ptcuts/plots.root");

  std::vector<TString> labels = {"RECO DEG 2016B","ReRECO DEG 2016B","ReRECO SPH 2016C + p_{T}","RECO SPH 2016C","ReRECO SPH 2016C","ReRECO SPH 2016C + p_{T}"};
  std::vector<Color_t> colors = {kBlack,kBlue,kViolet-1,kRed+1,kOrange+1,kYellow-7,kGreen+1,kAzure+10,kMagenta,kYellow+3};

  std::vector<TString> hists;
  hists.push_back("nphotons");
  hists.push_back("phE");
  hists.push_back("phseedtime");
  hists.push_back("phseedOOT");
  hists.push_back("phnrh");
  hists.push_back("phnrhOOT");

  for (UInt_t ihist = 0; ihist < hists.size(); ihist++)
  { 
    TCanvas * canv = new TCanvas(); canv->cd(); canv->SetLogy(1);
    TLegend * leg  = new TLegend(0.65,0.65,1.0,1.0);

    std::vector<TH1F*> th1fs(files.size());
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
      leg->AddEntry(th1fs[ith1f],Form("%s: %4.2f",labels[ith1f].Data(),th1fs[ith1f]->GetMean()),"l");
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
