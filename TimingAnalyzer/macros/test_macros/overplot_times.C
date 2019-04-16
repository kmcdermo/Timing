void scale(TH1F *& hist)
{
  for (auto ibin = 1; ibin <= hist->GetXaxis()->GetNbins(); ibin++)
  {
    const auto width = hist->GetXaxis()->GetBinWidth(ibin);
    hist->SetBinContent(ibin,hist->GetBinContent(ibin)/width);
    hist->SetBinError  (ibin,hist->GetBinError  (ibin)/width);
  }
}

void overplot_times()
{
  gStyle->SetOptStat(0);

  auto file = TFile::Open("signals_sr_nolepveto.root");
  std::vector<TString> samples = {"GMSB_L200_CTau10","GMSB_L200_CTau200","GMSB_L200_CTau1000"};
  std::vector<TString> labels  = {"#Lambda=200TeV, c#tau=10cm","#Lambda=200TeV, c#tau=200cm","#Lambda=200TeV, c#tau=1000cm"};
  const auto nsamples = samples.size();
  const std::vector<Color_t> colors = {kBlue,kRed+1,kGreen+1};
  const std::vector<Double_t> xbins = {-2.0,-1.5,-1.0,-0.75,-0.5,-0.25,0.0,0.25,0.5,0.75,1.0,1.5,2,3,5,10};
  const auto & binsX = &xbins[0];
  const auto nbinsX = xbins.size()-1;

  auto leg = new TLegend(0.58,0.56,0.86,0.86);

  std::vector<TH1F*> before_hists(nsamples);
  std::vector<TH1F*> after_hists(nsamples);

  auto canv = new TCanvas();
  canv->cd();
  canv->SetLogy();
  canv->SetTickx();
  canv->SetTicky();

  for (auto isample = 0U; isample < nsamples; isample++)
  {
    const auto & sample = samples[isample];
    auto tree = (TTree*)file->Get(sample+"_Tree");

    const auto color = colors[isample];
    const auto & label = labels[isample];
    auto & before_hist = before_hists[isample];
    auto & after_hist  = after_hists [isample];

    before_hist = new TH1F("before_"+sample,"Before/After Time Corrections;Weighted Cluster Time [ns];Fraction of Events/ns",nbinsX,binsX);
    before_hist->Sumw2();
    before_hist->SetLineColor(color);
    before_hist->SetMarkerColor(color);
    before_hist->SetLineWidth(2);
    before_hist->SetLineStyle(1);
    before_hist->GetYaxis()->SetRangeUser(1e-3,1e3);

    after_hist = new TH1F("after_"+sample,"Before/After Time Corrections;Weighted Cluster Time [ns];Fraction of Events/ns",nbinsX,binsX);
    after_hist->Sumw2();
    after_hist->SetLineColor(color);
    after_hist->SetMarkerColor(color);
    after_hist->SetLineWidth(2);
    after_hist->SetLineStyle(7);
    after_hist->GetYaxis()->SetRangeUser(1e-3,1e3);

    tree->Draw(Form("phoweightedtimeLT120_0>>%s",before_hist->GetName()),"puwgt * evtwgt","goff");
    tree->Draw(Form("phoweightedtimeLT120_0+phoweightedtimeLT120SHIFT_0+phoweightedtimeLT120SMEAR_0>>%s",after_hist->GetName()),"puwgt * evtwgt","goff");
  
    scale(before_hist);
    scale(after_hist);

    leg->AddEntry(before_hist,label+" (Before)","l");
    leg->AddEntry(after_hist ,label+" (After)" ,"l");

    before_hist->Draw(isample>0?"hist same":"hist");
    after_hist ->Draw("hist same");
  }
  leg->Draw("same");
}
