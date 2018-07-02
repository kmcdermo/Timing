#ifndef _skimmertypes_ 
#define _skimmertypes_ 

#include <string>
#include <vector>

#include "TBranch.h"

struct Configuration
{
  // branches
  UInt_t  blindSF;
  Bool_t  applyBlindSF;
  Float_t blindMET;
  Bool_t  applyBlindMET;
  Float_t jetpTmin;
  Float_t jetEtamax;
  Int_t   jetIDmin;
  Float_t rhEmin;
  Float_t phpTmin;
  std::string * phIDmin;
  std::string phIDmin_s;
  Float_t seedTimemin;
  Bool_t  splitPho;
  Bool_t  onlyGED;
  Bool_t  onlyOOT;
  Bool_t  storeRecHits;
  Bool_t  applyTrigger;
  Float_t minHT;
  Bool_t  applyHT;
  Float_t phgoodpTmin;
  std::string * phgoodIDmin;
  std::string phgoodIDmin_s;
  Bool_t  applyPhGood;
  Float_t dRmin;
  Float_t pTres;
  Float_t genpTres;
  Float_t trackdRmin;
  Float_t trackpTmin;
  std::string * inputPaths;
  std::string inputPaths_s;
  std::string * inputFilters;
  std::string inputFilters_s;
  std::string * inputFlags;
  std::string inputFlags_s;
  Bool_t  isGMSB;
  Bool_t  isHVDS;
  Bool_t  isBkgd;
  Bool_t  isToy;
  Bool_t  isADD;
  Float_t xsec;
  Float_t filterEff;
  Float_t BR;
  Float_t sumWgts;
  Float_t sampleWeight;
  std::vector<Float_t> puWeights;

  // branch names
  std::string s_blindSF = "blindSF";
  std::string s_applyBlindSF = "applyBlindSF";
  std::string s_blindMET = "blindMET";
  std::string s_applyBlindMET = "applyBlindMET";
  std::string s_jetpTmin = "jetpTmin";
  std::string s_jetEtamax = "jetEtamax";
  std::string s_jetIDmin = "jetIDmin";
  std::string s_rhEmin = "rhEmin";
  std::string s_phpTmin = "phpTmin";
  std::string s_phIDmin = "phIDmin";
  std::string s_seedTimemin = "seedTimemin";
  std::string s_splitPho = "splitPho";
  std::string s_onlyGED = "onlyGED";
  std::string s_onlyOOT = "onlyOOT";
  std::string s_storeRecHits = "storeRecHits";
  std::string s_applyTrigger = "applyTrigger";
  std::string s_minHT = "minHT";
  std::string s_applyHT = "applyHT";
  std::string s_phgoodpTmin = "phgoodpTmin";
  std::string s_phgoodIDmin = "phgoodIDmin";
  std::string s_applyPhGood = "applyPhGood";
  std::string s_dRmin = "dRmin";
  std::string s_pTres = "pTres";
  std::string s_genpTres = "genpTres";
  std::string s_trackdRmin = "trackdRmin";
  std::string s_trackpTmin = "trackpTmin";
  std::string s_inputPaths = "inputPaths";
  std::string s_inputFilters = "inputFilters";
  std::string s_inputFlags = "inputFlags";
  std::string s_isGMSB = "isGMSB";
  std::string s_isHVDS = "isHVDS";
  std::string s_isBkgd = "isBkgd";
  std::string s_isToy  = "isToy";
  std::string s_isADD  = "isADD";
  std::string s_xsec = "xsec";
  std::string s_filterEff = "filterEff";
  std::string s_BR = "BR";
  std::string s_sumWgts = "sumWgts"; 
  std::string s_sampleWeight = "sampleWeight";
  std::string s_puWeights = "puWeights";
};

struct Event
{
  // branches
  UInt_t    run;
  UInt_t    lumi;
  ULong64_t event;
  Bool_t    hltSignal;
  Bool_t    hltRefPhoID;
  Bool_t    hltRefDispID;
  Bool_t    hltRefHT;
  Bool_t    hltPho50;
  Bool_t    hltPho200;
  Bool_t    hltDiPho70;
  Bool_t    hltDiPho3022M90;
  Bool_t    hltDiPho30PV18PV;
  Bool_t    hltDiEle33MW;
  Bool_t    hltDiEle27WPT;
  Bool_t    hltJet500;
  Bool_t    metPV;
  Bool_t    metBeamHalo;
  Bool_t    metHBHENoise;
  Bool_t    metHBHEisoNoise;
  Bool_t    metECALTP;
  Bool_t    metPFMuon;
  Bool_t    metPFChgHad;
  Bool_t    metEESC;
  Bool_t    metECALCalib;

  Int_t     nvtx;
  Float_t   vtxX;
  Float_t   vtxY;
  Float_t   vtxZ;
  Float_t   rho;
  Float_t   t1pfMETpt;
  Float_t   t1pfMETphi;
  Float_t   t1pfMETsumEt;

  Int_t     njets;
  Int_t     nrechits;
  Int_t     nphotons;
  Float_t   evtwgt;

  // MC Types
  Float_t   genwgt;
  Float_t   genx0;
  Float_t   geny0;
  Float_t   genz0;
  Float_t   gent0;
  Int_t     genpuobs;
  Int_t     genputrue;
  Float_t   puwgt;
  // GMSB
  Int_t     nNeutoPhGr;
  // HVDS
  Int_t     nvPions;
  // ToyMC 
  Int_t     nToyPhs;

  // branch names
  std::string s_run = "run";
  std::string s_lumi = "lumi";
  std::string s_event = "event";
  std::string s_hltSignal = "hltSignal";
  std::string s_hltRefPhoID = "hltRefPhoID";
  std::string s_hltRefDispID = "hltRefDispID";
  std::string s_hltRefHT = "hltRefHT";
  std::string s_hltPho50 = "hltPho50";
  std::string s_hltPho200 = "hltPho200";
  std::string s_hltDiPho70 = "hltDiPho70";
  std::string s_hltDiPho3022M90 = "hltDiPho3022M90";
  std::string s_hltDiPho30PV18PV = "hltDiPho30PV18PV";
  std::string s_hltDiEle33MW = "hltDiEle33MW";
  std::string s_hltDiEle27WPT = "hltDiEle27WPT";  
  std::string s_hltJet500 = "hltJet500";
  std::string s_metPV = "metPV";
  std::string s_metBeamHalo = "metBeamHalo";
  std::string s_metHBHENoise = "metHBHENoise";
  std::string s_metHBHEisoNoise = "metHBHEisoNoise";
  std::string s_metECALTP = "metECALTP";
  std::string s_metPFMuon = "metPFMuon";
  std::string s_metPFChgHad = "metPFChgHad";
  std::string s_metEESC = "metEESC";
  std::string s_metECALCalib = "metECALCalib";
  std::string s_nvtx = "nvtx";
  std::string s_vtxX = "vtxX";
  std::string s_vtxY = "vtxY";
  std::string s_vtxZ = "vtxZ";
  std::string s_rho = "rho";
  std::string s_t1pfMETpt = "t1pfMETpt";
  std::string s_t1pfMETphi = "t1pfMETphi";
  std::string s_t1pfMETsumEt = "t1pfMETsumEt";
  std::string s_njets = "njets";
  std::string s_nrechits = "nrechits";
  std::string s_nphotons = "nphotons";
  std::string s_evtwgt = "evtwgt";
  std::string s_genwgt = "genwgt";  
  std::string s_genx0 = "genx0";
  std::string s_geny0 = "geny0";
  std::string s_genz0 = "genz0";
  std::string s_gent0 = "gent0";
  std::string s_genpuobs = "genpuobs";
  std::string s_genputrue = "genputrue";
  std::string s_puwgt = "puwgt";
  std::string s_nNeutoPhGr = "nNeutoPhGr";
  std::string s_nvPions = "nvPions";
  std::string s_nToyPhs = "nToyPhs";

  // tbranches
  TBranch * b_run;
  TBranch * b_lumi;
  TBranch * b_event;
  TBranch * b_hltSignal;
  TBranch * b_hltRefPhoID;
  TBranch * b_hltRefDispID;
  TBranch * b_hltRefHT;
  TBranch * b_hltPho50;
  TBranch * b_hltPho200;
  TBranch * b_hltDiPho70;
  TBranch * b_hltDiPho3022M90;
  TBranch * b_hltDiPho30PV18PV;
  TBranch * b_hltDiEle33MW;
  TBranch * b_hltDiEle27WPT;  
  TBranch * b_hltJet500;
  TBranch * b_metPV;
  TBranch * b_metBeamHalo;
  TBranch * b_metHBHENoise;
  TBranch * b_metHBHEisoNoise;
  TBranch * b_metECALTP;
  TBranch * b_metPFMuon;
  TBranch * b_metPFChgHad;
  TBranch * b_metEESC;
  TBranch * b_metECALCalib;
  TBranch * b_nvtx;
  TBranch * b_vtxX;
  TBranch * b_vtxY;
  TBranch * b_vtxZ;
  TBranch * b_rho;
  TBranch * b_t1pfMETpt;
  TBranch * b_t1pfMETphi;
  TBranch * b_t1pfMETsumEt;
  TBranch * b_njets;
  TBranch * b_nrechits;
  TBranch * b_nphotons;
  TBranch * b_evtwgt;
  TBranch * b_genwgt;  
  TBranch * b_genx0;
  TBranch * b_geny0;
  TBranch * b_genz0;
  TBranch * b_gent0;
  TBranch * b_genpuobs;
  TBranch * b_genputrue;
  TBranch * b_puwgt;
  TBranch * b_nNeutoPhGr;
  TBranch * b_nvPions;
  TBranch * b_nToyPhs;
};

struct Jet
{
  std::vector<Float_t> * E;
  std::vector<Float_t> * pt;
  std::vector<Float_t> * phi;
  std::vector<Float_t> * eta;
  std::vector<Int_t>   * ID;

  std::vector<Float_t> * NHF;
  std::vector<Float_t> * NEMF;
  std::vector<Float_t> * CHF;
  std::vector<Float_t> * CEMF;
  std::vector<Float_t> * MUF;
  std::vector<Float_t> * NHM;
  std::vector<Float_t> * CHM;

  std::vector<Float_t> E_f;
  std::vector<Float_t> pt_f;
  std::vector<Float_t> phi_f;
  std::vector<Float_t> eta_f;
  std::vector<Int_t>   ID_i;
  
  std::vector<Float_t> NHF_f;
  std::vector<Float_t> NEMF_f;
  std::vector<Float_t> CHF_f;
  std::vector<Float_t> CEMF_f;
  std::vector<Float_t> MUF_f;
  std::vector<Float_t> NHM_f;
  std::vector<Float_t> CHM_f;

  std::string s_E = "jetE";
  std::string s_pt = "jetpt";
  std::string s_phi = "jetphi";
  std::string s_eta = "jeteta";
  std::string s_ID = "jetID";

  std::string s_NHF = "jetNHF";
  std::string s_NEMF = "jetNEMF";
  std::string s_CHF = "jetCHF";
  std::string s_CEMF = "jetCEMF";
  std::string s_MUF = "jetMUF";
  std::string s_NHM = "jetNHM";
  std::string s_CHM = "jetCHM";

  TBranch * b_E;
  TBranch * b_pt;
  TBranch * b_phi;
  TBranch * b_eta;
  TBranch * b_ID;

  TBranch * b_NHF;
  TBranch * b_NEMF;
  TBranch * b_CHF;
  TBranch * b_CEMF;
  TBranch * b_MUF;
  TBranch * b_NHM;
  TBranch * b_CHM;
};

struct RecHits
{
  std::vector<Float_t> * eta;
  std::vector<Float_t> * phi;
  std::vector<Float_t> * E;
  std::vector<Float_t> * time;
  std::vector<Int_t>   * OOT;
  std::vector<UInt_t>  * ID;

  std::string s_eta = "rheta";
  std::string s_phi = "rhphi";
  std::string s_E = "rhE";
  std::string s_time = "rhtime";
  std::string s_OOT = "rhOOT";
  std::string s_ID = "rhID";

  TBranch * b_eta;
  TBranch * b_phi;
  TBranch * b_E;
  TBranch * b_time;
  TBranch * b_OOT;
  TBranch * b_ID;
};

struct Pho
{
  Float_t E;
  Float_t pt;
  Float_t eta;
  Float_t phi;
  Float_t scE;
  Float_t sceta;
  Float_t scphi;
  Float_t HoE;
  Float_t r9;
  Float_t ChgHadIso;
  Float_t NeuHadIso;
  Float_t PhoIso;
  Float_t EcalPFClIso;
  Float_t HcalPFClIso;
  Float_t TrkIso;
  Float_t ChgHadIsoC;
  Float_t NeuHadIsoC;
  Float_t PhoIsoC;
  Float_t EcalPFClIsoC;
  Float_t HcalPFClIsoC;
  Float_t TrkIsoC;
  Float_t sieie;
  Float_t sipip;
  Float_t sieip;
  Float_t e2x2;
  Float_t e3x3;
  Float_t e5x5;
  Float_t smaj;
  Float_t smin;
  Float_t alpha;
  Int_t   seed;
  std::vector<Int_t> * recHits;
  Float_t suisseX;
  Bool_t  isOOT;
  Bool_t  isEB;
  Bool_t  isHLT;
  Bool_t  isTrk;
  Bool_t  passEleVeto;
  Bool_t  hasPixSeed;
  Int_t   gedID;
  Int_t   ootID;
  // !storeRecHits
  Float_t seedtime;
  Float_t seedE;
  UInt_t  seedID;
  // MC types
  Bool_t  isGen;
  Int_t   isSignal;

  // branch names
  std::string s_E = "phoE";
  std::string s_pt = "phopt";
  std::string s_eta = "phoeta";
  std::string s_phi = "phophi";
  std::string s_scE = "phoscE";
  std::string s_sceta = "phosceta";
  std::string s_scphi = "phoscphi";
  std::string s_HoE = "phoHoE";
  std::string s_r9 = "phor9";
  std::string s_ChgHadIso = "phoChgHadIso";
  std::string s_NeuHadIso = "phoNeuHadIso";
  std::string s_PhoIso = "phoPhoIso";
  std::string s_EcalPFClIso = "phoEcalPFClIso";
  std::string s_HcalPFClIso = "phoHcalPFClIso";
  std::string s_TrkIso = "phoTrkIso";
  std::string s_ChgHadIsoC = "phoChgHadIsoC";
  std::string s_NeuHadIsoC = "phoNeuHadIsoC";
  std::string s_PhoIsoC = "phoPhoIsoC";
  std::string s_EcalPFClIsoC = "phoEcalPFClIsoC";
  std::string s_HcalPFClIsoC = "phoHcalPFClIsoC";
  std::string s_TrkIsoC = "phoTrkIsoC";
  std::string s_sieie = "phosieie";
  std::string s_sipip = "phosipip";
  std::string s_sieip = "phosieip";
  std::string s_e2x2 = "phoe2x2";
  std::string s_e3x3 = "phoe3x3";  
  std::string s_e5x5 = "phoe5x5";
  std::string s_smaj = "phosmaj";
  std::string s_smin = "phosmin";
  std::string s_alpha = "phoalpha";
  std::string s_seed = "phoseed";
  std::string s_recHits = "phorecHits";
  std::string s_suisseX = "phosuisseX";
  std::string s_isOOT = "phoisOOT";
  std::string s_isEB = "phoisEB";
  std::string s_isHLT = "phoisHLT";
  std::string s_isTrk = "phoisTrk";
  std::string s_passEleVeto = "phopassEleVeto";
  std::string s_hasPixSeed = "phohasPixSeed";
  std::string s_gedID = "phogedID";
  std::string s_ootID = "phoootID";
  std::string s_seedtime = "phoseedtime";
  std::string s_seedE = "phoseedE";
  std::string s_seedID = "phoseedID";
  std::string s_isGen = "phoisGen";
  std::string s_isSignal = "phoisSignal";

  // tbranches
  TBranch * b_E;
  TBranch * b_pt;
  TBranch * b_eta;
  TBranch * b_phi;
  TBranch * b_scE;
  TBranch * b_sceta;
  TBranch * b_scphi;
  TBranch * b_HoE;
  TBranch * b_r9;
  TBranch * b_ChgHadIso;
  TBranch * b_NeuHadIso;
  TBranch * b_PhoIso;
  TBranch * b_EcalPFClIso;
  TBranch * b_HcalPFClIso;
  TBranch * b_TrkIso;
  TBranch * b_ChgHadIsoC;
  TBranch * b_NeuHadIsoC;
  TBranch * b_PhoIsoC;
  TBranch * b_EcalPFClIsoC;
  TBranch * b_HcalPFClIsoC;
  TBranch * b_TrkIsoC;
  TBranch * b_sieie;
  TBranch * b_sipip;
  TBranch * b_sieip;
  TBranch * b_e2x2;
  TBranch * b_e3x3;
  TBranch * b_e5x5;
  TBranch * b_smaj;
  TBranch * b_smin;
  TBranch * b_alpha;
  TBranch * b_seed;
  TBranch * b_recHits;
  TBranch * b_suisseX;
  TBranch * b_isOOT;
  TBranch * b_isEB;
  TBranch * b_isHLT;
  TBranch * b_isTrk;
  TBranch * b_passEleVeto;
  TBranch * b_hasPixSeed;
  TBranch * b_gedID;
  TBranch * b_ootID;
  TBranch * b_seedtime;
  TBranch * b_seedE;
  TBranch * b_seedID;
  TBranch * b_isGen;
  TBranch * b_isSignal;
};
typedef std::vector<Pho> PhoVec;

struct Gmsb
{
  Float_t genNmass;
  Float_t genNE;
  Float_t genNpt;
  Float_t genNphi;
  Float_t genNeta;
  Float_t genNprodvx;
  Float_t genNprodvy;
  Float_t genNprodvz;
  Float_t genNdecayvx;
  Float_t genNdecayvy;
  Float_t genNdecayvz;
  Float_t genphE;
  Float_t genphpt;
  Float_t genphphi;
  Float_t genpheta;
  Int_t   genphmatch;
  Float_t gengrmass;
  Float_t gengrE;
  Float_t gengrpt;
  Float_t gengrphi;
  Float_t gengreta;

  std::string s_genNmass = "genNmass";
  std::string s_genNE = "genNE";
  std::string s_genNpt = "genNpt";
  std::string s_genNphi = "genNphi";
  std::string s_genNeta = "genNeta";
  std::string s_genNprodvx = "genNprodvx";
  std::string s_genNprodvy = "genNprodvy";
  std::string s_genNprodvz = "genNprodvz";
  std::string s_genNdecayvx = "genNdecayvx";
  std::string s_genNdecayvy = "genNdecayvy";
  std::string s_genNdecayvz = "genNdecayvz";
  std::string s_genphE = "genphE";
  std::string s_genphpt = "genphpt";
  std::string s_genphphi = "genphphi";
  std::string s_genpheta = "genpheta";
  std::string s_genphmatch = "genphmatch";
  std::string s_gengrmass = "gengrmass";
  std::string s_gengrE = "gengrE";
  std::string s_gengrpt = "gengrpt";
  std::string s_gengrphi = "gengrphi";
  std::string s_gengreta = "gengreta";

  TBranch * b_genNmass;
  TBranch * b_genNE;
  TBranch * b_genNpt;
  TBranch * b_genNphi;
  TBranch * b_genNeta;
  TBranch * b_genNprodvx;
  TBranch * b_genNprodvy;
  TBranch * b_genNprodvz;
  TBranch * b_genNdecayvx;
  TBranch * b_genNdecayvy;
  TBranch * b_genNdecayvz;
  TBranch * b_genphE;
  TBranch * b_genphpt;
  TBranch * b_genphphi;
  TBranch * b_genpheta;
  TBranch * b_genphmatch;
  TBranch * b_gengrmass;
  TBranch * b_gengrE;
  TBranch * b_gengrpt;
  TBranch * b_gengrphi;
  TBranch * b_gengreta;
};
typedef std::vector<Gmsb> GmsbVec;

struct Hvds
{
  Float_t genvPionmass;
  Float_t genvPionE;
  Float_t genvPionpt;
  Float_t genvPionphi;
  Float_t genvPioneta;
  Float_t genvPionprodvx;
  Float_t genvPionprodvy;
  Float_t genvPionprodvz;
  Float_t genvPiondecayvx;
  Float_t genvPiondecayvy;
  Float_t genvPiondecayvz;
  Float_t genHVph0E;
  Float_t genHVph0pt;
  Float_t genHVph0phi;
  Float_t genHVph0eta;
  Int_t   genHVph0match;
  Float_t genHVph1E;
  Float_t genHVph1pt;
  Float_t genHVph1phi;
  Float_t genHVph1eta;
  Int_t   genHVph1match;

  std::string s_genvPionmass = "genvPionmass";
  std::string s_genvPionE = "genvPionE";
  std::string s_genvPionpt = "genvPionpt";
  std::string s_genvPionphi = "genvPionphi";
  std::string s_genvPioneta = "genvPioneta";
  std::string s_genvPionprodvx = "genvPionprodvx";
  std::string s_genvPionprodvy = "genvPionprodvy";
  std::string s_genvPionprodvz = "genvPionprodvz";
  std::string s_genvPiondecayvx = "genvPiondecayvx";
  std::string s_genvPiondecayvy = "genvPiondecayvy";
  std::string s_genvPiondecayvz = "genvPiondecayvz";
  std::string s_genHVph0E = "genHVph0E";
  std::string s_genHVph0pt = "genHVph0pt";
  std::string s_genHVph0phi = "genHVph0phi";
  std::string s_genHVph0eta = "genHVph0eta";
  std::string s_genHVph0match = "genHVph0match";
  std::string s_genHVph1E = "genHVph1E";
  std::string s_genHVph1pt = "genHVph1pt";
  std::string s_genHVph1phi = "genHVph1phi";
  std::string s_genHVph1eta = "genHVph1eta";
  std::string s_genHVph1match = "genHVph1match";

  TBranch * b_genvPionmass;
  TBranch * b_genvPionE;
  TBranch * b_genvPionpt;
  TBranch * b_genvPionphi;
  TBranch * b_genvPioneta;
  TBranch * b_genvPionprodvx;
  TBranch * b_genvPionprodvy;
  TBranch * b_genvPionprodvz;
  TBranch * b_genvPiondecayvx;
  TBranch * b_genvPiondecayvy;
  TBranch * b_genvPiondecayvz;
  TBranch * b_genHVph0E;
  TBranch * b_genHVph0pt;
  TBranch * b_genHVph0phi;
  TBranch * b_genHVph0eta;
  TBranch * b_genHVph0match;
  TBranch * b_genHVph1E;
  TBranch * b_genHVph1pt;
  TBranch * b_genHVph1phi;
  TBranch * b_genHVph1eta;
  TBranch * b_genHVph1match;
};
typedef std::vector<Hvds> HvdsVec;

struct Toy
{
  Float_t genphE;
  Float_t genphpt;
  Float_t genphphi;
  Float_t genpheta;
  Int_t   genphmatch;
  Int_t   genphmatch_ptres;
  Int_t   genphmatch_status;

  std::string s_genphE = "genphE";
  std::string s_genphpt = "genphpt";
  std::string s_genphphi = "genphphi";
  std::string s_genpheta = "genpheta";
  std::string s_genphmatch = "genphmatch";
  std::string s_genphmatch_ptres = "genphmatch_ptres";
  std::string s_genphmatch_status = "genphmatch_status";

  TBranch * b_genphE;
  TBranch * b_genphpt;
  TBranch * b_genphphi;
  TBranch * b_genpheta;
  TBranch * b_genphmatch;
  TBranch * b_genphmatch_ptres;
  TBranch * b_genphmatch_status;
};
typedef std::vector<Toy> ToyVec;

#endif
