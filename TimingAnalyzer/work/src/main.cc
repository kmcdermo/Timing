#include "../interface/Config.hh"
#include "../interface/Common.hh"
#include "../interface/PUReweight.hh"
#include "../interface/Analysis.hh"
#include "../interface/StackPlots.hh"

#include "TROOT.h"

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
        ,
        argv[0],
        Config::outdir.Data(),
	(Config::doPURW      ? "true" : "false"),
	(Config::doAnalysis  ? "true" : "false"),
	(Config::doStacks    ? "true" : "false"),
	(Config::doDemo      ? "true" : "false")
      );
      exit(0);
    }
    else if (*i == "--outdir")      { next_arg_or_die(mArgs, i); Config::outdir = i->c_str(); }
    else if (*i == "--do-purw")     { Config::doPURW     = true; }
    else if (*i == "--do-analysis") { Config::doAnalysis = true; }
    else if (*i == "--do-stacks")   { Config::doStacks   = true; }
    else if (*i == "--do-demo")     { Config::doDemo     = true; Config::doAnalysis = true; }
    else    { fprintf(stderr, "Error: Unknown option/argument '%s'.\n", i->c_str()); exit(1); }
    mArgs.erase(start, ++i);
  }

  ////////////////////
  // Initialization //
  ////////////////////

  // do this at least once!
  MakeOutDir(Config::outdir);

  // yields
  ofstream yields(Form("%s/yields.txt",Config::outdir.Data()),std::ios_base::app);

  // Variables needed in all functions for plotting and the like so it is universal
  // Color for MC Stacks
  ColorMap colorMap;
  colorMap["dyll"]   = kCyan;
  colorMap["qcd"]    = kYellow;
  colorMap["gamma"]  = kGreen;
  colorMap["demomc"] = kPink;

  ////////////////////////
  // Set official style //
  ////////////////////////

  // set TDR Style (need to force it!)
  TStyle * tdrStyle = new TStyle("tdrStyle","Style for P-TDR");
  SetTDRStyle(tdrStyle);
  gROOT->ForceStyle();

  /////////////////////////
  // Pile-up reweighting //
  /////////////////////////

  if (Config::doPURW) {
    TStrBoolMap PUSampleMap;
    PUSampleMap["demo"]   = false;
    PUSampleMap["demomc"] = true;

    std::cout << "Calculating pile-up weights" << std::endl;
    PUReweight reweight(PUSampleMap);
    reweight.GetPUWeights();
  }
  else {
    std::cout << "Skipping calculating pile-up weights" << std::endl;
  }
  
  ///////////////////
  // Main Analysis //
  ///////////////////
  TStrBoolMap SampleMap;
  SampleMap["demo"]   = false;
  SampleMap["demomc"] = true;
    
  if (Config::doAnalysis) {
    for (TStrBoolMapIter mapiter = SampleMap.begin(); mapiter != SampleMap.end(); ++mapiter) {
      Analysis analysis((*mapiter).first,(*mapiter).second,colorMap);
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
    StackPlots Stacker(SampleMap,colorMap);
    Stacker.DoStacks(yields);
  }
  else {
    std::cout << "Skipping stacking data over MC" << std::endl;
  }

  // end of the line
  yields.close(); 
  delete tdrStyle; 
}
