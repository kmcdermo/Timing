#include "Common.cpp+"

// config
const std::vector<Double_t> xbins = {75,100,125,150,175,225,275,325,375,475,600,750,1700,2250};
const auto nGoodBins = 12;
const auto nBins = 15;
const auto sigmaN = 0.0887; // GeV
const TString data_hist_name = "Data_sigma";
const TString mc_hist_name = "MC_sigma";
const TString dir = "test_macros/extra_fits/inputfiles";

struct Bin
{
  Bin () {}
  Bin (const Float_t val, const Float_t err) : val(val), err(err) {}

  Float_t val;
  Float_t err;
};
std::map<TString,Bin> binMap;

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

TH1F * makeMergedHist(const TH1F * input, const TString & binlabel)
{
  auto hist = new TH1F(Form("%s_merged",input->GetName()),
		       Form("%s;%s;%s",input->GetTitle(),input->GetXaxis()->GetTitle(),input->GetYaxis()->GetTitle()),
		       xbins.size()-1,&xbins[0]);
  
  for (auto ibin = 1; ibin < nGoodBins; ibin++) 
  {
    hist->SetBinContent(ibin,input->GetBinContent(ibin));
    hist->SetBinError  (ibin,input->GetBinError  (ibin));
  }

  const auto & bin = binMap[binlabel];

  hist->SetBinContent(nGoodBins,bin.val);
  hist->SetBinError  (nGoodBins,bin.err);

  return hist;
}

void setupParameters(const TF1 * hist_fit, TF1 *& graph_fit, const Int_t ipar)
{
  auto x_low = 0.0, x_up = 0.0;
  hist_fit->GetParLimits(ipar,x_low,x_up);

  graph_fit->SetParName  (ipar,hist_fit->GetParName  (ipar)); 
  graph_fit->SetParameter(ipar,hist_fit->GetParameter(ipar)); 
  graph_fit->SetParLimits(ipar,x_low,x_up);
}

void makeGraphAndFit(const TH1F * input, const TString & histname, const TString & label, TFile * file)
{
  std::cout << "  Generating graph and fit from hist: " << histname.Data() << std::endl;

  // first make the merged histogram
  auto hist = makeMergedHist(input,label+"_"+histname);

  // then make graph
  auto graph = new TGraphErrors(hist);
  graph->SetName(histname+"_graph");

  // special change for Zee: drop point(s)
  graph->RemovePoint(nGoodBins);
  if (label.EqualTo("zee")) graph->RemovePoint(0);

  // get fit + form
  const auto hist_form = (TFormula*)file->Get(histname+"_form");
  const auto hist_fit  = (TF1*)     file->Get(histname+"_fit");
  
  // make new form
  auto graph_form = new TFormula(*hist_form);
  graph_form->SetName(Form("%s_graph",graph_form->GetName()));
  
  // get range of fit
  auto x_low = 0.0, x_up = 0.0;
  hist_fit->GetRange(x_low,x_up);
  
  // make new fit
  auto graph_fit = new TF1(Form("%s_graph",hist_fit->GetName()),graph_form->GetName(),x_low,x_up);

  // get parameter limits
  setupParameters(hist_fit,graph_fit,0);
  setupParameters(hist_fit,graph_fit,1);

  // fit graph
  graph->Fit(graph_fit->GetName());

  // save all to the file
  Common::Write(file,graph);
  Common::Write(file,graph_form);
  Common::Write(file,graph_fit);

  // delete it all
  delete graph_fit;
  delete graph_form;
  delete hist_fit;
  delete hist_form;
  delete graph;
  delete hist;
}

void makeTransformation(const TString & label)
{
  std::cout << "Working on: " << label.Data() << std::endl;

  // get file
  const auto filename = dir+"/"+label+".root";
  auto file = TFile::Open(filename.Data(),"UPDATE");
  Common::CheckValidFile(file,filename);

  // get hists
  auto datahist = (TH1F*)file->Get(data_hist_name.Data());
  Common::CheckValidHist(datahist,data_hist_name,filename);

  auto mchist = (TH1F*)file->Get(mc_hist_name.Data());
  Common::CheckValidHist(mchist,mc_hist_name,filename);

  // make into tgrapherrors
  makeGraphAndFit(datahist,data_hist_name,label,file);
  makeGraphAndFit(mchist,mc_hist_name,label,file);

  // delete it all
  delete mchist;
  delete datahist;
  delete file;
}

void transformIntoGraphs()
{
  setupBinMap();

  makeTransformation("zee");
  makeTransformation("dixtal_inclusive");
  makeTransformation("dixtal_diffTT");
  makeTransformation("dixtal_sameTT");
}
