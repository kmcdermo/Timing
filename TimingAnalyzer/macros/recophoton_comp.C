void recophoton_comp()
{
  gStyle->SetOptStat(0);

  TString dataset = "sph_2016C";
  TString rhE = "";
  std::vector<TString> filenames = 
  {
    Form("output/recophotons/reco/%s%s/plots.root",dataset.Data(),rhE.Data()),
    Form("output/recophotons/rereco/%s%s/plots.root",dataset.Data(),rhE.Data()),
    Form("output/recophotons/rereco/%s_pt%s/plots.root",dataset.Data(),rhE.Data()),
    Form("output/recophotons/rereco/%s_hoe%s/plots.root",dataset.Data(),rhE.Data()),
    Form("output/recophotons/rereco/%s_r9%s/plots.root",dataset.Data(),rhE.Data()),
    Form("output/recophotons/rereco/%s_sieie%s/plots.root",dataset.Data(),rhE.Data())
  };

  std::vector<TString> labels = {"RECO","ReRECO","ReRECO+p_{T}","ReRECO+p_{T}+H/E","ReRECO+p_{T}+H/E+R_{9}","ReRECO+p_{T}+H/E+R_{9}+#sigma_{i#eta i#eta}"};
  std::vector<Color_t> colors = {kBlack,kBlue,kRed+1,kGreen+1,kMagenta,kOrange+1,kYellow-7,kViolet-1,kAzure+10,kYellow+3};

  std::vector<TFile*> files(filenames.size());
  for (UInt_t ifile = 0; ifile < filenames.size(); ifile++)
  {
    files[ifile] = TFile::Open(filenames[ifile].Data());
  }

  std::vector<TString> hists = 
  {
    "nphotons","nphotonsOOT","phnrh","phnrhOOT",
    "phE","phpt","phphi","pheta","phE_OOT","phpt_OOT","phphi_OOT","pheta_OOT",
    "phHoE_OOT","phr9_OOT","phsieieEB_OOT","phsieieEE_OOT",
    "phseedtime","phseedOOT","phseedE",
    "phseedtime_OOT","phseedE_OOT",
    "phrhtime","phrhE"
  };

  for (UInt_t ihist = 0; ihist < hists.size(); ihist++)
  { 
    TCanvas * canv = new TCanvas(); canv->cd(); canv->SetLogy(1);
    TLegend * leg;
    if (hists[ihist].Contains("pheta") || hists[ihist].Contains("phphi")) leg = new TLegend(0.35,0.35,0.65,0.65);
    else leg = new TLegend(0.65,0.65,0.95,0.95);

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
      if (hists[ihist].Contains("phseedE_OOT")){th1fs[ith1f]->SetMaximum(100);th1fs[ith1f]->SetMinimum(1);}
      th1fs[ith1f]->Draw(ith1f>0?"same epl":"epl");
      leg->AddEntry(th1fs[ith1f],Form("%s: %5.3f",labels[ith1f].Data(),th1fs[ith1f]->GetMean()),"epl");
    }

    if (!hists[ihist].Contains("pheta_OOT") && !hists[ihist].Contains("phphi_OOT")) leg->Draw("same");    
    canv->SaveAs(Form("output/recophotons/%s%s_%s.png",dataset.Data(),rhE.Data(),hists[ihist].Data()));

    for (UInt_t ith1f = 0; ith1f < th1fs.size(); ith1f++)   
    {
      delete th1fs[ith1f];
    }

    delete leg;
    delete canv;
  }
}
