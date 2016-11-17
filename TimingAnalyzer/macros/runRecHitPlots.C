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
  // applyrhidcut, rhlistfile
  // applyrhEcut, rhEcut 
  // applyEcalAcceptcut
  
  PlotRecHits recHitPlots("input/recorechits.root","output/full",true,"output/reco/cuts_phpt50.0_rhE1.0_ecalaccept/rhlist.txt",true,1.f,false);
  recHitPlots.DoPlots();
}
