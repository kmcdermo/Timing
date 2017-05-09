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

void runRecHitPlots() 
{
  setupcpp11(); 

  gROOT->LoadMacro("PlotRecHits.cc+g");

  // config is:
  // input file, output directory
  // applyrhEcut, rhEcut 

  Bool_t rhE = true;

  std::cout << "RECO DEG 2016B" << std::endl;
  PlotRecHits reco_deg_2016B_Plots("input/DATA/RECO/recorechits-deg_2016B-reco.root","output/rechits/reco/deg_2016B",rhE,1.f);
  reco_deg_2016B_Plots.DoPlots();
  std::cout << std::endl;

  std::cout << "ReRECO DEG 2016B" << std::endl;
  PlotRecHits rereco_deg_2016B_Plots("input/DATA/RECO/recorechits-deg_2016B-rereco.root","output/rechits/rereco/deg_2016B",rhE,1.f);
  rereco_deg_2016B_Plots.DoPlots();
  std::cout << std::endl;

  std::cout << "RECO SPH 2016C" << std::endl;
  PlotRecHits reco_sph_2016C_Plots("input/DATA/RECO/recorechits-sph_2016C-reco.root","output/rechits/reco/sph_2016C",rhE,1.f);
  reco_sph_2016C_Plots.DoPlots();
  std::cout << std::endl;

  std::cout << "ReRECO SPH 2016C" << std::endl;
  PlotRecHits rereco_sph_2016C_Plots("input/DATA/RECO/recorechits-sph_2016C-rereco.root","output/rechits/rereco/sph_2016C",rhE,1.f);
  rereco_sph_2016C_Plots.DoPlots();
  std::cout << std::endl;
}
