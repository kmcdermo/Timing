// config
const auto zee_low = 99.0;
const auto dixal_low = 75.0;
const auto x_up = 2250;
const auto sigmaN = 0.0887; // GeV
const TString data_base_name = "Data_sigma";
const TString mc_base_name = "MC_sigma";
const TString dir = "test_macros/extra_fits/inputfiles";

// functions
void SetTDRStyle(TStyle * tdrStyle);
void makePlots(const TString & label, const TString & pavelabel);
void SaveAs(TCanvas * canv, const TString & label);
void CheckValidFile(const TFile * file, const TString & filename);
void CheckValidF1(const TF1 * f1, const TString & f1name, const TString & filename);
void CheckValidGraph(const TGraphErrors * graph, const TString & graphname, const TString & filename);

// main call
void style_time_plots()
{
  // make style
  auto tdrStyle = new TStyle("TDRStyle","Style for P-TDR");
  SetTDRStyle(tdrStyle);

  // make plots
  makePlots("dixtal_inclusive","EB");
  makePlots("dixtal_sameTT","EB same RO unit");
  makePlots("dixtal_diffTT","EB diff. RO unit");
  makePlots("zee","EB Z Study");

  // delete it all
  delete tdrStyle;
}

void makePlots(const TString & label, const TString & pavelabel)
{
  // get x_low
  auto x_low = (label.EqualTo("zee") ? zee_low : dixal_low);

  // get file
  const auto filename = dir+"/"+label+".root";
  auto file = TFile::Open(filename.Data());
  CheckValidFile(file,filename);

  // get graphs
  const auto data_graph_name = data_base_name+"_graph";
  auto data_graph = (TGraphErrors*)file->Get(data_graph_name.Data());
  CheckValidGraph(data_graph,data_graph_name,file->GetName());

  const auto mc_graph_name = mc_base_name+"_graph";
  auto mc_graph = (TGraphErrors*)file->Get(mc_graph_name.Data());
  CheckValidGraph(mc_graph,mc_graph_name,file->GetName());

  // get fits
  const auto data_fit_name = data_base_name+"_fit_graph";
  auto data_fit = (TF1*)file->Get(data_fit_name.Data());
  CheckValidF1(data_fit,data_fit_name,file->GetName());

  const auto mc_fit_name = mc_base_name+"_fit_graph";
  auto mc_fit = (TF1*)file->Get(mc_fit_name.Data());
  CheckValidF1(mc_fit,mc_fit_name,file->GetName());

  // remove fits from tgraphs
  data_graph->GetFunction(data_fit_name.Data())->SetBit(TF1::kNotDraw);
  mc_graph  ->GetFunction(mc_fit_name  .Data())->SetBit(TF1::kNotDraw);

  // make canvas
  auto canv = new TCanvas();

  // set cms pave
  auto cms_pave = new TPaveText(0.20,0.855,0.3,0.895,"NDC");
  cms_pave->SetFillColorAlpha(0,0);
  cms_pave->SetTextFont(61);
  cms_pave->AddText("CMS");

  // set prelim pave
  auto prelim_pave = new TPaveText(0.25,0.855,0.45,0.89,"NDC");
  prelim_pave->SetFillColorAlpha(0,0);
  prelim_pave->SetTextFont(52);
  prelim_pave->AddText("Preliminary");

  // make legend
  auto leg = new TLegend(0.215,0.715,0.415,0.80);
  leg->SetTextFont(42);
  leg->SetFillColorAlpha(0,0);

  // set lumi pave
  auto lumi_pave = new TPaveText(0.218,0.80,0.418,0.86,"NDC");
  lumi_pave->SetFillColorAlpha(0,0);
  lumi_pave->SetTextFont(42);
  lumi_pave->AddText("41.53 fb^{-1} (13 TeV)");

  // set label pave
  auto label_pave = new TPaveText(0.6,0.855,0.8,0.895,"NDC");
  label_pave->SetFillColorAlpha(0,0);
  label_pave->SetTextFont(42);
  label_pave->AddText(pavelabel.Data());

  // set form pave
  auto form_pave = new TPaveText(0.55,0.73,0.8,0.85,"NDC");
  form_pave->SetFillColorAlpha(0,0);
  form_pave->SetTextFont(42);
  form_pave->SetTextAlign(11);
  form_pave->AddText("#sigma(t)=#frac{N}{A_{eff}/#sigma_{n}} #oplus #sqrt{2}C");

  // set form pave
  auto fit_pave = new TPaveText(0.55,0.565,0.8,0.735,"NDC");
  fit_pave->SetFillColorAlpha(0,0);
  fit_pave->SetTextFont(42);
  fit_pave->SetTextAlign(11);
  fit_pave->AddText(Form("N^{Data} = %4.1f #pm %3.1f [ns]",data_fit->GetParameter(0),data_fit->GetParError(0)));
  fit_pave->AddText(Form("C^{Data} = %5.3f #pm %5.3f [ns]",data_fit->GetParameter(1),data_fit->GetParError(1)));
  fit_pave->AddText(Form("N^{Sim}  = %4.1f #pm %3.1f [ns]",mc_fit  ->GetParameter(0),mc_fit  ->GetParError(0)));
  fit_pave->AddText(Form("C^{Sim}  = %5.3f #pm %5.3f [ns]",mc_fit  ->GetParameter(1),mc_fit  ->GetParError(1)));

  // set upper axis
  auto top_axis = new TGaxis(x_low,1,x_up,1,x_low*sigmaN,x_up*sigmaN,505,"-G");
  top_axis->SetTextFont(42);
  top_axis->SetLabelFont(42);
  top_axis->SetNoExponent(1);
  top_axis->SetTitle("E in EB [GeV]");
  top_axis->SetTitleSize(0.035);
  top_axis->SetLabelSize(0.035);

  // set marker color
  data_graph->SetMarkerColor(kRed);
  mc_graph  ->SetMarkerColor(kBlue);

  // set markers
  data_graph->SetMarkerStyle(20);
  mc_graph  ->SetMarkerStyle(21);

  // set lines
  data_graph->SetLineColor(kRed);
  mc_graph  ->SetLineColor(kBlue);

  // set line width
  data_graph->SetLineWidth(1);
  mc_graph  ->SetLineWidth(1);

  // set fit color
  data_fit->SetLineColor(kBlack);
  mc_fit  ->SetLineColor(kBlack);

  // set fit width
  data_fit->SetLineWidth(2);
  mc_fit  ->SetLineWidth(2);

  // set legend
  leg->AddEntry(data_graph,"Data","epl");
  leg->AddEntry(mc_graph,"Simulation","epl");

  // draw it all
  data_graph ->Draw("apz");
  data_fit   ->Draw("same");
  data_graph ->Draw("pz same");
  mc_graph   ->Draw("pz same");
  mc_fit     ->Draw("same");
  cms_pave   ->Draw("same");
  prelim_pave->Draw("same");
  lumi_pave  ->Draw("same");
  label_pave ->Draw("same");
  form_pave  ->Draw("same");
  fit_pave   ->Draw("same");
  leg        ->Draw("same");
  top_axis   ->Draw("same");

  // set range
  data_graph->GetXaxis()->SetLimits(x_low,x_up);
  data_graph->GetYaxis()->SetRangeUser(7e-2,1);

  // set titles
  data_graph->GetXaxis()->SetTitle("A_{eff}/#sigma_{n}");
  data_graph->GetYaxis()->SetTitle("#sigma(t_{1}-t_{2}) [ns]");

  // save as
  SaveAs(canv,"datamc_"+label);

  // now make the data only graph
  delete fit_pave;
  fit_pave = new TPaveText(0.55,0.650,0.8,0.735,"NDC");
  fit_pave->SetFillColorAlpha(0,0);
  fit_pave->SetTextFont(42);
  fit_pave->SetTextAlign(11);
  fit_pave->AddText(Form("N = %4.1f #pm %3.1f [ns]",data_fit->GetParameter(0),data_fit->GetParError(0)));
  fit_pave->AddText(Form("C = %5.3f #pm %5.3f [ns]",data_fit->GetParameter(1),data_fit->GetParError(1)));
  
  // set colors
  data_graph->SetLineColor(kBlack);
  data_graph->SetMarkerColor(kBlack);
  data_fit ->SetLineColor(kRed+1);

  data_graph ->Draw("apz");
  data_fit   ->Draw("same");
  data_graph ->Draw("pz same");
  cms_pave   ->Draw("same");
  prelim_pave->Draw("same");
  label_pave ->Draw("same");
  lumi_pave  ->Draw("same");
  form_pave  ->Draw("same");
  fit_pave   ->Draw("same");
  top_axis   ->Draw("same");
  
  // save as
  SaveAs(canv,"dataonly_"+label);

  // delete it all
  delete top_axis;
  delete leg;
  delete fit_pave;
  delete form_pave;
  delete label_pave;
  delete lumi_pave;
  delete prelim_pave;
  delete cms_pave;
  delete canv;
  delete mc_fit;
  delete data_fit;
  delete mc_graph;
  delete data_graph;
  delete file;
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

void CheckValidGraph(const TGraphErrors * graph, const TString & graphname, const TString & filename)
{
  if (graph == (TGraphErrors*) NULL) // check if valid graph
  {
    std::cerr << "Input Graph is bad pointer: " << graphname.Data() << " in input file: " << filename.Data() << " ...exiting..." << std::endl;
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
