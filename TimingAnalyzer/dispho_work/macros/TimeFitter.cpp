#include "TimeFitter.hh"
#include "TVirtualFitter.h"

TimeFitter::TimeFitter(const TString & infilename, const TString & plotconfig, const TString & miscconfig,
		       const TString & timefitconfig, const TString & era, const TString & outfiletext) :
  fInFileName(infilename), fPlotConfig(plotconfig), fMiscConfig(miscconfig),
  fTimeFitConfig(timefitconfig), fEra(era), fOutFileText(outfiletext)
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
  TimeFitter::SetupDefaults();
  TimeFitter::SetupCommon();
  TimeFitter::SetupPlotConfig();
  TimeFitter::SetupMiscConfig();
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
  FitStruct MCInfo("MC",Common::BkgdHistName.Data());
  TimeFitter::MakeTimeFit(MCInfo);

  // Fit sigma plot if asked
  if (fDoSigmaFit)
  {
    TimeFitter::MakeSigmaFit(DataInfo);
    TimeFitter::MakeSigmaFit(MCInfo);
  }

  // Make Plots
  TimeFitter::MakePlots(DataInfo,MCInfo);

  // MakeConfigPave
  TimeFitter::MakeConfigPave();

  // Delete infos
  TimeFitter::DeleteInfo(DataInfo);
  TimeFitter::DeleteInfo(MCInfo);
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

void TimeFitter::MakeSigmaFit(FitStruct & FitInfo)
{
  const auto & label = FitInfo.label;
  std::cout << "Making sigma fit for: " << label.Data() << std::endl;
  
  // Prep sigma fit
  TimeFitter::PrepSigmaFit(FitInfo);

  // Fit sigma hist
  TimeFitter::FitSigmaHist(FitInfo);
}

void TimeFitter::MakePlots(FitStruct & DataInfo, FitStruct & MCInfo)
{
  std::cout << "Make overlay plots..." << std::endl;

  // make temp vector of hist key names
  std::vector<TString> keys = {"mu","sigma"}; // can add chi2prob and chi2ndf

  // loop over keys
  for (const auto & key : keys)
  {
    // get hists
    const auto & DataHist = DataInfo.ResultsMap[key];
    const auto & MCHist   = MCInfo  .ResultsMap[key];

    // tmp max, min
    Float_t min =  1e9;
    Float_t max = -1e9;
    
    TimeFitter::GetMinMax(DataHist,min,max,key);
    TimeFitter::GetMinMax(MCHist  ,min,max,key);

    // lin first, then log --> log disabled for now
    TimeFitter::PrintCanvas(DataInfo,MCInfo,min,max,key,false);
    //    TimeFitter::PrintCanvas(DataInfo,MCInfo,min,max,key,true);
  }
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

void TimeFitter::Project2Dto1DHists(FitStruct & FitInfo)
{
  const auto & label = FitInfo.label;
  std::cout << "Projecting to 1D from 2D plot: " << label.Data() << std::endl;
  
  // get inputs/outputs
  const auto & Hist2D = FitInfo.Hist2D;
  auto & Hist1DMap = FitInfo.Hist1DMap;
  const TString histname = Hist2D->GetName();

  for (auto ibinX = 1U; ibinX <= fXBins.size(); ibinX++)
  {
    auto & hist1D = Hist1DMap[ibinX];
    hist1D = (TH1F*)Hist2D->ProjectionY(Form("%s_ibin%i",histname.Data(),ibinX),ibinX,ibinX);
  }
}

void TimeFitter::Fit1DHists(FitStruct & FitInfo)
{
  const auto & label = FitInfo.label;
  std::cout << "Fitting hists for: " << label.Data() << std::endl;
  
  // get inputs/outputs
  auto & Hist1DMap = FitInfo.Hist1DMap;
  auto & FitMap    = FitInfo.FitMap;

  for (auto ibinX = 1U; ibinX <= fXBins.size(); ibinX++)
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
  ResultsMap["mu"]       = TimeFitter::SetupHist(Form("#mu_{%s} [ns]",fTimeText.Data()),"mu",label);
  ResultsMap["sigma"]    = TimeFitter::SetupHist(Form("#sigma_{%s} [ns]",fTimeText.Data()),"sigma",label);

  // set bin content!
  for (auto ibinX = 1U; ibinX <= fXBins.size(); ibinX++)
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

void TimeFitter::PrepSigmaFit(FitStruct & FitInfo)
{
  const auto & label = FitInfo.label;
  std::cout << "Prepping sigma fit for: " << label.Data() << std::endl;

  // get input hist
  const auto & hist = FitInfo.ResultsMap["sigma"];

  // get range
  const auto x_low = hist->GetXaxis()->GetBinLowEdge(hist->GetXaxis()->GetFirst());
  const auto x_up  = hist->GetXaxis()->GetBinUpEdge (hist->GetXaxis()->GetLast());

  // set names
  const TString histname = hist->GetName();
  const TString formname = histname+"_form";
  const TString fitname  = histname+"_fit";

  // set formula
  TFormula form(formname.Data(),Form("sqrt((([0]*[0])/(x*x))+(2*[1]*[1]))"));
    
  // get and set fit
  auto & fit = FitInfo.SigmaFit;
  fit = new TF1(fitname.Data(),formname.Data(),x_low,x_up);

  // init params
  fit->SetParameter(0,fSigmaInitN.val);
  fit->SetParLimits(0,fSigmaInitN.low,fSigmaInitN.up);
  fit->SetParameter(1,fSigmaInitC.val);
  fit->SetParLimits(1,fSigmaInitC.low,fSigmaInitC.up);

  // set line color
  fit->SetLineColor(hist->GetLineColor());
}

void TimeFitter::FitSigmaHist(FitStruct & FitInfo)
{
  const auto & label = FitInfo.label;
  std::cout << "Fitting sigma hist for: " << label.Data() << std::endl;
  
  // get inputs/outputs
  auto & hist = FitInfo.ResultsMap["sigma"];
  auto & fit  = FitInfo.SigmaFit;
  
  // and fit it
  hist->Fit(fit->GetName(),"RBQ0");

  // save to output
  fOutFile->cd();
  fit->Write(fit->GetName(),TObject::kWriteDelete);
}

void TimeFitter::PrintCanvas(FitStruct & DataInfo, FitStruct & MCInfo, Float_t min, Float_t max, 
			     const TString & key, const Bool_t isLogy)
{
  std::cout << "Printing canvas for: " << key.Data() << " isLogy: " << Common::PrintBool(isLogy).Data() << std::endl;

  // do sigma fit?
  const Bool_t doSigmaFit = (fDoSigmaFit && (key.EqualTo("sigma",TString::kExact)));
  
  // get hists
  auto & DataHist = DataInfo.ResultsMap[key];
  auto & MCHist   = MCInfo  .ResultsMap[key];

  // get labels
  const auto & DataLabel = DataInfo.label;
  const auto & MCLabel   = MCInfo  .label;

  // bool to allow negative values
  const Bool_t canBeNeg = (key.Contains("mu",TString::kExact));
  
  // make canvas first
  auto Canvas = new TCanvas("Canvas_"+key,"");
  Canvas->cd();
  Canvas->SetLogy(isLogy);

  const Float_t factor = (isLogy ? 3.f : 1.5f);
  min = (min > 0.f ? (min / factor) : (min * factor));
  max = (max > 0.f ? (max * factor) : (max / factor));

  // set min, max
  DataHist->SetMinimum(min);
  DataHist->SetMaximum(max);

  // draw!
  DataHist->Draw("ep");
  MCHist  ->Draw("ep same");

  // draw sigma fit
  TF1 * DataFit;
  TF1 * MCFit;
  TPaveText * FitText = 0;
  
  if (doSigmaFit)
  {
    DataFit = DataInfo.SigmaFit;
    MCFit   = MCInfo  .SigmaFit;

    // draw fits
    DataFit->Draw("same");
    MCFit  ->Draw("same");

    // setup output text
    FitText = new TPaveText(0.22,0.6,0.52,0.92,"NDC");
    FitText->SetName("SigmaFitText");
    
    FitText->AddText(Form("#sigma(t)=#frac{N}{%s} #oplus #sqrt{2}C",fSigmaVarText.Data()));
    FitText->AddText(Form("N^{%s} = %4.1f #pm %3.1f [%s ns]",DataLabel.Data(),DataFit->GetParameter(0),DataFit->GetParError(0),fSigmaVarUnit.Data()));
    FitText->AddText(Form("C^{%s} = %6.4f #pm %6.4f [ns]"   ,DataLabel.Data(),DataFit->GetParameter(1),DataFit->GetParError(1)));
    FitText->AddText(Form("N^{%s} = %4.1f #pm %3.1f [%s ns]",MCLabel  .Data(),MCFit  ->GetParameter(0),MCFit  ->GetParError(0),fSigmaVarUnit.Data()));
    FitText->AddText(Form("C^{%s} = %6.4f #pm %6.4f [ns]"   ,MCLabel  .Data(),MCFit  ->GetParameter(1),MCFit  ->GetParError(1)));
    FitText->SetTextAlign(11);
    FitText->SetFillColorAlpha(FitText->GetFillColor(),0);

    // draw text!
    FitText->Draw("same");
  }

  // make legend
  auto Legend = new TLegend(0.72,0.85,0.82,0.92);
  Legend->SetName("Legend_"+key);
  Legend->SetBorderSize(1);
  Legend->SetLineColor(kBlack);
  
  // add to legend
  Legend->AddEntry(DataHist,DataInfo.label.Data(),"epl");
  Legend->AddEntry(MCHist  ,MCInfo  .label.Data(),"epl");

  // draw legend
  Legend->Draw("same");
  
  // pretty up
  Common::CMSLumi(Canvas,0,fEra);

  // make images
  Common::SaveAs(Canvas,Form("%s_%s_%s",key.Data(),fOutFileText.Data(),(isLogy?"log":"lin")));

  // do log-x?
  if (fDoLogX)
  {
    Canvas->cd();
    Canvas->SetLogx();

    // make images
    Common::SaveAs(Canvas,Form("%s_%s_%s_logx",key.Data(),fOutFileText.Data(),(isLogy?"log":"lin")));
  }

  // save output if lin
  if (!isLogy)
  {
    fOutFile->cd();
    Legend->Write(Legend->GetName(),TObject::kWriteDelete);
    Canvas->Write(Canvas->GetName(),TObject::kWriteDelete);
    if (doSigmaFit)
    {
      FitText->Write(FitText->GetName(),TObject::kWriteDelete);
    }
  }

  // delete all
  if (doSigmaFit)
  {
    delete FitText;
  }
  delete Legend;
  delete Canvas;
}

void TimeFitter::PrepFit(TH1F *& hist1D, TF1 *& fit)
{
  // Word on fit notation
  // "GausN" == N Gaussians fit
  // "fm" == "fixed mean", i.e. for N Gaussian fit, all Gaussians share the same mu
  // "core" == mid point of range of fit is mean of the histogram, range is n times the std. dev of hist

  // set tmp init vals
  Float_t norm  = hist1D->Integral(fXVarBins?"width":"") / Common::SqrtPI;
  Float_t mu    = hist1D->GetMean();
  Float_t sigma = hist1D->GetStdDev(); 

  // range vars
  auto rangelow = 0.f;
  auto rangeup  = 0.f;

  // make tmp fit first if not gausNcore, set range
  if (fFit == Gaus1 || fFit == Gaus2fm || fFit == Gaus3fm)
  {
    // set range for tmp and main fit
    rangelow = fRangeLow;
    rangeup  = fRangeUp;

    TFormula tmp_formula("tmp_formula","[0]*exp(-0.5*((x-[1])/[2])**2)");
    auto tmp_fit = new TF1("tmp_fit",tmp_formula.GetName(),rangelow,rangeup);

    tmp_fit->SetParameter(0,norm);
    tmp_fit->SetParameter(1,mu);
    tmp_fit->SetParameter(2,sigma); tmp_fit->SetParLimits(2,0,10);

    // fit hist with tmp tf1
    hist1D->Fit(tmp_fit->GetName(),"RBQ0");

    norm  = tmp_fit->GetParameter(0); // constant
    mu    = tmp_fit->GetParameter(1); // mu
    sigma = tmp_fit->GetParameter(2); // sigma

    delete tmp_fit;
  }
  else // "core" fits
  {
    // set range for main fit
    rangelow = (mu-(fRangeLow*sigma));
    rangeup  = (mu+(fRangeUp *sigma));
  }
  
  // names for fits and formulas
  const TString histname = hist1D->GetName();
  const TString formname = histname+"_formula";
  const TString fitname  = histname+"_fit";

  if (fFit == Gaus1 || fFit == Gaus1core)
  {
    TFormula formula(formname.Data(),"[0]*exp(-0.5*((x-[1])/[2])**2)");
    fit = new TF1(fitname.Data(),formname.Data(),rangelow,rangeup);

    fit->SetParName(0,"N");      fit->SetParameter(0,norm);
    fit->SetParName(1,"#mu");    fit->SetParameter(1,mu);
    fit->SetParName(2,"#sigma"); fit->SetParameter(2,sigma); fit->SetParLimits(2,0,10);
  }
  else if (fFit == Gaus2fm || fFit == Gaus2fmcore)
  {
    TFormula formula(formname.Data(),"[0]*exp(-0.5*((x-[1])/[2])**2)+[3]*exp(-0.5*((x-[1])/[4])**2)");
    fit  = new TF1(fitname.Data(),formname.Data(),rangelow,rangeup);

    fit->SetParName(0,"N_{1}");      fit->SetParameter(0,norm);
    fit->SetParName(1,"#mu");        fit->SetParameter(1,mu);
    fit->SetParName(2,"#sigma_{1}"); fit->SetParameter(2,sigma);   fit->SetParLimits(2,0,10);
    fit->SetParName(3,"N_{2}");      fit->SetParameter(3,norm/10);
    fit->SetParName(4,"#sigma_{2}"); fit->SetParameter(4,sigma*4); fit->SetParLimits(4,0,10);
  }
  else if (fFit == Gaus3fm || fFit == Gaus3fmcore)
  {
    TFormula formula(formname.Data(),"[0]*exp(-0.5*((x-[1])/[2])**2)+[3]*exp(-0.5*((x-[1])/[4])**2)+[5]*exp(-0.5*((x-[1])/[6])**2)");
    fit  = new TF1(fitname.Data(),formname.Data(),rangelow,rangeup);

    fit->SetParName(0,"N_{1}");      fit->SetParameter(0,norm*0.8);  fit->SetParLimits(0,norm*0.5,norm);
    fit->SetParName(1,"#mu");        fit->SetParameter(1,mu);
    fit->SetParName(2,"#sigma_{1}"); fit->SetParameter(2,sigma*0.7); fit->SetParLimits(2,sigma*0.5,sigma);
    fit->SetParName(3,"N_{2}");      fit->SetParameter(3,norm*0.3);  fit->SetParLimits(3,norm*0.1,norm*0.5);
    fit->SetParName(4,"#sigma_{2}"); fit->SetParameter(4,sigma*1.4); fit->SetParLimits(4,sigma,sigma*1.5);
    fit->SetParName(5,"N_{3}");      fit->SetParameter(5,norm*0.01); fit->SetParLimits(5,norm*0.005,norm*0.1);
    fit->SetParName(6,"#sigma_{3}"); fit->SetParameter(6,sigma*2.5); fit->SetParLimits(6,sigma*1.5,sigma*5.0);
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
  else if (fFit == Gaus2fm || fFit == Gaus2fmcore)
  {
    const Float_t const1 = fit->GetParameter(0); 
    const Float_t const2 = fit->GetParameter(3);
    const Float_t denom  = const1 + const2;

    result.sigma  = (const1*fit->GetParameter(2)+const2*fit->GetParameter(4))/denom;
    result.esigma = std::hypot(const1*fit->GetParError(2),const2*fit->GetParError(4))/denom;
  }
  else if (fFit == Gaus3fm || fFit == Gaus3fmcore)
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

void TimeFitter::GetMinMax(const TH1F * hist, Float_t & min, Float_t & max, const TString & key)
{
  for (auto ibinX = 1U; ibinX < fXBins.size(); ibinX++)
  {
    const auto content = hist->GetBinContent(ibinX);
    
    // bool to allow negative values
    const Bool_t canBeNeg = (key.Contains("mu",TString::kExact));
    
    if ( ((!canBeNeg && min > 0.f) || (canBeNeg)) && content < min) min = content;
    if ( ((!canBeNeg && max > 0.f) || (canBeNeg)) && content > max) max = content;
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

  // dump misc config
  Common::AddTextFromInputConfig(fConfigPave,"Misc Config",fMiscConfig);

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

void TimeFitter::SetupDefaults()
{
  std::cout << "Setting up defaults for some params..." << std::endl;

  fDoLogX = false;
  fDoSigmaFit = false;
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
      str = Common::RemoveDelim(str,"x_bins=");
      Common::SetupBins(str,fXBins,fXVarBins);
    }
    else if (str.find("y_title=") != std::string::npos)
    {
      fYTitle = Common::RemoveDelim(str,"y_title=");
    }
  }
}

void TimeFitter::SetupMiscConfig()
{
  std::cout << "Reading misc config..." << std::endl;

  std::ifstream infile(Form("%s",fMiscConfig.Data()),std::ios::in);
  std::string str;
  while (std::getline(infile,str))
  {
    if (str == "") continue;
    else if (str.find("do_logx=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"do_logx=");
      Common::SetupBool(str,fDoLogX);
    }
    else
    {
      std::cerr << "Aye... your fit config is messed up, try again! Offending line: " << str.c_str() << std::endl;
      std::cerr << "Offending line: " << str.c_str() << std::endl;
      exit(1);
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
      if      (str.find("Gaus1")       != std::string::npos) fFit = Gaus1;
      if      (str.find("Gaus1core")   != std::string::npos) fFit = Gaus1core;
      else if (str.find("Gaus2fm")     != std::string::npos) fFit = Gaus2fm;
      else if (str.find("Gaus2fmcore") != std::string::npos) fFit = Gaus2fmcore;
      else if (str.find("Gaus3fm")     != std::string::npos) fFit = Gaus3fm;
      else if (str.find("Gaus3fmcore") != std::string::npos) fFit = Gaus3fmcore;
      else
      {
	std::cerr << "Specified a non-supported fit type: " << str.c_str() << " ... Exiting..." << std::endl;
	exit(1);
      }
    }
    else if (str.find("range_low=") != std::string::npos) // if "core" = how many sigma from mean down, else absolute low edge of fit
    {
      str = Common::RemoveDelim(str,"range_low=");
      fRangeLow = std::atof(str.c_str());
    }
    else if (str.find("range_up=") != std::string::npos) // if "core" = how many sigma from mean up, else absolute up edge of fit
    {
      str = Common::RemoveDelim(str,"range_up=");
      fRangeUp = std::atof(str.c_str());
    }
    else if (str.find("time_text=") != std::string::npos)
    {
      fTimeText = Common::RemoveDelim(str,"time_text=");
    }
    else if (str.find("do_sigma_fit=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"do_sigma_fit=");
      Common::SetupBool(str,fDoSigmaFit);
    }
    else if (str.find("sigma_var_text=") != std::string::npos)
    {
      fSigmaVarText = Common::RemoveDelim(str,"sigma_var_text=");
    }
    else if (str.find("sigma_var_unit=") != std::string::npos)
    {
      fSigmaVarUnit = Common::RemoveDelim(str,"sigma_var_unit=");
    }
    else if (str.find("sigma_init_N_params=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"sigma_init_N_params=");
      TimeFitter::ReadInitParams(str,fSigmaInitN);
    }
    else if (str.find("sigma_init_C_params=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"sigma_init_C_params=");
      TimeFitter::ReadInitParams(str,fSigmaInitC);
    }
    else
    {
      std::cerr << "Aye... your fit config is messed up, try again! Offending line: " << str.c_str() << std::endl;
      std::cerr << "Offending line: " << str.c_str() << std::endl;
      exit(1);
    }
  }
}

void TimeFitter::ReadInitParams(const std::string & str, SigmaFitParams & params)
{
  std::stringstream ss(str);
  ss >> params.low >> params.val >> params.up;
}

TH1F * TimeFitter::SetupHist(const TString & ytitle, const TString & yextra, const TString & label)
{
  // get bins
  const auto xbins = &fXBins[0];

  // make new hist
  auto hist = new TH1F(label+"_"+yextra,fTitle+" "+ytitle+";"+fXTitle+";"+ytitle,fXBins.size()-1,xbins);
  hist->Sumw2();

  Color_t color = kBlack;
  if (label.Contains("Data",TString::kExact))
  {
    color = kRed;
  }
  else if (label.Contains("MC",TString::kExact))
  {
    color = kBlue;
  }
  else
  {
    std::cerr << "How did this happen?? Specified neither Data nor MC for hist! Exiting..." << std::endl;
    exit(1);
  }

  hist->SetLineColor(color);
  hist->SetMarkerColor(color);

  return hist;
}

void TimeFitter::DeleteInfo(FitStruct & FitInfo)
{
  const auto & label = FitInfo.label;
  std::cout << "Deleting info for: " << label.Data() << std::endl;

  delete FitInfo.Hist2D;

  if (fDoSigmaFit) delete FitInfo.SigmaFit;

  TimeFitter::DeleteMap(FitInfo.Hist1DMap);
  TimeFitter::DeleteMap(FitInfo.FitMap);
  TimeFitter::DeleteMap(FitInfo.ResultsMap);
}

template <typename T>
void TimeFitter::DeleteMap(T & Map)
{
  for (auto & Pair : Map) delete Pair.second;
}
