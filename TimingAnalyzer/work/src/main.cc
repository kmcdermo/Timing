#include "../interface/Config.hh"
#include "../interface/Common.hh"
#include "../interface/PUReweight.hh"
#include "../interface/Analysis.hh"
#include "../interface/StackPlots.hh"

#include "TROOT.h"

void InitializeMain(std::ofstream & yields, TStyle *& tdrStyle) {
  // set TDR Style (need to force it!)
  tdrStyle = new TStyle("tdrStyle","Style for P-TDR");
  SetTDRStyle(tdrStyle);
  gROOT->ForceStyle();

  // have to do this at least once!
  MakeOutDir(Config::outdir);

  // yields 
  yields.open(Form("%s/yields.txt",Config::outdir.Data()),std::ios_base::app);

  // set sample map
  if (Config::useDEG)   Config::SampleMap["doubleeg"] = false; // !isMC
  if (Config::useDYll)  Config::SampleMap["dyll"]     = true;  //  isMC
  if (Config::useQCD)   Config::SampleMap["qcd"]      = true;  //  isMC
  if (Config::useGJets) Config::SampleMap["gamma"]    = true;  //  isMC

  // Color for MC Stacks
  Config::colorMap["dyll"]   = kCyan;
  Config::colorMap["qcd"]    = kYellow;
  Config::colorMap["gamma"]  = kGreen;

  // define title map
  Config::SampleTitleMap["dyll"]   = "Z #rightarrow l^{+}l^{-}";
  Config::SampleTitleMap["qcd"]    = "QCD";
  Config::SampleTitleMap["gamma"]  = "#gamma + Jets";
}

void DestroyMain(std::ofstream & yields, TStyle *& tdrStyle) {
  yields.close();
  delete tdrStyle;
}

int main(int argc, const char* argv[]) {
  //////////////////////////////////
  // Read in command line options //
  //////////////////////////////////
  lStr_t mArgs; 
  for (int i = 1; i < argc; ++i) { mArgs.push_back(argv[i]); }
  lStr_i i  = mArgs.begin();
  while (i != mArgs.end()) {
    lStr_i start = i;
    if (*i == "-h" || *i == "-help" || *i == "--help") {
      printf(
        "Usage: %s [options]\n"
        "Options:\n"
	"  --outdir        <string>      name of ouput directory (def: %s)\n"
	"  --do-purw       <bool>        calculate pile-up weights (def: %s)\n"
	"  --do-analysis   <bool>        make analysis plots (def: %s)\n"
	"  --do-stacks     <bool>        stack data/MC plots (def: %s)\n"
	"  --do-demo       <bool>        demo analysis (def: %s)\n"
	"  --use-DEG       <bool>        use doubleEG for data (def: %s)\n"
	"  --use-DYll      <bool>        use Drell-Yan (LL+Jets) with MC (def: %s)\n"
	"  --use-QCD       <bool>        use QCD with MC (def: %s)\n"
	"  --use-GJets     <bool>        use Gamma+Jets with MC (def: %s)\n"
	"  --skip-runs     <bool>        skip timing plots vs run number (def: %s)\n"
        ,
        argv[0],
        Config::outdir.Data(),
	(Config::doPURW      ? "true" : "false"),
	(Config::doAnalysis  ? "true" : "false"),
	(Config::doStacks    ? "true" : "false"),
	(Config::doDemo      ? "true" : "false"),
	(Config::useDEG      ? "true" : "false"),
	(Config::useDYll     ? "true" : "false"),
	(Config::useQCD      ? "true" : "false"),
	(Config::useGJets    ? "true" : "false"),
	(Config::skipRuns    ? "true" : "false")
      );
      exit(0);
    }
    else if (*i == "--outdir")      { next_arg_or_die(mArgs, i); Config::outdir = i->c_str(); }
    else if (*i == "--do-purw")     { Config::doPURW     = true; }
    else if (*i == "--do-analysis") { Config::doAnalysis = true; }
    else if (*i == "--do-stacks")   { Config::doStacks   = true; }
    else if (*i == "--do-demo")     { Config::doDemo     = true; Config::doAnalysis = true; }
    else if (*i == "--use-DEG")     { Config::useDEG     = true; }
    else if (*i == "--use-DYll")    { Config::useDYll    = true; }
    else if (*i == "--use-QCD")     { Config::useQCD     = true; }
    else if (*i == "--use-GJets")   { Config::useGJets   = true; }
    else if (*i == "--skip-runs")   { Config::skipRuns   = true; Config::doAnalysis = true; }
    else    { fprintf(stderr, "Error: Unknown option/argument '%s'.\n", i->c_str()); exit(1); }
    mArgs.erase(start, ++i);
  }

  ////////////////////
  // Initialization //
  ////////////////////

  // do this thing at least once!
  ofstream yields; TStyle * tdrStyle; 
  InitializeMain(yields,tdrStyle);

  /////////////////////////
  // Pile-up reweighting //
  /////////////////////////

  if (Config::doPURW) {
    std::cout << "Calculating pile-up weights" << std::endl;
    PUReweight reweight;
    reweight.GetPUWeights();
  }
  else {
    std::cout << "Skipping calculating pile-up weights" << std::endl;
  }
  
  ///////////////////
  // Main Analysis //
  ///////////////////

  if (Config::doAnalysis) {
    for (TStrBoolMapIter mapiter = Config::SampleMap.begin(); mapiter != Config::SampleMap.end(); ++mapiter) {
      Analysis analysis((*mapiter).first,(*mapiter).second);
      analysis.StandardPlots();
      analysis.TimeResPlots();
    }
  }
  else {
    std::cout << "Skipping analysis section" << std::endl;
  }

  ///////////////////
  // Stack data/mc //
  ///////////////////

  if (Config::doStacks) {
    StackPlots Stacker;
    Stacker.DoStacks(yields);
  }
  else {
    std::cout << "Skipping stacking data over MC" << std::endl;
  }

  // end of the line
  DestroyMain(yields,tdrStyle);
}
