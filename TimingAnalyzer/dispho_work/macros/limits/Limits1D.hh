#ifndef __Limits1D__
#define __Limits1D__

#include "TROOT.h"
#include "TStyle.h"
#include "TGraph.h"
#include "TLegend.h"
#include "TCanvas.h"

#include "../Common.hh"
#include "Combine.hh"

class Limits1D
{
public:
  Limits1D(const TString & indir, const TString & infilename, const Bool_t doobserved, const TString & outtext);
  ~Limits1D();

  // setup functions
  void SetupCombine();

  // main plotting routine
  void MakeLimits1D();

private:
  const TString fInDir;
  const TString fInFileName;
  const Bool_t fDoObserved;
  const TString fOutText;

  // style
  TStyle * fTDRStyle;

  // output
  TFile * fOutFile;
};

#endif
