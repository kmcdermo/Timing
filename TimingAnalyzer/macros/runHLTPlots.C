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

  const TString infile = "input/DATA/2017/HLT_CHECK/30_06_17/hltdump_DCS_SM.root";
  const TString outdir = "hltcheck_SM_vid";
  const TString runs = "runs.txt";
  const Bool_t  isoph = true;
  const Bool_t  isidL = false;
  const Bool_t  iser  = true;
  const Bool_t  applyht = true;
  const Float_t htcut = 400.f;
  const Bool_t applyphdenom = true;
  const Bool_t applylast = false;
  const Bool_t applyphpt = false;

  HLTPlots plots(infile,outdir,runs,isoph,isidL,iser,applyht,htcut,applyphdenom,applylast,applyphpt);
  plots.DoPlots();
}
