#include "TimeFitter.hh"
#include "TVirtualFitter.h"

TimeFitter::TimeFitter(const TString & infilename, const TString & plotconfig, const TString & timefitconfig, 
		       const TString & era, const TString & outfiletext) :
  fInFileName(infilename), fPlotConfig(plotconfig), fTimeFitConfig(timefitconfig), 
  fEra(era), fOutFileText(outfiletext)
{
  std::cout << "Initializing TimeFitter..." << std::endl;

  ////////////////
  //            //
  // Initialize //
  //            //
  ////////////////

  // Get input file
  fInFile = TFile::Open(Form("%s",fInFileName.Data()));
  Common::CheckValidFile(fInFile,fInFileName);
 
  // set style
  fTDRStyle = new TStyle("TDRStyle","Style for P-TDR");
  Common::SetTDRStyle(fTDRStyle);

  // output root file for quick inspection
  fOutFile = TFile::Open(Form("%s.root",fOutFileText.Data()),"UPDATE");

  // setup config
  TimeFitter::SetupCommon();
  TimeFitter::SetupPlotConfig();
  TimeFitter::SetupTimeFitConfig();

  // set fitter
  TVirtualFitter::SetDefaultFitter("Minuit2");
}

TimeFitter::~TimeFitter()
{
  std::cout << "Tidying up in destructor" << std::endl;

  delete fConfigPave;

  delete fOutFile;
  delete fTDRStyle;
  delete fInFile;
}

void TimeFitter::MakeTimeFits()
{
  std::cout << "Making time fits..." << std::endl;
  
  // Do data first
  FitStruct DataInfo("Data",Common::HistNameMap["Data"].Data());
  TimeFitter::MakeTimeFit(DataInfo);

  // Do MC bkgd first
  FitStruct BkgdInfo("Bkgd",Common::BkgdHistName.Data());
  TimeFitter::MakeTimeFit(BkgdInfo);

  // Make Plots
  TimeFitter::MakePlots(DataInfo,BkgdInfo);

  // MakeConfigPave
  TimeFitter::MakeConfigPave();

  // Delete infos
  TimeFitter::DeleteFitInfo(DataInfo);
  TimeFitter::DeleteFitInfo(BkgdInfo);
}

void TimeFitter::MakeTimeFit(FitStruct & FitInfo)
{
  const auto & label = FitInfo.label;
  std::cout << "Making time fits for: " << label.Data() << std::endl;
  
  // Get input hist
  TimeFitter::GetInputHist(FitInfo);

  // Project out 2D hists into map
  TimeFitter::Project2Dto1DHists(FitInfo);

  // Fit each 1D hist
  TimeFitter::Fit1DHists(FitInfo);

  // Extract mu and sigma into maps
  TimeFitter::ExtractFitResults(FitInfo);
}

void TimeFitter::GetInputHist(FitStruct & FitInfo)
{
  const auto & label = FitInfo.label;
  std::cout << "Getting input hist: " << label.Data() << std::endl;
  
  // get input
  const auto & inHistName = FitInfo.inHistName;
  auto & Hist2D = FitInfo.Hist2D;

  // get the hist
  Hist2D = (TH2F*)fInFile->Get(inHistName.Data());

  // save to output
  fOutFile->cd();
  Hist2D->Write(Hist2D->GetName(),TObject::kWriteDelete);
}

void TimeFitter::Project2Dto1DHists(FitStruct & FitInfo); 
{
  const auto & label = FitInfo.label;
  std::cout << "Projecting to 1D from 2D plot: " << label.Data() << std::endl;
  
  // get inputs/outputs
  const auto & Hist2D = FitInfo.Hist2D;
  auto & Hist1DMap = FitInfo.Hist1DMap;
  const TString histname = Hist2D->GetName();

  for (auto ibinX = 1; ibinX <= fXBins.size(); ibinX++)
  {
    auto & hist1D = Hist1DMap[ibinX];
    hist1D = (TH1F*)Hist2D->ProjectionY(Form("%s_ibin%i",histname.Data(),ibinX),ibinX,ibinX);
    hist1D->Sumw2();
  }
}

void TimeFitter::Fit1DHists(FitStruct & FitInfo)
{
  const auto & label = FitInfo.label;
  std::cout << "Fitting hists for: " << label.Data() << std::endl;
  
  // get inputs/outputs
  auto & Hist1DMap = FitInfo.Hist1DMap;
  auto & FitMap    = FitInfo.FitMap;

  for (auto ibinX = 1; ibinX <= fXBins.size(); ibinX++)
  {
    // get hist, and skip if no entries
    auto & hist1D = Hist1DMap[ibinX];
    if (hist1D->GetEntries() == 0) continue;

    // get fit, and prep it
    auto & fit = FitMap[ibinX];
    TimeFitter::PrepFit(hist1D,fit);
    
    // do the fit!
    hist1D->Fit(fit->GetName(),"RBQ0");

    // save output
    fOutFile->cd();
    hist1D->Write(hist1D->GetName(),TObject::kWriteDelete);
    fit->Write(fit->GetName(),TObject::kWriteDelete);
  }
}

void TimeFitter::ExtractFitResults(FitStruct & FitInfo)
{
  const auto & label = FitInfo.label;
  std::cout << "Extracting results for: " << label.Data() << std::endl;

  // get inputs/outputs
  const auto & FitMap = FitInfo.FitMap;
  auto & ResultsMap   = FitInfo.ResultsMap;
  
  // setup hists
  ResultsMap["chi2ndf"]  = TimeFitter::SetupHist("#chi^{2}/NDF","chi2ndf",label);
  ResultsMap["chi2prob"] = TimeFitter::SetupHist("#chi^{2} Prob.","chi2prob",label);
  ResultsMap["mu"]       = TimeFitter::SetupHist("#mu","mu",label);
  ResultsMap["sigma"]    = TimeFitter::SetupHist("#sigma","sigma",label);

  // set bin content!
  for (auto ibinX = 1; ibinX <= fXBins.size(); ibinX++)
  {
    // skip if fit not present
    if (!FitMap.count(ibinX)) continue;

    // get fit
    const auto & fit = FitMap.at(ibinX);

    // get results
    FitResult result;
    TimeFitter::GetFitResult(fit,result);

    // set bin content
    ResultsMap["chi2ndf"] ->SetBinContent(ibinX,result.chi2ndf);
    ResultsMap["chi2prob"]->SetBinContent(ibinX,result.chi2prob);
    ResultsMap["mu"]      ->SetBinContent(ibinX,result.mu);
    ResultsMap["mu"]      ->SetBinError  (ibinX,result.emu);
    ResultsMap["sigma"]   ->SetBinContent(ibinX,result.sigma);
    ResultsMap["sigma"]   ->SetBinError  (ibinX,result.esigma);
  }

  // save output
  fOutFile->cd();
  for (const auto & ResultsPair : ResultsMap) ResultsPair.second->Write(ResultsPair.second->GetName(),TObject::kWriteDelete);
}

void TimeFitter::MakePlots(FitStruct & DataInfo, FitStruct & BkgdInfo)
{
  std::cout << "Make overlay plots..." << std::endl;

  // make temp vector of hist key names
  std::vector<TString> keys;
  for (const auto & ResultsPair : DataInfo.ResultsMap) keys.emplace_back(ResultsPair.first);

  // loop over keys
  for (const auto & key : keys)
  {
    const auto & DataHist = DataInfo[key];
    const auto & BkgdHist = BkgdInfo[key];

    // tmp max, min
    const Float_t min =  1e9;
    const Float_t max = -1e9;
    
    TimeFitter::GetMinMax(DataHist,min,max);
    TimeFitter::GetMinMax(BkgdHist,min,max);

    // lin first, then log
    TimeFitter::PrintCanvas(DataHist,BkgdHist,min,max,key,false);
    TimeFitter::PrintCanvas(DataHist,BkgdHist,min,max,key,true);
  }
}

void TimeFitter::PrintCanvas(TH1F *& DataHist, TH1F *& BkgdHist, Float_t min, Float_t max, 
			     const TString & key, const Bool_t isLogy)
{
  std::cout << "Printing canvas for: " << key.Data() << " isLogy: " << Common::PrintBool(isLogy).Data() << std::endl;

  // bool to allow negative values
  const Bool_t canBeNeg = (key.Contains("mu",TString::kExact));
  
  // make canvas first
  auto TCanvas = new TCanvas("Canvas_"+key,"");
  canv->cd();
  canv->SetLogy(isLogy);

  const Float_t factor = (isLogy ? 1.5f : 1.05f);
  min = (min > 0.f ? (min / factor) : (min * factor));
  max = (max > 0.f ? (max * factor) : (max / factor));

  // set min, max
  DataHist->SetMinimum(min);
  DataHist->SetMaximum(max);

  // draw!
  DataHist->Draw("ep");
  BkgdHist->Draw("ep same");

  // make legend
  auto leg = new TLegend(0.75,0.75,0.825,0.92);
  leg->SetName("Legend_"+key);
  leg->SetBorderSize(1);
  leg->SetLineColor(kBlack);
  
  // add to legend
  leg->AddEntry(DataHist,"Data","epl");
  leg->AddEntry(BkgdHist,"MC","epl");

  // draw legend
  leg->Draw("same");
  
  // pretty up
  Common::CMSLumi(canv,0,fEra);

  // save output if lin
  if (!isLogy)
  {
    fOutFile->cd();
    leg->Write(leg->GetName(),TObject::kWriteDelete);
    canv->Write(canv->GetName(),TObject::kWriteDelete);
  }

  // delete all
  delete leg;
  delete canv;
}

void TimeFitter::PrepFit(TH1F *& hist1D, TF1 *& fit)
{
  // make tmp fit first
  auto tempfit = new TF1("temp","gaus(0)",fRangeLow,fRangeUp);
  tempfit->SetParLimits(2,0,10);
  hist1D->Fit("temp","RQ0B");
  const Float_t norm  = tempfit->GetParameter(0); // constant
  const Float_t mu    = tempfit->GetParameter(1); // mu
  const Float_t sigma = tempfit->GetParameter(2); // sigma
  delete tempfit;
  
  // names for fits and formulas
  const TString formname = Form("%s_form");
  const TString fitname  = Form("%s_fit");

  if (fFit == Gaus1)
  {
    TFormula form(formname.Data(),"[0]*exp(-0.5*((x-[1])/[2])**2)");
    fit  = new TF1(fitname.Data(),formname.Data(),fRangeLow,fRangeUp);
    fit->SetParameters(norm,mu,sigma);
    fit->SetParName(0,"norm");
    fit->SetParName(1,"mu");
    fit->SetParName(2,"sigma");
    fit->SetParLimits(2,0,10);
  }
  else if (fFit == Gaus1core)
  {
    TFormula form(formname.Data(),"[0]*exp(-0.5*((x-[1])/[2])**2)");
    const Float_t hmu    = hist1D->GetMean();
    const Float_t hsigma = hist1D->GetStdDev(); 
    
    fit  = new TF1(fitname.Data(),formname.Data(),hmu-2.f*hsigma,hmu+2.f*hsigma);
    fit->SetParameters(norm,mu,sigma);
    fit->SetParName(0,"norm");
    fit->SetParName(1,"mu");
    fit->SetParName(2,"sigma");
    fit->SetParLimits(2,0,10);
  }
  else if (fFit == Gaus2fm)
  {
    TFormula form(formname.Data(),"[0]*exp(-0.5*((x-[1])/[2])**2)+[3]*exp(-0.5*((x-[1])/[4])**2)");
    fit  = new TF1(fitname.Data(),formname.Data(),fRangeLow,fRangeUp);
    fit->SetParameters(norm,mu,sigma,norm/10,sigma*4);
    fit->SetParName(0,"norm1");
    fit->SetParName(1,"mu");
    fit->SetParName(2,"sigma1");
    fit->SetParName(3,"norm2");
    fit->SetParName(4,"sigma2");
    fit->SetParLimits(2,0,10);
    fit->SetParLimits(4,0,10);
  }
  else if (fFit == Gaus3fm)
  {
    TFormula form(formname.Data(),"[0]*exp(-0.5*((x-[1])/[2])**2)+[3]*exp(-0.5*((x-[1])/[4])**2)+[5]*exp(-0.5*((x-[1])/[6])**2)");
    fit  = new TF1(fitname.Data(),formname.Data(),fRangeLow,fRangeUp);
    fit->SetParName(0,"norm1");  fit->SetParameter(0,norm*0.8);  fit->SetParLimits(0,norm*0.5,norm);
    fit->SetParName(1,"mu");     fit->SetParameter(1,mu);
    fit->SetParName(2,"sigma1"); fit->SetParameter(2,sigma*0.7); fit->SetParLimits(2,sigma*0.5,sigma);
    fit->SetParName(3,"norm2");  fit->SetParameter(3,norm*0.3);  fit->SetParLimits(3,norm*0.1,norm*0.5);
    fit->SetParName(4,"sigma2"); fit->SetParameter(4,sigma*1.4); fit->SetParLimits(4,sigma,sigma*1.5);
    fit->SetParName(5,"norm3");  fit->SetParameter(5,norm*0.01); fit->SetParLimits(5,norm*0.005,norm*0.1);
    fit->SetParName(6,"sigma3"); fit->SetParameter(6,sigma*2.5); fit->SetParLimits(6,sigma*1.5,sigma*5.0);
  }
  else
  {
    std::cerr << "How did this happen?? Fit was not set for prepping fits! Exiting..." << std::endl;
    exit(1);
  }
}

void TimeFitter::GetFitResult(const TF1 * fit, FitResult & result)
{
  // get common results
  result.mu       = fit->GetParameter(1);
  result.emu      = fit->GetParError (1);
  result.chi2ndf  = fit->GetChisquare();
  result.chi2prob = fit->GetProb();

  if (fFit == Gaus1 || fFit == Gaus1core)
  {
    result.sigma  = fit->GetParameter(2);
    result.esigma = fit->GetParError (2);
  }
  else if (fFit == Gaus2fm)
  {
    const Float_t const1 = fit->GetParameter(0); 
    const Float_t const2 = fit->GetParameter(3);
    const Float_t denom  = const1 + const2;

    result.sigma  = (const1*fit->GetParameter(2)+const2*fit->GetParameter(4))/denom;
    result.esigma = std::hypot(const1*fit->GetParError(2),const2*fit->GetParError(4))/denom;
  }
  else if (fFit == Gaus3fm)
  {
    const Double_t const1 = fit->GetParameter(0); 
    const Double_t const2 = fit->GetParameter(3);
    const Double_t const3 = fit->GetParameter(5);
    const Double_t denom  = const1 + const2 + const3;
    
    result.sigma  = (const1*fit->GetParameter(2) + const2*fit->GetParameter(4) + const3*fit->GetParameter(6))/denom;
    result.esigma = std::hypot(std::hypot(const1*fit->GetParError(2),const2*fit->GetParError(4)),const3*fit->GetParError(6))/denom; // need c++17...
  }
  else
  {
    std::cerr << "How did this happen?? Fit was not set for getting results! Exiting..." << std::endl;
    exit(1);
  }
}

void TimeFitter::MakeConfigPave()
{
  std::cout << "Dumping config to a pave..." << std::endl;

  // create the pave, copying in old info
  fOutFile->cd();
  fConfigPave = new TPaveText();
  fConfigPave->SetName(Form("%s",Common::pavename.Data()));

  // give grand title
  fConfigPave->AddText("***** TimeFitter Config *****");

  // add era info
  Common::AddEraInfoToPave(fConfigPave,fEra);

  // dump time fit config
  Common::AddTextFromInputConfig(fConfigPave,"TimeFit Config",fTimeFitConfig);

  // dump plot config
  Common::AddTextFromInputConfig(fConfigPave,"Plot Config",fPlotConfig);

  // padding
  Common::AddPaddingToPave(fConfigPave,3);

  // save name of infile, redundant
  fConfigPave->AddText(Form("InFile name: %s",fInFileName.Data()));

  // dump in old config
  Common::AddTextFromInputPave(fConfigPave,fInFile);

  // save to output file
  fOutFile->cd();
  fConfigPave->Write(fConfigPave->GetName(),TObject::kWriteDelete);
}

void TimeFitter::SetupCommon() 
{
  std::cout << "Setting up Common..." << std::endl;

  Common::SetupEras();
  Common::SetupSamples();
  Common::SetupGroups();
  Common::SetupHistNames();
}
 
void TimeFitter::SetupPlotConfig()
{
  std::cout << "Reading plot config..." << std::endl;

  std::ifstream infile(Form("%s",fPlotConfig.Data()),std::ios::in);
  std::string str;
  while (std::getline(infile,str))
  {
    if (str.find("plot_title=") != std::string::npos)
    {
      fTitle = Common::RemoveDelim(str,"plot_title=");
    }
    else if (str.find("x_title=") != std::string::npos)
    {
      fXTitle = Common::RemoveDelim(str,"x_title=");
    }
    else if (str.find("x_bins=") != std::string::npos)
    {
      Bool_t tmp_bool = false;
      str = Common::RemoveDelim(str,"x_bins=");
      Common::SetupBins(str,fXBins,tmp_bool);
    }
    else if (str.find("y_title=") != std::string::npos)
    {
      fYTitle = Common::RemoveDelim(str,"y_title=");
    }
  }
}
 
void TimeFitter::SetupTimeFitConfig()
{
  std::cout << "Reading time fit config..." << std::endl;

  std::ifstream infile(Form("%s",fTimeFitConfig.Data()),std::ios::in);
  std::string str;
  while (std::getline(infile,str))
  {
    if (str == "") continue;
    else if (str.find("fit_type=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"fit_type=");
      if      (str.find("Gaus1")     != std::string::npos) fFit = Gaus1;
      if      (str.find("Gaus1core") != std::string::npos) fFit = Gaus1core;
      else if (str.find("Gaus2fm")   != std::string::npos) fFit = Gaus2fm;
      else if (str.find("Gaus3fm")   != std::string::npos) fFit = Gaus3fm;
      else
      {
	std::cerr << "Specified a non-supported fit type: " << str.c_str() << " ... Exiting..." << std::endl;
	exit(1);
      }
    }
    else if (str.find("range_low=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"range_low=");
      fRangeLow = std::atof(str.c_str());
    }
    else if (str.find("range_up=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"range_up=");
      fRangeUp = std::atof(str.c_str());
    }
    else 
    {
      std::cerr << "Aye... your fit config is messed up, try again! Offending line: " << str.c_str() << std::endl;
      std::cerr << "Offending line: " << str.c_str() << std::endl;
      exit(1);
    }
  }
}

TH1F * TimeFitter::SetupHist(const TString & ytitle, const TString & yextra, const TString & label)
{
  // get bins
  const auto xbins = &fXBins[0];

  // make new hist
  auto hist = new TH1F(label+"_"+yextra,fTitle+" "+ytitle+";"+fXTitle+";"+fTitle+" "+ytitle,fXbins.size()-1,xbins);
  hist->Sumw2();

  Color_t color = kBlack;
  if (label.Contains("Data",TString::kExact))
  {
    color = kRed;
  }
  else if (label.Contains("Bkgd",TString::kExact))
  {
    color = kBlue;
  }
  else
  {
    std::cerr << "How did this happen?? Specified neither Data nor Bkgd for hist! Exiting..." << std::endl;
    exit(1);
  }

  hist->SetLineColor(color);
  hist->SetaMarkerColor(color);

  return hist;
}

void TimeFitter::DeleteInfo(FitStruct & FitInfo)
{
  delete FitInfo.Hist2D;

  TimeFitter::DeleteMap(FitInfo.Hist1DMap);
  TimeFitter::DeleteMap(FitInfo.FitMap);
  TimeFitter::DeleteMap(FitInfo.ResultsMap);
}

template <typename T>
void TimeFitter::DeleteMap(T & Map)
{
  for (auto & Pair : Map) delete Pair.second;
}
