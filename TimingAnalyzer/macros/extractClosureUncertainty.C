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

struct ValPair
{
  ValPair() {}
  ValPair(const Double_t val, const Double_t unc)
    : val(val), unc(unc) {}

  Double_t val;
  Double_t unc;
};

// subroutines
inline Double_t GetRatioUnc(const Double_t ratio, const std::vector<ValPair> & valPairs)
{
  Double_t unc = 0;
  for (const auto & valPair : valPairs)
  {
    unc += std::pow(valPair.unc/valPair.val,2);
  }
  return (ratio*std::sqrt(unc));
}
inline void GetMinMaxXY(const TGraphErrors * graph, Double_t & minX, Double_t & maxX, Double_t & minY, Double_t & maxY)
{
  Double_t x,y;
  for (auto i = 0; i < graph->GetN(); i++)
  {
    graph->GetPoint(i,x,y);

    if (x < minX) minX = x;
    if (x > maxX) maxX = x;

    if ((y == y) && (y < minY) && (y > 0.0)) minY = y;
    if ((y == y) && (y > maxY) && (y > 0.0)) maxY = y;
  }
}
void makeClosureDump(const TH2F * hist2D, const TString & time_split, const TString & met_split,
		     const TString & label, std::ofstream & outtextfile,
		     std::map<TString,TGraphErrors*> & BovA_graphs, std::map<TString,TGraphErrors*> & CovD_graphs,
		     std::map<TString,TGraphErrors*> & DovA_graphs, std::map<TString,TGraphErrors*> & CovB_graphs);
void makeGraphCanvas(TFile * iofile, TGraphErrors * A_graph, const TString & A_label, TGraphErrors * C_graph, const TString & C_label,
		     const TString & x_title, const TString & split, const TString & y_prefix, const TString & y_suffix, 
		     const TString & splittext, const TString & label, const TString & outfiletext);
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

  // make map of ratios
  iofile->cd();
  std::map<TString,TGraphErrors*> BovA_graphs;
  std::map<TString,TGraphErrors*> CovD_graphs;
  std::map<TString,TGraphErrors*> DovA_graphs;
  std::map<TString,TGraphErrors*> CovB_graphs;

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

  // make splits
  std::vector<TString> time_splits = {"0p0","0p25","0p5","0p75","1p0","1p25","1p5","1p75","2p0"};
  std::vector<TString> met_splits  = {"50","75","100","125","150","175","200","225","250","275","300"};

  // init graphs (B/A, C/D)
  iofile->cd();
  for (const auto & met_split : met_splits)
  {
    auto & BovA_graph = BovA_graphs[met_split];
    BovA_graph = new TGraphErrors();
    BovA_graph->SetName("BovA_MET"+met_split);
    BovA_graph->SetLineColor  (kRed+1);
    BovA_graph->SetMarkerColor(kRed+1);

    auto & CovD_graph = CovD_graphs[met_split];
    CovD_graph = new TGraphErrors();
    CovD_graph->SetName("CovD_MET"+met_split);
    CovD_graph->SetLineColor  (kBlue);
    CovD_graph->SetMarkerColor(kBlue);
  }

  // init graphs (D/A, C/B)
  iofile->cd();
  for (const auto & time_split : time_splits)
  {
    auto & DovA_graph = DovA_graphs[time_split];
    DovA_graph = new TGraphErrors();
    DovA_graph->SetName("DovA_Time"+time_split);
    DovA_graph->SetLineColor  (kRed+1);
    DovA_graph->SetMarkerColor(kRed+1);

    auto & CovB_graph = CovB_graphs[time_split];
    CovB_graph = new TGraphErrors();
    CovB_graph->SetName("CovB_Time"+time_split);
    CovB_graph->SetLineColor  (kBlue);
    CovB_graph->SetMarkerColor(kBlue);
  }

  // make dumps
  for (const auto time_split : time_splits)
    for (const auto met_split : met_splits)
      makeClosureDump(hist2D,time_split,met_split,label,outtextfile,
		      BovA_graphs,CovD_graphs,DovA_graphs,CovB_graphs);

  // make graph canvases
  for (const auto & met_split : met_splits)
  {
    auto & BovA_graph = BovA_graphs[met_split];
    auto & CovD_graph = CovD_graphs[met_split];

    makeGraphCanvas(iofile,BovA_graph,"B/A",CovD_graph,"C/D","Photon Cluster Time Split (ns) [A|D,B|C]",
		    met_split,"p_{T}^{miss} Split = "," (GeV) [A|B,D|C]","MET",label,outfiletext);
  }

  for (const auto & time_split : time_splits)
  {
    auto & DovA_graph = DovA_graphs[time_split];
    auto & CovB_graph = CovB_graphs[time_split];

    makeGraphCanvas(iofile,DovA_graph,"D/A",CovB_graph,"C/B","p_{T}^{miss} Split (Gev) [A|B,D|C]",
		    time_split,"Photon Cluster Time Split = "," (ns) [A|D,B|C]","Time",label,outfiletext);
  }

  // make1D time slice plots
  make1DSlices(iofile,hist2D,label,outfiletext);

  // write graphs out
  Common::WriteMap(iofile,BovA_graphs);
  Common::WriteMap(iofile,CovD_graphs);
  Common::WriteMap(iofile,DovA_graphs);
  Common::WriteMap(iofile,CovB_graphs);

  // delete it all
  Common::DeleteMap(CovB_graphs);
  Common::DeleteMap(DovA_graphs);
  Common::DeleteMap(CovD_graphs);
  Common::DeleteMap(BovA_graphs);
  delete hist2D;
  delete iofile;
  delete tdrStyle;
}

void makeClosureDump(const TH2F * hist2D, const TString & time_split, const TString & met_split,
		     const TString & label, std::ofstream & outtextfile,
		     std::map<TString,TGraphErrors*> & BovA_graphs, std::map<TString,TGraphErrors*> & CovD_graphs,
		     std::map<TString,TGraphErrors*> & DovA_graphs, std::map<TString,TGraphErrors*> & CovB_graphs)
{
  // make atof
  TString time_split_tmp = time_split;
  TString met_split_tmp  = met_split;
  time_split_tmp.ReplaceAll("p",".");
  met_split_tmp.ReplaceAll("p",".");

  const auto time_split_val = time_split_tmp.Atof();
  const auto met_split_val  = met_split_tmp .Atof();

  // get bin assignments
  const auto ibinX = hist2D->GetXaxis()->FindBin(time_split_val);
  const auto ibinY = hist2D->GetYaxis()->FindBin(met_split_val);

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
  const auto BovAunc = GetRatioUnc(BovA,{{obsA,obsAunc},{obsB,obsBunc}});

  const auto DovA    = obsD/obsA;
  const auto DovAunc = GetRatioUnc(DovA,{{obsA,obsAunc},{obsD,obsDunc}});

  const auto predC    = obsB*obsD/obsA;
  const auto predCunc = GetRatioUnc(predC,{{obsA,obsAunc},{obsB,obsBunc},{obsD,obsDunc}});

  //////////////////
  // other ratios //
  //////////////////

  const auto CovD    = obsC/obsD;
  const auto CovDunc = GetRatioUnc(CovD,{{obsC,obsCunc},{obsD,obsDunc}});

  const auto CovB    = obsC/obsB;
  const auto CovBunc = GetRatioUnc(CovB,{{obsB,obsBunc},{obsC,obsCunc}});

  ///////////
  // diffs //
  ///////////

  const auto percent_diffC    = (1.f-(predC/obsC))*100.f;
  const auto percent_diffCunc = percent_diffC*Common::hypot(obsCunc/obsC,predCunc/predC);
  const auto pullC = (obsC-predC)/std::sqrt(std::pow(obsCunc,2)+std::pow(predCunc,2));

  // fill output file
  std::cout << "Filling text file..." << std::endl;

  outtextfile << label.Data() << "," << time_split_val << "," << met_split_val << ","
	      << std::setprecision(3) << obsA << " +/- " << std::setprecision(3) << obsAunc << ","
	      << std::setprecision(3) << obsB << " +/- " << std::setprecision(3) << obsBunc << ","
	      << std::setprecision(3) << obsC << " +/- " << std::setprecision(3) << obsCunc << ","
	      << std::setprecision(3) << obsD << " +/- " << std::setprecision(3) << obsDunc << ","
	      << std::setprecision(3) << BovA << " +/- " << std::setprecision(3) << BovAunc << ","
	      << std::setprecision(3) << DovA << " +/- " << std::setprecision(3) << DovAunc << ","
	      << std::setprecision(3) << predC << " +/- " << std::setprecision(3) << predCunc << ","
	      << std::setprecision(3) << percent_diffC << " +/- " << std::setprecision(3) << percent_diffCunc << ","
	      << std::setprecision(3) << pullC << "," << std::setprecision(3) << ","
	      << std::setprecision(3) << BovA << " +/- " << std::setprecision(3) << BovAunc << ","
	      << std::setprecision(3) << CovD << " +/- " << std::setprecision(3) << CovDunc << ","
	      << std::setprecision(3) << DovA << " +/- " << std::setprecision(3) << DovAunc << ","
	      << std::setprecision(3) << CovB << " +/- " << std::setprecision(3) << CovBunc << ","
	      << std::endl;

  // make tgraph points : B/A
  auto & BovA_graph = BovA_graphs[met_split];
  const auto BovA_point = BovA_graph->GetN();
  BovA_graph->SetPoint(BovA_point,time_split_val,BovA);
  BovA_graph->SetPointError(BovA_point,0,BovAunc);

  // make tgraph points : C/D
  auto & CovD_graph = CovD_graphs[met_split];
  const auto CovD_point = CovD_graph->GetN();
  CovD_graph->SetPoint(CovD_point,time_split_val,CovD);
  CovD_graph->SetPointError(CovD_point,0,CovDunc);

  // make tgraph points : D/A
  auto & DovA_graph = DovA_graphs[time_split];
  const auto DovA_point = DovA_graph->GetN();
  DovA_graph->SetPoint(DovA_point,met_split_val,DovA);
  DovA_graph->SetPointError(DovA_point,0,DovAunc);

  // make tgraph points : C/B
  auto & CovB_graph = CovB_graphs[time_split];
  const auto CovB_point = CovB_graph->GetN();
  CovB_graph->SetPoint(CovB_point,met_split_val,CovB);
  CovB_graph->SetPointError(CovB_point,0,CovBunc);
}

void makeGraphCanvas(TFile * iofile, TGraphErrors * A_graph, const TString & A_label, TGraphErrors * C_graph, const TString & C_label,
		     const TString & x_title, const TString & split, const TString & y_prefix, const TString & y_suffix, 
		     const TString & splittext, const TString & label, const TString & outfiletext)
{
  // make labels
  TString split_tmp = split;
  split_tmp.ReplaceAll("p",".");
  const auto splitlabel = splittext+split;

  // make legend
  auto leg = new TLegend(0.75,0.8,0.825,0.92);
  leg->SetName("SplitLeg_"+splitlabel);
  leg->AddEntry(A_graph,A_label.Data(),"ep");
  leg->AddEntry(C_graph,C_label.Data(),"ep");
  
  // make canvas
  auto canv = new TCanvas();
  canv->SetName("SplitCanv_"+splitlabel);
  canv->cd();

  // draw graphs
  A_graph->Draw("APZ");
  C_graph->Draw("PZ SAME");
  leg->Draw("SAME");

  // get/set min, max
  auto minX = 1e9, maxX = -1e9;
  auto minY = 1e9, maxY = -1e9;
  
  // GetMinMaxXY
  GetMinMaxXY(A_graph,minX,maxX,minY,maxY);
  GetMinMaxXY(C_graph,minX,maxX,minY,maxY);

  // set min max
  const auto centerX = (minX+maxX)/2.0;
  const auto widthX  = 1.2*((maxX-minX)/2.0);
  A_graph->GetXaxis()->SetLimits(centerX-widthX,centerX+widthX);
  A_graph->GetYaxis()->SetRangeUser(minY/1.05,maxY*1.05);

  // set titles
  A_graph->GetXaxis()->SetTitle(x_title.Data());
  A_graph->GetYaxis()->SetTitle("Ratio for "+y_prefix+split_tmp+y_suffix);
  A_graph->GetYaxis()->SetTitleOffset(2.0);

  // modify the canvas
  const auto outname = outfiletext+"_"+label+"_"+splitlabel;
  Common::CMSLumi(canv,0,"Full");
  Common::SaveAs(canv,outname);
  
  // write it out
  Common::Write(iofile,A_graph);
  Common::Write(iofile,C_graph);
  Common::Write(iofile,leg);
  Common::Write(iofile,canv);

  // delete it all
  delete canv;
  delete leg;
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
  Common::DeleteVec(hist1Ds);
  delete leg;
  delete canv;
}
