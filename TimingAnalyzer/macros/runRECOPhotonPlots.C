#include "TString.h"
#include "TSystem.h"
#include "TROOT.h"

#include <iostream>

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

void runRECOPhotonPlots() 
{
  setupcpp11(); 

  gROOT->LoadMacro("PlotRECOPhotons.cc+g");

  // config is:
  // input file, output directory
  // applyrhEcut, rhEcut
  // applyptcut, ptcut
  // applyr9cut, r9cut
  // applyhoecut, hoecut
  // applysieiecut, sieieEBcut, sieieEEcut
  
  Bool_t rhE = false;

  PlotRECOPhotons plots("input/DATA/2017/B/recophoton.root","output/2017B",rhE,1.f);
  plots.DoPlots();
}
