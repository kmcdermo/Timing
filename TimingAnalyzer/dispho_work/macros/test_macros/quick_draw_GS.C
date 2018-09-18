static const TString var = "phoseedE_0";
static const TString title = "Leading Electron Seed Energy vs Gain;Leading Electron Seed Energy [GeV];Events";
static const TString commoncut = "(phoisEB_0)*";

TH1F * makePlot(const TString & gain, const Bool_t isMC)
{
  const std::vector<Double_t> xbins = {1,5,10,20,30,40,50,60,80,100,150,200,250,300,500,1000,2000,5000};
  const Double_t * bins = &xbins[0];

  const std::map<TString,Color_t> colors = {{"12",kRed},{"6",kBlue}, {"1",kGreen-1}};

  auto hist = new TH1F((isMC?"mc":"data")+gain,title.Data(),xbins.size()-1,bins); 
  hist->Sumw2();

  hist->SetLineColor(colors.at(gain));
  hist->SetMarkerColor(colors.at(gain));
  if (isMC) hist->SetLineStyle(7);

  return hist;
}

void fillPlot(TH1F *& hist, TTree *& tree, const TString & gain, const Bool_t isMC)
{
  const TString mcwgt = "*(evtwgt*puwgt)";
  const std::map<TString,TString> cuts = {{"12","(!phoseedisGS6_0&&!phoseedisGS1_0)"},{"6","(phoseedisGS6_0)"},{"1","(phoseedisGS1_0)"}};

  hist->GetDirectory()->cd(); // fuck ROOT: https://root-forum.cern.ch/t/filling-histograms-with-ttree-draw-in-macro-fails/4796/2
  tree->Draw(var+">>"+(isMC?"mc":"data")+gain,commoncut+cuts.at(gain)+(isMC?mcwgt:""),"goff");
}

void scalePlot(TH1F *& hist)
{
  hist->Scale(1.f/hist->Integral("width"));
}

void legPlot(TLegend *& leg, TH1F *& hist, const TString & gain, const Bool_t isMC)
{
  leg->AddEntry(hist,(isMC?"MC":"Data")+TString(" [Gain")+gain+"]","epl");
}

void quick_draw_GS()
{
  gStyle->SetOptStat(0);

  // make plots
  auto data12 = makePlot("12",false);
  auto data6  = makePlot("6" ,false);
  auto data1  = makePlot("1" ,false);
  auto mc12   = makePlot("12",true);
  auto mc6    = makePlot("6" ,true);
  auto mc1    = makePlot("1" ,true);
  
  // get files
  auto datafile = TFile::Open("madv2_test_v1/Data/DoubleEG/D/v1/tree.root");
  auto mcfile   = TFile::Open("madv2_test_v1/MC/DYJetsToLL/base/tree.root");

  // get trees
  auto datatree = (TTree*)datafile->Get("disphotree");
  auto mctree   = (TTree*)mcfile  ->Get("disphotree");

  // fill plots
  fillPlot(data12,datatree,"12",false);
  fillPlot(data6 ,datatree,"6" ,false);
  fillPlot(data1 ,datatree,"1" ,false);
  fillPlot(mc12  ,mctree  ,"12",true);
  fillPlot(mc6   ,mctree  ,"6" ,true);
  fillPlot(mc1   ,mctree  ,"1" ,true);

  // scale plot
  scalePlot(data12);
  scalePlot(data6);
  scalePlot(data1);
  scalePlot(mc12);
  scalePlot(mc6);
  scalePlot(mc1);

  // canv
  auto canv = new TCanvas();
  canv->cd();
  canv->SetLogx();
  canv->SetLogy();
  canv->SetTickx();
  canv->SetTicky();

  // leg
  auto leg = new TLegend(0.5,0.75,0.87,0.87);
  leg->SetNColumns(2);
  legPlot(leg,data12,"12",false);
  legPlot(leg,mc12  ,"12",true);
  legPlot(leg,data6 ,"6" ,false);
  legPlot(leg,mc6   ,"6" ,true);
  legPlot(leg,data1 ,"1" ,false);
  legPlot(leg,mc1   ,"1" ,true);

  // set range
  data12->GetYaxis()->SetRangeUser(1e-8,1);
  
  // draw
  data12->Draw("ep");
  mc12->Draw("ep same");
  data6->Draw("ep same");
  mc6->Draw("ep same");
  data1->Draw("ep same");
  mc1->Draw("ep same");

  leg->Draw("same");
}
