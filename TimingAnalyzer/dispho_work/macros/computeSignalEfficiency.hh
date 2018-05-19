#ifndef __computeSignalEfficiency__
#define __computeSignalEfficiency__

#include "Common.cpp+"

#include "TStyle.h"
#include "TFile.h"
#include "TH1F.h"
#include "TEfficiency.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TString.h"
#include "TColor.h"

#include <map>
#include <vector>

void Setup(TStyle *& tdrStyle);
void computeSignalEfficiency(const TString & infilename, const TString & outtext);

#endif
