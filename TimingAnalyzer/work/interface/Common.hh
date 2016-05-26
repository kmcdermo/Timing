#ifndef _common_
#define _common_

#include "TString.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TH1F.h"

#include <map>

typedef std::map<TString,TString> TStrMap;
typedef TStrMap::iterator TStrMapIter;

void ComputeRatioPlot(const TH1F * numer, const TH1F * denom, TH1F *& ratioPlot);
void MakeOutDir(TString outdir);
void MakeSubDirs(TStrMap & subdirmap, TString outdir);
void MoveInput(TString infile, TString outdir);
void CMSLumi(TCanvas *& canv, Float_t lumi, TString extraText, Int_t iPosX = 0);
void SetTDRStyle(TStyle *& tdrStyle);

#endif
