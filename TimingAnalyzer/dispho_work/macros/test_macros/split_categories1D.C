#include "Common.cpp+"

// static const TString xvar     = "phoseedtime_0";
// static const TString xtitle   = "Seed Time [ns]";
// static const Int_t   xnbins   = 40;
// static const Float_t xlow     = -5.f;
// static const Float_t xhigh    = 5.f;
// static const TString cutvar   = "t1pfMETpt";
// static const TString cutlabel = "MET";
// std::vector<Float_t> cutbins  = {0.f,200.f,1000.f};

static const TString xvar     = "t1pfMETpt";
static const TString xtitle   = "MET [GeV]";
static const Int_t   xnbins   = 40;
static const Float_t xlow     = 0.f;
static const Float_t xhigh    = 1000.f;
static const TString cutvar   = "phoseedtime_0";
static const TString cutlabel = "Time";
std::vector<Float_t> cutbins  = {-2.f,1.f,3.f,25.f};

static const TString filename  = "skims/v1/orig_2phosCR/qcd.root";
static const TString selection = "1";
static const TString label     = "QCDCR_Data_MET_TimeSplit";

// static const TString filename  = "skims/v1/zee/skim.root";
// static const TString selection = "phoisEB_0&&phopt_0>40&&phoisEB_1&&phopt_1>40&&hltDiEle33MW";
// static const TString label     = "DYCR_Data_MET_TimeSplits";

// static const TString filename  = "skims/v1/orig_2phosCR/gjets.root";
// static const TString selection = "1";
// static const TString label     = "GJetsCR_Data_MET_TimeSplit";

static const TString outdir = "/eos/user/k/kmcdermo/www/dispho/plots/madv2_v1/test/adish_abcd_v2";

void split_categories1D()
{
  // set style
  auto tdrStyle = new TStyle("TDRStyle","Style for P-TDR");
  Common::SetTDRStyle(tdrStyle);
  Common::SetupEras();

  // get inputs
  auto file = TFile::Open(filename.Data());
  Common::CheckValidFile(file,filename);
  
  const TString treename = "Data_Tree";
  auto tree = (TTree*)file->Get(treename.Data());
  Common::CheckValidTree(tree,treename,filename);

  // tmp min/max
  Float_t max = -1e9;
  Float_t min =  1e9;

  // setup and fill hists
  const auto nHists = cutbins.size()-1;
  std::vector<TH1F*> hists(nHists);
  for (auto i = 0U; i < hists.size(); i++)
  {
    auto & hist = hists[i];
    hist = new TH1F(Form("hist_%i",i),Form(";%s;Events",xtitle.Data()),xnbins,xlow,xhigh);
    hist->Sumw2();
    hist->SetLineColor  (Common::ColorVec[i]);
    hist->SetMarkerColor(Common::ColorVec[i]);

    // fill hist
    std::cout << "Filling hist in bin: " << (i+1) << " of " << nHists << std::endl;
    tree->Draw(Form("%s>>%s",xvar.Data(),hist->GetName()),
	       Form("(%s) && (%s>=%f && %s<%f)",selection.Data(),cutvar.Data(),cutbins[i],cutvar.Data(),cutbins[i+1]),
	       "goff");

    hist->Scale(1.f/hist->Integral());

    for (auto ibinX = 1; ibinX <= hist->GetXaxis()->GetNbins(); ibinX++)
    {
      const auto content = hist->GetBinContent(ibinX);
      
      if (content > max) max = content;
      if (content < min && content > 0.f) min = content;
    }
  }

  // set min/max
  for (auto & hist : hists)
  {
    hist->SetMinimum(min/1.5f);
    hist->SetMaximum(max*1.5f);
  }

  // make canv
  auto canv = new TCanvas();
  canv->cd();

  // draw upper pad
  std::cout << "Drawing upper pad..." << std::endl;
  auto upad = new TPad("upad","", Common::left_up, Common::bottom_up, Common::right_up, Common::top_up);
  upad->SetBottomMargin(Common::merged_margin);

  canv->cd();
  upad->Draw();
  upad->cd();
  upad->SetLogy(1);

  for (auto i = 0U; i < hists.size(); i++)
  {
    auto & hist = hists[i];

    hist->GetXaxis()->SetLabelSize(0);
    hist->GetXaxis()->SetTitleSize(0);
    
    hist->GetYaxis()->SetLabelSize  (Common::LabelSize / Common::height_up); 
    hist->GetYaxis()->SetTitleSize  (Common::TitleSize / Common::height_up);
    hist->GetYaxis()->SetTitleOffset(Common::TitleFF * Common::TitleYOffset * Common::height_up);

    hist->Draw(i>0?"ep same":"ep");
  }

  // make legend
  auto leg = new TLegend(0.6,0.7,0.82,0.93);
  for (auto i = 0U; i < hists.size(); i++)
  {
    auto & hist = hists[i];
    leg->AddEntry(hist,Form("Bin: %i (%4.0f<%s#leq%4.0f)",i,cutbins[i],cutlabel.Data(),cutbins[i+1]),"epl");
  }  
  leg->Draw("same");

  // draw lower pad
  std::cout << "Drawing lower pad..." << std::endl;
  auto lpad = new TPad("lpad", "", Common::left_lp, Common::bottom_lp, Common::right_lp, Common::top_lp);
  lpad->SetTopMargin(Common::merged_margin);
  lpad->SetBottomMargin(Common::bottom_margin);

  canv->cd();
  lpad->Draw();
  lpad->cd(); 

  // ratio line
  auto line = new TLine();
  line->SetLineColor(kRed);
  line->SetLineWidth(2);

  // set line params
  line->SetX1(xlow);
  line->SetX2(xhigh);
  line->SetY1(1.0);
  line->SetY2(1.0);

  // make ratio plots
  const auto nRatios = hists.size()-1;
  std::vector<TH1F*> ratios(nRatios);
  for (auto j = 0U; j < ratios.size(); j++)
  {
    std::cout << "Making ratio for bin: " << (j+1) << " of " << nRatios << std::endl;
    const auto & hist = hists[j+1];

    auto & ratio = ratios[j];
    ratio = (TH1F*)hist->Clone(Form("%s_ratio",hist->GetName()));
    ratio->SetLineColor(hist->GetLineColor());
    ratio->SetMarkerColor(hist->GetMarkerColor());

    ratio->Divide(hists[0]);

    ratio->GetYaxis()->SetTitle("Bin N / 0");
    ratio->SetMinimum(-0.1);
    ratio->SetMaximum( 2.1);
    ratio->SetStats(0);     

    ratio->GetYaxis()->SetNdivisions(505);
    ratio->GetXaxis()->SetLabelSize  (Common::LabelSize   / Common::height_lp); 
    ratio->GetXaxis()->SetLabelOffset(Common::LabelOffset / Common::height_lp); 
    ratio->GetXaxis()->SetTitleSize  (Common::TitleSize   / Common::height_lp);
    ratio->GetXaxis()->SetTickLength (Common::TickLength  / Common::height_lp);
    ratio->GetYaxis()->SetLabelSize  (Common::LabelSize   / Common::height_lp); 
    ratio->GetYaxis()->SetTitleSize  (Common::TitleSize   / Common::height_lp);
    ratio->GetYaxis()->SetTitleOffset(Common::TitleFF * Common::TitleYOffset * Common::height_lp);
  }

  // first draw line so it appears
  ratios[0]->Draw("ep");
  line->Draw("same");

  // now draw ratios again
  for (auto j = 0U; j < ratios.size(); j++)
  {
    auto & ratio = ratios[j];
    ratio->Draw("ep same");
  }

  // save it
  std::cout << "Saving..." << std::endl;
  canv->cd();
  Common::CMSLumi(canv,0,"Full");
  Common::SaveAs(canv,label);

  // move it to www
  gSystem->Exec("mv "+label+".png "+outdir);
  gSystem->Exec("mv "+label+".pdf "+outdir);

  // delete it all
  std::cout << "Deleting..." << std::endl;
  for (auto & ratio : ratios) delete ratio;
  delete line;
  delete lpad;
  delete leg;
  delete upad;
  delete canv;
  for (auto & hist : hists) delete hist;
  delete tree;
  delete file;
  delete tdrStyle;
}
