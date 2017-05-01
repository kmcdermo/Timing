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

void runRECOPhotonPlots() 
{
  setupcpp11(); 

  gROOT->LoadMacro("PlotRECOPhotons.cc+g");

  // config is:
  // input file, output directory
  // applyptcut, ptcut

  // DEG 2016B
  PlotRECOPhotons reco_deg_2016B_Plots("input/DATA/RECO/recophoton-deg_2016B-reco.root","output/recophotons/reco/deg_2016B",false,10.f);
  reco_deg_2016B_Plots.DoPlots();

  PlotRECOPhotons rereco_deg_2016B_Plots("input/DATA/RECO/recophoton-deg_2016B-rereco.root","output/recophotons/rereco/deg_2016B",false,10.f);
  rereco_deg_2016B_Plots.DoPlots();

  PlotRECOPhotons rereco_deg_2016B_ptcuts_Plots("input/DATA/RECO/recophoton-deg_2016B-rereco.root","output/recophotons/rereco/deg_2016B_ptcuts",true,10.f);
  rereco_deg_2016B_ptcuts_Plots.DoPlots();

  // SPH 2016C
  PlotRECOPhotons reco_sph_2016C_Plots("input/DATA/RECO/recophoton-sph_2016C-reco.root","output/recophotons/reco/sph_2016C",false,10.f);
  reco_sph_2016C_Plots.DoPlots();

  PlotRECOPhotons rereco_sph_2016C_Plots("input/DATA/RECO/recophoton-sph_2016C-rereco.root","output/recophotons/rereco/sph_2016C",false,10.f);
  rereco_sph_2016C_Plots.DoPlots();

  PlotRECOPhotons rereco_sph_2016C_ptcuts_Plots("input/DATA/RECO/recophoton-sph_2016C-rereco.root","output/recophotons/rereco/sph_2016C_ptcuts",true,10.f);
  rereco_sph_2016C_ptcuts_Plots.DoPlots();
}
