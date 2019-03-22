#include "Common.cpp+"
#include "TString.h"

// special enums
enum FitType {Horizontal, Linear, Quadratic, Exponential};
enum CutType {none, eta_LT0p8, eta_GTE0p8_LT1p4442, eta_LT0p5, eta_GTE0p5_LT1p0, eta_GTE1p0_LT1p4442};
enum XType {pt, rho, eta, sceta};
enum YType {ecalpfcliso, hcalpfcliso, trkiso, smaj};
enum CorrType {uncorr, pt_lin_q0p7, pt_quad_q0p7, pt_lin_q0p8, pt_quad_q0p8, 
	       rho_lin_q0p7_eta_LT0p8, rho_lin_q0p7_eta_GTE0p8_LT1p4442, rho_lin_q0p8_eta_LT0p8, rho_lin_q0p8_eta_GTE0p8_LT1p4442, 
	       pt_corrs, pt_corrs_v2, rho_corrs, rho_corrs_v2, pt_rho_corrs, pt_rho_corrs_v2,
	       eta_lin_q0p7_eta_GTE0p8_LT1p4442, pt_exp_q0p7, eta_corrs, eta_pt_corrs,
	       eta_lin_q0p8_eta_GTE0p8_LT1p4442, pt_exp_q0p8, eta_corrs_v2, eta_pt_corrs_v2 };

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
void makePlots(TTree * tree, const TString & commoncut, TFile * outfile, 
	       const TString & xname, const XInfo & xInfo, 
	       const TString & yname, const YInfo & yInfo, const TString & yCorrection,
	       const FitType fitType, const CutType & cutType);

void makeQuantiles(TCanvas * canv, TH2F * hist2D, TFile * outfile,
		   const TString & yname, const FitType & fitType,
		   const TString & quantile);

// config
namespace Config
{
  // fits
  static const std::map<FitType,FitInfo> fits =
  {
    {FitType::Horizontal,{"[0]","y=c",{"c"}}},
    {FitType::Linear,{"[0]*x+[1]","y=mx+b",{"m","b"}}},
    {FitType::Quadratic,{"[0]*x*x+[1]*x+[2]","y=ax^{2}+bx+c",{"a","b","c"}}},
    {FitType::Exponential,{"[0]*exp([1]*x+[2])+[3]","y=ae^{bx+c}+d",{"a","b","c","d"}}}
  };

  // cuts
  static const std::map<CutType,TString> cuts =
  {
    {CutType::none,""},
    {CutType::eta_LT0p8,"&&(abs(phosceta_0)<0.8)"},
    {CutType::eta_GTE0p8_LT1p4442,"&&((abs(phosceta_0)>=0.8)&&(abs(phosceta_0)<1.4442))"},
    {CutType::eta_LT0p5,"&&(abs(phosceta_0)<0.5)"},
    {CutType::eta_GTE0p5_LT1p0,"&&((abs(phosceta_0)>=0.5)&&(abs(phosceta_0)<1.0))"},
    {CutType::eta_GTE1p0_LT1p4442,"&&((abs(phosceta_0)>=1.0)&&(abs(phosceta_0)<1.4442))"}
  };

  // make xnames
  static const std::map<XType,TString> xNames = 
  {
    {XType::pt,"pt"},
    {XType::rho,"rho"},
    {XType::eta,"eta"},
    {XType::sceta,"sceta"}
  };
  
  // make xinfos: variables to correct for
  static const std::map<XType,XInfo> xInfos =
  {
    {XType::pt,{"phopt_0","Leading Photon p_{T} [GeV]",{0,10,20,30,40,50,60,70,80,90,100,120,140,160,180,200,250,300,350,400,500,600,800,1000}}},
    {XType::rho,{"rho","#rho [GeV]",{0,5,10,12,14,16,18,20,22,24,26,28,30,35,40,45,50,70}}},
    {XType::eta,{"abs(phoeta_0)","Leading Photon #eta",{0.0,0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,1.0,1.1,1.2,1.3,1.4,1.5}}},
    {XType::sceta,{"abs(phosceta_0)","Leading Photon SC #eta",{0.0,0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,1.0,1.1,1.2,1.3,1.4,1.5}}}
  };

  // make ynames
  static const std::map<YType,TString> yNames = 
  {
    {YType::ecalpfcliso,"ecalpfcliso"},
    {YType::hcalpfcliso,"hcalpfcliso"},
    {YType::trkiso,"trkiso"},
    {YType::smaj,"smajor"}
  };
  
  // make yinfos : isolations
  static const std::map<YType,YInfo> yIsos =
  {
    {YType::ecalpfcliso,{"phoEcalPFClIso_0","Leading Photon ECAL PF Cluster Iso"}},
    {YType::hcalpfcliso,{"phoHcalPFClIso_0","Leading Photon HCAL PF Cluster Iso"}},
    {YType::trkiso,{"phoTrkIso_0","Leading Photon Tracker Iso"}}
  };

  // for y -vars that are eta dependent
  static const std::map<YType,YInfo> ySmaj =
  {
    {YType::smaj,{"phosmaj_0","Leading Photon S_{Major}"}}
  };
  
  // make corrections
  static std::map<CorrType,std::map<YType,TString> > yCorrectionsMap =
  {
    {CorrType::uncorr,
     {
       {YType::ecalpfcliso,""},
       {YType::hcalpfcliso,""},
       {YType::trkiso,""},
       {YType::smaj,""}
     }
    },

    {CorrType::pt_lin_q0p7,
     {
       {YType::ecalpfcliso,"-(0.001578*phopt_0)"},
       {YType::hcalpfcliso,"-(0.007156*phopt_0)"},
       {YType::trkiso,"-(0.0004748*phopt_0)"}
     }
    },
    {CorrType::pt_quad_q0p7,
     {
       {YType::ecalpfcliso,"-((3.052e-6*phopt_0*phopt_0)+(-0.0001885*phopt_0))"},
       {YType::hcalpfcliso,"-((1.691e-5*phopt_0*phopt_0)+(-0.002597*phopt_0))"},
       {YType::trkiso,"-((3.068e-6*phopt_0*phopt_0)+(-0.001327*phopt_0))"}
     }
    },

    {CorrType::pt_lin_q0p8,
     {
       {YType::ecalpfcliso,"-(0.003008*phopt_0)"},
       {YType::hcalpfcliso,"-(0.01107*phopt_0)"},
       {YType::trkiso,"-(0.001175*phopt_0)"}
     }
    },
    {CorrType::pt_quad_q0p8,
     {
       {YType::ecalpfcliso,"-((6.612e-6*phopt_0*phopt_0)+(-0.0008183*phopt_0))"},
       {YType::hcalpfcliso,"-((2.921e-5*phopt_0*phopt_0)+(-0.005802*phopt_0))"},
       {YType::trkiso,"-((7.07e-6*phopt_0*phopt_0)+(-0.002976*phopt_0))"}
     }
    },

    {CorrType::rho_lin_q0p7_eta_LT0p8, // first apply pt_corrs!
     {
       {YType::ecalpfcliso,"-(0.1073*rho)"},
       {YType::hcalpfcliso,"-(0.073*rho)"},
       {YType::trkiso,"-(0.01147*rho)"}
     }
    },
    {CorrType::rho_lin_q0p7_eta_GTE0p8_LT1p4442, // first apply pt_corrs!
     {
       {YType::ecalpfcliso,"-(0.08317*rho)"},
       {YType::hcalpfcliso,"-(0.07983*rho)"},
       {YType::trkiso,"-(0.005256*rho)"}
     }
    },

    {CorrType::rho_lin_q0p8_eta_LT0p8, // first apply pt_corrs!
     {
       {YType::ecalpfcliso,"-(0.1324*rho)"},
       {YType::hcalpfcliso,"-(0.1094*rho)"},
       {YType::trkiso,"-(0.02276*rho)"}
     }
    },
    {CorrType::rho_lin_q0p8_eta_GTE0p8_LT1p4442, // first apply pt_corrs!
     {
       {YType::ecalpfcliso,"-(0.08638*rho)"},
       {YType::hcalpfcliso,"-(0.09392*rho)"},
       {YType::trkiso,"-(0.00536*rho)"}
     }
    },

    {CorrType::eta_lin_q0p7_eta_GTE0p8_LT1p4442,
     {
       {YType::smaj,"-(0.1407*(abs(phosceta_0)-0.8))"}
     }
    },
    {CorrType::pt_exp_q0p7,
     {
       {YType::smaj,"-(0.1054*exp(-0.01662*phopt_0+1.333))"}
     }
    },
    
    {CorrType::eta_lin_q0p8_eta_GTE0p8_LT1p4442,
     {
       {YType::smaj,"-(0.311*(abs(phosceta_0)-0.8))"}
     }
    },
    {CorrType::pt_exp_q0p8,
     {
       {YType::smaj,"-(0.1465*exp(-0.01775*phopt_0+1.948))"}
     }
    }
   
  };
  
  void setupComboCorrs()
  {
    // change cuts from && to *
    auto cut_eta_LT0p8 = Config::cuts.at(CutType::eta_LT0p8);
    cut_eta_LT0p8.ReplaceAll("&&","*");
    
    auto cut_eta_GTE0p8_LT1p4442 = Config::cuts.at(CutType::eta_GTE0p8_LT1p4442);
    cut_eta_GTE0p8_LT1p4442.ReplaceAll("&&","*");

    // pt corrections
    Config::yCorrectionsMap[CorrType::pt_corrs] =
    {
      {YType::ecalpfcliso,Config::yCorrectionsMap[CorrType::pt_lin_q0p7][YType::ecalpfcliso]},
      {YType::hcalpfcliso,Config::yCorrectionsMap[CorrType::pt_quad_q0p7][YType::hcalpfcliso]},
      {YType::trkiso,""}
    };

    Config::yCorrectionsMap[CorrType::pt_corrs_v2] =
    {
      {YType::ecalpfcliso,Config::yCorrectionsMap[CorrType::pt_lin_q0p8][YType::ecalpfcliso]},
      {YType::hcalpfcliso,Config::yCorrectionsMap[CorrType::pt_quad_q0p8][YType::hcalpfcliso]},
      {YType::trkiso,""}
    };

    // rho corrections
    Config::yCorrectionsMap[CorrType::rho_corrs] =
    {
      {YType::ecalpfcliso,
       Config::yCorrectionsMap[CorrType::rho_lin_q0p7_eta_LT0p8][YType::ecalpfcliso]+cut_eta_LT0p8+
       Config::yCorrectionsMap[CorrType::rho_lin_q0p7_eta_GTE0p8_LT1p4442][YType::ecalpfcliso]+cut_eta_GTE0p8_LT1p4442},

      {YType::hcalpfcliso,
       Config::yCorrectionsMap[CorrType::rho_lin_q0p7_eta_LT0p8][YType::hcalpfcliso]+cut_eta_LT0p8+
       Config::yCorrectionsMap[CorrType::rho_lin_q0p7_eta_GTE0p8_LT1p4442][YType::hcalpfcliso]+cut_eta_GTE0p8_LT1p4442},

      {YType::trkiso,
       Config::yCorrectionsMap[CorrType::rho_lin_q0p7_eta_LT0p8][YType::trkiso]+cut_eta_LT0p8+
       Config::yCorrectionsMap[CorrType::rho_lin_q0p7_eta_GTE0p8_LT1p4442][YType::trkiso]+cut_eta_GTE0p8_LT1p4442}
    };

    Config::yCorrectionsMap[CorrType::rho_corrs_v2] =
    {
      {YType::ecalpfcliso,
       Config::yCorrectionsMap[CorrType::rho_lin_q0p8_eta_LT0p8][YType::ecalpfcliso]+cut_eta_LT0p8+
       Config::yCorrectionsMap[CorrType::rho_lin_q0p8_eta_GTE0p8_LT1p4442][YType::ecalpfcliso]+cut_eta_GTE0p8_LT1p4442},

      {YType::hcalpfcliso,
       Config::yCorrectionsMap[CorrType::rho_lin_q0p8_eta_LT0p8][YType::hcalpfcliso]+cut_eta_LT0p8+
       Config::yCorrectionsMap[CorrType::rho_lin_q0p8_eta_GTE0p8_LT1p4442][YType::hcalpfcliso]+cut_eta_GTE0p8_LT1p4442},

      {YType::trkiso,
       Config::yCorrectionsMap[CorrType::rho_lin_q0p8_eta_LT0p8][YType::trkiso]+cut_eta_LT0p8+
       Config::yCorrectionsMap[CorrType::rho_lin_q0p8_eta_GTE0p8_LT1p4442][YType::trkiso]+cut_eta_GTE0p8_LT1p4442}
    };

    // pt and rho corrections
    Config::yCorrectionsMap[CorrType::pt_rho_corrs] =
    {
      {YType::ecalpfcliso,
       Config::yCorrectionsMap[CorrType::pt_corrs][YType::ecalpfcliso]+Config::yCorrectionsMap[CorrType::rho_corrs][YType::ecalpfcliso]},
      {YType::hcalpfcliso,
       Config::yCorrectionsMap[CorrType::pt_corrs][YType::hcalpfcliso]+Config::yCorrectionsMap[CorrType::rho_corrs][YType::hcalpfcliso]},
      {YType::trkiso,
       Config::yCorrectionsMap[CorrType::pt_corrs][YType::trkiso]+Config::yCorrectionsMap[CorrType::rho_corrs][YType::trkiso]}
    };

    Config::yCorrectionsMap[CorrType::pt_rho_corrs_v2] =
    {
      {YType::ecalpfcliso,
       Config::yCorrectionsMap[CorrType::pt_corrs_v2][YType::ecalpfcliso]+Config::yCorrectionsMap[CorrType::rho_corrs_v2][YType::ecalpfcliso]},
      {YType::hcalpfcliso,
       Config::yCorrectionsMap[CorrType::pt_corrs_v2][YType::hcalpfcliso]+Config::yCorrectionsMap[CorrType::rho_corrs_v2][YType::hcalpfcliso]},
      {YType::trkiso,
       Config::yCorrectionsMap[CorrType::pt_corrs_v2][YType::trkiso]+Config::yCorrectionsMap[CorrType::rho_corrs_v2][YType::trkiso]}
    };

    // eta corrections
    Config::yCorrectionsMap[CorrType::eta_corrs] =
    {
      {YType::smaj,
       Config::yCorrectionsMap[CorrType::eta_lin_q0p7_eta_GTE0p8_LT1p4442][YType::smaj]+cut_eta_GTE0p8_LT1p4442}
    };

    Config::yCorrectionsMap[CorrType::eta_corrs_v2] =
    {
      {YType::smaj,
       Config::yCorrectionsMap[CorrType::eta_lin_q0p8_eta_GTE0p8_LT1p4442][YType::smaj]+cut_eta_GTE0p8_LT1p4442}
    };

    // eta pt corrections for smaj
    Config::yCorrectionsMap[CorrType::eta_pt_corrs] =
    {
      {YType::smaj,
       Config::yCorrectionsMap[CorrType::eta_corrs][YType::smaj]+Config::yCorrectionsMap[CorrType::pt_exp_q0p7][YType::smaj]}
    };

    Config::yCorrectionsMap[CorrType::eta_pt_corrs_v2] =
    {
      {YType::smaj,
       Config::yCorrectionsMap[CorrType::eta_corrs_v2][YType::smaj]+Config::yCorrectionsMap[CorrType::pt_exp_q0p8][YType::smaj]}
    };
  };

  // ybins
  std::vector<Double_t> ybins;
  void setupYbins()
  {
    for (auto ibin = 0 ; ibin < 100; ibin++) Config::ybins.emplace_back(  0.01 * ibin);
    for (auto ibin = 10; ibin < 100; ibin++) Config::ybins.emplace_back(  0.10 * ibin);
    for (auto ibin = 10; ibin < 100; ibin++) Config::ybins.emplace_back(  1.00 * ibin);
    for (auto ibin = 10; ibin < 100; ibin++) Config::ybins.emplace_back( 10.00 * ibin);
    for (auto ibin = 10; ibin < 101; ibin++) Config::ybins.emplace_back(100.00 * ibin);
  }
};
