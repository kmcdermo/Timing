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

  std::cout << "Vanilla" << std::endl;
  PlotRECOPhotons vanilla_Plots(Form("input/DATA/MINIAOD/patphoton-%s-vanilla.root",dataset.Data()),Form("output/patphotons/vanilla/%s",dataset.Data()),rhE,1.f);
  vanilla_Plots.DoPlots();
  std::cout << std::endl;

  std::cout << "GEDPhotons" << std::endl;
  PlotRECOPhotons gedPhotons_Plots(Form("input/DATA/MINIAOD/patphoton-%s-gedPho.root",dataset.Data()),Form("output/patphotons/gedPhotons/%s",dataset.Data()),rhE,1.f);
  gedPhotons_Plots.DoPlots();
  std::cout << std::endl;

  std::cout << "OOT" << std::endl;
  PlotRECOPhotons OOT_Plots(Form("input/DATA/MINIAOD/patphoton-%s-OOT.root",dataset.Data()),Form("output/patphotons/OOT/%s",dataset.Data()),rhE,1.f);
  OOT_Plots.DoPlots();
  std::cout << std::endl;

  std::cout << "OOT_Clusters" << std::endl;
  PlotRECOPhotons OOT_Clusters_Plots(Form("input/DATA/MINIAOD/patphoton-%s-OOT_Clusters.root",dataset.Data()),Form("output/patphotons/OOT_Clusters/%s",dataset.Data()),rhE,1.f);
  OOT_Clusters_Plots.DoPlots();
  std::cout << std::endl;

  std::cout << "OOT_sep" << std::endl;
  PlotRECOPhotons OOT_sep_Plots(Form("input/DATA/MINIAOD/patphoton-%s-OOT_sep.root",dataset.Data()),Form("output/patphotons/OOT_sep/%s",dataset.Data()),rhE,1.f);
  OOT_sep_Plots.DoPlots();
  std::cout << std::endl;

  std::cout << "OOT_NoHoE" << std::endl;
  PlotRECOPhotons OOT_NoHoE_Plots(Form("input/DATA/MINIAOD/patphoton-%s-OOT_NoHoE.root",dataset.Data()),Form("output/patphotons/OOT_NoHoE/%s",dataset.Data()),rhE,1.f);
  OOT_NoHoE_Plots.DoPlots();
  std::cout << std::endl;

  std::cout << "OOT_Clusters_NoHoE" << std::endl;
  PlotRECOPhotons OOT_Clusters_NoHoE_Plots(Form("input/DATA/MINIAOD/patphoton-%s-OOT_Clusters_NoHoE.root",dataset.Data()),Form("output/patphotons/OOT_Clusters_NoHoE/%s",dataset.Data()),rhE,1.f);
  OOT_Clusters_NoHoE_Plots.DoPlots();
  std::cout << std::endl;
}
