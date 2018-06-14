#ifndef __SRPlotter__
#define __SRPlotter__

#include "Common.hh"
#include "TreePlotter.hh"

class SRPlotter : TreePlotter 
{
public:
  SRPlotter(const TString & srplotconfig, const TString & miscconfig, const TString & outfiletext);
  ~SRPlotter() {}

  // setup functions
  void SetupConfig();
  void SetupSRPlotConfig();
  void SetupHists();
  
  // main calls!
  void MakeSRPlot();
  void ScaleCRtoSR();
  void MakeConfigPave();
  void DeleteMemory();

private:
  const TString fSRPlotConfig;
  const TString fMiscConfig;
  const TString fOutFileText;

  // input sr plot config
  TString fGJetsFileName;
  TString fQCDFileName;
  TString fSRFileName;
  TString fPlotConfig;

  // input files
  TFile * fGJetsFile;
  TFile * fQCDFile;
  TFile * fSRFile;

  // input extra hists
  TH1F * GJetsHistMC_CR;
  TH1F * QCDHistMC_CR;
  TH1F * GJetsHistMC_SR;
  TH1F * QCDHistMC_SR;
};

#endif
