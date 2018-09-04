#ifndef __EraTreePlotter2D__
#define __EraTreePlotter2D__

#include "Common.hh"
#include "TreePlotter2D.hh"

class EraTreePlotter2D : TreePlotter2D
{
public:
  EraTreePlotter2D(const TString & infilename, const TString & insignalfilename, const TString & cutconfig,
		   const TString & varwgtmapconfig, const TString & plotconfig, const TString & miscconfig,
		   const TString & era, const TString & outfiletext);
  ~EraTreePlotter2D() {}

  // Initialize
  void SetupConfig();

  // Main call
  void MakeEraTreePlot2D();

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
