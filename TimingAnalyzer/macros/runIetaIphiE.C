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

void runIetaIphiE()
{
  setupcpp11(); 

  gROOT->LoadMacro("ietaiphiE.cc++g");

  ietaiphiE ieipE;
  //  ieipE.CheckForGoodPhotons();
  //ieipE.DumpGoodPhotonRHIDs();
  ieipE.DoPlotNatural();
}
