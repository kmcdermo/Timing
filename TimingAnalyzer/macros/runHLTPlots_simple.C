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

void runHLTPlots_simple() 
{
  setupcpp11(); 

  gROOT->LoadMacro("HLTPlots_simple.cc+g");

  const TString infile = "input/DATA/2017/HLT_CHECK/30_06_17/hltdump_DCS_SM.root";
  const TString outdir = "dump_dcs_SM";
  const Bool_t isoph = true;
  const Bool_t isidL = true;
  const Bool_t iser  = true;
  
  const Int_t psfactor = 10;

  HLTPlots_simple plots(infile,outdir,isoph,isidL,iser,psfactor);
  plots.DoPlots();
}
