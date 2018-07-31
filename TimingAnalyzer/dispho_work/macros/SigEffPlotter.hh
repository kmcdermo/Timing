#ifndef __SigEffPlotter__
#define __SigEffPlotter__

#include "TStyle.h"
#include "TFile.h"
#include "TH1F.h"
#include "TEfficiency.h"
#include "TGraphAsymmErrors.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TString.h"
#include "TPaveText.h"
#include "TColor.h"

#include <map>
#include <vector>

#include "Common.hh"

class SigEffPlotter
{
public:
  SigEffPlotter(const TString & infilename, const TString & outtext);
  ~SigEffPlotter();

  // Setup
  void SetupConfig();
  void SetupSignalSubGroups();

  // Main routines
  void MakeSigEffPlot();
  void MakeInputGraphs();
  void MakeLegend();
  void MakeOutput();

  // save meta data
  void MakeConfigPave();

  // Helper functions for making graphs
  TEfficiency * MakeEfficiency(const TString & groupname);
  void MakeGraph(const TEfficiency * efficiency, const TString & groupname);

  // Helper functions for plotting
  void DrawOutput(const Bool_t isLogY);
  void GetMinYMaxY();

private:
  // Settings
  const TString fInFileName;
  const TString fOutText;
  
  // Inputs
  TFile * fInFile;
  std::vector<TString> fSignalSubGroupVec;

  // Outputs
  TFile * fOutFile;
  std::map<TString,TGraphAsymmErrors*> fGraphMap;
  TStyle * fTDRStyle;
  TLegend * fLegend;
  TCanvas * fCanvas;
  TPaveText * fConfigPave;

  // Output plot info
  Double_t fMinY;
  Double_t fMaxY;
};

#endif
