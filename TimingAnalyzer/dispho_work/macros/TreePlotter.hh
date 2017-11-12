#ifndef __TreePlotter__
#define __TreePlotter__

#include "TSyle.h"
#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TString.h"
#include "TColor.h"
#include "TPad.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TLine.h"
#include "THStack.h"

#include <iostream>
#include <fstream>
#include <cmath>
#include <map>
#include <vector>
#include <string>
#include <algorithm>

void InitPlotter(TStyle * tdrStyle, const TString & title, 
		 const Int_t nbinsx, const Float_t xlow, const Float_t xhigh,
		 const TString & xtitle, const TString & ytitle);
Float_t GetSampleWeight(TFile * file);
TH1F * SetupHist(const TString & name, const TString & title, 
		 const Int_t nbinsx, const Float_t xlow, const Float_t xhigh,
		 const TString & xtitle, const TString & ytitle);

enum SampleType {Data, GMSB, QCD, GJets};

namespace
{
  constexpr Bool_t isLogy = true;
  constexpr Float_t lumi = 35.0; // CMS lumi

  std::map<TString,SampleType> SampleMap;
  std::map<SampleType,TColor> ColorMap;
  std::map<SampleType,TString> CutMap;
  std::map<SampleType,TH1F*> HistMap;
  std::map<SampleType,TString> LabelMap;

  void SetupSamples()
  {
    // QCD
    SampleMap["MC/qcd/Pt-15to20"] = QCD;
    SampleMap["MC/qcd/Pt-20to30"] = QCD;
    SampleMap["MC/qcd/Pt-30to50"] = QCD;
    SampleMap["MC/qcd/Pt-50to80"] = QCD;
    SampleMap["MC/qcd/Pt-80to120"] = QCD;
    //    SampleMap["MC/qcd/Pt-120to170"] = QCD;
    SampleMap["MC/qcd/Pt-170to300"] = QCD;

    // GJets
    SampleMap["MC/gjets-EM"] = GJets;
   
    // GMSB
    SampleMap["MC/gmsb"] = GMSB;

    // Data
    SampleMap["DATA/singleph"] = Data;
  }
  void SetupColors()
  {
    ColorMap[QCD] = kGreen;
    ColorMap[GJets] = kRed;
    ColorMap[GMSB] = kBlue;
    ColorMap[Data] = kBlack;
  }
  void SetupCuts()
  {
    CutMap[QCD] = "";
    CutMap[GJets] = "";
    CutMap[GMSB] = ""
    CutMap[Data] = "";
  }
  void SetupLabels()
  {
    LabelMap[QCD] = "QCD"
    LabelMap[GJets] = "#gamma + Jets (EM Enriched)"
    LabelMap[GMSB] = "GMSB c#tau = 4 m, #Lambda = 200 TeV"
    LabelMap[Data] = "Data";
  }
  void SetupHists(const TString & title, 
		 const Int_t nbinsx, const Float_t xlow, const Float_t xhigh,
		 const TString & xtitle, const TString & ytitle)
  {
    HistMap[QCD]   = SetupHist("QCD_hist",title,nbinsx,xlow,xhigh,xtitle,ytitle);
    HistMap[GJets] = SetupHist("GJets_hist",title,nbinsx,xlow,xhigh,xtitle,ytitle);
    HistMap[GMSB]  = SetupHist("GMSB_hist",title,nbinsx,xlow,xhigh,xtitle,ytitle);
    HistMap[Data]  = SetupHist("Data_hist",title,nbinsx,xlow,xhigh,xtitle,ytitle);

    for (auto & HistPair : HistMap)
    {
      const auto & sample = HistPair.first;
      const Bool_t isMC = (sample != Data);
      HistPair->SetLineColor(ColorMap[sample]);
      HistPair->SetMarkerColor(ColorMap[sample]);
      if (isMC) HistPair->SetFillColor(ColorMap[sample]);
    }
  }

  // plotting style
  constexpr Float_t TitleSize    = 0.035;
  constexpr Float_t TitleXOffset = 1.1;
  constexpr Float_t TitleYOffset = 1.1;
  constexpr Float_t LabelOffset = 0.007;
  constexpr Float_t LabelSize   = 0.03;
  constexpr Float_t TickLength  = 0.03;
  constexpr Float_t TitleFF     = 1.4; 

  // up = upper pad, lp = lower pad
  constexpr Float_t left_up   = 0.0;
  constexpr Float_t bottom_up = 0.3;
  constexpr Float_t right_up  = 1.0;
  constexpr Float_t top_up    = 0.99;
  constexpr Float_t height_up = top_up - bottom_up;

  constexpr Float_t left_lp   = left_up;
  constexpr Float_t bottom_lp = 0.05;
  constexpr Float_t right_lp  = right_up;
  constexpr Float_t top_lp    = bottom_up;
  constexpr Float_t height_lp = top_lp - bottom_lp;
};

#endif
