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

void Setup(std::map<TString,vector<TString> > & SignalSubGroupMap, std::map<TString,Color_t> & SignalSubGroupColorMap, TStyle *& tdrStyle);
void SetupSignalSubGroups(std::map<TString,vector<TString> > & SignalSubGroupMap);
void SetupSignalSubGroupColors(std::map<TString,Color_t> & SignalSubGroupColorMap);

void computeSignalEfficiency(const TString & infilename, const TString & outtext);

#endif
