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

inline Double_t rad2 (Double_t x, Double_t y){return x*x + y*y;}
