#ifndef __Combine__
#define __Combine__

#include "../Common.hh"

#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"
#include "TString.h"

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
  std::map<TString,Float_t> rvalmap;
};

namespace Combine
{
  // setup functions
  void SetupEntryVec(const Bool_t doObserved);
  void SetupGMSB(const TString & filename);
  void RemoveGMSBSamples();
  void SetupGMSBSubGroups();

  // input maps
  extern std::vector<TString> EntryVec;
  extern std::map<TString,GMSBinfo> GMSBMap;
  extern std::map<TString,std::vector<TString> > GMSBSubGroupMap;
};

#endif
