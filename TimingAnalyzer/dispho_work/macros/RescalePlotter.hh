#ifndef __RescalePlotter__
#define __RescalePlotter__

#include "Common.hh"
#include "TreePlotter.hh"

class RescalePlotter : TreePlotter 
{
public:
  RescalePlotter(const TString & infilename, const TString & rescaleconfig,
		 const TString & miscconfig, const TString & outfiletext);
  ~RescalePlotter();

  // setup functions
  void SetupDefaults();
  void SetupConfig();
  void SetupRescaleConfig();
  void SetupHists();
  
  // main calls!
  void MakeRescaledPlot();
  void RescaleHists();
  void MakeConfigPave();

private:
  const TString fInFileName;
  const TString fRescaleConfig;
  const TString fMiscConfig;
  const TString fOutFileText;

  TFile * fInFile;
};

#endif
