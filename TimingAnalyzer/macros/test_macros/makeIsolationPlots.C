#include "makeIsolationPlots.hh"

// main function
void makeIsolationPlots()
{
  // style
  gStyle->SetOptStat(0);

  // get inputs
  const TString infile_name = "skims/v4/ootVID/gmsb_isGen.root";
  auto infile = TFile::Open(infile_name.Data());
  Common::CheckValidFile(infile,infile_name);
  
  const TString tree_name = "GMSB_L200_CTau200_Tree";
  auto tree = (TTree*)infile->Get(tree_name.Data());
  Common::CheckValidTree(tree,tree_name,infile_name);

  // setup common cut based on input
  const TString commoncut = "(puwgt*evtwgt)*((phopt_0>70)&&(phoisEB_0==1)&&(phoisOOT_0==0)&&(phoisGen_0==1))"; // could be evtwgt*, or isGen==0

  // setup combo corrections
  Config::setupComboCorrs();

  // setup isobins
  Config::setupYbins();

  // make output
  auto outfile = TFile::Open("isoplots.root","recreate");
  const TString eosdir = "/eos/user/k/kmcdermo/www";
  const TString outdir = "dispho/plots/ootVID_v3/quantiles/rho/eta_LT0p8";

  // config
  const auto fitType  = FitType::Linear;
  const auto cutType  = CutType::eta_LT0p8;
  const auto xType    = XType::rho;
  const auto corrType = CorrType::pt_corrs_v2;
  const auto & yInfos = Config::yIsos;
  const auto & yCorrections = Config::yCorrectionsMap.at(corrType);

  // make plots!
  for (const auto & yInfoPair : yInfos)
  {
    const auto   yType = yInfoPair.first;
    const auto & yInfo = yInfoPair.second;

    makePlots(tree,commoncut,outfile,Config::xNames.at(xType),Config::xInfos.at(xType),
	      Config::yNames.at(yType),yInfo,yCorrections.at(yType),fitType,cutType);
  }

  // delete all
  delete outfile;
  delete tree;
  delete infile;

  // move it all
  const auto fulldir = eosdir+"/"+outdir;
  gSystem->Exec("mkdir -p "+fulldir);
  gSystem->Exec("mv *png *pdf *root "+fulldir);
  gSystem->Exec("pushd "+eosdir+"; ./copyphp.sh "+outdir+"; popd;");

}

void makePlots(TTree * tree, const TString & commoncut, TFile * outfile, 
	       const TString & xname, const XInfo & xInfo, 
	       const TString & yname, const YInfo & yInfo, const TString & yCorrection,
	       const FitType fitType, const CutType & cutType)
{
  // echo
  std::cout << "xname: " << xname.Data() << " yname: " << yname.Data() << std::endl;

  // decode the inputs
  const auto & xvar   = xInfo.var;
  const auto & xtitle = xInfo.title;
  const auto & xbins  = xInfo.bins;
  const auto & binsX  = &xbins[0];

  const auto & yvar   = yInfo.var;
  const auto & ytitle = yInfo.title;
  const auto & binsY  = &Config::ybins[0];

  // tmp canvas
  auto canv = new TCanvas();
  canv->cd();
  canv->SetTickx();
  canv->SetTicky();

  // make 2D hist
  outfile->cd();
  auto hist2D = new TH2F(yname+"_vs_"+xname,ytitle+" vs. "+xtitle+";"+xtitle+";"+ytitle,
			 xbins.size()-1,binsX,Config::ybins.size()-1,binsY);
  hist2D->Sumw2();

  // fill 2D hist
  const TString draw = Form("max(%s%s,0.0):%s>>%s",yvar.Data(),yCorrection.Data(),xvar.Data(),hist2D->GetName());
  const TString cut  = commoncut+Config::cuts.at(cutType);
  std::cout << draw.Data() << std::endl;
  std::cout << cut .Data() << std::endl;

  tree->Draw(draw.Data(),cut.Data(),"goff");
  
  // fill 1D hist (inclusive)
  outfile->cd();
  const auto hist_name = yname+"_inclusive";
  auto hist1D_before = hist2D->ProjectionY(hist_name.Data());
  Common::Scale(hist1D_before,false);

  // get quantile plots
  makeQuantiles(canv,hist2D,outfile,yname,fitType,"0.5");
  makeQuantiles(canv,hist2D,outfile,yname,fitType,"0.6");
  makeQuantiles(canv,hist2D,outfile,yname,fitType,"0.7");
  makeQuantiles(canv,hist2D,outfile,yname,fitType,"0.8");
  makeQuantiles(canv,hist2D,outfile,yname,fitType,"0.9");

  // draw before plot
  canv->cd();
  canv->SetLogx();
  hist1D_before->GetXaxis()->SetTitleOffset(1.1);
  hist1D_before->GetYaxis()->SetTitle("Events");
  hist1D_before->Draw("ep");

  // SaveAs: lin y
  Common::SaveAs(canv,hist_name+"_lin");
  
  // SaveAs: log y
  canv->SetLogy();
  Common::SaveAs(canv,hist_name+"_log");

  // save hists
  outfile->cd();
  hist1D_before->Write(hist1D_before->GetName(),TObject::kWriteDelete);
  hist2D->Write(hist2D->GetName(),TObject::kWriteDelete);

  // delete it all
  delete hist1D_before;
  delete hist2D;
  delete canv;
}

void makeQuantiles(TCanvas * canv, TH2F * hist2D, TFile * outfile,
		   const TString & yname, const FitType & fitType,
		   const TString & quantile)
{
  // decode inputs
  const auto & fit     = Config::fits.at(fitType);
  const auto & formula = fit.formula;
  const auto & text    = fit.text;
  const auto & names   = fit.names;

  // make quantile plot
  outfile->cd();
  auto hist1D_quant = hist2D->QuantilesX(quantile.Atof(),yname+"_q"+Common::ReplaceDotWithP(quantile));
  hist1D_quant->GetXaxis()->SetTitle(hist2D->GetXaxis()->GetTitle());
  hist1D_quant->GetYaxis()->SetTitle(hist2D->GetYaxis()->GetTitle());

  // get range for fit
  const auto x_low = hist2D->GetXaxis()->GetBinLowEdge(1);
  const auto x_up  = hist2D->GetXaxis()->GetBinUpEdge(hist2D->GetXaxis()->GetNbins());

  // get names for fitters
  const TString formname = Form("%s_form",hist1D_quant->GetName());
  const TString fitname  = Form("%s_fit" ,hist1D_quant->GetName());

  // get and set formula
  auto form_hist = new TFormula(formname.Data(),formula.Data());
    
  // get and set fit
  auto fit_hist = new TF1(fitname.Data(),formname.Data(),x_low,x_up);

  // init params
  for (auto iname = 0U; iname < names.size(); iname++) fit_hist->SetParName(iname,names[iname]);

  // because root is an utter piece of trash
  if (FitType::Exponential)
  {
    fit_hist->SetParameter(0,0.1);
    fit_hist->SetParameter(1,-0.02);
    fit_hist->SetParameter(2,1.3);
    fit_hist->SetParameter(3,0.3);
  }
  
  // and then do da fit
  hist1D_quant->Fit(fitname.Data(),"Q");

  // add some text
  auto pavetext = new TPaveText(0.12,0.77,0.32,0.87,"NDC");
  pavetext->SetFillColorAlpha(pavetext->GetFillColor(),0);
  pavetext->AddText("Fit: "+text);
  pavetext->AddText("Quantile: "+quantile);

  // ensure plot can be seen
  auto miny = 1e9, maxy = -1e9;
  for (auto ibin = 1; ibin <= hist1D_quant->GetXaxis()->GetNbins(); ibin++)
  {
    const auto content = hist1D_quant->GetBinContent(ibin);
    if (content > maxy) maxy = content;
    if (content < miny && content > 0.0) miny = content;
  }

  if (miny/maxy > 0.5) hist1D_quant->GetYaxis()->SetRangeUser(miny/1.2,maxy*2.f);
  
  // draw it!
  canv->cd();
  hist1D_quant->Draw("ep");
  pavetext->Draw("same");
  canv->Update();

  // get stats and draw!
  auto stats = (TPaveStats*)(hist1D_quant->GetListOfFunctions()->FindObject("stats"));
  stats->SetX1NDC(0.34);
  stats->SetX2NDC(0.64);
  stats->SetY1NDC(0.6);
  stats->SetY2NDC(0.87);

  Common::SaveAs(canv,hist1D_quant->GetName());

  // save it all!
  outfile->cd();
  hist1D_quant->Write(hist1D_quant->GetName(),TObject::kWriteDelete);
  form_hist->Write(form_hist->GetName(),TObject::kWriteDelete);
  fit_hist->Write(fit_hist->GetName(),TObject::kWriteDelete);

  // delete
  delete stats;
  delete pavetext;
  delete fit_hist;
  delete form_hist;
  delete hist1D_quant;
}
