#ifndef _skimmertypes_ 
#define _skimmertypes_ 

#include "TH1F.h"
#include "TBranch.h"

#include <vector>

struct Configuration
{
  UInt_t  blindSF;
  Bool_t  applyBlindSF;
  Float_t blindMET;
  Bool_t  applyBlindMET;
  Float_t jetpTmin;
  Int_t   jetIDmin;
  Float_t rhEmin;
  Float_t phpTmin;
  std::string * phIDmin;
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
  Bool_t  applyPhGood;
  Float_t dRmin;
  Float_t pTres;
  Float_t trackdRmin;
  Float_t trackpTmin;
  std::string * inputPaths;
  std::string * inputFilters;
  // MC types
  Bool_t  isGMSB;
  Bool_t  isHVDS;
  Bool_t  isBkgd;
  Float_t xsec;
  Float_t filterEff;
  Float_t BR;

  TBranch * b_blindSF;
  TBranch * b_applyBlindSF;
  TBranch * b_blindMET;
  TBranch * b_applyBlindMET;
  TBranch * b_jetpTmin;
  TBranch * b_jetIDmin;
  TBranch * b_rhEmin;
  TBranch * b_phpTmin;
  TBranch * b_phIDmin;
  TBranch * b_seedTimemin;
  TBranch * b_splitPho;
  TBranch * b_storeRecHits;
  TBranch * b_onlyGED;
  TBranch * b_onlyOOT;
  TBranch * b_applyTrigger;
  TBranch * b_minHT;
  TBranch * b_applyHT;
  TBranch * b_phgoodpTmin;
  TBranch * b_phgoodIDmin;
  TBranch * b_applyPhGood;
  TBranch * b_dRmin;
  TBranch * b_pTres;
  TBranch * b_trackdRmin;
  TBranch * b_trackpTmin;
  TBranch * b_inputPaths;
  TBranch * b_inputFilters;
  // MC only
  TBranch * b_isGMSB;
  TBranch * b_isHVDS;
  TBranch * b_isBkgd;
  TBranch * b_xsec;
  TBranch * b_filterEff;
  TBranch * b_BR;
};

struct Event
{
  UInt_t    run;
  UInt_t    lumi;
  ULong64_t event;
  Bool_t    hltSignal;
  Bool_t    hltRefPhoID;
  Bool_t    hltRefDispID;
  Bool_t    hltRefHT;
  Bool_t    hltPho50;
  Int_t     nvtx;
  Float_t   vtxX;
  Float_t   vtxY;
  Float_t   vtxZ;
  Float_t   rho;
  Float_t   t1pfMETpt;
  Float_t   t1pfMETphi;
  Float_t   t1pfMETsumEt;
  Float_t   jetHT;
  Int_t     njets;
  Int_t     nrechits;
  Int_t     nphotons;

  // MC Types
  Float_t   genwgt;
  Int_t     genpuobs;
  Int_t     genputrue;
  // GMSB
  Int_t     nNeutoPhGr;
  // HVDS
  Int_t     nvPions;

  TBranch * b_run;
  TBranch * b_lumi;
  TBranch * b_event;
  TBranch * b_hltSignal;
  TBranch * b_hltRefPhoID;
  TBranch * b_hltRefDispID;
  TBranch * b_hltRefHT;
  TBranch * b_hltPho50;
  TBranch * b_nvtx;
  TBranch * b_vtxX;
  TBranch * b_vtxY;
  TBranch * b_vtxZ;
  TBranch * b_rho;
  TBranch * b_t1pfMETpt;
  TBranch * b_t1pfMETphi;
  TBranch * b_t1pfMETsumEt;
  TBranch * b_jetHT;
  TBranch * b_njets;
  TBranch * b_nrechits;
  TBranch * b_nphotons;

  // MC types
  TBranch * b_genwgt;
  TBranch * b_genpuobs;
  TBranch * b_genputrue;
  // GMSB
  TBranch * b_nNeutoPhGr;
  // HVDS
  TBranch * b_nvPions;
};

struct Jet
{
  Float_t E;
  Float_t pt;
  Float_t eta;
  Float_t phi;
  
  TBranch * b_E;
  TBranch * b_pt;
  TBranch * b_phi;
  TBranch * b_eta;
};
typedef std::vector<Jet> JetVec;

struct RecHits
{
  std::vector<Float_t> * eta;
  std::vector<Float_t> * phi;
  std::vector<Float_t> * E;
  std::vector<Float_t> * time;
  std::vector<Int_t>   * OOT;
  std::vector<UInt_t>  * ID;

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
  Float_t sieie;
  Float_t sipip;
  Float_t sieip;
  Float_t smaj;
  Float_t smin;
  Float_t alpha;
  Int_t   seed;
  std::vector<Int_t> * recHits;
  Bool_t  isOOT;
  Bool_t  isEB;
  Bool_t  isHLT;
  Bool_t  isTrk;
  Int_t   ID;
  // !storeRecHits
  Float_t seedtime;
  Float_t seedE;
  UInt_t  seedID;
  // MC types
  Int_t   isSignal;
  Bool_t  isGen;

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
  TBranch * b_sieie;
  TBranch * b_sipip;
  TBranch * b_sieip;
  TBranch * b_smaj;
  TBranch * b_smin;
  TBranch * b_alpha;
  TBranch * b_seed;
  TBranch * b_recHits;
  TBranch * b_isOOT;
  TBranch * b_isEB;
  TBranch * b_isHLT;
  TBranch * b_isTrk;
  TBranch * b_ID;
  // !storeRecHits
  TBranch * b_seedtime;
  TBranch * b_seedE;
  TBranch * b_seedID;
  // MC Types
  TBranch * b_isSignal;
  TBranch * b_isGen;
};
typedef std::vector<Pho> PhoVec;

struct GMSB
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
typedef std::vector<GMSB> GMSBVec;

struct HVDS
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
typedef std::vector<HVDS> HVDSVec;

#endif
