static const TString outtext = "effseedE_vs_gain_seedE1E210to120_EBEB";
static const TString var = "(phoseedE_0*phoseedE_1)/(sqrt(pow(phoseedE_0,2)+pow(phoseedE_1,2)))";
static const TString title = "Effective Electron Seed Energy vs Gain (EBEB);Effective Leading Electron Seed Energy [GeV];Events";
//static const TString commoncut = "((phoisEB_0&&phoisEB_1))*";
static const TString commoncut = "((phoisEB_0&&phoseedE_0>10&&phoseedE_0<120)&&(phoisEB_1&&phoseedE_1>10&&phoseedE_1<120))*";

TH1F * makePlot(const TString & gain, const Bool_t isMC)
{
  const std::vector<Double_t> xbins = {1,5,10,20,30,40,50,60,80,100,150,200,250,300,500};
  const Double_t * bins = &xbins[0];

  const std::map<TString,Color_t> colors = {
    {"12_12",kBlue},
    {"12_6" ,kRed+1},
    {"12_1" ,kGreen+1},
    {"6_6"  ,kMagenta},
    {"6_1"  ,kAzure+10},
    {"1_1"  ,kOrange+1}
  };

  auto hist = new TH1F((isMC?"mc":"data")+gain,title.Data(),xbins.size()-1,bins); 
  hist->Sumw2();

  hist->SetLineColor(colors.at(gain));
  hist->SetMarkerColor(colors.at(gain));
  if (isMC) hist->SetLineStyle(7);

  return hist;
}

void fillPlot(TH1F *& hist, TTree *& tree, const TString & gain, const Bool_t isMC)
{
  std::cout << "Filling for: " << (isMC?"MC":"Data") << " " << gain.Data() << std::endl;

  const TString mcwgt = "*(evtwgt*puwgt)";
  const std::map<TString,TString> cuts = {
    {"12_12","(((!phoseedisGS6_0&&!phoseedisGS1_0)&&(!phoseedisGS6_1&&!phoseedisGS1_1)))"},
    {"12_6" ,"(((!phoseedisGS6_0&&!phoseedisGS1_0)&&(phoseedisGS6_1))||((!phoseedisGS6_1&&!phoseedisGS1_1)&&(phoseedisGS6_0)))"},
    {"12_1" ,"(((!phoseedisGS6_0&&!phoseedisGS1_0)&&(phoseedisGS1_1))||((!phoseedisGS6_1&&!phoseedisGS1_1)&&(phoseedisGS1_0)))"},
    {"6_6"  ,"(((phoseedisGS6_0)&&(phoseedisGS6_1)))"},
    {"6_1"  ,"(((phoseedisGS6_0)&&(phoseedisGS1_1))||((phoseedisGS6_1)&&(phoseedisGS1_0)))"},
    {"1_1"  ,"(((phoseedisGS1_0)&&(phoseedisGS1_1)))"}
  };

  hist->GetDirectory()->cd(); // fuck ROOT: https://root-forum.cern.ch/t/filling-histograms-with-ttree-draw-in-macro-fails/4796/2
  tree->Draw(var+">>"+(isMC?"mc":"data")+gain,commoncut+cuts.at(gain)+(isMC?mcwgt:""),"goff");
}

void scalePlot(TH1F *& hist)
{
  hist->Scale(1.f/hist->Integral("width"));
}

void legPlot(TLegend *& leg, TH1F *& hist, const TString & gain, const Bool_t isMC)
{
  TString sgain = gain;
  sgain.ReplaceAll("_",",");
  leg->AddEntry(hist,(isMC?"MC":"Data")+TString(" [Gain: ")+sgain+"]","epl");
}

void quick_draw_GS()
{
  gStyle->SetOptStat(0);

  const std::vector<TString> gains = {"12_12","12_6","12_1","6_6","6_1","1_1"};

  // make plots
  std::map<TString,TH1F*> datahists;
  std::map<TString,TH1F*> mchists;
  for (const auto & gain : gains)
  {
    datahists[gain] = makePlot(gain,false);
    mchists  [gain] = makePlot(gain,true);
  }
  
  // get files
  auto datafile = TFile::Open("madv2_test_v1/Data/DoubleEG/D/v1/tree.root");
  auto mcfile   = TFile::Open("madv2_test_v1/MC/DYJetsToLL/base/tree.root");

  // get trees
  auto datatree = (TTree*)datafile->Get("disphotree");
  auto mctree   = (TTree*)mcfile  ->Get("disphotree");

  // fill plots
  for (const auto & gain : gains)
  {
    fillPlot(datahists[gain],datatree,gain,false);
    fillPlot(mchists  [gain],mctree  ,gain,true);
  }

  // scale plots
  for (const auto & gain : gains)
  {
    scalePlot(datahists[gain]);
    scalePlot(mchists  [gain]);
  }
  
  // canv
  auto canv = new TCanvas();
  canv->cd();
  canv->SetLogx();
  canv->SetLogy();
  canv->SetTickx();
  canv->SetTicky();
  canv->SetGridy();

  // leg
  auto leg = new TLegend(0.13,0.13,0.6,0.35);
  leg->SetNColumns(2);
  for (const auto & gain : gains)
  {
    legPlot(leg,datahists[gain],gain,false);
    legPlot(leg,mchists  [gain],gain,true);
  }

  // set range
  datahists["12_12"]->GetYaxis()->SetRangeUser(1e-6,1e-1);
  
  // draw
  for (const auto & gain : gains)
  {
    datahists[gain]->Draw(gain.EqualTo("12_12")?"ep":"ep same");
    mchists  [gain]->Draw("ep same");
  }
  leg->Draw("same");

  canv->SaveAs(outtext+".png");
}
