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
  // applyevcut, outdir, savehists
  // applyjetptcut, jetptcut, applynjetscut, njetscut
  // applyph1ptcut, ph1ptcut, applyph1vidcut, ph1vid
  // applyphanyptcut, phanyptcut, applyphanyvidcut, phanyvid
  // applyrhecut, rhEcut
  // applyecalacceptcut, applyEBonly, applyEEonly
  // applyphmcmatchingcut, applyexactphmcmatch, applyantiphmcmatch

  PlotPhotons photonPlots("input/MC/signal/GMSB/photondump-gmsb-ctau6000-HLT3.root",true,false,false,
			  false,true,
			  false,"output/MC/signal/GMSB/ctau6000",false,
			  true,35.f,true,3,
			  true,50.f,true,"medium",
			  true,10.f,true,"loose",
			  true,1.f,
			  true,false,false,
			  true,true,false);

  // which plots to do:
  // geninfo, vtxs, met, jets
  // photons, ph1, phdelay
  // trigger, analysis
  photonPlots.DoPlots(false,false,false,false,
 		      false,false,false,
		      false,false);
}
