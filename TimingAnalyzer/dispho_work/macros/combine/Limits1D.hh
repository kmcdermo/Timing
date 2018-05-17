#include "../Common.hh"

#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"
#include "TString.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TStyle.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <map>

struct GMSBinfo
{
  GMSBinfo() {}
  GMSBinfo(const Int_t lambda, const Float_t ctau, const Float_t mass, const Float_t width, const Float_t br)
    : lambda(lambda), ctau(ctau), mass(mass), width(width), br(br) {}

  // model params
  Int_t lambda; // TeV
  Float_t ctau; // cm
  Float_t mass; // GeV/c^2
  Float_t width; // GeV
  Float_t br;
  
  // theoretical
  Float_t xsec; // pb
  Float_t exsec;

  // r-values
  Float_t rexp;
  Float_t r1sigup;
  Float_t r1sigdown;
  Float_t r2sigup;
  Float_t r2sigdown;
};

class Limits1D
{
public:
  Limits1D(const TString & indir, const TString & infilename, const TString & outtext);
  ~Limits1D();

  void SetupGMSB();
  void MakeLimits1D();

private:
  const TString fInDir;
  const TString fInFileName;
  const TString fOutText;

  std::map<TString,GMSBinfo> fGMSBMap;

};
