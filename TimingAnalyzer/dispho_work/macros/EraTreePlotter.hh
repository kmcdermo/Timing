#ifndef __EraTreePlotter__
#define __EraTreePlotter__

#include "Common.hh"
#include "TreePlotter.hh"

class EraTreePlotter : TreePlotter
{
public:
  EraTreePlotter(const TString & infilename, const TString & insignalfilename, const TString & cutconfig,
		 const TString & varwgtmapconfig, const TString & plotconfig, const TString & miscconfig,
		 const TString & era, const TString & outfiletext);
  ~EraTreePlotter() {}

  // Initialize
  void SetupConfig();

  // Main call
  void MakeEraTreePlot();

  // Save Meta data
  void MakeConfigPave();

  // Delete Function
  void DeleteMemory();

private:
  // Settings
  const TString fInFileName;
  const TString fInSignalFileName;
  const TString fCutConfig;
  const TString fVarWgtMapConfig;
  const TString fPlotConfig;
  const TString fMiscConfig;
  const TString fEra;
  const TString fOutFileText;

  // input
  TFile * fInFile;
  TFile * fInSignalFile;
};

#endif
