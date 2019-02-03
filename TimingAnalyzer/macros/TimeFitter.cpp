#include "TimeFitter.hh"
#include "TVirtualFitter.h"

TimeFitter::TimeFitter(const TString & infilename, const TString & plotconfig, const TString & miscconfig, const TString & timefitconfig,
		       const TString & era, const Bool_t savemetadata, const TString & outfiletext) :
  fInFileName(infilename), fPlotConfig(plotconfig), fMiscConfig(miscconfig), fTimeFitConfig(timefitconfig),
  fEra(era), fSaveMetaData(savemetadata), fOutFileText(outfiletext)
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
  std::cout << "Tidying up in destructor..." << std::endl;

  if (fSaveMetaData) delete fConfigPave;

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
  if (fSaveMetaData) TimeFitter::MakeConfigPave();

  // Dump mu's and sigma's into text file
  TimeFitter::DumpFitInfo(DataInfo,MCInfo);

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

  // Init time fits
  TimeFitter::InitTimeFits(FitInfo);

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
    if (key.EqualTo("sigma",TString::kExact)) TimeFitter::PrintCanvas(DataInfo,MCInfo,min,max,key,true);
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

void TimeFitter::InitTimeFits(FitStruct & FitInfo)
{
  const auto & label = FitInfo.label;
  std::cout << "Initializing TimeFitStructMap for: " << label.Data() << std::endl;
  
  // get inputs/outputs
  auto & TimeFitStructMap = FitInfo.TimeFitStructMap;

  // setup a time fit struct for each bin!
  for (auto ibinX = 1; ibinX <= fNBinsX; ibinX++)
  {
    TimeFitStructMap[ibinX] = new TimeFitStruct(fTimeFitType,fRangeLow,fRangeUp);
  }
}

void TimeFitter::Project2Dto1DHists(FitStruct & FitInfo)
{
  const auto & label = FitInfo.label;
  std::cout << "Projecting to 1D from 2D plot: " << label.Data() << std::endl;
  
  // get inputs/outputs
  const auto & Hist2D = FitInfo.Hist2D;
  auto & TimeFitStructMap = FitInfo.TimeFitStructMap;
  const TString histname = Hist2D->GetName();

  for (auto ibinX = 1; ibinX <= fNBinsX; ibinX++)
  {
    auto & hist = TimeFitStructMap[ibinX]->hist;
    hist = (TH1F*)Hist2D->ProjectionY(Form("%s_ibin%i",histname.Data(),ibinX),ibinX,ibinX);
  }
}

void TimeFitter::Fit1DHists(FitStruct & FitInfo)
{
  const auto & label = FitInfo.label;
  std::cout << "Fitting hists for: " << label.Data() << std::endl;
  
  // get inputs/outputs
  auto & TimeFitStructMap = FitInfo.TimeFitStructMap;

  for (auto ibinX = 1; ibinX <= fNBinsX; ibinX++)
  {
    // get pair input
    auto & TimeFit = TimeFitStructMap[ibinX];

    // get hist, and skip if no entries
    if (TimeFit->isEmpty()) continue;

    // Prep the fit
    TimeFit->PrepFit();
    
    // do the fit!
    TimeFit->DoFit();

    // save output
    fOutFile->cd();
    TimeFit->hist->Write(TimeFit->hist->GetName(),TObject::kWriteDelete);
    TimeFit->form->Write(TimeFit->form->GetName(),TObject::kWriteDelete);
    TimeFit->fit->Write(TimeFit->fit->GetName(),TObject::kWriteDelete);
  }
}

void TimeFitter::ExtractFitResults(FitStruct & FitInfo)
{
  const auto & label = FitInfo.label;
  std::cout << "Extracting results for: " << label.Data() << std::endl;

  // get inputs/outputs
  auto & TimeFitStructMap = FitInfo.TimeFitStructMap;
  auto & ResultsMap = FitInfo.ResultsMap;
  
  // setup hists
  ResultsMap["chi2ndf"]  = TimeFitter::SetupHist("#chi^{2}/NDF","chi2ndf",label);
  ResultsMap["chi2prob"] = TimeFitter::SetupHist("#chi^{2} Prob.","chi2prob",label);
  ResultsMap["mu"]       = TimeFitter::SetupHist(Form("#mu(%s) [ns]",fTimeText.Data()),"mu",label);
  ResultsMap["sigma"]    = TimeFitter::SetupHist(Form("#sigma(%s) [ns]",fTimeText.Data()),"sigma",label);

  // set bin content!
  for (auto ibinX = 1; ibinX <= fNBinsX; ibinX++)
  {
    // get time fit
    auto & TimeFit = TimeFitStructMap[ibinX];

    // skip if fit not present
    if (TimeFit->isEmpty()) continue;

    // get results
    TimeFit->GetFitResult();
    const auto & result = TimeFit->result;

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

  // get and set formula
  auto & form = FitInfo.SigmaForm;
  form = new TFormula(formname.Data(),Form("sqrt((([0]*[0])/(x*x))+(%s[1]*[1]))",fUseSqrt2?"2*":""));
    
  // get and set fit
  auto & fit = FitInfo.SigmaFit;
  fit = new TF1(fitname.Data(),form->GetName(),x_low,x_up);

  // init params
  fit->SetParName(0,"N"); fit->SetParameter(0,fSigmaInitN.val); fit->SetParLimits(0,fSigmaInitN.low,fSigmaInitN.up);
  fit->SetParName(1,"C"); fit->SetParameter(1,fSigmaInitC.val); fit->SetParLimits(1,fSigmaInitC.low,fSigmaInitC.up);

  // set line color
  fit->SetLineColor(hist->GetLineColor());

  // save form
  fOutFile->cd();
  form->Write(form->GetName(),TObject::kWriteDelete);
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

  // make canvas first
  auto Canvas = new TCanvas("Canvas_"+key,"");
  Canvas->cd();
  Canvas->SetGridx();
  Canvas->SetGridy();
  Canvas->SetLogy(isLogy);

  if (key.EqualTo("sigma",TString::kExact))
  {
    min = (isLogy ? 0.07f : 0.f);
    max = 1.f;
  }
  else
  {
    const Float_t factor = (isLogy ? 3.f : 1.5f);
    min = (min > 0.f ? (min / factor) : (min * factor));
    max = (max > 0.f ? (max * factor) : (max / factor));

    if (key.EqualTo("mu",TString::kExact))
    {
      if (min < -10.f) 
      {
	min = -1.f;
	if (max < min) max = 0.f;
      }
      if (max > 10.f)
      {
	max = 1.f;
	if (max < min) min = 0.f;
      }
    }
  }

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
    FitText = new TPaveText(0.52,0.605,0.82,0.925,"NDC");
    FitText->SetName("SigmaFitText");
    
    FitText->AddText(Form("#sigma(t)=#frac{N}{%s} #oplus %sC",fSigmaVarText.Data(),fUseSqrt2?"#sqrt{2}":""));
    FitText->AddText(Form("N^{%s} = %4.1f #pm %3.1f [%sns]",DataLabel.Data(),DataFit->GetParameter(0),DataFit->GetParError(0),fSigmaVarUnit.Data()));
    FitText->AddText(Form("C^{%s} = %6.4f #pm %6.4f [ns]"  ,DataLabel.Data(),DataFit->GetParameter(1),DataFit->GetParError(1)));
    FitText->AddText(Form("N^{%s} = %4.1f #pm %3.1f [%sns]",MCLabel  .Data(),MCFit  ->GetParameter(0),MCFit  ->GetParError(0),fSigmaVarUnit.Data()));
    FitText->AddText(Form("C^{%s} = %6.4f #pm %6.4f [ns]"  ,MCLabel  .Data(),MCFit  ->GetParameter(1),MCFit  ->GetParError(1)));
    FitText->SetTextAlign(11);
    FitText->SetFillColorAlpha(FitText->GetFillColor(),0);

    // draw text!
    FitText->Draw("same");
  }

  // make legend
  auto Legend = new TLegend(0.72,0.855,0.82,0.925);
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

void TimeFitter::GetMinMax(const TH1F * hist, Float_t & min, Float_t & max, const TString & key)
{
  for (auto ibinX = 1; ibinX <= fNBinsX; ibinX++)
  {
    const auto content = hist->GetBinContent(ibinX);
    
    // bool to allow negative values
    const Bool_t canBeNeg = (key.EqualTo("mu",TString::kExact));
    
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

void TimeFitter::DumpFitInfo(FitStruct & DataInfo, FitStruct & MCInfo)
{
  std::cout << "Dumping fit info into text file..." << std::endl;

  // get histograms!
  const auto & data_mu_hist = DataInfo.ResultsMap["mu"];
  const auto & mc_mu_hist   = MCInfo  .ResultsMap["mu"];

  const auto & data_sigma_hist = DataInfo.ResultsMap["sigma"];
  const auto & mc_sigma_hist   = MCInfo  .ResultsMap["sigma"];

  // make dumpfile object
  const TString filename = fOutFileText+Common::outFitText+"."+Common::outTextExt; 
  std::ofstream dumpfile(Form("%s",filename.Data()),std::ios_base::out);

  dumpfile << std::setw(5)  << "Bin |"
	   << std::setw(18) << "     pT range    |"
	   << std::setw(19) << "      Data mu     |"
	   << std::setw(19) << "       MC mu      |"
	   << std::setw(19) << "    Data sigma    |"
	   << std::setw(19) << "     MC sigma     |"
  	   << std::setw(17) << "    Diff sigma   "
	   << std::endl;
  
  std::string space = "";
  const auto nw = 5+18+19+19+19+19+17;
  for (auto i = 0; i < nw; i++) space += "-";

  dumpfile << space.c_str() << std::endl;

  for (auto ibinX = 1; ibinX <= fNBinsX; ibinX++)
  {
    const auto pt_low = fXBins[ibinX-1];
    const auto pt_up  = fXBins[ibinX];

    const auto data_mu   = data_mu_hist->GetBinContent(ibinX);
    const auto data_mu_e = data_mu_hist->GetBinError  (ibinX);
    const auto mc_mu     = mc_mu_hist  ->GetBinContent(ibinX);
    const auto mc_mu_e   = mc_mu_hist  ->GetBinError  (ibinX);

    const auto data_sigma   = data_sigma_hist->GetBinContent(ibinX);
    const auto data_sigma_e = data_sigma_hist->GetBinError  (ibinX);
    const auto mc_sigma     = mc_sigma_hist  ->GetBinContent(ibinX);
    const auto mc_sigma_e   = mc_sigma_hist  ->GetBinError  (ibinX);
    const auto diff_sigma   = std::sqrt(std::pow(data_sigma,2.f)-std::pow(mc_sigma,2.f));
    const auto diff_sigma_e = std::sqrt(std::pow(data_sigma*data_sigma_e/diff_sigma,2.f)+std::pow(mc_sigma*mc_sigma_e/diff_sigma,2.f));

    dumpfile << std::setw(5)  << Form("%i |",ibinX) 
	     << std::setw(17) << Form(" %6.1f - %6.1f |",pt_low,pt_up)
	     << std::setw(19) << Form(" %6.3f +/- %5.3f |",data_mu,data_mu_e)
	     << std::setw(19) << Form(" %6.3f +/- %5.3f |",mc_mu  ,mc_mu_e)
	     << std::setw(19) << Form(" %6.3f +/- %5.3f |",data_sigma,data_sigma_e)
	     << std::setw(19) << Form(" %6.3f +/- %5.3f |",mc_sigma  ,mc_sigma_e)
	     << std::setw(17) << Form(" %6.3f +/- %5.3f"  ,diff_sigma,diff_sigma_e)
	     << std::endl;

    if (ibinX % 20 == 0) dumpfile << space.c_str() << std::endl;
  }

  if (fDoSigmaFit)
  {
    // get fits!
    const auto & data_sigma_fit = DataInfo.SigmaFit;
    const auto & mc_sigma_fit   = MCInfo  .SigmaFit;

    dumpfile << space.c_str() << std::endl << std::endl;
    dumpfile << "Sigma Fit Results" << std::endl;
    dumpfile << std::setw(5)  << "Par |"
	     << std::setw(23) << "         Data         |"
	     << std::setw(23) << "          MC          |"
	     << std::endl;

    // loop over params and dump
    for (auto ipar = 0; ipar < data_sigma_fit->GetNpar(); ipar++)
    {
      // get constants
      const auto data_sigma_fit_par   = data_sigma_fit->GetParameter(ipar);
      const auto data_sigma_fit_par_e = data_sigma_fit->GetParError (ipar);
      const auto mc_sigma_fit_par     = mc_sigma_fit  ->GetParameter(ipar);
      const auto mc_sigma_fit_par_e   = mc_sigma_fit  ->GetParError (ipar);
      
      dumpfile << std::setw(5)  << Form("%s |",data_sigma_fit->GetParName(ipar)) 
	       << std::setw(23) << Form(" %8.3f +/- %7.3f |",data_sigma_fit_par,data_sigma_fit_par_e)
	       << std::setw(23) << Form(" %8.3f +/- %7.3f |",mc_sigma_fit_par  ,mc_sigma_fit_par_e)
	       << std::endl;
    }
  }
}

void TimeFitter::SetupDefaults()
{
  std::cout << "Setting up defaults for some params..." << std::endl;

  fDoLogX = false;
  fDoSigmaFit = false;
  fUseSqrt2 = false;
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
      fNBinsX = fXBins.size()-1;
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
      Common::SetupTimeFitType(str,fTimeFitType);
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
    else if (str.find("use_sqrt2=") != std::string::npos)
    {
      str = Common::RemoveDelim(str,"use_sqrt2=");
      Common::SetupBool(str,fUseSqrt2);
    }
    else if (str.find("sigma_var_text=") != std::string::npos)
    {
      fSigmaVarText = Common::RemoveDelim(str,"sigma_var_text=");
    }
    else if (str.find("sigma_var_unit=") != std::string::npos)
    {
      fSigmaVarUnit = Common::RemoveDelim(str,"sigma_var_unit=");

      // set to null character if none is specified
      if (fSigmaVarUnit.EqualTo("NONE",TString::kExact)) fSigmaVarUnit = "";
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
  auto hist = new TH1F(label+"_"+yextra,fTitle+" "+ytitle+";"+fXTitle+";"+ytitle,fNBinsX,xbins);
  hist->Sumw2();

  Color_t color = kBlack;
  if (label.EqualTo("Data",TString::kExact))
  {
    color = kBlue;
  }
  else if (label.EqualTo("MC",TString::kExact))
  {
    color = kRed;
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

  if (fDoSigmaFit) 
  {
    delete FitInfo.SigmaFit;
    delete FitInfo.SigmaForm;
  }

  Common::DeleteMap(FitInfo.ResultsMap);

  // loop over good bins to delete things
  auto & TimeFitStructMap = FitInfo.TimeFitStructMap;
  for (auto ibinX = 1; ibinX <= fNBinsX; ibinX++)
  {
    // get pair input
    auto & TimeFit = TimeFitStructMap[ibinX];

    // delete internal members
    TimeFit->DeleteInternal();

    // finally, delete the object itself
    delete TimeFit;
  }

  delete FitInfo.Hist2D;
}
