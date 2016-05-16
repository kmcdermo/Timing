#ifndef _common_
#define _common_

#include "TString.h"
#include "TCanvas.h"
#include "TStyle.h"

void MakeOutDir(TString outdir);
void MoveInput(TString infile, TString outdir);
void CMSLumi(TCanvas *& canv, Float_t lumi, TString extraText, Int_t iPosX = 0);
void SetTDRStyle(TStyle *& tdrStyle);

#endif
