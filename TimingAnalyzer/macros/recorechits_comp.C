void recorechits_comp()
{
  gStyle->SetOptStat(0);

  TString dataset = "sph_2016C";
  TString rhE = "";
  std::vector<TString> filenames = 
  {
    Form("output/rechits/reco/%s%s/plots.root",dataset.Data(),rhE.Data()),
    Form("output/rechits/rereco/%s%s/plots.root",dataset.Data(),rhE.Data())
  };
  
  std::vector<TString> labels = {"RECO","ReRECO"};
  std::vector<Color_t> colors = {kRed,kBlue};

  std::vector<TFile*> files(filenames.size());
  for (UInt_t ifile = 0; ifile < filenames.size(); ifile++)
  {
    files[ifile] = TFile::Open(filenames[ifile].Data());
  }

  std::vector<TString> hists = 
  {
    "nrh","nrhOOT","nrhEB","nrhEE","rhE","rhE_zoom","rhtime","rhOOT",
    "rhE_OOTT","rhE_OOTT_zoom","rhtime_OOTT","rhtime_OOTT_zoom",
    "rhE_OOTF","rhE_OOTF_zoom","rhtime_OOTF","rhtime_OOTF_zoom"
  };
  
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
    canv->SaveAs(Form("output/rechits/%s%s_%s.png",dataset.Data(),rhE.Data(),hists[ihist].Data()));

    for (UInt_t ith1f = 0; ith1f < th1fs.size(); ith1f++)   
    {
      delete th1fs[ith1f];
    }

    delete leg;
    delete canv;
  }
}
