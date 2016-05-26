#ifndef _common_
#define _common_

#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TCanvas.h"
#include "TString.h"
#include "TStyle.h"
#include "TColor.h"

#include <map>
#include <vector>
#include <iostream>
#include <fstream>

// global typedefs

typedef std::map<TString,TString> TStrMap;
typedef TStrMap::iterator         TStrMapIter;

typedef std::map<TString,Bool_t> TStrBoolMap;
typedef TStrBoolMap::iterator    TStrBoolMapIter;

typedef std::map<TString,Color_t> ColorMap;

typedef std::vector<TString>  TStrVec;
typedef std::vector<Int_t>    IntVec;
typedef std::vector<Float_t>  FltVec;
typedef std::vector<Double_t> DblVec;

// global functions

void ComputeRatioPlot(const TH1F * numer, const TH1F * denom, TH1F *& ratioPlot);
void MakeOutDir(TString outdir);
void MakeSubDirs(TStrMap & subdirmap, TString outdir);
void MoveInput(TString infile, TString outdir);
void CheckValidFile(TFile *& file, TString fname);
void CheckValidTree(TTree *& tree, TString tname, TString fname);
void CheckValidTH1F(TH1F *& plot, TString pname, TString fname);
void CMSLumi(TCanvas *& canv, Float_t lumi, TString extraText, Int_t iPosX = 0);
void SetTDRStyle(TStyle *& tdrStyle);

#endif
