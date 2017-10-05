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

const char* PrintBool(const Bool_t var);
void next_arg_or_die(lStr_t& args, lStr_i& i, const Bool_t allow_single_minus=false);
void ComputeRatioPlot(const TH1F * numer, const TH1F * denom, TH1F *& ratioPlot);
void MakeOutDir(const TString & outdir);
void MakeSubDirs(TStrMap & subdirmap, const TString & outdir, const TString & extra = "");
void MoveInput(const TString & infile, const TString & outdir);
void CheckValidFile(const TFile * file, const TString & fname);
void CheckValidTree(const TTree * tree, const TString & tname, const TString & fname);
void CheckValidTH1F(const TH1F * plot, const TString & pname, const TString & fname);
void CheckValidTH1D(const TH1D * plot, const TString & pname, const TString & fname);
void CMSLumi(TCanvas *& canv, const Int_t iPosX = 10);
void SetTDRStyle(TStyle *& tdrStyle);

#endif
