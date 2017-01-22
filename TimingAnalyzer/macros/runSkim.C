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
  // filename, outdir, saveHistNames
  // jetptcut, njetptcut, phptcut, phsieieEBcut, phsieieEEcut
  // phsmajcut, phsmincut, phsmin_over_smaj_cut
  //  applyEBonly, applyEEonly

  //  SkimRECO skimmer("input/MC/signal/skim/recoskim-pat-ctau6000.root","output/skim6000");
  SkimRECO dataskimmer    ("input/DATA/doubleeg/skim/recoskim.root"         ,"output/dataskim"    ,true,25,2,30,0.01022,0.03001);
  SkimRECO ctau100skimmer ("input/MC/signal/skim/recoskim-pat-ctau100.root" ,"output/ctau100skim" ,true,25,2,30,0.01022,0.03001);
  SkimRECO ctau2000skimmer("input/MC/signal/skim/recoskim-pat-ctau2000.root","output/ctau2000skim",true,25,2,30,0.01022,0.03001);
  SkimRECO ctau6000skimmer("input/MC/signal/skim/recoskim-pat-ctau6000.root","output/ctau6000skim",true,25,2,30,0.01022,0.03001);

  dataskimmer    .DoSkim();
  ctau100skimmer .DoSkim();
  ctau2000skimmer.DoSkim();
  ctau6000skimmer.DoSkim();
}
