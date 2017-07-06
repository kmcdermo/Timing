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

void runHLTPlots() 
{
  setupcpp11(); 

  gROOT->LoadMacro("HLTPlots.cc+g");

  const TString infile = "input/DATA/2017/HLT_CHECK/30_06_17/jets/hltdump_SM_HFGood.root";
  const TString outdir = "hltcheck_SP";
  const Bool_t isoph = false;
  const Bool_t isidL = false;
  const Bool_t iser  = false;
  const Float_t htcut = 400.f;

  HLTPlots plots(infile,outdir,isoph,isidL,iser,htcut);
  plots.DoPlots();
}
