#include "TString.h"
#include "TSystem.h"
#include "TROOT.h"

void setupcpp11() // customize ACLiC's behavior ...
{
  TString o;
  // customize MakeSharedLib
  o = TString(gSystem->GetMakeSharedLib());
  o = o.ReplaceAll(" -c ", " -std=c++0x -c ");
  gSystem->SetMakeSharedLib(o.Data());
  // customize MakeExe
  o = TString(gSystem->GetMakeExe());
  o = o.ReplaceAll(" -c ", " -std=c++0x -c ");
  gSystem->SetMakeExe(o.Data());
} 

void runRHDumper() 
{
  setupcpp11(); 

  gROOT->LoadMacro("PlotConfig.cc+g");
  gROOT->LoadMacro("RecHitDumper.cc+g");

  // config is:
  // filename, outdir, 
  // leading photon config, any photon config, all photon config
  RecHitDumper dumper("input/MC/signal/HVDS/photondump-hvds-ctau1000-HLT2.root","output/rhdump/MC/signal/HVDS/ctau1000",
		      "config/plotter-ph1.txt","config/plotter-phany.txt","config/plotter-photon.txt");

  // which plots to do:
  // all, leading, most delayed
  dumper.DoDump(false,false,false);
}
