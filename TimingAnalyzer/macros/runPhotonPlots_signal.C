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

void runPhotonPlots_signal(TString VID, TString ctau, Bool_t isEB, Bool_t isEE) 
{
  setupcpp11(); 

  gROOT->LoadMacro("PlotPhotons.cc+g");

  // config is:
  // filename, isGMSB, isBkg, applyevcut, outdir, 
  // applyjetptcut, jetptcut, applyphptcut, phptcut,
  // applyphvidcut, phvid, applyrhecut, 
  // applyecalacceptcut, applyEBonly, applyEEonly

  // apply analysis cuts to individual plots?
  bool apply = true;

  Float_t phpt = 50.f;

  PlotPhotons photonPlots(Form("input/MC/signal/GMSB/photondump-ctau%s.root",ctau.Data()),true,false,false,Form("output/MC/signal/GMSB/photondump/ctau%s",ctau.Data()),apply,35.f,apply,phpt,apply,VID.Data(),true,1.f,true,isEB,isEE);

  // which plots to do
  // first bool = generic plots
  // second bool = efficiency
  // third bool = analysis plots
  photonPlots.DoPlots(true,false,false);
}
