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

void runPhotonPlots_HT(TString cuts, TString sample, TString bin, Bool_t isEB, Bool_t isEE) 
{
  setupcpp11(); 

  gROOT->LoadMacro("PlotPhotons.cc++g");

  // config is:
  // filename, isMC, applyevcut, outdir, 
  // applyjetptcut, jetptcut, applyphptcut, phptcut,
  // applyphvidcut, phvid, applyrhecut, 
  // applyecalacceptcut, applyEBonly, applyEEonly
  
  // apply analysis cuts to individual plots?
  bool apply = (cuts.EqualTo("cuts",TString::kExact)?true:false);

  TString VID = apply?"medium":"loose";

  PlotPhotons photonPlots(Form("input/MC/bkg/%s/%s_HT%s.root",sample.Data(),sample.Data(),bin.Data()),false,apply,Form("output/MC/bkg/%s/%s/%s",sample.Data(),cuts.Data(),bin.Data()),true,35.f,apply,100.f,true,VID.Data(),true,1.f,true,isEB,isEE);

  // which plots to do
  // first bool = generic plots
  // second bool = efficiency
  // third bool = analysis plots
  photonPlots.DoPlots(true,false,false);
}
