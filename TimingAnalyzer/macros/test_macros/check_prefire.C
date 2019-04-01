#include "Common.cpp+"

void setupCanvas(TCanvas *& canv, TPad *& upad, TPad *& lpad, const Bool_t isLogx);
void setupHist(TH1F * hist, const Color_t color, Double_t & minY, Double_t & maxY, const Bool_t isLogx);
void setMinMax(TH1F * hist, const Double_t minY, const Double_t maxY);
void setupRatio(TH1F * ratio);
void setupLine(TLine * line, const TH1F * ratio);

void check_prefire()
{
  gStyle->SetOptStat(0);

  // var to test
  const TString var = "met"; // "time"
  const Bool_t isLogx = true;

  // set up samples
  std::vector<TString> ctaus   = {"0p1","200","1000"};
  std::vector<TString> lambdas = {"100","200","300","400"};
  std::vector<TString> samples;
  for (const auto & lambda : lambdas) 
    for (const auto & ctau : ctaus)
      samples.emplace_back("GMSB_L"+lambda+"_CTau"+ctau);

  // get files
  const TString std_file_name = var+"_std.root";
  auto std_file = TFile::Open(std_file_name.Data());
  Common::CheckValidFile(std_file,std_file_name);

  const TString mod_file_name = var+"_mod.root";
  auto mod_file = TFile::Open(mod_file_name.Data());
  Common::CheckValidFile(mod_file,mod_file_name);

  // make plots
  for (const auto & sample : samples)
  {
    std::cout << "Working on: " << sample.Data() << std::endl;

    // make canvas
    TCanvas * canv = NULL;
    TPad * upad = NULL, * lpad = NULL;
    setupCanvas(canv,upad,lpad,isLogx);

    // setup hists
    const TString hist_name = sample+"_Hist_Plotted";
    Double_t minY = 1e9, maxY = -1e9;

    auto std_hist = (TH1F*)std_file->Get(hist_name.Data());
    Common::CheckValidHist(std_hist,hist_name,std_file_name);
    setupHist(std_hist,kBlue,minY,maxY,isLogx);

    auto mod_hist = (TH1F*)mod_file->Get(hist_name.Data());
    Common::CheckValidHist(mod_hist,hist_name,mod_file_name);
    setupHist(mod_hist,kRed+1,minY,maxY,isLogx);

    // set min max
    setMinMax(std_hist,minY,maxY);
    setMinMax(mod_hist,minY,maxY);

    // add to legend
    auto leg = new TLegend(0.85,0.75,1.0,1.0);
    leg->AddEntry(std_hist,"Standard Sel.","epl");
    leg->AddEntry(mod_hist,"Prefire Test","epl");

    // draw upper pad
    canv->cd();
    upad->cd();
    std_hist->Draw("ep");
    mod_hist->Draw("same ep");
    leg->Draw("same");
    
    // make ratio bottom
    auto ratio = (TH1F*)mod_hist->Clone(hist_name+"_ratio");
    ratio->Divide(std_hist);
    setupRatio(ratio);

    // make line
    auto line = new TLine();
    setupLine(line,ratio);

    // draw lower pad
    canv->cd();
    lpad->cd();
    ratio->Draw("ep");
    line->Draw();
    ratio->Draw("ep same");

    // save canvas
    Common::SaveAs(canv,var+"_"+sample);

    // delete it all
    delete line;
    delete ratio;
    delete leg;
    delete mod_hist;
    delete std_hist;
    delete lpad;
    delete upad;
    delete canv;
  }

  // delete the rest
  delete mod_file;
  delete std_file;
}

void setupCanvas(TCanvas *& canv, TPad *& upad, TPad *& lpad, const Bool_t isLogx)
{
  canv = new TCanvas();
  canv->cd();

  upad = new TPad("upad","",Common::left_up,Common::bottom_up,Common::right_up,Common::top_up);
  upad->SetBottomMargin(Common::merged_margin);
  upad->Draw();
  upad->SetTickx();
  upad->SetTicky();
  upad->SetLogy();
  upad->SetLogx(isLogx);  

  lpad = new TPad("lpad","",Common::left_lp,Common::bottom_lp,Common::right_lp,Common::top_lp);
  lpad->SetTopMargin(Common::merged_margin);
  lpad->SetBottomMargin(Common::bottom_margin);
  lpad->Draw();
  lpad->SetTickx();
  lpad->SetTicky();
  lpad->SetLogx(isLogx);
}

void setupHist(TH1F * hist, const Color_t color, Double_t & minY, Double_t & maxY, const Bool_t isLogx)
{
  hist->SetLineColor(color);
  hist->SetMarkerColor(color);
  hist->Scale(1.f/hist->Integral());
    
  if (isLogx) hist->GetXaxis()->SetRangeUser(10.f,hist->GetXaxis()->GetBinUpEdge(hist->GetXaxis()->GetNbins()));

  hist->SetMarkerStyle(20);
  hist->SetMarkerSize(0.6);
  hist->SetLineColor(color);
  hist->SetMarkerColor(color);
  
  hist->GetXaxis()->SetLabelSize(0);
  hist->GetXaxis()->SetTitleSize(0);
  hist->GetYaxis()->SetLabelSize  (Common::LabelSize / Common::height_up); 
  hist->GetYaxis()->SetTitleSize  (Common::TitleSize / Common::height_up);
  hist->GetYaxis()->SetTitleOffset(Common::TitleYOffset * Common::height_up);

  for (auto ibin = 1; ibin <= hist->GetXaxis()->GetNbins(); ibin++)
  {
    const auto content = hist->GetBinContent(ibin);
    if (content > maxY && content > 0.f) maxY = content;
    if (content < minY && content > 0.f) minY = content;
  }
}

void setMinMax(TH1F * hist, const Double_t minY, const Double_t maxY)
{
  hist->GetYaxis()->SetRangeUser(minY/1.5f,maxY*1.5f);
}

void setupRatio(TH1F * ratio)
{
  ratio->SetLineColor(kBlack);
  ratio->SetMarkerColor(kBlack);

  ratio->SetTitle("");
  ratio->GetYaxis()->SetTitle("Test/Std.");
  ratio->GetYaxis()->SetRangeUser(0.9,1.1);
  
  ratio->GetXaxis()->SetLabelSize  (Common::LabelSize   / Common::height_lp); 
  ratio->GetXaxis()->SetLabelOffset(Common::LabelOffset / Common::height_lp); 
  ratio->GetXaxis()->SetTitleSize  (Common::TitleSize   / Common::height_lp);
  ratio->GetXaxis()->SetTickLength (Common::TickLength  / Common::height_lp);
  
  ratio->GetYaxis()->SetLabelSize  (Common::LabelSize   / Common::height_lp); 
  ratio->GetYaxis()->SetTitleSize  (Common::TitleSize   / Common::height_lp);
  ratio->GetYaxis()->SetTitleOffset(Common::TitleYOffset * Common::height_lp);
  ratio->GetYaxis()->SetNdivisions(505);
}

void setupLine(TLine * line, const TH1F * ratio)
{
  line->SetLineColor(kRed);
  line->SetLineWidth(2);
  line->SetX1(ratio->GetXaxis()->GetXmin());
  line->SetX2(ratio->GetXaxis()->GetXmax());
  line->SetY1(1.0);
  line->SetY2(1.0);
}
