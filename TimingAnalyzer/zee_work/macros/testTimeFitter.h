#include "TStyle.h"
#include "TString.h"
#include "TFile.h"
#include "TH1F.h"
#include "TVirtualFitter.h"
#include "TF1.h"
#include "TFormula.h"
#include "TPaveText.h"
#include "TCanvas.h"
#include "TSystem.h"

#include <fstream>
#include <array>

typedef std::array<Double_t, 3> arr3;  

namespace Config{
  Double_t fitrange;
  TString  formname;
  TString  nbins;
  TString  histname;
  TString  outdir;
  Double_t ncore;
};

Int_t doFit(TH1F *& hist, TF1 *& fit, arr3 & temps);
void getFitParams(TF1 *& fit, Double_t & mean, Double_t & emean, Double_t & sigma, Double_t & esigma, Double_t & chi2ndf, Double_t & chi2prob);
void dumpFit(TF1*& fit, arr3 & temps);
void drawFit(TF1 *& fit, TCanvas *& canv, TF1 *& sub1, TF1 *& sub2, TF1 *& sub3);
void drawStats(TF1 *& fit, TPaveText *& stats, TCanvas *& canv);
void deleteAll(TF1 *& fit, TF1 *& sub1, TF1 *& sub2, TF1 *& sub3, TPaveText *& text, TCanvas *& lincanv, TPaveText *& stats, TCanvas *& logcanv);
