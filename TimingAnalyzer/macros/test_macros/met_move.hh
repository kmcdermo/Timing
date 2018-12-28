#ifndef __met_move__
#define __met_move__

#include "TString.h"
#include "TStyle.h"
#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"
#include "TH1F.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TColor.h"

#include <vector>
#include <map>

#include "Common.cpp+"

struct Photon
{
  Photon(){}

  Float_t pt = 0.f; 
  Float_t phi = 0.f;
  Float_t eta = 0.f;
  Bool_t isOOT = 0;
  
  TBranch * b_pt = 0;
  TBranch * b_phi = 0;
  TBranch * b_eta = 0;
  TBranch * b_isOOT = 0;

  const std::string s_pt = "pt";
  const std::string s_phi = "phi";
  const std::string s_eta = "eta";
  const std::string s_isOOT = "isOOT";
};

struct HistInfo
{
  HistInfo(){}
  HistInfo(const TString name, const TString & title, const TString & xtitle, const TString & ytitle,
	   const Int_t nBinsX, const Float_t xlow, const Float_t xhigh, 
	   const TString & label, const Color_t color) :
    name(name), title(title), xtitle(xtitle), ytitle(ytitle),
    nBinsX(nBinsX), xlow(xlow), xhigh(xhigh),
    label(label), color(color) {}

  TString name;
  TString title;
  TString xtitle;
  TString ytitle;
  Int_t nBinsX;
  Float_t xlow;
  Float_t xhigh;
  TString label;
  Color_t color;
};

void MakeHists(std::map<TString,TH1F*> & HistMap, const std::vector<HistInfo> & Infos)
{
  for (const auto & Info : Infos)
  {
    HistMap[Info.name] = new TH1F(Info.name.Data(),Info.title.Data(),Info.nBinsX,Info.xlow,Info.xhigh);
    
    auto & hist = HistMap[Info.name];
    
    hist->GetXaxis()->SetTitle(Info.xtitle.Data());
    hist->GetYaxis()->SetTitle(Info.ytitle.Data());

    hist->SetLineColor(Info.color);
    hist->SetMarkerColor(Info.color);
    
    hist->Sumw2();
  }
}

inline Float_t deltaR(const Float_t phi1, const Float_t eta1, const Float_t phi2, const Float_t eta2)
{
  auto delphi = std::abs(phi1-phi2);
  if (delphi > Common::PI) delphi -= Common::TwoPI;
  return Common::hypot(eta1-eta2,delphi);
}

inline void resetBoolVec(std::vector<Bool_t> & vec)
{
  for (auto && obj : vec) obj = false;
}

#endif
