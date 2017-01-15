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

void runrhEmap()
{
  setupcpp11(); 

  gROOT->LoadMacro("rhEmap.cc++g");

  rhEmap obj;
  //obj.CheckForGoodPhotons();
  //obj.DumpGoodPhotonRHIDs();

  // For plots, these are the "good entries", with iph == 0
  // 6494 --> prompt1, 17664 --> prompt2, 31665 --> delayed1, 35300 --> delayed2
  // bool is applyrhEcut ... last float is value
  //obj.DoPlotNatural(0,0,true,1.f);
  
  obj.DoAllPlotNatural(1.f);
}
