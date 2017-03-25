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

  gROOT->LoadMacro("PlotConfig.cc+g");
  gROOT->LoadMacro("PlotPhotons.cc+g");

  // config is:
  // filename, isGMSB, isHVDS, isBkg
  // outdir, savehists, savesub
  // generic config, hlt config, jet config
  // leading photon config, any photon config, all photon config
  PlotPhotons photonPlots("input/MC/signal/HVDS/photondump-hvds-ctau1000-HLT2.root",false,false,false,
			  "output/trigger/MC/signal/HVDS/ctau1000",false,false,
			  "config/plotter-generic.txt","config/plotter-hlt.txt","config/plotter-jet.txt",
			  "config/plotter-ph1.txt","config/plotter-phany.txt","config/plotter-photon.txt");

  // which plots to do:
  // geninfo, vtxs, met, jets
  // photons, ph1, phdelay
  // trigger, analysis
  photonPlots.DoPlots(false,false,false,false,
		      false,false,false,
		      false,false);
}
