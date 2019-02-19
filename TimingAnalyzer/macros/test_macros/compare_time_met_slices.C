#include "Common.cpp+"

struct Range
{
  Range() {}
  Range(const Double_t low, const Double_t up, const Int_t color)
    : low(low), up(up), color(color) {}

  Double_t low;
  Double_t up;
  Int_t color;
};

void compare_time_met_slices(const TString & infilename, const TString & outfiletext)
{
  // set style
  auto tdrStyle = new TStyle();
  Common::SetTDRStyle(tdrStyle);
  Common::SetupEras();
  Common::SetupSamples();
  Common::SetupGroup();
  Common::SetupHistNames();

  // inputs
  auto infile = TFile::Open(infilename.Data());
  Common::CheckValidFile(infile,infilename);
  infile->cd();

  // need to use constant fine binning of MET vs time, otherwise comment out Rebin
  const TString hist2Dname = Common::HistNameMap["Data"]+"_Plotted";
  auto hist2D = (TH2F*)infile->Get(hist2Dname.Data());
  Common::CheckValidHist(hist2D,hist2Dname,infilename);

  // dump input
  std::cout << "corr. factor: " << hist2D->GetCorrelationFactor() << " covar: " << hist2D->GetCovariance() << std::endl;

  // met ranges
  std::vector<Range> ranges = 
  { 
    {0.0  , 50.0  , 417}, 
    {50.0 , 100.0 , 600}, 
    {100.0, 200.0 , 633},
    {200.0, 300.0 , 880},
    {300.0, 500.0 , 810},
    {500.0, 3000.0, 1} 
  };
  const auto size = ranges.size();
  const auto half = size/2;
  
  // output
  auto outfile = TFile::Open(outfiletext+".root","RECREATE");
  outfile->cd();
  hist2D->Write(hist2D->GetName(),TObject::kWriteDelete);

  std::vector<TH1D*> hist1Ds(size);

  // log only output
  auto canv = new TCanvas();
  canv->cd();
  canv->SetLogy();
  canv->SetTickx();
  canv->SetTicky();

  auto leg = new TLegend(0.6,0.6,1.0,1.0);
  leg->SetNColumns(2);
  
  // project time hists, set style, scale to unity
  for (auto i = 0U; i < size; i++)
  {
    const auto & range = ranges[i];
    auto & hist1D = hist1Ds[i];

    const auto low = hist2D->GetYaxis()->FindBin(range.low);
    const auto bin_up = hist2D->GetYaxis()->FindBin(range.up);
    const auto up = ( (i == (size-1)) ? (bin_up) : (bin_up-1) );

    hist1D = hist2D->ProjectionX(Form("%i_%i",low,up),low,up);
    hist1D->SetLineColor(range.color);
    hist1D->SetMarkerColor(range.color);
    hist1D->SetMarkerStyle(1);
    hist1D->SetTitle("");
    hist1D->GetYaxis()->SetTitle("Fraction of Events");
    hist1D->Rebin(10);
    hist1D->Scale(1.0/hist1D->Integral());
  }

  // get min and max of plot
  auto min = 1.0e9;
  auto max = 1.0e-9;
  for (auto i = 0U; i < size; i++)
  {
    auto & hist1D = hist1Ds[i];
    
    for (auto j = 1; j <= hist1D->GetXaxis()->GetNbins(); j++)
    {
      const auto content = hist1D->GetBinContent(j);
      if (content > max) max = content;
      if (content < min && content != 0.0) min = content;
    }
  }

  // set min/max and draw
  for (auto i = 0U; i < size; i++)
  {
    const auto & range = ranges[i];
    auto & hist1D = hist1Ds[i];

    hist1D->SetMinimum(min/1.5);
    hist1D->SetMaximum(max*1.5);

    hist1D->Draw(i>0?"ep same":"ep");

    // save it
    outfile->cd();
    hist1D->Write(hist1D->GetName(),TObject::kWriteDelete);
  }

  // order columns in legend nicely
  for (auto i = 0U; i < half; i++)
  {
    const auto & range_L = ranges[i];
    auto & hist1D_L = hist1Ds[i];

    const auto & range_R = ranges[i+half];
    auto & hist1D_R = hist1Ds[i+half];
  
    leg->AddEntry(hist1D_L,Form("%4.0f #leq p_{T}^{miss} %s %4.0f",range_L.low,( ((i)      == (size-1)) ? "#leq" : "<" ),range_L.up),"epl");
    leg->AddEntry(hist1D_R,Form("%4.0f #leq p_{T}^{miss} %s %4.0f",range_R.low,( ((i+half) == (size-1)) ? "#leq" : "<" ),range_R.up),"epl");
  }

  // draw legend
  leg->Draw("same");
  outfile->cd();
  leg->Write(leg->GetName(),TObject::kWriteDelete);

  // save it all
  Common::CMSLumi(canv,0,"Full");
  Common::SaveAs(canv,outfiletext);

  outfile->cd();
  canv->Write(canv->GetName(),TObject::kWriteDelete);

  // delete things
  for (auto & hist1D : hist1Ds) delete hist1D;
  delete leg;
  delete canv;
  delete outfile;
  delete hist2D;
  delete infile;
  delete tdrStyle;
}
