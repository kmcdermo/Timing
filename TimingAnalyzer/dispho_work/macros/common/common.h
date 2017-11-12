#ifndef __common__
#define __common__

#include "TCanvas.h"
#include "TStyle.h"
#include "TString.h"

// global functions

void CMSLumi(TCanvas *& canv, TString extraText = "", Int_t iPosX = 10);
void SetTDRStyle(TStyle *& tdrStyle);

#endif
