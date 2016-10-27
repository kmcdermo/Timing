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

void runRecHitPlots() 
{
  setupcpp11(); 

  gROOT->LoadMacro("PlotRecHits.cc++g");

  // config is:
  // filename, isMC, outdir, 
  // applyjetptcut, jetptcut, applyphptcut, phptcut,
  // applyphvidcut, phvid, applyrhecut, 
  // applyecalacceptcut
  
  PlotRecHits recHitPlots("input/rereco/phrhs-addrhs.root","output/rhaddons",true,100.f,true,1.f,true);
  recHitPlots.DoPlots();
}
