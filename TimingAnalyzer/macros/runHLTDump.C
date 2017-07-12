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

void runHLTDump() 
{
  setupcpp11(); 

  gROOT->LoadMacro("HLTDump.cc+g");

  const TString infile = "input/DATA/2017/HLT_CHECK/30_06_17/hltdump_DCS_SM.root";
  const TString outdir = "dump_dcs_SM_vid";
  const Int_t psfactor = 10;

  // first = nopho, second = jetIdL, third = jet eta < 3.0, fourth is jetpt > 4.0
  
  std::cout << std::endl << "No cuts" << std::endl;
  HLTDump plots_nocuts(infile,outdir,false,false,false,true,psfactor);
  plots_nocuts.DoPlots();

  std::cout << std::endl << "Jet Eta Cut" << std::endl;
  HLTDump plots_jetER(infile,outdir,false,false,true,true,psfactor);
  plots_jetER.DoPlots();

  std::cout << std::endl << "Jet IdL Cut" << std::endl;
  HLTDump plots_jetIdL(infile,outdir,false,true,true,true,psfactor);
  plots_jetIdL.DoPlots();

  std::cout << std::endl << "Photon dR Cut" << std::endl;
  HLTDump plots_nopho(infile,outdir,true,true,true,true,psfactor);
  plots_nopho.DoPlots();
}
