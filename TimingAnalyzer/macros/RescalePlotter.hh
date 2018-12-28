#ifndef __RescalePlotter__
#define __RescalePlotter__

#include "Common.hh"
#include "TreePlotter.hh"

class RescalePlotter : TreePlotter 
{
public:
  RescalePlotter(const TString & infilename, const TString & rescaleconfig, const TString & plotconfig,
		 const TString & miscconfig, const TString & era, const TString & outfiletext);
  ~RescalePlotter() {}

  // setup functions
  void SetupCommon();
  void SetupRescaleConfig();
  void SetupHists();
  
  // main calls!
  void MakeRescaledPlot();
  void RescaleHist();
  void MakeConfigPave();
  void DeleteMemory();

private:
  const TString fInFileName;
  const TString fRescaleConfig;
  const TString fPlotConfig;
  const TString fMiscConfig;
  const TString fEra;
  const TString fOutFileText;

  // input hist file
  TFile * fInFile;

  // which plot to scale from which shape
  TString fScaleSample;
  TString fShapeSample;
  
  // output hist
  TH1F * RescaledHist;
};

#endif
