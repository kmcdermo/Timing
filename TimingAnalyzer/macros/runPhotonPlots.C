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

  PlotPhotons photonPlots("input/MC/signal/HVDS/photondump-hvds-ctau1000-HLT2.root",false,true,false,
			  false,false,
			  false,"output/MC/signal/HVDS/ctau1000",true,
			  true,35.f,true,3,
			  false,50.f,false,"medium",
			  false,10.f,false,"loose",
			  true,1.f,
			  true,true,false,
			  false,false,false);

  // which plots to do:
  // geninfo, vtxs, met, jets
  // photons, ph1, phdelay
  // trigger, analysis
  photonPlots.DoPlots(false,false,false,false,
 		      true,true,true,
		      false,true);
}
