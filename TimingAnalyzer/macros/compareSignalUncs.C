#include "Common.cpp+"

struct PlotInfo
{
  PlotInfo() {}
  PlotInfo(const TString & name, const TString & var, const std::vector<Double_t> & bins, const TString & title, const TString & unit)
    : name(name), var(var), bins(bins), title(title), unit(unit) {}
  
  TString name;
  TString var;
  std::vector<Double_t> bins;
  TString title;
  TString unit;
};

void readSampleInfo(const TString & scan_log, std::vector<TString> & samples);
void makePlot(const TString & sample, const PlotInfo & plotInfo, TTree * nom_tree, TTree * unc_tree, TFile * outfile, const TString & syst_unc_name, const TString & syst_unc_label);
TH1F * setupPlot(const TString & name, const TString & title, const std::vector<Double_t> & bins, const Color_t color);
void getMinMax(const TH1F * hist, Double_t & minY, Double_t & maxY);

void compareSignalUncs(const TString & scan_log, const TString & nom_file_name, const TString & unc_file_name, const TString & syst_unc_name, const TString & syst_unc_label)
{
  // style
  gStyle->SetOptStat(0);

  // get inputs
  auto nom_file = TFile::Open(nom_file_name);
  Common::CheckValidFile(nom_file,nom_file_name);

  auto unc_file = TFile::Open(unc_file_name);
  Common::CheckValidFile(unc_file,unc_file_name);
  
  // output file
  auto outfile = TFile::Open(syst_unc_name+"_plots.root","RECREATE");
  
  // setup samples
  std::vector<TString> samples;
  readSampleInfo(scan_log,samples);

  // hist info
  const std::vector<PlotInfo> plotInfos = 
  {
    {"phowgttime","phoweightedtimeLT120_0+phoweightedtimeLT120SHIFT_0+phoweightedtimeLT120SMEAR_0",{-2,-1.5,-1,-0.75,-0.5,-0.25,0,0.25,0.5,0.75,1.0,1.5,2.0,3.0,5.0,10.0},"Leading Photon Weighted Cluster Time","ns"},
    {"met","t1pfMETpt",{0,50,75,100,125,150,200,300,500,1000,3000},"p_{T}^{miss}","GeV"}
  };

  // loop samples, hists
  for (const auto & sample : samples)
  {
    // get trees
    auto nom_tree = (TTree*)nom_file->Get(sample+"_Tree");
    auto unc_tree = (TTree*)unc_file->Get(sample+"_Tree");

    // loop plots
    for (const auto & plotInfo : plotInfos)
    {
      makePlot(sample,plotInfo,nom_tree,unc_tree,outfile,syst_unc_name,syst_unc_label);
    }

    // delete it all
    delete unc_tree;
    delete nom_tree;
  }

  // delete it all
  delete outfile;
  delete unc_file;
  delete nom_file;
}

void readSampleInfo(const TString & scan_log, std::vector<TString> & samples)
{
  std::ifstream input(scan_log.Data(),std::ios::in);
  TString sample, fulldir, xy;

  while (input >> sample >> fulldir >> xy)
  {
    // emplace back sample
    samples.emplace_back(sample);
  }
}

void makePlot(const TString & sample, const PlotInfo & plotInfo, TTree * nom_tree, TTree * unc_tree, TFile * outfile, const TString & syst_unc_name, const TString & syst_unc_label)
{
  // make canvas
  auto canv = new TCanvas();
  canv->cd();

  auto upad = new TPad("upad","",Common::left_up,Common::bottom_up,Common::right_up,Common::top_up);
  upad->SetBottomMargin(Common::merged_margin);
  upad->Draw();
  upad->SetTickx();
  upad->SetTicky();
  upad->SetLogy();

  auto lpad = new TPad("lpad","",Common::left_lp,Common::bottom_lp,Common::right_lp,Common::top_lp);
  lpad->SetTopMargin(Common::merged_margin);
  lpad->SetBottomMargin(Common::bottom_margin);
  lpad->Draw();
  lpad->SetTickx();
  lpad->SetTicky();
  
  // make legend
  auto leg = new TLegend(0.85,0.85,0.99,0.99);

  // get plotInfo
  const auto & name   = plotInfo.name;
  const auto & var    = plotInfo.var;
  const auto & bins   = plotInfo.bins;
  const auto & unit   = plotInfo.unit;
  const auto & xtitle = plotInfo.title+" ["+unit+"]";
  const auto title    = sample+" "+xtitle+";"+xtitle+";"+"Events/"+unit;

  // make plots
  outfile->cd();
  auto nom_hist = setupPlot(sample+"_"+name+"_nom_hist",title,bins,kBlue);
  auto unc_hist = setupPlot(sample+"_"+name+"_unc_hist",title,bins,kRed+1);
  
  // fill hists
  const TString cutwgt = "(puwgt*evtwgt)";
  nom_tree->Draw(Form("%s>>%s",var.Data(),nom_hist->GetName()),cutwgt.Data(),"goff");
  unc_tree->Draw(Form("%s>>%s",var.Data(),unc_hist->GetName()),cutwgt.Data(),"goff");

  // scale to bin widths
  Common::Scale(nom_hist,false);
  Common::Scale(unc_hist,false);

  // set min max
  auto minY = 1e9, maxY = -1e9;
  getMinMax(nom_hist,minY,maxY);
  getMinMax(unc_hist,minY,maxY);

  minY /= 1.5;
  maxY *= 1.5;

  nom_hist->GetYaxis()->SetRangeUser(minY,maxY);
  unc_hist->GetYaxis()->SetRangeUser(minY,maxY);

  // add to legend
  leg->AddEntry(nom_hist,"Nominal","epl");
  leg->AddEntry(unc_hist,syst_unc_label.Data(),"epl");

  // draw
  canv->cd();
  upad->cd();
  nom_hist->Draw("ep");
  unc_hist->Draw("ep same");
  leg->Draw("same");
 
  // make ratio
  auto ratio = (TH1F*)unc_hist->Clone(name+"_ratio");
  ratio->Divide(nom_hist);
  ratio->SetTitle("");
  ratio->GetYaxis()->SetRangeUser(0.f,2.f);

  // draw style
  ratio->GetXaxis()->SetLabelSize  (Common::LabelSize   / Common::height_lp); 
  ratio->GetXaxis()->SetLabelOffset(Common::LabelOffset / Common::height_lp); 
  ratio->GetXaxis()->SetTitleSize  (Common::TitleSize   / Common::height_lp);
  ratio->GetXaxis()->SetTickLength (Common::TickLength  / Common::height_lp);
  
  ratio->GetYaxis()->SetTitle("Unc/Nom");
  ratio->GetYaxis()->SetLabelSize  (Common::LabelSize   / Common::height_lp); 
  ratio->GetYaxis()->SetTitleSize  (Common::TitleSize   / Common::height_lp);
  ratio->GetYaxis()->SetTitleOffset(Common::TitleYOffset * Common::height_lp);
  ratio->GetYaxis()->SetNdivisions(505);

  // draw
  canv->cd();
  lpad->cd();
  ratio->Draw("ep");

  // make line (then redraw
  auto line = new TLine();
  line->SetLineColor(kRed);
  line->SetLineWidth(2);
  line->SetX1(ratio->GetXaxis()->GetXmin());
  line->SetX2(ratio->GetXaxis()->GetXmax());
  line->SetY1(1.0);
  line->SetY2(1.0);

  // draw line
  line->Draw("same");
  ratio->Draw("ep same");

  // save it
  Common::SaveAs(canv,syst_unc_name+"_"+sample+"_"+name);
  
  // also to root file
  Common::Write(outfile,nom_hist);
  Common::Write(outfile,unc_hist);
  Common::Write(outfile,ratio);
  Common::Write(outfile,canv);

  // delete it all
  delete line;
  delete ratio;
  delete unc_hist;
  delete nom_hist;
  delete leg;
  delete lpad;
  delete upad;
  delete canv;
}

TH1F * setupPlot(const TString & name, const TString & title, const std::vector<Double_t> & bins, const Color_t color)
{
  const auto binsX  = &bins[0];
  const auto nbinsX = bins.size()-1;

  auto hist = new TH1F(name.Data(),title.Data(),nbinsX,binsX); 
  hist->Sumw2();
  hist->SetLineColor(color);
  hist->SetMarkerColor(color);

  hist->SetMarkerStyle(20);
  hist->SetMarkerSize(0.6);
  
  hist->GetXaxis()->SetLabelSize(0);
  hist->GetXaxis()->SetTitleSize(0);
  hist->GetYaxis()->SetLabelSize  (Common::LabelSize / Common::height_up); 
  hist->GetYaxis()->SetTitleSize  (Common::TitleSize / Common::height_up);
  hist->GetYaxis()->SetTitleOffset(Common::TitleYOffset * Common::height_up);

  return hist;
}

void getMinMax(const TH1F * hist, Double_t & minY, Double_t & maxY)
{
  for (auto ibinX = 1; ibinX <= hist->GetXaxis()->GetNbins(); ibinX++)
  {
    const auto content = hist->GetBinContent(ibinX);
    if (content > maxY && content > 0.0) maxY = content;
    if (content < minY && content > 0.0) minY = content;
  }
}
