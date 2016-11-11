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

void runRECORecHitPlots() 
{
  setupcpp11(); 

  gROOT->LoadMacro("PlotRECORecHits.cc++g");

  // config is:
  // appendRHList
  // applyHLTcut
  // applyPhPtcut, phptcut,
  // applyrhEcut, rhEcut 
  // applyEcalAcceptcut
  
  PlotRECORecHits recHitPlots("input/ootrecotree.root","output/reco",true,false,true,50.f,true,1.f,true);
  recHitPlots.DoPlots();
}
