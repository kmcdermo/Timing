#ifndef __SigEffPlotter__
#define __SigEffPlotter__

#include "TStyle.h"
#include "TFile.h"
#include "TH1F.h"
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
  void SetupCommon();
  void SetupSignalSubGroups();

  // Main routines
  void MakeSigEffPlot();
  void MakeInputHists();
  void MakeLegend();
  void MakeOutput();

  // save meta data
  void MakeConfigPave();

  // Helper functions for making graphs
  void MakeHist(const TString & groupname);
  TString GetLambda(TString sample);
  
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
  std::map<TString,TH1F*> fEffMap;
  TStyle * fTDRStyle;
  TLegend * fLegend;
  TCanvas * fCanvas;
  TPaveText * fConfigPave;

  // Output plot info
  Double_t fMinY;
  Double_t fMaxY;
};

#endif
