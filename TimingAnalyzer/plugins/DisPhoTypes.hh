#ifndef __DisPhoTypes__
#define __DisPhoTypes__

#include <vector>

struct gmsbStruct
{
  float genNmass_;
  float genNE_;
  float genNpt_;
  float genNphi_;
  float genNeta_;

  float genNprodvx_;
  float genNprodvy_;
  float genNprodvz_;

  float genNdecayvx_;
  float genNdecayvy_;
  float genNdecayvz_;
  
  float genphE_;
  float genphpt_;
  float genphphi_;
  float genpheta_;
  int   genphmatch_;

  float gengrmass_;
  float gengrE_;
  float gengrpt_;
  float gengrphi_;
  float gengreta_;
};

struct hvdsStruct
{
  float genvPionmass_;
  float genvPionE_;
  float genvPionpt_;
  float genvPionphi_;
  float genvPioneta_;

  float genvPionprodvx_;
  float genvPionprodvy_;
  float genvPionprodvz_;

  float genvPiondecayvx_;
  float genvPiondecayvy_;
  float genvPiondecayvz_;

  float genHVph0E_;
  float genHVph0pt_;
  float genHVph0phi_;
  float genHVph0eta_;
  int genHVph0match_;
 
  float genHVph1E_;
  float genHVph1pt_;
  float genHVph1phi_;
  float genHVph1eta_;
  int genHVph1match_;
};

struct toyStruct
{
  float genphE_;
  float genphpt_;
  float genphphi_;
  float genpheta_;

  int genphmatch_;
  int genphmatch_ptres_;
  int genphmatch_status_;
};

struct jetStruct
{
  float E_;
  float Pt_;
  float Phi_;
  float Eta_;
  int   ID_;
};

struct phoStruct
{
  float E_;
  float Pt_;
  float Phi_;
  float Eta_;

  float scE_;
  float scPhi_;
  float scEta_;

  float HoE_;
  float r9_;

  float ChgHadIso_;
  float NeuHadIso_;
  float PhoIso_;

  float EcalPFClIso_;
  float HcalPFClIso_;
  float TrkIso_;

  float ChgHadIsoC_;
  float NeuHadIsoC_;
  float PhoIsoC_;

  float EcalPFClIsoC_;
  float HcalPFClIsoC_;
  float TrkIsoC_;

  float Sieie_; 
  float Sipip_;
  float Sieip_;

  float e2x2_;
  float e3x3_;
  float e5x5_;

  float Smaj_;
  float Smin_;
  float alpha_;

  int seed_;
  std::vector<int> recHits_;
  float seedtime_;
  float seedE_;
  unsigned int seedID_;

  float suisseX_;
  
  bool isOOT_;
  bool isEB_;
  bool isHLT_;

  bool isTrk_;
  bool passEleVeto_;
  bool hasPixSeed_;

  int  gedID_;
  int  ootID_;

  // MC only
  int  isSignal_;
  bool isGen_;

  float Scale_;
  float Smear_;
};

#endif
