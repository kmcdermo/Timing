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

  const TString dataset = "SP";
  const TString era = "2017C";
  const Bool_t  eteff = false;
  const Bool_t  dispeff = false;
  const Bool_t  hteff = true;

  const UInt_t  start = 0;
  const UInt_t  end   = 0;
  const TString infile = Form("input/DATA/2017/HLT_CHECK/%s/hltdump-%s.root",dataset.Data(),era.Data());
  const TString outdir = Form("HLT_Golden_%s_%s",dataset.Data(),era.Data());
  const TString runs = "noruns.txt";
  const Bool_t  isoph = false;
  const Bool_t  isidL = true;
  const Bool_t  iser  = true;
  const Bool_t  applyht = false;
  const Float_t htcut = 400.f;

  HLTPlots plots(infile,start,end,outdir,runs,isoph,isidL,iser,applyht,htcut,eteff,dispeff,hteff);
  plots.DoPlots();

  //  plots.DoOverplot();
}
