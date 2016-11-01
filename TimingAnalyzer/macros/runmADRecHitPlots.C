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

void runmADRecHitPlots() 
{
  setupcpp11(); 

  gROOT->LoadMacro("PlotmADRecHits.cc++g");

  // config is:
  // applyHLTcut
  // applyPhPtcut, phptcut,
  // applyrhEcut, rhEcut 
  // applyEcalAcceptcut
  // applyVIDcut, VID string
  
  PlotmADRecHits recHitPlots("input/ootmADtree.root","output/mAD",false,true,50.f,true,1.f,true,false,"medium");
  recHitPlots.DoPlots();
}
