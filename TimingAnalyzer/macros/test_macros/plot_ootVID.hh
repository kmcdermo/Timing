#include "Common.cpp+"

// special enums
enum XType {pt, eta, sceta, nvtx, wgttime};
enum CutType {hoe, sieie, ecal, hcal, trk, smaj, smin, chghad, neuhad, pho};

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
  Category(const Bool_t isSig, const TString & cut, const TString & name, const TString & label, const Color_t color)
    : isSig(isSig), cut(cut), name(name), label(label), color(color) {}

  Bool_t  isSig;
  TString cut;
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
void make_CutFlows(TTree * tree, TFile * outfile, const Bool_t isGEDVID, const Bool_t isLoose, const Bool_t isSig);
void make_InclusiveEffs(TTree * bkgd_tree, TTree * sign_tree, TFile * outfile, const Bool_t isGEDVID);
void setup_CutFlowCategories(std::vector<Category> & categories, const Bool_t isLoose, const Bool_t isSig, const CutType cutType, const Int_t icolor);
void setup_InclusiveEffCategories(std::vector<Category> & categories, const Bool_t isGEDVID, const Bool_t isLoose, const Bool_t isSig);
TString get_commoncut(const Bool_t isSig, const Bool_t isGEDVID);
void set_eff(TH1F * hist, const TH1F * numer, const TH1F * denom);
void move_output(const TString & eosdir, const TString & outdir, const TString & dir);

// Config namespace
namespace Config
{
  // cut observables
  static const std::map<CutType,Observable> observables =
  {
    {CutType::hoe,{"phoHoE_0","<","0.02148","0.02148","HoE","H/E"}},
    {CutType::sieie,{"phosieie_0","<","0.017","0.00996","sieie","#sigma_{i#eta i#eta}"}},
    {CutType::chghad,{"max(phoChgHadIso_0-((0.0112*rho)*(abs(phosceta_0)<1.0))-((0.0108*rho)*((abs(phosceta_0)>=1.0)*(abs(phosceta_0)<1.479))),0)","<","0.65","0.65","chghad","Chg Had Iso"}},
    {CutType::neuhad,{"max(phoNeuHadIso_0-((0.01512*phopt_0)+(2.259e-5*phopt_0*phopt_0))-((0.0668*rho)*(abs(phosceta_0)<1.0))-((0.01054*rho)*((abs(phosceta_0)>=1.0)*(abs(phosceta_0)<1.479))),0)","<","0.317","0.317","neuhad","Neu Had Iso"}},
    {CutType::pho,{"max(phoPhoIso_0-(0.004017*phopt_0)-((0.1113*rho)*(abs(phosceta_0)<1.0))-((0.0953*rho)*((abs(phosceta_0)>=1.0)*(abs(phosceta_0)<1.479))),0)","<","2.044","2.044","pho","Pho Iso"}},
    {CutType::ecal,{"max(phoEcalPFClIso_0-(0.003008*phopt_0)-(0.1324*rho)*(abs(phosceta_0)<0.8)-(0.08638*rho)*((abs(phosceta_0)>=0.8)*(abs(phosceta_0)<1.4442)),0.0)","<","5.0","5.0","ecalpfcliso","ECAL PF Cluster Iso"}},
    {CutType::hcal,{"max(phoHcalPFClIso_0-((2.921e-5*phopt_0*phopt_0)+(-0.005802*phopt_0))-(0.1094*rho)*(abs(phosceta_0)<0.8)-(0.09392*rho)*((abs(phosceta_0)>=0.8)*(abs(phosceta_0)<1.4442)),0.0)","<","4.0","4.0","hcalpfcliso","HCAL PF Cluster Iso"}},
    {CutType::trk,{"max(phoTrkIso_0-(0.02276*rho)*(abs(phosceta_0)<0.8)-(0.00536*rho)*((abs(phosceta_0)>=0.8)*(abs(phosceta_0)<1.4442)),0.0)","<","4.0","4.0","trkiso","Tracker Iso"}},
    {CutType::smaj,{"phosmaj_0","<","1.3","1.3","smaj","S_{Major}"}},
    {CutType::smin,{"phosmin_0","<","0.4","0.4","smin","S_{Minor}"}},
  };

  // make plot vector
  static const std::map<XType,PlotInfo> plotinfos = 
  {
    {XType::pt,{"phopt_0","phopt_0",{70,90,110,150,200,300,500},"Leading Photon p_{T} [GeV]"}},
    {XType::eta,{"phosceta_0","phosceta_0",{-1.5,-1.2,-0.9,-0.6,-0.3,0.0,0.3,0.6,0.9,1.2,1.5},"Leading Photon SC #eta"}},
    {XType::nvtx,{"nvtx","nvtx",{0,5,10,15,20,25,30,35,40,45,50,60},"Number of Vertices"}},
    {XType::wgttime,{"photime_0","phoweightedtimeLT120_0+phoweightedtimeLT120SHIFT_0+phoweightedtimeLT120SMEAR_0",{-2,-1,0,1,2,5,10},"Leading Photon Cluster Time [ns]"}}
  };
};
