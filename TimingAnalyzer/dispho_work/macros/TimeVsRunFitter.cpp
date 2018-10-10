#include "TimeVsRunFitter.hh"
#include "TVirtualFitter.h"

TimeVsRunFitter::TimeVsRunFitter(const TString & infilename, const TString & plotconfig,
				 const TString & timefitconfig, const TString & outfiletext) :
  fInFileName(infilename), fPlotConfig(plotconfig), 
  fTimeFitConfig(timefitconfig), fOutFileText(outfiletext)
{
  std::cout << "Initializing TimeVsRunFitter..." << std::endl;

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
  TimeVsRunFitter::SetupCommon();
  TimeVsRunFitter::SetupPlotConfig();
  TimeVsRunFitter::SetupTimeFitConfig();

  // set fitter
  TVirtualFitter::SetDefaultFitter("Minuit2");
}

TimeVsRunFitter::~TimeVsRunFitter()
{
  std::cout << "Tidying up in destructor..." << std::endl;

  delete fConfigPave;

  Common::DeleteMap(ResultsMap);

  // loop over bins to delete things
  for (auto ibinX = 1; ibinX <= fNBinsX; ibinX++)
  {
    // get pair input
    auto & TimeFit = TimeFitStructMap[ibinX];

    // delete internal members
    TimeFit->DeleteInternal();

    // finally, delete the object itself
    delete TimeFit;
  }

  delete Hist2D;

  delete fOutFile;
  delete fTDRStyle;
  delete fInFile;
}

void TimeVsRunFitter::MakeTimeVsRunFits()
{
  std::cout << "Making time fits..." << std::endl;
  
  // Get input hist
  TimeVsRunFitter::GetInputHist();

  // Init time fits
  TimeFitter::InitTimeFits();

  // Project out 2D hists into map
  TimeVsRunFitter::Project2Dto1DHists();

  // Fit each 1D hist
  TimeVsRunFitter::Fit1DHists();

  // Extract mu and sigma into maps
  TimeVsRunFitter::ExtractFitResults();

  // Make Plots
  TimeVsRunFitter::MakePlots();

  // MakeConfigPave
  TimeVsRunFitter::MakeConfigPave();

  // Dump mu's and sigma's into text file
  TimeVsRunFitter::DumpFitInfo();
}

void TimeVsRunFitter::GetInputHist()
{
  std::cout << "Getting input hists..." << std::endl;
  
  // get the hist
  Hist2D = (TH2F*)fInFile->Get(Common::HistNameMap["Data"]);

  // save to output
  fOutFile->cd();
  Hist2D->Write(Hist2D->GetName(),TObject::kWriteDelete);
}

void TimeVsRunFitter::InitTimeFits()
{
  std::cout << "Initializing TimeFitStructMap..." << std::endl;
  
  // setup a time fit struct for each bin!
  for (auto ibinX = 1; ibinX <= fNBinsX; ibinX++)
  {
    TimeFitStructMap[ibinX] = new TimeFitStruct(fTimeFitType,fRangeLow,fRangeUp);
  }
}

void TimeVsRunFitter::Project2Dto1DHists()
{
  std::cout << "Projecting to 1D from 2D plot..." << std::endl;
  
  // get inputs/outputs
  const TString histname = Hist2D->GetName();

  for (auto ibinX = 1; ibinX <= fNBinsX; ibinX++)
  {
    auto & hist = TimeFitStructMap[ibinX]->hist;
    hist = (TH1F*)Hist2D->ProjectionY(Form("%s_ibin%i",histname.Data(),ibinX),ibinX,ibinX);
  }
}

void TimeVsRunFitter::Fit1DHists()
{
  std::cout << "Fitting hists..." << std::endl;
  
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

void TimeVsRunFitter::ExtractFitResults()
{
  std::cout << "Extracting results..." << std::endl;

  // setup hists
  ResultsMap["chi2ndf"]  = TimeVsRunFitter::SetupHist("#chi^{2}/NDF","chi2ndf",label);
  ResultsMap["chi2prob"] = TimeVsRunFitter::SetupHist("#chi^{2} Prob.","chi2prob",label);
  ResultsMap["mu"]       = TimeVsRunFitter::SetupHist(Form("#mu_{%s} [ns]",fTimeText.Data()),"mu",label);
  ResultsMap["sigma"]    = TimeVsRunFitter::SetupHist(Form("#sigma_{%s} [ns]",fTimeText.Data()),"sigma",label);

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

void TimeVsRunFitter::MakePlots()
{
  std::cout << "Make overlay plots..." << std::endl;

  // make temp vector of hist key names
  std::vector<TString> keys = {"mu","sigma"}; // can add chi2prob and chi2ndf

  // loop over keys
  for (const auto & key : keys)
  {
    // get hist
    auto & DataHist = ResultsMap[key];

    // tmp max, min
    Float_t min =  1e9;
    Float_t max = -1e9;
    TimeVsRunFitter::GetMinMax(DataHist,min,max,key);

    // lin first, then log if applicable
    TimeVsRunFitter::PrintCanvas(min.max,key,false);
    if (key.EqualTo("sigma",TString::kExact)) TimeVsRunFitter::PrintCanvas(min,max,key,true);
  }
}

void TimeVsRunFitter::PrintCanvas(Float_t min, Float_t max, const TString & key, const Bool_t isLogy)
{
  std::cout << "Printing canvas for: " << key.Data() << " isLogy: " << Common::PrintBool(isLogy).Data() << std::endl;

  // get hists
  auto & DataHist = ResultsMap[key];

  // make canvas first
  auto Canvas = new TCanvas("Canvas_"+key,"");
  Canvas->cd();
  Canvas->SetGridx();
  Canvas->SetGridy();
  Canvas->SetLogy(isLogy);

  // set min, max
  if (key.EqualTo("sigma",TString::kExact))
  {
    min = (isLogy ? 0.1f : 0.f);
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

  // pretty up
  Common::CMSLumi(Canvas,0,"Full");

  // make images
  Common::SaveAs(Canvas,Form("%s_%s_%s",key.Data(),fOutFileText.Data(),(isLogy?"log":"lin")));

  // save output if lin
  if (!isLogy)
  {
    fOutFile->cd();
    Canvas->Write(Canvas->GetName(),TObject::kWriteDelete);
  }

  // delete all
  delete Canvas;
}

void TimeVsRunFitter::GetMinMax(const TH1F * hist, Float_t & min, Float_t & max, const TString & key)
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

void TimeVsRunFitter::MakeConfigPave()
{
  std::cout << "Dumping config to a pave..." << std::endl;

  // create the pave, copying in old info
  fOutFile->cd();
  fConfigPave = new TPaveText();
  fConfigPave->SetName(Form("%s",Common::pavename.Data()));

  // give grand title
  fConfigPave->AddText("***** TimeVsRunFitter Config *****");

  // add era info
  Common::AddEraInfoToPave(fConfigPave,"Full");

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

void TimeVsRunFitter::DumpFitInfo()
{
  std::cout << "Dumping fit info into text file..." << std::endl;

  // get histograms!
  const auto & data_mu_hist    = ResultsMap["mu"];
  const auto & data_sigma_hist = ResultsMap["sigma"];

  // make dumpfile object
  const TString filename = fOutFileText+Common::outFitText+"."+Common::outTextExt; 
  std::ofstream dumpfile(Form("%s",filename.Data()),std::ios_base::out);

  dumpfile << std::setw(5)  << "Bin |"
	   << std::setw(9)  << "   Run  |"
	   << std::setw(19) << "      Data mu     |"
	   << std::setw(18) << "    Data sigma    "
	   << std::endl;
  
  std::string space = "";
  const auto nw = 5+9+19+18;
  for (auto i = 0; i < nw; i++) space += "-";

  dumpfile << space.c_str() << std::endl;

  for (auto ibinX = 1; ibinX <= fNBinsX; ibinX++)
  {
    const auto run_low = fXBins[ibinX-1];
    const auto run_up  = fXBins[ibinX];
    const Int_t run = (run_up-run_low)/2.f;
    
    const auto data_mu   = data_mu_hist->GetBinContent(ibinX);
    const auto data_mu_e = data_mu_hist->GetBinError  (ibinX);

    const auto data_sigma   = data_sigma_hist->GetBinContent(ibinX);
    const auto data_sigma_e = data_sigma_hist->GetBinError  (ibinX);

    dumpfile << std::setw(5)  << Form("%i |",ibinX) 
	     << std::setw(9) << Form(" %6i |",run)
	     << std::setw(19) << Form(" %6.3f +/- %5.3f |",data_mu,data_mu_e)
	     << std::setw(19) << Form(" %6.3f +/- %5.3f |",data_sigma,data_sigma_e)
	     << std::endl;

    if (ibinX % 20 == 0) dumpfile << space.c_str() << std::endl;
  }
}

void TimeVsRunFitter::SetupCommon() 
{
  std::cout << "Setting up Common..." << std::endl;

  Common::SetupEras();
  Common::SetupSamples();
  Common::SetupGroups();
  Common::SetupHistNames();
}

void TimeVsRunFitter::SetupPlotConfig()
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
 
void TimeVsRunFitter::SetupTimeFitConfig()
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
    else
    {
      std::cerr << "Aye... your fit config is messed up, try again! Offending line: " << str.c_str() << std::endl;
      std::cerr << "Offending line: " << str.c_str() << std::endl;
      exit(1);
    }
  }
}

TH1F * TimeVsRunFitter::SetupHist(const TString & ytitle, const TString & yextra, const TString & label)
{
  // get bins
  const auto xbins = &fXBins[0];

  // make new hist
  auto hist = new TH1F(label+"_"+yextra,fTitle+" "+ytitle+";"+fXTitle+";"+ytitle,fNBinsX,xbins);
  hist->Sumw2();

  return hist;
}
