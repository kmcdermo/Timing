void recophoton_comp()
{
  gStyle->SetOptStat(0);

  TString dataset = "sph_2016H";
  TString rhE = "";
  std::vector<TString> filenames = 
  {
    //Form("output/patphotons/vanilla/%s%s/plots.root",dataset.Data(),rhE.Data()),
    //    Form("output/patphotons/gedPhotons/%s%s/plots.root",dataset.Data(),rhE.Data()),
    Form("output/patphotons/OOT_Clusters/%s%s/plots.root",dataset.Data(),rhE.Data()),
    Form("output/patphotons/OOT_Clusters_NoHoE/%s%s/plots.root",dataset.Data(),rhE.Data()),
  };

  //  std::vector<TString> labels = {"Vanilla","gedPhotons","ootPhotons","ootPhotons (No H/E slim)","ootPhotons-sep"};
  //  std::vector<Color_t> colors = {kBlack,kBlue,kRed+1,kGreen+1,kMagenta,kOrange+1,kYellow-7,kViolet-1,kAzure+10,kYellow+3};
  std::vector<TString> labels = {"ootPhotons","ootPhotons (No H/E slim)","ootPhotons-sep"};
  std::vector<Color_t> colors = {kRed+1,kGreen+1,kMagenta,kOrange+1,kYellow-7,kViolet-1,kAzure+10,kYellow+3};

  std::vector<TFile*> files(filenames.size());
  for (UInt_t ifile = 0; ifile < filenames.size(); ifile++)
  {
    files[ifile] = TFile::Open(filenames[ifile].Data());
  }

  std::vector<TString> hists = 
  {
    "nphotonsAll","nPhotons","nPhotonsOOT","phnrh","phnrhOOT",
    "phE","phpt","phphi","pheta",
    //    "phHoE_OOT","phr9_OOT","phsieieEB_OOT","phsieieEE_OOT","phE_OOT","phpt_OOT","phphi_OOT","pheta_OOT",
    "phHoEEB","phr9EB","phsieieEB","phsieipEB","phsipipEB","phsmajEB","phsminEB","phEcalIsoEB","phHcalIsoEB",
    "phHoEEE","phr9EE","phsieieEE","phsieipEE","phsipipEE","phsmajEE","phsminEE","phEcalIsoEE","phHcalIsoEE",
    "phseedtime","phseedOOT","phseedE",
    "phseedtime_OOT","phseedE_OOT",
    "phrhtime","phrhE"
  };

  for (UInt_t ihist = 0; ihist < hists.size(); ihist++)
  { 
    TCanvas * canv = new TCanvas(); canv->cd(); canv->SetLogy(1);
    TLegend * leg  = new TLegend(0.75,0.8,0.99,0.99);

    std::vector<TH1F*> th1fs(filenames.size());
    Float_t max = -1e7;
    Float_t min =  1e7;
    for (UInt_t ith1f = 0; ith1f < th1fs.size(); ith1f++)
    {
      th1fs[ith1f] = (TH1F*)files[ith1f]->Get(hists[ihist]);

      th1fs[ith1f]->SetLineColor(colors[ith1f]);
      th1fs[ith1f]->SetMarkerColor(colors[ith1f]);
      //      th1fs[ith1f]->Scale(1.f/th1fs[ith1f]->GetEntries());
      th1fs[ith1f]->Scale(1.f/th1fs[ith1f]->Integral());

      Int_t nBinsX = th1fs[ith1f]->GetNbinsX();
      for (Int_t ibin = 1; ibin <= nBinsX; ibin++)
      {
	Float_t tmp = th1fs[ith1f]->GetBinContent(ibin);
	if (tmp > max) max = tmp;
	if (tmp != 0.f && tmp < min) min = tmp;
      }
    }

    for (UInt_t ith1f = 0; ith1f < th1fs.size(); ith1f++)
    {
      th1fs[ith1f]->SetMaximum(max*2);
      th1fs[ith1f]->SetMinimum(min/2);
      th1fs[ith1f]->Draw(ith1f>0?"same epl":"epl");
      leg->AddEntry(th1fs[ith1f],Form("%s: %5.3f",labels[ith1f].Data(),th1fs[ith1f]->GetMean()),"epl");
    }

    leg->Draw("same");    
    canv->SaveAs(Form("output/patphotons/nohoe/%s%s_%s.png",dataset.Data(),rhE.Data(),hists[ihist].Data()));

    for (UInt_t ith1f = 0; ith1f < th1fs.size(); ith1f++)   
    {
      delete th1fs[ith1f];
    }

    delete leg;
    delete canv;
  }
}
