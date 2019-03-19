#include "Common.cpp+"
#include "TString.h"

// special enums
enum FitType {Linear, Quadratic};
enum CutType {none, eta1, eta2};
enum XType {pt, rho};
enum YType {ecalpfcliso, hcalpfcliso, trkiso};
enum CorrType {uncorr, pt_quad_q0p5, pt_quad_q0p7, pt_quad_q0p9};

// FitInfo
struct FitInfo
{
  FitInfo() {}
  FitInfo(const TString & formula, const TString & text, const std::vector<TString> & names)
    : formula(formula), text(text), names(names) {}

  TString formula;
  TString text;
  std::vector<TString> names;
};
	   
// XInfo
struct XInfo
{
  XInfo() {}
  XInfo(const TString & var, const TString & title, const std::vector<Double_t> & bins)
    : var(var), title(title), bins(bins) {}

  TString var;
  TString title;
  std::vector<Double_t> bins;
};

// YInfo
struct YInfo
{
  YInfo() {}
  YInfo(const TString & var, const TString & title)
    : var(var), title(title) {}

  TString var;
  TString title;
};

// prototypes
void makePlots(TTree * tree, TFile * outfile, 
	       const TString & xname, const XInfo & xInfo, 
	       const TString & yname, const YInfo & yInfo, const TString & yCorrection,
	       const FitType fitType, const CutType & cutType);

void makeQuantiles(TCanvas * canv, TH2F * hist2D, TFile * outfile,
		   const TString & yname, const FitType & fitType,
		   const TString & quantile);

// config
namespace Config
{
  static const TString commoncut = "(evtwgt*puwgt)*((phopt_0>70)&&(phoisEB_0==1)&&(phoisOOT_0==0)&&(phoisGen_0==1))";
  static const TString indir = Common::eosPreFix+"//eos/cms/store/user/kmcdermo/nTuples/skims/ootID/madv2_v4/MC";
  static const TString subdir = "GMSB/L-200TeV_Ctau-400cm"; // "GJet/Pt-15To6000"
  static const TString outdir = "/eos/user/k/kmcdermo/www/dispho/plots/ootVID";

  // fits
  static const std::map<FitType,FitInfo> fits =
  {
    {FitType::Linear,{"[0]*x+[1]","mx+b",{"m","b"}}},
    {FitType::Quadratic,{"[0]*x*x+[1]*x+[2]","ax^{2}+bx+c",{"a","b","c"}}}
  };

  // cuts
  static const std::map<CutType,TString> cuts =
  {
    {CutType::none,""},
    {CutType::eta1,"&&(abs(phosceta_0)<0.8)"},
    {CutType::eta2,"&&((abs(phosceta_0)>=0.8)&&(abs(phosceta_0)<1.4442))"},
  };

  // make xnames
  static const std::map<XType,TString> xNames = 
  {
    {XType::pt,"pt"},
    {XType::rho,"rho"}
  };
  
  // make xinfos: variables to correct for
  static const std::map<XType,XInfo> xInfos =
  {
    {XType::pt,{"phopt_0","Leading Photon p_{T} [GeV]",{0,10,20,30,40,50,60,70,80,90,100,120,140,160,180,200,250,300,350,400,500,600,800,1000}}},
    {XType::rho,{"rho","#rho [GeV]",{0,5,10,12,14,16,18,20,22,24,26,28,30,35,40,45,50,70}}}
  };

  // make ynames
  static const std::map<YType,TString> yNames = 
  {
    {YType::ecalpfcliso,"ecalpfcliso"},
    {YType::hcalpfcliso,"hcalpfcliso"},
    {YType::trkiso,"trkiso"}
  };
  
  // make yinfos : isolations
  static const std::map<YType,YInfo> yInfos =
  {
    {YType::ecalpfcliso,{"phoEcalPFClIso_0","Leading Photon ECAL PF Cluster Iso"}},
    {YType::hcalpfcliso,{"phoHcalPFClIso_0","Leading Photon HCAL PF Cluster Iso"}},
    {YType::trkiso,{"phoTrkIso_0","Leading Photon Tracker Iso"}}
  };
  
  // make corrections
  static const std::map<CorrType,std::map<YType,TString> > yCorrectionsMap =
  {
    {CorrType::uncorr,
     {
       {YType::ecalpfcliso,""},
       {YType::hcalpfcliso,""},
       {YType::trkiso,""}
     }
    },
    {CorrType::pt_quad_q0p5,
     {
       {YType::ecalpfcliso,"-((5.742e-6*phopt_0*phopt_0)+(-0.00298*phopt_0))"},
       {YType::hcalpfcliso,"-((1.503e-5*phopt_0*phopt_0)+(-0.003694*phopt_0))"},
       {YType::trkiso,"-((1.51e-6*phopt_0*phopt_0)+(-0.000692*phopt_0))"}
     }
    },
    {CorrType::pt_quad_q0p7,
     {
       {YType::ecalpfcliso,"-((1.139e-5*phopt_0*phopt_0)+(-0.004931*phopt_0))"},
       {YType::hcalpfcliso,"-((2.571e-5*phopt_0*phopt_0)+(-0.007202*phopt_0))"},
       {YType::trkiso,"-((1.629e-5*phopt_0*phopt_0)+(-0.008355*phopt_0))"}
     }
    },
    {CorrType::pt_quad_q0p9,
     {
       {YType::ecalpfcliso,"-((0.0001457*phopt_0*phopt_0)+(-0.0674*phopt_0))"},
       {YType::hcalpfcliso,"-((0.00015*phopt_0*phopt_0)+(-0.03763*phopt_0))"},
       {YType::trkiso,"-((0.0002049*phopt_0*phopt_0)+(-0.1092*phopt_0))"}
     }
    }    
  };

  // ybins
  std::vector<Double_t> ybins;
  void setupYbins()
  {
    for (auto ibin = 0 ; ibin < 100; ibin++) ybins.emplace_back(  0.01 * ibin);
    for (auto ibin = 10; ibin < 100; ibin++) ybins.emplace_back(  0.10 * ibin);
    for (auto ibin = 10; ibin < 100; ibin++) ybins.emplace_back(  1.00 * ibin);
    for (auto ibin = 10; ibin < 100; ibin++) ybins.emplace_back( 10.00 * ibin);
    for (auto ibin = 10; ibin < 101; ibin++) ybins.emplace_back(100.00 * ibin);
  }
};
