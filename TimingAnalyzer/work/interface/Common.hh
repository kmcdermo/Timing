#ifndef _common_
#define _common_

#include "Config.hh"

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
#include <list>

// global typedefs

typedef std::list<std::string> lStr_t;
typedef lStr_t::iterator       lStr_i;

typedef std::map<TString,TString> TStrMap;
typedef TStrMap::iterator         TStrMapIter;

typedef std::map<TString,Bool_t> TStrBoolMap;
typedef TStrBoolMap::iterator    TStrBoolMapIter;

typedef std::map<TString,Color_t> ColorMap;
typedef std::map<TString,Int_t>   TStrIntMap;

typedef std::map<TString,Float_t> TStrFltMap;
typedef TStrFltMap::iterator      TStrFltMapIter;

typedef std::vector<TString>  TStrVec;
typedef std::vector<Int_t>    IntVec;
typedef std::vector<Float_t>  FltVec;
typedef std::vector<Double_t> DblVec;

// global functions

void next_arg_or_die(lStr_t& args, lStr_i& i, bool allow_single_minus=false);
void ComputeRatioPlot(const TH1F * numer, const TH1F * denom, TH1F *& ratioPlot);
void MakeOutDir(TString outdir);
void MakeSubDirs(TStrMap & subdirmap, TString outdir);
void MoveInput(TString infile, TString outdir);
void CheckValidFile(TFile *& file, TString fname);
void CheckValidTree(TTree *& tree, TString tname, TString fname);
void CheckValidTH1F(TH1F *& plot, TString pname, TString fname);
void CMSLumi(TCanvas *& canv, Int_t iPosX = 0);
void SetTDRStyle(TStyle *& tdrStyle);

#endif
