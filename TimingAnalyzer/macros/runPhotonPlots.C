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

void runPhotonPlots() 
{
  setupcpp11(); 

  gROOT->LoadMacro("PlotPhotons.cc+g");

  // config is:
  // filename, isGMSB, isHVDS, isBkg
  // isHLT2, isHLT3
  // applyevcut, rhdump, outdir, savehists,
  // applyjetptcut, jetptcut, 
  // applyphptcut, phptcut, applyphvidcut, phvid, applyrhecut, 
  // applyecalacceptcut, applyEBonly, applyEEonly
  
  PlotPhotons photonPlots("input/MC/signal/GMSB/photondump-gmsb-ctau6000-HLT3.root",true,false,false,
			  false,true,
			  false,false,"output/MC/signal/GMSB/ctau6000",false,
			  true,35.f,true,
			  50.f,true,"loose",true,1.f,
			  true,false,false);

  // which plots to do
  // first bool = generic plots
  // second bool = efficiency
  // third bool = analysis plots
  // fourth bool = trigger plots
  photonPlots.DoPlots(true,false,false,true);
}
