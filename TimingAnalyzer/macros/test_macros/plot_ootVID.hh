#include "Common.cpp+"

// special enums
enum XType {pt, eta, sceta, nvtx, wgttime};
enum CutType {hoe, sieie, ecal, hcal, trk, smaj};

// structs
struct Observable
{
  Observable() {}
  Observable(const TString & var, const TString & inequality, const TString & loose, const TString & tight, const TString & name, const TString & label)
    : var(var), inequality(inequality), loose(loose), tight(tight), name(name), label(label) {}
  
  TString cut(const Bool_t isLoose) const {return "(("+var+")"+inequality+(isLoose?loose:tight)+")";}

  TString var;
  TString inequality;
  TString loose;
  TString tight;
  TString name;
  TString label;
};

struct Category
{
  Category() {}
  Category(const Bool_t isSig, const TString & denom_cut, const TString & numer_cut, const TString & name, const TString & label, const Color_t color)
    : isSig(isSig), denom_cut(denom_cut), numer_cut(numer_cut), name(name), label(label), color(color) {}

  Bool_t  isSig;
  TString denom_cut;
  TString numer_cut;
  TString name;
  TString label;
  Color_t color;
};

struct PlotInfo
{
  PlotInfo() {}
  PlotInfo(const TString & name, const TString & var, const std::vector<Double_t> & bins, const TString & title)
    : name(name), var(var), bins(bins), title(title) {}
  
  TString name;
  TString var;
  std::vector<Double_t> bins;
  TString title;
};

// protoypes
void make_plots(TTree * tree, TFile * outfile, const Bool_t isRatios, const Bool_t isLoose, const Bool_t isSig);
void make_inclusivePlots(TTree * bkgd_tree, TTree * sign_tree, TFile * outfile);
void set_eff(TH1F * hist, const TH1F * numer, const TH1F * denom);
TString get_commoncut(const Bool_t isSig);
void move_output(const TString & eosdir, const TString & outdir, const TString & dir);

// Config namespace
namespace Config
{
  // cut observables
  static const std::map<CutType,Observable> observables =
  {
    {CutType::hoe,{"phoHoE_0","<","0.04596","0.02148","HoE","H/E"}}, // old values = "0.0185","0.0165", current values are GED cut-based v2 VID (L,T)
    {CutType::sieie,{"phosieie_0","<","0.0106","0.00996","sieie","#sigma_{i#eta i#eta}"}}, // old values = "0.0125","0.011", current values are GED cut-based v2 VID (L,T)
    {CutType::ecal,{"max(phoEcalPFClIso_0-(0.003008*phopt_0)-(0.1324*rho)*(abs(phosceta_0)<0.8)-(0.08638*rho)*((abs(phosceta_0)>=0.8)*(abs(phosceta_0)<1.4442)),0.0)","<","8.0","5.0","ecalpfcliso","ECAL PF Cluster Iso"}}, // old values are "8.0","5.0" (q80 = 1.0)
    {CutType::hcal,{"max(phoHcalPFClIso_0-((2.921e-5*phopt_0*phopt_0)+(-0.005802*phopt_0))-(0.1094*rho)*(abs(phosceta_0)<0.8)-(0.09392*rho)*((abs(phosceta_0)>=0.8)*(abs(phosceta_0)<1.4442)),0.0)","<","10.0","4.0","hcalpfcliso","HCAL PF Cluster Iso"}}, // old values are = "12.0","10.0" (q80 = 2.0)
    {CutType::trk,{"max(phoTrkIso_0-(0.02276*rho)*(abs(phosceta_0)<0.8)-(0.00536*rho)*((abs(phosceta_0)>=0.8)*(abs(phosceta_0)<1.4442)),0.0)","<","10.0","4.0","trkiso","Tracker Iso"}}, // old values are = "8.5","5.5" (q80 = 2.0)
    {CutType::smaj,{"max(phosmaj_0-(0.311*(abs(phosceta_0)-0.8))*((abs(phosceta_0)>=0.8)*(abs(phosceta_0)<1.4442))-(0.1465*exp(-0.01775*phopt_0+1.948)),0.0)","<","1.0","0.35","smaj","S_{Major}"}} // old values are = "1.0","0.5"
  };

  // make plot vector
  static const std::map<XType,PlotInfo> plotinfos = 
  {
    {XType::pt,{"phopt_0","phopt_0",{0,20,40,60,80,100,125,150,200,250,300,400,500,750,1000,2000},"Leading Photon p_{T} [GeV]"}},
    {XType::eta,{"phoeta_0","phoeta_0",{-1.5,-1.4,-1.3,-1.2,-1.1,-1.0,-0.9,-0.8,-0.7,-0.6,-0.5,-0.4,-0.3,-0.2,-0.1,0.0,0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,1.0,1.1,1.2,1.3,1.4,1.5},"Leading Photon #eta"}},
    {XType::nvtx,{"nvtx","nvtx",{0,2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,35,40,45,50,60},"Number of Vertices"}},
    {XType::wgttime,{"photime_0","phoweightedtimeLT120_0+phoweightedtimeLT120SHIFT_0+phoweightedtimeLT120SMEAR_0",{-2,-1.5,-1,-0.75,-0.5,-0.25,0,0.25,0.5,0.75,1,1.5,2,3,5,10,15,25},"Leading Photon Cluster Time [ns]"}}
  };
};
