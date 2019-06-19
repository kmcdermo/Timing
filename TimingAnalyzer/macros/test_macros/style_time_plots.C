// config
const std::vector<Double_t> xbins = {75,100,125,150,175,225,275,325,375,475,600,750,1700,2250};
const auto nGoodBins = 12;
const TString data_hist_name = "Data_sigma";
const TString mc_hist_name = "MC_sigma";

struct Bin
{
  Bin () {}
  Bin (const Float_t val, const Float_t err) : val(val), err(err) {}

  Float_t val;
  Float_t err;
};
std::map<TString,Bin> binMap;

// functions
void SetTDRStyle(TStyle * tdrStyle);
void setupBinMap();
void makePlots(const TString & label, const TString & pavelabel);
TH1F * makeMergedHist(TH1F * input, const Bin & bin);
void SaveAs(TCanvas * canv, const TString & label);
void CheckValidFile(const TFile * file, const TString & filename);
void CheckValidF1(const TF1 * f1, const TString & f1name, const TString & filename);
void CheckValidHist(const TH1F * hist, const TString & histname, const TString & filename);

// main call
void style_time_plots()
{
  // make style
  auto tdrStyle = new TStyle("TDRStyle","Style for P-TDR");
  SetTDRStyle(tdrStyle);

  // setup values for merged bins histogram
  setupBinMap();

  // make plots
  makePlots("dixtal_inclusive","EB");
  makePlots("dixtal_sameTT","EB same RO unit");
  makePlots("dixtal_diffTT","EB diff. RO unit");
  makePlots("zee","EB Z Study");

  // delete it all
  delete tdrStyle;
}

void setupBinMap()
{
  binMap["dixtal_inclusive_"+data_hist_name] = {0.137996,0.000676859};
  binMap["dixtal_inclusive_"+mc_hist_name  ] = {0.156907, 0.00876678};

  binMap["dixtal_sameTT_"+data_hist_name] = {0.129849,0.000678997};
  binMap["dixtal_sameTT_"+mc_hist_name  ] = {0.155851,0.00894815};

  binMap["dixtal_diffTT_"+data_hist_name] = {0.23868,0.00468409};
  binMap["dixtal_diffTT_"+mc_hist_name  ] = {0.153607,0.0415417};

  binMap["zee_"+data_hist_name] = {0.2774,0.00735598};
  binMap["zee_"+mc_hist_name  ] = {0.171635,0.0111146};
}

void makePlots(const TString & label, const TString & pavelabel)
{
  // get file
  const auto filename = "inputfiles/"+label+".root";
  auto file = TFile::Open(filename.Data());
  CheckValidFile(file,filename);

  // get hists
  auto input_data_hist = (TH1F*)file->Get(data_hist_name.Data());
  CheckValidHist(input_data_hist,data_hist_name,file->GetName());
  auto data_hist = makeMergedHist(input_data_hist,binMap[label+"_"+data_hist_name]);

  auto input_mc_hist = (TH1F*)file->Get(mc_hist_name.Data());
  CheckValidHist(input_mc_hist,mc_hist_name,file->GetName());
  auto mc_hist = makeMergedHist(input_mc_hist,binMap[label+"_"+mc_hist_name]);

  // get fits
  const auto data_fit_name = data_hist_name+"_fit";
  auto data_fit = (TF1*)file->Get(data_fit_name.Data());
  CheckValidF1(data_fit,data_fit_name,file->GetName());

  const auto mc_fit_name = mc_hist_name+"_fit";
  auto mc_fit = (TF1*)file->Get(mc_fit_name.Data());
  CheckValidF1(mc_fit,mc_fit_name,file->GetName());

  // make canvas
  auto canv = new TCanvas();

  // set cms pave
  auto cms_pave = new TPaveText(0.18,0.85,0.6,0.885,"NDC");
  cms_pave->SetFillColorAlpha(0,0);
  cms_pave->AddText("CMS Preliminary - Run2 (2017)");
  
  // set label pave
  auto label_pave = new TPaveText(0.6,0.855,0.8,0.885,"NDC");
  label_pave->SetFillColorAlpha(0,0);
  label_pave->AddText(pavelabel.Data());

  // set fit pave
  auto fit_pave = new TPaveText(0.5,0.57,0.85,0.84,"NDC");
  fit_pave->SetFillColorAlpha(0,0);
  fit_pave->SetTextAlign(11);
  fit_pave->AddText("#sigma(t)=#frac{N}{A_{eff}/#sigma_{n}} #oplus #sqrt{2}C");
  fit_pave->AddText(Form("N^{Data} = %4.1f #pm %4.2f [ns]",data_fit->GetParameter(0),data_fit->GetParError(0)));
  fit_pave->AddText(Form("C^{Data} = %6.4f #pm %6.4f [ns]",data_fit->GetParameter(1),data_fit->GetParError(1)));
  fit_pave->AddText(Form("N^{MC}   = %4.1f #pm %4.2f [ns]",mc_fit  ->GetParameter(0),mc_fit  ->GetParError(0)));
  fit_pave->AddText(Form("C^{MC}   = %6.4f #pm %6.4f [ns]",mc_fit  ->GetParameter(1),mc_fit  ->GetParError(1)));

  // legend
  auto leg = new TLegend(0.21,0.745,0.5,0.845);
  leg->SetFillColorAlpha(0,0);
  leg->AddEntry(data_hist,"Data","epl");
  leg->AddEntry(mc_hist,"Simulation","epl");

  // set upper axis
  auto axis_tf1 = new TF1("axis_fit","log10(x)",1,101);
  auto top_axis = new TGaxis(xbins.front(),1,xbins.back(),1,axis_tf1->GetName(),505,"-");
  top_axis->SetNoExponent(1);
  top_axis->SetTitle("E in EB [GeV]");
  top_axis->SetTitleSize(0.035);
  top_axis->SetLabelSize(0.035);

  // set range
  data_hist->GetYaxis()->SetRangeUser(7e-2,1);

  // set titles
  data_hist->GetXaxis()->SetTitle("A_{eff}/#sigma_{n}");

  // set markers
  data_hist->SetMarkerColor(kRed+1);
  mc_hist  ->SetMarkerColor(kBlue);

  // set lines
  data_hist->SetLineColor(kRed+1);
  mc_hist  ->SetLineColor(kBlue);

  // set fit color
  data_fit->SetLineColor(kBlack);
  mc_fit  ->SetLineColor(kBlack);

  // set fit width
  data_fit->SetLineWidth(2);
  mc_fit  ->SetLineWidth(2);

  // draw it all  
  data_hist ->Draw("ep");
  data_fit  ->Draw("same");
  data_hist ->Draw("ep same");
  mc_hist   ->Draw("ep same");
  mc_fit    ->Draw("same");
  cms_pave  ->Draw("same");
  label_pave->Draw("same");
  fit_pave  ->Draw("same");
  leg       ->Draw("same");
  top_axis  ->Draw("same");

  // save as
  SaveAs(canv,"datamc_"+label);

  // now make the data only hist
  delete fit_pave;
  fit_pave = new TPaveText(0.5,0.60,0.85,0.81,"NDC");
  fit_pave->SetFillColorAlpha(0,0);
  fit_pave->SetTextAlign(11);
  fit_pave->AddText("#sigma(t)=#frac{N}{A_{eff}/#sigma_{n}} #oplus #sqrt{2}C");
  fit_pave->AddText(Form("N = %4.1f #pm %4.2f [ns]",data_fit->GetParameter(0),data_fit->GetParError(0)));
  fit_pave->AddText(Form("C = %6.4f #pm %6.4f [ns]",data_fit->GetParameter(1),data_fit->GetParError(1)));
  
  // set colors
  data_hist->SetLineColor(kBlack);
  data_hist->SetMarkerColor(kBlack);
  data_fit ->SetLineColor(kRed+1);

  data_hist ->Draw("ep");
  data_fit  ->Draw("same");
  data_hist ->Draw("ep same");
  cms_pave  ->Draw("same");
  label_pave->Draw("same");
  fit_pave  ->Draw("same");
  top_axis  ->Draw("same");
  
  // save as
  SaveAs(canv,"dataonly_"+label);

  // delete it all
  delete top_axis;
  delete axis_tf1;
  delete leg;
  delete fit_pave;
  delete label_pave;
  delete cms_pave;
  delete canv;
  delete mc_fit;
  delete data_fit;
  delete mc_hist;
  delete input_mc_hist;
  delete data_hist;
  delete input_data_hist;
  delete file;
}

TH1F * makeMergedHist(TH1F * input, const Bin & bin)
{
  auto hist = new TH1F(Form("%s_merged",input->GetName()),
		       Form("%s;%s;%s",input->GetTitle(),input->GetXaxis()->GetTitle(),input->GetYaxis()->GetTitle()),
		       xbins.size()-1,&xbins[0]);
  
  for (auto ibin = 1; ibin < nGoodBins; ibin++) 
  {
    hist->SetBinContent(ibin,input->GetBinContent(ibin));
    hist->SetBinError  (ibin,input->GetBinError  (ibin));
  }

  hist->SetBinContent(nGoodBins,bin.val);
  hist->SetBinError  (nGoodBins,bin.err);

  return hist;
}

void SaveAs(TCanvas * canv, const TString & label)
{
  const std::vector<TString> extensions = {"png","pdf","C"};

  canv->cd();
  for (const auto & extension : extensions)
  {
    canv->SaveAs(Form("%s.%s",label.Data(),extension.Data()));
  }
}

void CheckValidFile(const TFile * file, const TString & filename)
{
  if (file == (TFile*) NULL) // check if valid file
  {
    std::cerr << "Input file is bad pointer: " << filename.Data() << " ...exiting..." << std::endl;
    exit(1);
  }
  else 
  {
    std::cout << "Successfully opened file: " << filename.Data() << std::endl;
  }
}

void CheckValidF1(const TF1 * f1, const TString & f1name, const TString & filename)
{
  if (f1 == (TF1*) NULL) // check if valid tf1
  {
    std::cerr << "Input TF1 is bad pointer: " << f1name.Data() << " in input file: " << filename.Data() << " ...exiting..." << std::endl;
    exit(1);
    }
}

void CheckValidHist(const TH1F * hist, const TString & histname, const TString & filename)
{
  if (hist == (TH1F*) NULL) // check if valid hist
  {
    std::cerr << "Input Hist is bad pointer: " << histname.Data() << " in input file: " << filename.Data() << " ...exiting..." << std::endl;
    exit(1);
  }
}

void SetTDRStyle(TStyle * tdrStyle)
{  
  // For the canvas:
  tdrStyle->SetCanvasBorderMode(0);
  tdrStyle->SetCanvasColor(kWhite);
  tdrStyle->SetCanvasDefH(600); //Height of canvas
  tdrStyle->SetCanvasDefW(700); //Width of canvas
  tdrStyle->SetCanvasDefX(0);   //Position on screen
  tdrStyle->SetCanvasDefY(0);
  
  // For the Pad:
  tdrStyle->SetPadBorderMode(0);
  tdrStyle->SetPadColor(kWhite);
  tdrStyle->SetPadGridX(1);
  tdrStyle->SetPadGridY(1);
  tdrStyle->SetGridColor(0);
  tdrStyle->SetGridStyle(3);
  tdrStyle->SetGridWidth(1);

  // For the frame:
  tdrStyle->SetFrameBorderMode(0);
  tdrStyle->SetFrameBorderSize(1);
  tdrStyle->SetFrameFillColor(0);
  tdrStyle->SetFrameFillStyle(0);
  tdrStyle->SetFrameLineColor(1);
  tdrStyle->SetFrameLineStyle(1);
  tdrStyle->SetFrameLineWidth(1);

  // For the histo:
  tdrStyle->SetHistLineColor(1);
  tdrStyle->SetHistLineStyle(0);
  tdrStyle->SetHistLineWidth(1);
  tdrStyle->SetEndErrorSize(2);
  tdrStyle->SetErrorX(0.5);
  tdrStyle->SetMarkerStyle(20);
  tdrStyle->SetMarkerSize(0.6);
  
  //For the fit/function:
  tdrStyle->SetOptFit(0);
  tdrStyle->SetFitFormat("5.4g");
  tdrStyle->SetFuncColor(2);
  tdrStyle->SetFuncStyle(1);
  tdrStyle->SetFuncWidth(1);
  
  //For the date:
  tdrStyle->SetOptDate(0);

  // For the statistics box:
  tdrStyle->SetOptFile(0);
  tdrStyle->SetOptStat(0); // To display the mean and RMS:   SetOptStat("mr");
  tdrStyle->SetStatColor(kWhite);
  tdrStyle->SetStatFont(42);
  tdrStyle->SetStatFontSize(0.025);
  tdrStyle->SetStatTextColor(1);
  tdrStyle->SetStatFormat("6.4g");
  tdrStyle->SetStatBorderSize(1);
  tdrStyle->SetStatH(0.1);
  tdrStyle->SetStatW(0.15);
  
  // Margins:
  tdrStyle->SetPadTopMargin(0.08);
  tdrStyle->SetPadBottomMargin(0.11);
  tdrStyle->SetPadLeftMargin(0.18);
  tdrStyle->SetPadRightMargin(0.15); 
  
  // For the Global title:
  tdrStyle->SetOptTitle(0);
  tdrStyle->SetTitleFont(42);
  tdrStyle->SetTitleColor(1);
  tdrStyle->SetTitleTextColor(1);
  tdrStyle->SetTitleFillColor(10);
  tdrStyle->SetTitleFontSize(0.05);

  // For the axis titles:
  tdrStyle->SetTitleColor(1, "XYZ");
  tdrStyle->SetTitleFont(42, "XYZ");

  // For the axis labels:
  tdrStyle->SetLabelColor(1, "XYZ");
  tdrStyle->SetLabelFont(42, "XYZ");
  tdrStyle->SetLabelOffset(0.007, "XYZ");
  tdrStyle->SetLabelSize(0.04, "XYZ");
  
  // For the axis:
  tdrStyle->SetAxisColor(1, "XYZ");
  tdrStyle->SetStripDecimals(1);
  tdrStyle->SetTickLength(0.03, "XYZ");
  tdrStyle->SetNdivisions(505, "X");
  tdrStyle->SetPadTickX(0);
  tdrStyle->SetPadTickY(0);
    
  // Change for log plots:
  tdrStyle->SetOptLogx(1);
  tdrStyle->SetOptLogy(1);
  tdrStyle->SetOptLogz(0);
  
  // color palette
  tdrStyle->SetPalette(kBird);
  tdrStyle->SetNumberContours(255);
  
  // now set the style!
  tdrStyle->cd();
  gROOT->ForceStyle();
}
