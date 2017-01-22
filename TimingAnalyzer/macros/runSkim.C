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

void runSkim() 
{
  setupcpp11(); 

  gROOT->LoadMacro("SkimRECO.cc++g");

  // config is:
  // filename, outdir, 
  // jetptcut, njetptcut, phptcut, phsieieEBcut, phsieieEEcut
  //  applyEBonly, applyEEonly

  SkimRECO skimmer("input/MC/signal/skim/recoskim-pat-ctau6000.root","output/skim6000");
  skimmer.DoSkim();
}
