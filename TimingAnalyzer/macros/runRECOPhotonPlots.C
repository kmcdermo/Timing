#include "TString.h"
#include "TSystem.h"
#include "TROOT.h"

#include <iostream>

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

void runRECOPhotonPlots() 
{
  setupcpp11(); 

  gROOT->LoadMacro("PlotRECOPhotons.cc+g");

  // config is:
  // input file, output directory
  // applyrhEcut, rhEcut
  // applyptcut, ptcut
  // applyr9cut, r9cut
  // applyhoecut, hoecut
  // applysieiecut, sieieEBcut, sieieEEcut
  
  TString dataset = "sph_2016H";
  Bool_t rhE = false;

  std::cout << "Prompt v1" << std::endl;
  PlotRECOPhotons promptv1_Plots(Form("input/DATA/AOD/recophoton-%s-Prompt-v1.root",dataset.Data()),Form("output/recophotons/Prompt-v1/%s",dataset.Data()),rhE,1.f);
  promptv1_Plots.DoPlots();
  std::cout << std::endl;

  std::cout << "Prompt v2" << std::endl;
  PlotRECOPhotons promptv2_Plots(Form("input/DATA/AOD/recophoton-%s-Prompt-v2.root",dataset.Data()),Form("output/recophotons/Prompt-v2/%s",dataset.Data()),rhE,1.f);
  promptv2_Plots.DoPlots();
  std::cout << std::endl;

  std::cout << "OOT" << std::endl;
  PlotRECOPhotons oot_Plots(Form("input/DATA/AOD/recophoton-%s-OOT.root",dataset.Data()),Form("output/recophotons/OOT/%s",dataset.Data()),rhE,1.f);
  oot_Plots.DoPlots();
  std::cout << std::endl;
}
