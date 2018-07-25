struct HistInfo
{
  HistInfo() {}
  HistInfo(const TString & name, const TString & cut, const TString & label, const Color_t color) :
    name(name), cut(cut), label(label), color(color) {}

  TString name;
  TString cut;
  TString label;
  Color_t color;
};

void setup(std::vector<HistInfo> & infos)
{
  infos.emplace_back("std","Sum$(jetpt*(jetpt>30))>400","HT>400",kBlack);
  infos.emplace_back("empty","","Empty",kBlue);
  infos.emplace_back("nj2","Sum$(1*(jetpt>100))>=2","nJ100#geq2",kRed+1);
  infos.emplace_back("nj3","Sum$(1*(jetpt>100))>=3","nJ100#geq3",kGreen+1);
  infos.emplace_back("nj4","Sum$(1*(jetpt>100))>=4","nJ100#geq4",kMagenta);
  infos.emplace_back("pho","phopt_0>100","p_{T}^{#gamma_{0}}>100",kOrange+1);
  infos.emplace_back("nj2_pho","Sum$(1*(jetpt>100))>=2&&phopt_0>100","nJ100#geq2 && p_{T}^{#gamma_{0}}>100",kYellow-7);
  infos.emplace_back("nj3_pho","Sum$(1*(jetpt>100))>=3&&phopt_0>100","nJ100#geq3 && p_{T}^{#gamma_{0}}>100",kViolet-1);
  infos.emplace_back("nj4_pho","Sum$(1*(jetpt>100))>=4&&phopt_0>100","nJ100#geq4 && p_{T}^{#gamma_{0}}>100",kAzure+10);
}

Float_t GetMin(const std::vector<TH1F*> & hists)
{
  Float_t min = 1e9;
  for (auto & hist : hists)
  {
    for (auto ibin = 1; ibin <= hist->GetXaxis()->GetNbins(); ibin++)
    {
      const Float_t tmpmin = hist->GetBinContent(ibin);
      if ((tmpmin > 0.f) && (tmpmin < min)) min = tmpmin;
    }
  }
  return min;
}

Float_t GetMax(const std::vector<TH1F*> & hists)
{
  Float_t max = -1e9;
  for (auto & hist : hists)
  {
    const Float_t tmpmax = hist->GetBinContent(hist->GetMaximumBin());
    if (tmpmax > max) max = tmpmax;
  }
  return max;
}

void draw(std::vector<TH1F*> & hists, TLegend *& leg, const TString & label, const Bool_t isLogy)
{
  // get min, max for unscaled hists
  const auto min = GetMin(hists);
  const auto max = GetMax(hists);

  // make canvas
  auto canv = new TCanvas();
  canv->cd();
  canv->SetTicky();
  canv->SetLogy(isLogy);

  for (auto i = 0U; i < hists.size(); i++)
  {
    auto & hist = hists[i];

    hist->SetMinimum(isLogy ? (min / 1.5) : (min / 1.05) );
    hist->SetMaximum(isLogy ? (max * 1.5) : (max * 1.05) );

    hist->Draw(i>0?"same ep":"ep");
  }
  
  leg->Draw("same");

  const std::vector<TString> extensions = {"png","pdf","eps"};
  for (const auto & extension : extensions)
  {
    canv->SaveAs(Form("hltSignal_SR_Data_%s_%s.%s",label.Data(),(isLogy?"log":"lin"),extension.Data()));
  }
}

void overplot_hltBool()
{
  gStyle->SetOptStat(0);
  
  auto file = TFile::Open("skims/sr_noht_nohlt.root");
  auto tree = (TTree*)file->Get("Data_Tree");

  std::vector<HistInfo> infos;
  setup(infos);
  
  // bin info
  const Int_t   nbins = 2;
  const Float_t low   = 0.f;
  const Float_t high  = 2.f;
  
  // labels
  const TString title  = "hltSignal vs cuts";
  const TString xtitle = "hltSignal";
  const TString ytitle = "Events";

  auto leg = new TLegend(0.15,0.65,0.45,0.95);
  leg->SetNColumns(2);

  std::vector<TH1F*> hists(infos.size());
  for (auto i = 0U; i < hists.size(); i++)
  {
    const auto & info = infos[i];
    auto & hist = hists[i];

    std::cout << "Working on: " << info.name.Data() << std::endl;

    hist = new TH1F(info.name.Data(),title.Data(),nbins,low,high);

    hist->GetXaxis()->SetTitle(xtitle.Data());
    hist->GetYaxis()->SetTitle(ytitle.Data());
    hist->SetLineColor(info.color);
    hist->SetMarkerColor(info.color);
    hist->Sumw2();

    tree->Draw(Form("hltSignal>>%s",hist->GetName()),info.cut.Data(),"goff");

    leg->AddEntry(hist,info.label.Data(),"epl");

    std::cout << hist->GetName() << ": " << hist->GetBinContent(2) << std::endl;
  }

  // unscaled
  draw(hists,leg,"norm",true);
  draw(hists,leg,"norm",false);

  // scale to unity
  for (auto & hist : hists)
  {
    hist->Scale(1.f/hist->Integral());

    std::cout << hist->GetName() << ": " << hist->GetBinContent(2) << std::endl;
  }

  draw(hists,leg,"scaled",true);
  draw(hists,leg,"scaled",false);
}
