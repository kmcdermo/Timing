#ifndef _common_
#define _common_

#include "CommonTypes.hh"
#include "Config.hh"

#include "TFile.h"
#include "TTree.h"
#include "TStyle.h"
#include "TCanvas.h"
#include "TH1F.h"

#include <iostream>
#include <fstream>

// global functions

void next_arg_or_die(lStr_t& args, lStr_i& i, bool allow_single_minus=false);
void ComputeRatioPlot(const TH1F * numer, const TH1F * denom, TH1F *& ratioPlot);
void MakeOutDir(TString outdir);
void MakeSubDirs(TStrMap & subdirmap, TString outdir);
void MoveInput(TString infile, TString outdir);
void CheckValidFile(TFile *& file, TString fname);
void CheckValidTree(TTree *& tree, TString tname, TString fname);
void CheckValidTH1F(TH1F *& plot, TString pname, TString fname);
void CheckValidTH1D(TH1D *& plot, TString pname, TString fname);
void CMSLumi(TCanvas *& canv, Int_t iPosX = 10);
void SetTDRStyle(TStyle *& tdrStyle);

#endif
