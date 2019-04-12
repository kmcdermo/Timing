#include "Common.cpp+"

struct PlotInfo
{
  PlotInfo() {}
  PlotInfo(const TString & name, const TString & title, 
	   const TString & corr, const TString & uncorr, const TString & gen,
	   const Int_t nbinsX, const Float_t lowX, const Float_t highX) 
    : name(name), title(title), corr(corr), uncorr(uncorr), gen(gen),
      nbinsX(nbinsX), lowX(lowX), highX(highX) {}
  
  TString name;
  TString title;
  TString corr;
  TString uncorr;
  TString gen;
  Int_t nbinsX;
  Float_t lowX;
  Float_t highX;
};

void makeHist(TH1F *& hist, const PlotInfo & plotInfo, const TString & name, const Color_t color);
void getMinMax(const TH1F * hist, Double_t & min, Double_t & max);
void setMinMax(TH1F * hist, const Double_t min, const Double_t max);

void compareMETresiduals(const TString & indir, const TString & outfiletext)
{
  gStyle->SetOptStat("nemMrR");

  // output file
  auto outfile = TFile::Open(outfiletext+".root","UPDATE");
  
  // input samples
  const std::vector<TString> samples = {"L200_CTau10", "L200_CTau200", "L200_CTau1000", "L400_CTau10", "L400_CTau200", "L400_CTau1000"};

  // different plots
  std::vector<PlotInfo> plotInfos =
  {
    {"pt","p_{T}^{miss} Residual [GeV];p_{T}^{miss (GEN)}-p_{T}^{miss (Reco)} [GeV];Fraction of Events","t1pfMETpt","t1pfMETptUncorr","genMETpt",50,-1000,1000},
    {"phi","#phi^{miss} Residual [GeV];#phi^{miss (GEN)}-#phi^{miss (Reco)} [GeV];Fraction of Events","t1pfMETphi","t1pfMETphiUncorr","genMETphi",50,-3.2,3.2}
  };

  // loop over samples
  for (const auto & sample : samples)
  {
    // get file
    const auto infilename = indir+"/"+sample+".root";
    auto infile = TFile::Open(infilename.Data());
    Common::CheckValidFile(infile,infilename);

    // get tree
    const TString treename = "tree/disphotree";
    auto tree = (TTree*)infile->Get(treename.Data());
    Common::CheckValidTree(tree,treename,infilename);

    // loop over all plotInfos
    for (const auto & plotInfo : plotInfos)
    {
      // label
      const auto label = plotInfo.name+"_"+sample;
      std::cout << "Working on plot: " << label.Data() << std::endl;

      // setup hists
      outfile->cd();
      TH1F * before_hist = NULL, * after_hist = NULL;
      makeHist(before_hist,plotInfo,"Uncorrected",kRed+1);
      makeHist(after_hist ,plotInfo,"Corrected"  ,kBlue);

      // read trees
      tree->Draw(Form("(%s-%s)>>%s",plotInfo.gen.Data(),plotInfo.uncorr.Data(),before_hist->GetName()),"(phoisOOT_0==1||phoisOOT_1==1||phoisOOT_2==1||phoisOOT_3==1)","goff");
      tree->Draw(Form("(%s-%s)>>%s",plotInfo.gen.Data(),plotInfo.corr  .Data(),after_hist ->GetName()),"(phoisOOT_0==1||phoisOOT_1==1||phoisOOT_2==1||phoisOOT_3==1)","goff");
    
      // scale
      before_hist->Scale(1.f/before_hist->Integral());
      after_hist ->Scale(1.f/after_hist ->Integral());
      
      // get max / set max
      auto min = 0.0, max = -1.0e9;
      getMinMax(before_hist,min,max);
      getMinMax(after_hist ,min,max);

      setMinMax(before_hist,min,max);
      setMinMax(after_hist ,min,max);

      // legend
      auto leg = new TLegend(0.14,0.69,0.37,0.86);
      leg->SetName("leg_"+label);
      leg->AddEntry(before_hist,"Uncorrected","epl");
      leg->AddEntry(after_hist ,"Corrected","epl");
      
      // canvas
      auto canv = new TCanvas();
      canv->SetName("canv_"+label);
      canv->cd();
      canv->SetTickx();
      canv->SetTicky();

      // draw
      before_hist->Draw("ep");
      after_hist ->Draw("ep sames");
      leg->Draw("same");
      canv->Update();

      // set stats
      auto before_stats = (TPaveStats*)(before_hist->GetListOfFunctions()->FindObject("stats"));
      auto after_stats  = (TPaveStats*)(after_hist ->GetListOfFunctions()->FindObject("stats"));

      before_stats->SetX1NDC(0.65);
      before_stats->SetX2NDC(0.86);
      after_stats->SetX1NDC(before_stats->GetX1NDC());
      after_stats->SetX2NDC(before_stats->GetX2NDC());
      
      before_stats->SetY1NDC(0.65); before_stats->SetY2NDC(0.86);
      const auto height = before_stats->GetY2NDC() - before_stats->GetY1NDC();
      const auto h_gap = 0.02;
      after_stats->SetY1NDC(before_stats->GetY1NDC() - height - h_gap);
      after_stats->SetY2NDC(before_stats->GetY1NDC() - h_gap);

      // Save Canvas
      Common::SaveAs(canv,outfiletext+"_"+label);
      
      // Rename hists (named the same for stats boxes
      before_hist->SetName("before_"+label);
      after_hist ->SetName("after_" +label);
      
      // write to file
      Common::Write(outfile,before_hist);
      Common::Write(outfile,after_hist);
      Common::Write(outfile,leg);
      Common::Write(outfile,canv);
    
      // delete it all
      delete after_stats;
      delete before_stats;
      delete canv;
      delete leg;
      delete after_hist;
      delete before_hist;
    }

    // delete it all 
    delete tree;
    delete infile;
  }

  // delete it all
  delete outfile;
}

void makeHist(TH1F *& hist, const PlotInfo & plotInfo, const TString & name, const Color_t color)
{
  hist = new TH1F(name.Data(),plotInfo.title.Data(),plotInfo.nbinsX,plotInfo.lowX,plotInfo.highX);
  hist->Sumw2();
  hist->SetLineColor(color);
  hist->SetMarkerColor(color);
}

void getMinMax(const TH1F * hist, Double_t & min, Double_t & max)
{
  for (auto ibin = 1; ibin <= hist->GetXaxis()->GetNbins(); ibin++)
  {
    const auto content = hist->GetBinContent(ibin);
    
    //    if (content < min && content > 0.0) min = content;
    if (content > max && content > 0.0) max = content;
  }
}

void setMinMax(TH1F * hist, const Double_t min, const Double_t max)
{
  hist->GetYaxis()->SetRangeUser(min/1.2f,max*1.2f);
}
