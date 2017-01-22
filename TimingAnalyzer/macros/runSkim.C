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
  // jetptcut, njetptcut, phptcut, phsieieEBcut (medium = 0.01022), phsieieEEcut (medium = 0.03001)
  // phsmajcut, phsmincut, phsmin_over_smaj_cut
  //  applyEBonly, applyEEonly

  TString outdir = "output/skim";
  
  Float_t jetpt = 25.f;
  Int_t   njets = 2;
  
  Float_t phpt    = 30.f;
  Float_t sieieEB = 1.f;
  Float_t sieieEE = 1.f;
  Float_t smajEB  = 0.015;
  Float_t smajEE  = 0.040;
  Float_t sminEB  = 0.015;
  Float_t sminEE  = 0.040;

  SkimRECO dataskimmer    ("input/DATA/doubleeg/skim/recoskim.root"         ,outdir,"data"    ,true ,jetpt,njets,phpt,sieieEB,sieieEE,smajEB,smajEE,sminEB,sminEE);
  SkimRECO ctau100skimmer ("input/MC/signal/skim/recoskim-pat-ctau100.root" ,outdir,"ctau100" ,false,jetpt,njets,phpt,sieieEB,sieieEE,smajEB,smajEE,sminEB,sminEE);
  SkimRECO ctau2000skimmer("input/MC/signal/skim/recoskim-pat-ctau2000.root",outdir,"ctau2000",false,jetpt,njets,phpt,sieieEB,sieieEE,smajEB,smajEE,sminEB,sminEE);
  SkimRECO ctau6000skimmer("input/MC/signal/skim/recoskim-pat-ctau6000.root",outdir,"ctau6000",false,jetpt,njets,phpt,sieieEB,sieieEE,smajEB,smajEE,sminEB,sminEE);

  dataskimmer    .DoSkim();
  ctau100skimmer .DoSkim();
  ctau2000skimmer.DoSkim();
  ctau6000skimmer.DoSkim();
}
