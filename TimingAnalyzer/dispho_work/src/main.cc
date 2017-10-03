#include "../interface/CommonTypes.hh"
#include "../interface/Config.hh"
#include "../interface/CommonUtils.hh"
#include "../interface/PUReweight.hh"
#include "../interface/Analysis.hh"
#include "../interface/StackPlots.hh"

#include "TROOT.h"
#include "TVirtualFitter.h"

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
  if (Config::useSPH)   Config::SampleMap["singleph"] = false; // !isMC
  if (Config::useGMSB)  Config::SampleMap["gmsb"]     = true;  //  isMC
  if (Config::useHVDS)  Config::SampleMap["hvds"]     = true;  //  isMC
  if (Config::useQCD)   Config::SampleMap["qcd"]      = true;  //  isMC
  if (Config::useGJets) Config::SampleMap["gamma"]    = true;  //  isMC

  // Color for MC Stacks
  Config::colorMap["gmsb"]  = kBlue;
  Config::colorMap["hvds"]  = kRed;
  Config::colorMap["qcd"]   = kYellow;
  Config::colorMap["gamma"] = kOrange+10;

  // define title map
  Config::SampleTitleMap["gmsb"]  = "GMSB";
  Config::SampleTitleMap["hvds"]  = "HVDS";
  Config::SampleTitleMap["qcd"]   = "QCD";
  Config::SampleTitleMap["gamma"] = "#gamma + Jets";

  // sample xsec map
  Config::SampleXsecMap["gmsb"] = 1;
  Config::SampleXsecMap["hvds"] = 1;

  // sample wgtsum map
  Config::SampleWgtsumMap["gmsb"] = 1;
  Config::SampleWgtsumMap["hvds"] = 1;
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
	"  --do-purw       <bool>        calculate pile-up weights (def: %s)\n"
	"  --do-analysis   <bool>        make analysis plots (def: %s)\n"
	"  --do-stacks     <bool>        stack data/MC plots (def: %s)\n"
	"  --do-demo       <bool>        demo analysis (def: %s)\n"
	"  --use-DEG       <bool>        use doubleEG for data (def: %s)\n"
	"  --use-SPH       <bool>        use singlePh for data (def: %s)\n"
	"  --use-GMSB      <bool>        use GMSB with MC (def: %s)\n"
	"  --use-HVDS      <bool>        use HVDS with MC (def: %s)\n"
	"  --use-QCD       <bool>        use QCD with MC (def: %s)\n"
	"  --use-GJets     <bool>        use Gamma+Jets with MC (def: %s)\n"
	"  --do-standard   <bool>        make standard validation plots (def: %s)\n"
	"  --do-iso        <bool>        make isolation plots (def: %s)\n"
	"  --do-isonvtx    <bool>        make isolation vs nvtx plots (def: %s)\n"
	"  --dump-status   <bool>        print out every N events in analysis loop (def: %s)\n"
	"  --in-year       <string>      which year to process (def: %s)\n"
	"  --save-hists    <bool>        save analysis histograms as images (def: %s)\n"
	"  --save-tmphists <bool>        save histograms used in projections to root file (def: %s)\n"
	"  --out-image     <string>      extension of file to save plots (def: %s)\n"
        ,
        argv[0],
        Config::outdir.Data(),
	PrintBool(Config::doPURW),
	PrintBool(Config::doAnalysis),
	PrintBool(Config::doStacks),
	PrintBool(Config::doDemo),
	PrintBool(Config::useDEG),
	PrintBool(Config::useSPH),
	PrintBool(Config::useGMSB),
	PrintBool(Config::useHVDS),
	PrintBool(Config::useQCD),
	PrintBool(Config::useGJets),
	PrintBool(Config::doStandard),
	PrintBool(Config::doIso),
	PrintBool(Config::doIsoNvtx),
	PrintBool(Config::dumpStatus),
	Config::year.Data(),
	PrintBool(Config::saveHists),
	PrintBool(Config::saveTempHists),
	Config::outtype.Data()
      );
      exit(0);
    }
    else if (*i == "--outdir")      { next_arg_or_die(mArgs, i); Config::outdir = i->c_str(); }
    else if (*i == "--do-purw")     { Config::doPURW     = true; }
    else if (*i == "--do-analysis") { Config::doAnalysis = true; }
    else if (*i == "--do-stacks")   { Config::doStacks   = true; }
    else if (*i == "--do-demo")     { Config::doDemo     = true; Config::doAnalysis = true; Config::doStandard = true; }
    else if (*i == "--use-DEG")     { Config::useDEG     = true; }
    else if (*i == "--use-SPH")     { Config::useSPH     = true; }
    else if (*i == "--use-GMSB")    { Config::useGMSB    = true; }
    else if (*i == "--use-HVDS")    { Config::useHVDS    = true; }
    else if (*i == "--use-QCD")     { Config::useQCD     = true; }
    else if (*i == "--use-GJets")   { Config::useGJets   = true; }
    else if (*i == "--do-standard") { Config::doAnalysis = true; Config::doStandard = true; }
    else if (*i == "--do-iso")      { Config::doAnalysis = true; Config::doIso      = true; }
    else if (*i == "--do-isonvtx")  { Config::doAnalysis = true; Config::doIsoNvtx  = true; }
    else if (*i == "--dump-status") { Config::doAnalysis = true; Config::dumpStatus = true; }
    else if (*i == "--in-year")     { next_arg_or_die(mArgs, i); Config::year     = i->c_str(); }
    else if (*i == "--save-hists")  { Config::saveHists  = true; }
    else if (*i == "--save-tmphists") { Config::saveTempHists  = true; }
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
