#include "../interface/CommonTypes.hh"
#include "../interface/Config.hh"
#include "../interface/Common.hh"
#include "../interface/PUReweight.hh"
#include "../interface/Analysis.hh"
#include "../interface/StackPlots.hh"

#include "TROOT.h"
#include "TVirtualFitter.h"

void setUpPlotMaps()
{
  // read in from config all the appropriate bins
  TStrVec effstrings = {"inclusive","EBEB","EEEE","EPEP","EMEM"};
  for (int i = 0; i < effstrings.size(); i++) 
  {
    std::ifstream ineffseedE;
    ineffseedE.open(Form("config/effseedE_%s_bins.txt",effstrings[i].Data()),std::ios::in);
    Int_t  effseedE = -1;
    while(ineffseedE >> effseedE)
    {
      Config::XBinsMap[Form("effseedE_%s",effstrings[i].Data())].push_back(effseedE);
    }
    ineffseedE.close();
  }

  Config::XTitleMap["E"]  = "Energy [GeV]";
  
  // sigma_n corrections!
  if (Config::useSigma_n) 
  {
    for (TStrDblVMap::iterator viter = Config::XBinsMap.begin(); viter != Config::XBinsMap.end(); ++viter)
    {
      for (int i = 0; i < (*viter).second.size(); i++) 
      {
	TString label = (*viter).first;
	if      (label.Contains("EB",TString::kExact)) {(*viter).second[i] /= Config::sigma_nEB;}
	else if (label.Contains("EE",TString::kExact)) {(*viter).second[i] /= Config::sigma_nEE;}
	else if (label.Contains("EM",TString::kExact)) {(*viter).second[i] /= Config::sigma_nEE;}
	else if (label.Contains("EP",TString::kExact)) {(*viter).second[i] /= Config::sigma_nEE;}
      }
    }
    Config::XTitleMap["E"]  = "Energy/#sigma_{n}";
  }

  if (Config::dumpRanges) 
  {
    std::ofstream range;
    range.open(Form("%s/ranges.txt",Config::outdir.Data()),std::ios_base::trunc);
    for (TStrDblVMap::iterator viter = Config::XBinsMap.begin(); viter != Config::XBinsMap.end(); ++viter) 
    {
      range << (*viter).first << std::endl;
      for (int i = 0; i < (*viter).second.size()-1; i++) 
      {
	range << "  " << (*viter).second[i] << "-" << (*viter).second[i+1] << std::endl;
      }
      range << "------------------------------------" << std::endl;
    }
    range.close();
  }
}

void InitializeMain(std::ofstream & yields, TStyle *& tdrStyle) 
{
  // set TDR Style (need to force it!)
  tdrStyle = new TStyle("tdrStyle","Style for P-TDR");
  SetTDRStyle(tdrStyle);
  gROOT->ForceStyle();

  // set the minimizer
  TVirtualFitter::SetDefaultFitter("Minuit2");

  // have to do this at least once!
  MakeOutDir(Config::outdir);

  // yields 
  yields.open(Form("%s/yields.txt",Config::outdir.Data()),std::ios_base::app);

  // set sample map
  if (Config::useDEG)   Config::SampleMap["doubleeg"] = false; // !isMC
  if (Config::useSEL)   Config::SampleMap["singleel"] = false; // !isMC
  if (Config::useDYll)  Config::SampleMap["dyll"]     = true;  //  isMC
  if (Config::useQCD)   Config::SampleMap["qcd"]      = true;  //  isMC
  if (Config::useGJets) Config::SampleMap["gamma"]    = true;  //  isMC

  // Color for MC Stacks
  Config::colorMap["dyll"]  = kGreen-6;
  Config::colorMap["qcd"]   = kYellow;
  Config::colorMap["gamma"] = kOrange+10;

  // define title map
  Config::SampleTitleMap["dyll"]  = "Z #rightarrow l^{+}l^{-}";
  Config::SampleTitleMap["qcd"]   = "QCD";
  Config::SampleTitleMap["gamma"] = "#gamma + Jets";

  // sample xsec map
  Config::SampleXsecMap["dyll"] = 5943.2; //6025.2; //6104. -- still do not understand how monoj got this number
  // multiply by 1000 to get to pb;
  for (TStrFltMapIter mapiter = Config::SampleXsecMap.begin(); mapiter != Config::SampleXsecMap.end(); ++mapiter) 
  {
    (*mapiter).second *= 1000.;
  }
  
  // sample wgtsum map
  Config::SampleWgtsumMap["dyll"] = 1.59464e+12;

  // X axis title stuff for dividing by sigma_n (assume p+E have same magnitude)
  setUpPlotMaps();

  // set up time res config
  if (Config::doTimeRes) 
  {
    Config::doEffE    = true;
    Config::doNvtx    = true;
    Config::doVtxZ    = true;
  }
}

void DestroyMain(std::ofstream & yields, TStyle *& tdrStyle) 
{
  yields.close();
  delete tdrStyle;
}

int main(int argc, const char* argv[]) 
{
  //////////////////////////////////
  // Read in command line options //
  //////////////////////////////////
  lStr_t mArgs; 
  for (int i = 1; i < argc; ++i) { mArgs.push_back(argv[i]); }
  lStr_i i  = mArgs.begin();
  while (i != mArgs.end()) 
  {
    lStr_i start = i;
    if (*i == "-h" || *i == "-help" || *i == "--help") 
    {
      printf(
        "Usage: %s [options]\n"
        "Options:\n"
	"  --outdir        <string>      name of ouput directory (def: %s)\n"
	"  --dump-ranges   <bool>        dump energy bin ranges (def: %s)\n"
	"  --do-purw       <bool>        calculate pile-up weights (def: %s)\n"
	"  --do-analysis   <bool>        make analysis plots (def: %s)\n"
	"  --do-stacks     <bool>        stack data/MC plots (def: %s)\n"
	"  --do-demo       <bool>        demo analysis (def: %s)\n"
	"  --use-DEG       <bool>        use doubleEG for data (def: %s)\n"
	"  --use-SEL       <bool>        use singleEl for data (def: %s)\n"
	"  --use-DYll      <bool>        use Drell-Yan (LL+Jets) with MC (def: %s)\n"
	"  --use-QCD       <bool>        use QCD with MC (def: %s)\n"
	"  --use-GJets     <bool>        use Gamma+Jets with MC (def: %s)\n"
	"  --do-standard   <bool>        make standard validation plots (def: %s)\n"
	"  --do-timeres    <bool>        make timing bias and resolution plots [all, except runs] (def: %s)\n"
	"  --do-effE       <bool>        make timing bias and resolution plots vs effective energy variables (def: %s)\n"
	"  --do-nvtx       <bool>        make timing bias and resolution plots vs nVertices (def: %s)\n"
	"  --do-eta        <bool>        make timing bias and resolution plots vs single electron eta (def: %s)\n"
	"  --do-vtxZ       <bool>        make timing bias and resolution plots vs z vertex position (def: %s)\n"
	"  --do-runs       <bool>        make timing bias and resolution plots vs run number [data only] (def: %s)\n"
	"  --do-trigeff    <bool>        make trigger efficiency plots (def: %s)\n"
	"  --apply-TOF     <bool>        apply TOF correction to times (def: %s)\n"
	"  --wgt-time      <bool>        use full SC weighted time (def: %s)\n"
	"  --use-sigman    <bool>        divide by sigma_n in E/pT plots (def: %s)\n"
	"  --save-fits     <bool>        save copies of fits as pngs (def: %s)\n"
	"  --in-year       <string>      which year to process (def: %s)\n"
	"  --fit-form      <string>      name of formula used for fitting time plots (def: %s)\n"
	"  --out-image     <string>      extension of file to save plots (def: %s)\n"
        ,
        argv[0],
        Config::outdir.Data(),
	(Config::dumpRanges ? "true" : "false"),
	(Config::doPURW     ? "true" : "false"),
	(Config::doAnalysis ? "true" : "false"),
	(Config::doStacks   ? "true" : "false"),
	(Config::doDemo     ? "true" : "false"),
	(Config::useDEG     ? "true" : "false"),
	(Config::useSEL     ? "true" : "false"),
	(Config::useDYll    ? "true" : "false"),
	(Config::useQCD     ? "true" : "false"),
	(Config::useGJets   ? "true" : "false"),
	(Config::doStandard ? "true" : "false"),
	(Config::doTimeRes  ? "true" : "false"),
	(Config::doEffE     ? "true" : "false"),
	(Config::doNvtx     ? "true" : "false"),
	(Config::doEta      ? "true" : "false"),
	(Config::doVtxZ     ? "true" : "false"),
	(Config::doRuns     ? "true" : "false"),
	(Config::doTrigEff  ? "true" : "false"),
	(Config::applyTOF   ? "true" : "false"),
	(Config::wgtedTime  ? "true" : "false"),
	(Config::useSigma_n ? "true" : "false"),
	(Config::saveFits   ? "true" : "false"),
	Config::year.Data(),
        Config::formname.Data(),
	Config::outtype.Data()
      );
      exit(0);
    }
    else if (*i == "--outdir")      { next_arg_or_die(mArgs, i); Config::outdir = i->c_str(); }
    else if (*i == "--dump-ranges") { Config::dumpRanges = true; }
    else if (*i == "--do-purw")     { Config::doPURW     = true; }
    else if (*i == "--do-analysis") { Config::doAnalysis = true; }
    else if (*i == "--do-stacks")   { Config::doStacks   = true; }
    else if (*i == "--do-demo")     { Config::doDemo     = true; Config::doAnalysis = true; Config::doStandard = true; Config::doTimeRes = true; }
    else if (*i == "--use-DEG")     { Config::useDEG     = true; }
    else if (*i == "--use-SEL")     { Config::useSEL     = true; }
    else if (*i == "--use-DYll")    { Config::useDYll    = true; }
    else if (*i == "--use-QCD")     { Config::useQCD     = true; }
    else if (*i == "--use-GJets")   { Config::useGJets   = true; }
    else if (*i == "--do-standard") { Config::doAnalysis = true; Config::doStandard = true; }
    else if (*i == "--do-timeres")  { Config::doAnalysis = true; Config::doTimeRes  = true; }
    else if (*i == "--do-effE")     { Config::doAnalysis = true; Config::doEffE     = true; }
    else if (*i == "--do-nvtx")     { Config::doAnalysis = true; Config::doNvtx     = true; }
    else if (*i == "--do-eta")      { Config::doAnalysis = true; Config::doEta      = true; }
    else if (*i == "--do-vtxZ")     { Config::doAnalysis = true; Config::doVtxZ     = true; }
    else if (*i == "--do-runs")     { Config::doAnalysis = true; Config::doRuns     = true; }
    else if (*i == "--do-trigeff")  { Config::doAnalysis = true; Config::doTrigEff  = true; }
    else if (*i == "--apply-TOF")   { Config::doAnalysis = true; Config::applyTOF   = true; }
    else if (*i == "--wgt-time")    { Config::doAnalysis = true; Config::wgtedTime  = true; Config::useSigma_n = true; }
    else if (*i == "--use-sigman")  { Config::doAnalysis = true; Config::useSigma_n = true; }
    else if (*i == "--save-fits")   { Config::doAnalysis = true; Config::saveFits   = true; }
    else if (*i == "--in-year")     { next_arg_or_die(mArgs, i); Config::year     = i->c_str(); }
    else if (*i == "--fit-form")    { next_arg_or_die(mArgs, i); Config::formname = i->c_str(); }
    else if (*i == "--out-image")   { next_arg_or_die(mArgs, i); Config::outtype  = i->c_str(); }
    else    { std::cerr << "Error: Unknown option/argument: " << i->c_str() << " ...exiting..." << std::endl; exit(1); }
    mArgs.erase(start, ++i);
  }

  ////////////////////
  // Initialization //
  ////////////////////

  // do this thing at least once!
  std::ofstream yields; TStyle * tdrStyle; 
  InitializeMain(yields,tdrStyle);

  /////////////////////////
  // Pile-up reweighting //
  /////////////////////////

  if (Config::doPURW) 
  {
    std::cout << "Calculating private pile-up weights" << std::endl;
    PUReweight reweight;
    reweight.GetPUWeights();
    std::cout << "Finished calculating pile-up weights" << std::endl;
  }
  else 
  {
    std::cout << "Skipping calculating pile-up weights" << std::endl;
  }
  
  ///////////////////
  // Main Analysis //
  ///////////////////

  if (Config::doAnalysis) 
  {
    std::cout << "Starting analyis section" << std::endl;
    for (TStrBoolMapIter mapiter = Config::SampleMap.begin(); mapiter != Config::SampleMap.end(); ++mapiter) 
    {
      Analysis analysis((*mapiter).first,(*mapiter).second);
      std::cout << "Analyzing: " << ((*mapiter).second?"MC":"DATA") << " sample: " << (*mapiter).first << std::endl;
      analysis.EventLoop();
      std::cout << "Done analyzing: " << ((*mapiter).second?"MC":"DATA") << " sample: " << (*mapiter).first << std::endl;
    }
    std::cout << "Finished analysis section" << std::endl;
  }
  else 
  {
    std::cout << "Skipping analysis section" << std::endl;
  }

  ///////////////////
  // Stack data/mc //
  ///////////////////

  if (Config::doStacks) 
  {
    std::cout << "Starting stacker" << std::endl;
    StackPlots Stacker;
    Stacker.DoStacks(yields);
    std::cout << "Finished stacking plots" << std::endl;
  }
  else 
  {
    std::cout << "Skipping stacking data over MC" << std::endl;
  }

  // end of the line
  DestroyMain(yields,tdrStyle);
}
