// hname must match name string in histo map! too lazy to pass extra parameter
#include "../interface/Common.hh"
#include "../interface/PUReweight.hh"
#include "../interface/Analysis.hh"
#include "../interface/StackPlots.hh"

#include "TROOT.h"

int main(){
  // to do:
  // add Config file/namespace
  // use command line arguments instead -- long term
  
  ////////////////////
  // Initialization //
  ////////////////////

  // parameters to determine analysis 
  const TString outdir    = "timeres0";
  const TString outtype   = "png";
  const Float_t lumi      = 2.301; // brilcalc lumi --normtag /afs/cern.ch/user/l/lumipro/public/normtag_file/moriond16_normtag.json -i rereco2015D.txt -u /fb
  const TString extratext = "Preliminary";
  
  // yields
  ofstream yields(Form("%s/yields.txt",outdir.Data()),std::ios_base::app);

  // Variables needed in all functions for plotting and the like so it is universal
  // Color for MC Stacks
  ColorMap colorMap;
  colorMap["dyll"]   = kCyan;
  colorMap["qcd"]    = kYellow;
  colorMap["gamma"]  = kGreen;
  colorMap["demomc"] = kPink;

  ////////////////////////
  // Set official style //
  ////////////////////////

  // set TDR Style (need to force it!)
  TStyle * tdrStyle = new TStyle("tdrStyle","Style for P-TDR");
  SetTDRStyle(tdrStyle);
  gROOT->ForceStyle();

  /////////////////////////
  // Pile-up reweighting //
  /////////////////////////

  Bool_t doPURW = true;
  
  if (doPURW) {
    TStrBoolMap PUSampleMap;
    PUSampleMap["demo"]   = false;
    PUSampleMap["demomc"] = true;

    const Int_t npubins = 50;

    std::cout << "Calculating pile-up weights" << std::endl;
    PUReweight reweight(PUSampleMap,outdir,outtype,lumi,npubins);
    reweight.GetPUWeights();
  }
  else {
    std::cout << "Skipping calculating pile-up weights" << std::endl;
  }
  
  ///////////////////
  // Main Analysis //
  ///////////////////
  TStrBoolMap SampleMap;
  SampleMap["demo"]   = false;
  SampleMap["demomc"] = true;
  
  Bool_t doAnalysis = true;
    
  if (doAnalysis) {
    for (TStrBoolMapIter mapiter = SampleMap.begin(); mapiter != SampleMap.end(); ++mapiter) {
      Analysis analysis((*mapiter).first,(*mapiter).second,outdir,outtype,lumi,extratext,colorMap);
      analysis.StandardPlots();
      analysis.TimeResPlots();
    }
  }
  else {
    std::cout << "Skipping analysis section" << std::endl;
  }

  ///////////////////
  // Stack data/mc //
  ///////////////////

  Bool_t doStacks = true;
  if (doStacks) {
    StackPlots Stacker(SampleMap,outdir,outtype,colorMap,lumi,extratext);
    Stacker.DoStacks(yields);
  }
  else {
    std::cout << "Skipping stacking data over MC" << std::endl;
  }

  // end of the line
  yields.close(); 
  delete tdrStyle; 
}
