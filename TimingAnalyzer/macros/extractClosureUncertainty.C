#include "Common.cpp+"

struct BinRange
{
  BinRange() {}
  BinRange(const Double_t low, const Double_t up, const Color_t color)
    : low(low), up(up), color(color) {}

  Double_t low;
  Double_t up;
  Color_t color;
};

// subroutines
void makeClosureDump(const TH2F * hist2D, const Double_t time_split, const Double_t met_split,
		     const TString & label, std::ofstream & outtextfile);
void make1DSlices(TFile * iofile, const TH2F * hist2D, const TString & label, const TString & outfiletext);

// main method
void extractClosureUncertainty(const TString & label, const TString & outfiletext)
{
  // set style
  auto tdrStyle = new TStyle();
  Common::SetTDRStyle(tdrStyle);
  Common::SetupEras();
  Common::SetupSamples();
  Common::SetupGroups();
  Common::SetupHistNames();

  // i/o root file
  const auto iofilename = outfiletext+"_"+label+".root";
  auto iofile = TFile::Open(iofilename.Data(),"UPDATE");
  Common::CheckValidFile(iofile,iofilename);

  // need to use constant fine binning of MET vs time, otherwise comment out Rebin
  const TString hist2Dname = Common::HistNameMap["Data"]+"_Plotted";
  auto hist2D = (TH2F*)iofile->Get(hist2Dname.Data());
  Common::CheckValidHist(hist2D,hist2Dname,iofilename);

  // get text file
  std::ofstream outtextfile(outfiletext+"."+Common::outTextExt,std::ios_base::app);

  // dump input
  const auto covariance = hist2D->GetCovariance();
  const auto corrfactor = hist2D->GetCorrelationFactor();
  const auto integral   = hist2D->Integral();
  const auto corrfactor_unc = (1.0-corrfactor*corrfactor)/std::sqrt(integral);

  outtextfile << label.Data() << ","
	      << "integral: " << integral << ","
	      << "covar: " << covariance << ","
    	      << "corr. factor: " << corrfactor << ","
    	      << "corr. factor unc.: " << corrfactor_unc << ","
	      << std::endl;

  // make dumps
  std::vector<Double_t> time_splits = {0.0,0.5,1.0,1.5,2.0};
  std::vector<Double_t> met_splits  = {50,100,150,200,300,500};
  for (const auto time_split : time_splits)
    for (const auto met_split : met_splits)
      makeClosureDump(hist2D,time_split,met_split,label,outtextfile);

  // make1D time slice plots
  make1DSlices(iofile,hist2D,label,outfiletext);

  // delete it all 
  delete hist2D;
  delete iofile;
  delete tdrStyle;
}

void makeClosureDump(const TH2F * hist2D, const Double_t time_split, const Double_t met_split,
		     const TString & label, std::ofstream & outtextfile)
{
  // get bin assignments
  const auto ibinX = hist2D->GetXaxis()->FindBin(time_split);
  const auto ibinY = hist2D->GetYaxis()->FindBin(met_split);

  const auto nbinsX = hist2D->GetXaxis()->GetNbins();
  const auto nbinsY = hist2D->GetYaxis()->GetNbins();

  //////////
  // data //
  //////////

  auto obsAunc = 0.0;
  const auto obsA = hist2D->IntegralAndError(1,ibinX-1,1,ibinY-1,obsAunc);

  auto obsBunc = 0.0;
  const auto obsB = hist2D->IntegralAndError(1,ibinX-1,ibinY,nbinsY,obsBunc);

  auto obsCunc = 0.0;
  const auto obsC = hist2D->IntegralAndError(ibinX,nbinsX,ibinY,nbinsY,obsCunc);

  auto obsDunc = 0.0;
  const auto obsD = hist2D->IntegralAndError(ibinX,nbinsX,1,ibinY-1,obsDunc);

  ////////////
  // params //
  ////////////

  const auto BovA    = obsB/obsA;
  const auto BovAunc = BovA*std::sqrt(std::pow(obsBunc/obsB,2)+std::pow(obsAunc/obsA,2));

  const auto DovA    = obsD/obsA;
  const auto DovAunc = DovA*std::sqrt(std::pow(obsDunc/obsD,2)+std::pow(obsAunc/obsA,2));

  const auto predC    = obsB*obsD/obsA;
  const auto predCunc = predC*std::sqrt(std::pow(obsBunc/obsB,2)+std::pow(obsDunc/obsD,2)+std::pow(obsAunc/obsA,2));

  ///////////
  // diffs //
  ///////////

  const auto percent_diffC    = (1.f-(predC/obsC))*100.f;
  const auto percent_diffCunc = percent_diffC*Common::hypot(obsCunc/obsC,predCunc/predC);
  const auto pullC = (obsC-predC)/std::sqrt(std::pow(obsCunc,2)+std::pow(predCunc,2));

  // fill output file
  std::cout << "Filling text file..." << std::endl;

  outtextfile << label.Data() << "," << time_split << "," << met_split << ","
	      << std::setprecision(3) << obsA << " +/- " << std::setprecision(3) << obsAunc << ","
	      << std::setprecision(3) << obsB << " +/- " << std::setprecision(3) << obsBunc << ","
	      << std::setprecision(3) << obsC << " +/- " << std::setprecision(3) << obsCunc << ","
	      << std::setprecision(3) << obsD << " +/- " << std::setprecision(3) << obsDunc << ","
	      << std::setprecision(3) << BovA << " +/- " << std::setprecision(3) << BovAunc << ","
	      << std::setprecision(3) << DovA << " +/- " << std::setprecision(3) << DovAunc << ","
	      << std::setprecision(3) << predC << " +/- " << std::setprecision(3) << predCunc << ","
	      << std::setprecision(3) << percent_diffC << " +/- " << std::setprecision(3) << percent_diffCunc << ","
	      << std::setprecision(3) << pullC << "," << std::setprecision(3)
	      << std::endl;
}

void make1DSlices(TFile * iofile, const TH2F * hist2D, const TString & label, const TString & outfiletext)
{
  // outname
  const auto outname = outfiletext+"_"+label;

  // met ranges
  auto icolor = 0;
  const std::vector<BinRange> ranges = 
  { 
    {0.0  , 50.0  , Common::ColorVec[icolor++]},
    {50.0 , 100.0 , Common::ColorVec[icolor++]},
    {100.0, 200.0 , Common::ColorVec[icolor++]},
    {200.0, 300.0 , Common::ColorVec[icolor++]},
    {300.0, 500.0 , Common::ColorVec[icolor++]},
    {500.0, 3000.0, Common::ColorVec[icolor++]}
  };
  const auto size = ranges.size();
  const auto half = size/2;
  
  // met slices
  iofile->cd();
  std::vector<TH1D*> hist1Ds(size);

  // log only output
  auto canv = new TCanvas();
  canv->SetName("SliceCanv");
  canv->cd();
  canv->SetLogy();

  // legend
  auto leg = new TLegend(0.55,0.6,0.825,0.92);
  leg->SetName("SliceLeg");
  leg->SetNColumns(2);
  
  // project time hists, set style, scale to unity
  for (auto i = 0U; i < size; i++)
  {
    const auto & range = ranges[i];
    auto & hist1D = hist1Ds[i];

    // set bins for projection
    const auto low = hist2D->GetYaxis()->FindBin(range.low);
    const auto bin_up = hist2D->GetYaxis()->FindBin(range.up);
    const auto up = ( (i == (size-1)) ? (bin_up) : (bin_up-1) );

    // project time hist
    iofile->cd();
    hist1D = hist2D->ProjectionX(Form("%s_MET_Proj_%i_%i",hist2D->GetName(),low,up),low,up);

    // set style
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
      if (content > max && content > 0.0) max = content;
      if (content < min && content > 0.0) min = content;
    }
  }

  // set min/max and write
  for (auto i = 0U; i < size; i++)
  {
    const auto & range = ranges[i];
    auto & hist1D = hist1Ds[i];

    hist1D->SetMinimum(min/1.5f);
    hist1D->SetMaximum(max*1.5f);

    // save it
    Common::Write(iofile,hist1D);
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

  // write legend
  Common::Write(iofile,leg);

  // draw all hists + legend
  for (auto i = 0U; i < hist1Ds.size(); i++)
  {
    auto & hist1D = hist1Ds[i];
    hist1D->Draw(i>0?"ep same":"ep");
  }
  leg->Draw("same");

  // make the canvas pretty, save it
  Common::CMSLumi(canv,0,"Full");
  Common::SaveAs(canv,outname);

  // delete things
  for (auto & hist1D : hist1Ds) delete hist1D;
  delete leg;
  delete canv;
}
