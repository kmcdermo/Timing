#include "../Common.hh"

#include "TROOT.h"
#include "TStyle.h"
#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"
#include "TString.h"
#include "TGraph.h"
#include "TLegend.h"
#include "TCanvas.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <map>

struct GMSBinfo
{
  GMSBinfo() {}
  GMSBinfo(const TString & s_lambda, const Int_t lambda, const TString & s_ctau, const Float_t ctau, const Float_t mass, const Float_t width, const Float_t br)
    : s_lambda(s_lambda), lambda(lambda), s_ctau(s_ctau), ctau(ctau), mass(mass), width(width), br(br) {}

  // model params
  TString s_lambda;
  Int_t lambda; // TeV
  TString s_ctau;
  Float_t ctau; // cm
  Float_t mass; // GeV/c^2
  Float_t width; // GeV
  Float_t br;
  
  // theoretical
  Float_t xsec; // pb
  Float_t exsec;

  // r-values
  Float_t robs;
  Float_t r2sigdown;
  Float_t r1sigdown;
  Float_t rexp;
  Float_t r1sigup;
  Float_t r2sigup;
};

class Limits1D
{
public:
  Limits1D(const TString & indir, const TString & infilename, const Bool_t doobserved, const TString & outtext);
  ~Limits1D();

  // setup functions
  void SetupEntryMap();
  void SetupGMSB();
  void RemoveGMSBSamples();
  void SetupGMSBSubGroups();

  // main plotting routine
  void MakeLimits1D();

private:
  const TString fInDir;
  const TString fInFileName;
  const Bool_t fDoObserved;
  const TString fOutText;

  // input maps
  std::map<TString,Int_t> fEntryMap;
  std::map<TString,GMSBinfo> fGMSBMap;
  std::map<TString,std::vector<TString> > fGMSBSubGroupMap;

  // style
  TStyle * fTDRStyle;

  // output
  TFile * fOutFile;
};
