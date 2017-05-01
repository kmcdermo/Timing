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
  
  TString dataset = "sph_2016C";
  Bool_t rhE = false;

  std::cout << "RECO" << std::endl;
  PlotRECOPhotons reco_Plots(Form("input/DATA/RECO/recophoton-%s-reco.root",dataset.Data()),Form("output/recophotons/reco/%s",dataset.Data()),rhE,1.f);
  reco_Plots.DoPlots();
  std::cout << std::endl;

  std::cout << "ReRECO" << std::endl;
  PlotRECOPhotons rereco_Plots(Form("input/DATA/RECO/recophoton-%s-rereco.root",dataset.Data()),Form("output/recophotons/rereco/%s",dataset.Data()),rhE,1.f);
  rereco_Plots.DoPlots();
  std::cout << std::endl;

  std::cout << "ReRECO+pT" << std::endl;
  PlotRECOPhotons rereco_pt_Plots(Form("input/DATA/RECO/recophoton-%s-rereco.root",dataset.Data()),Form("output/recophotons/rereco/%s_pt",dataset.Data()),rhE,1.f,true,10.f);
  rereco_pt_Plots.DoPlots();
  std::cout << std::endl;

  std::cout << "ReRECO+pT+H/E" << std::endl;
  PlotRECOPhotons rereco_hoe_Plots(Form("input/DATA/RECO/recophoton-%s-rereco.root",dataset.Data()),Form("output/recophotons/rereco/%s_hoe",dataset.Data()),rhE,1.f,true,10.f,true,0.3);
  rereco_hoe_Plots.DoPlots();
  std::cout << std::endl;

  std::cout << "ReRECO+pT+H/E+R9" << std::endl;
  PlotRECOPhotons rereco_r9_Plots(Form("input/DATA/RECO/recophoton-%s-rereco.root",dataset.Data()),Form("output/recophotons/rereco/%s_r9",dataset.Data()),rhE,1.f,true,10.f,true,0.3,true,0.5);
  rereco_r9_Plots.DoPlots();
  std::cout << std::endl;

  std::cout << "ReRECO+pT+H/E+R9+Sieie" << std::endl;
  PlotRECOPhotons rereco_sieie_Plots(Form("input/DATA/RECO/recophoton-%s-rereco.root",dataset.Data()),Form("output/recophotons/rereco/%s_sieie",dataset.Data()),rhE,1.f,true,10.f,true,0.3,true,0.5,true,0.030,0.070);
  rereco_sieie_Plots.DoPlots();
  std::cout << std::endl;
}
