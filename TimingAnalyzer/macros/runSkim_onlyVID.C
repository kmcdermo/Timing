#include "TString.h"
#include "TSystem.h"
#include "TROOT.h"

#include <fstream>

void setupcpp11() // customize ACLiC's behavior ...
{
  TString o;
  // customize MakeSharedLib
  o = TString(gSystem->GetMakeSharedLib());
  o = o.ReplaceAll(" -c ", " -std=c++0x -c ");
  gSystem->SetMakeSharedLib(o.Data());
  // customize MakeExe
  o = TString(gSystem->GetMakeExe());
  o = o.ReplaceAll(" -c ", " -std=c++0x -c ");
  gSystem->SetMakeExe(o.Data());
} 

void runSkim_onlyVID(Int_t iphpt = 50, TString ionlyvid = "none", Int_t ijetpt = 35, Int_t injets = 0, Bool_t batch = false) 
{
  setupcpp11(); 

  gROOT->LoadMacro("SkimRECO.cc+");

  // config is:
  // filename, outdir, dump cuts/histnames
  // jetptcut, njetscut, 
  // phptcut
  // id cuts: phHoEcut, phsieiecut, phchgisocut, phneuisocut, phisocut --> 3 =tight, 2 =medium, 1 =loose, 0 =nothing
  // phsmajcut, phsmincut, 
  // nphscut
  // applyEBonly, applyEEonly

  TString outdir = Form("output/recoskim/onlyvid-sph1/phpt%i_%s_jetpt%i_njets%i",iphpt,ionlyvid.Data(),ijetpt,injets);

  Float_t jetpt = ijetpt;
  Int_t   njets = injets;
  
  Float_t phpt  = iphpt;
  Int_t   phvid = 1; // loose id for now
  Int_t   phhoe = 0, phsieie = 0, phchgiso = 0, phneuiso = 0, phiso = 0;
  Float_t smajEB = 10000.f;// 0.6;
  Float_t smajEE = 10000.f;// 0.5;
  Float_t sminEB = 10000.f;// 0.3;
  Float_t sminEE = 10000.f;// 0.3;
  Int_t   nphs   = 1;

  if      (ionlyvid.EqualTo("PhIso"   ,TString::kExact)) {phiso    = 1;}
  else if (ionlyvid.EqualTo("NeuIso"  ,TString::kExact)) {phneuiso = 1;}
  else if (ionlyvid.EqualTo("ChgIso"  ,TString::kExact)) {phchgiso = 1;}
  else if (ionlyvid.EqualTo("Sieie"   ,TString::kExact)) {phsieie  = 1;}
  else if (ionlyvid.EqualTo("HoE"     ,TString::kExact)) {phhoe    = 1;}
  else if (ionlyvid.EqualTo("Sminor"  ,TString::kExact)) {sminEB   = 0.4f; sminEE = 0.4f;}
  else if (ionlyvid.EqualTo("ClShape" ,TString::kExact)) {phsieie  = 1; phhoe = 1;}
  else if (ionlyvid.EqualTo("ClShapeP",TString::kExact)) {phsieie  = 1; phhoe = 1; sminEB = 0.4f; sminEE = 0.4f;}

  //  SkimRECO deg2016Bskimmer("input/DATA/doubleeg/2016B/recoskim-doubleeg-2016B.root",outdir,"deg2016B",true,batch,jetpt,njets,phpt,phhoe,phsieie,phchgiso,phneuiso,phiso,smajEB,smajEE,sminEB,sminEE,nphs);
  SkimRECO sph2016Bskimmer("input/DATA/singleph/2016B/recoskim-singleph-2016B.root",outdir,"sph2016B",true,batch,jetpt,njets,phpt,phhoe,phsieie,phchgiso,phneuiso,phiso,smajEB,smajEE,sminEB,sminEE,nphs);
  SkimRECO ctau100skimmer ("input/MC/signal/GMSB/recoskim-pat-ctau100.root" ,outdir,"ctau100" ,false,batch,jetpt,njets,phpt,phhoe,phsieie,phchgiso,phneuiso,phiso,smajEB,smajEE,sminEB,sminEE,nphs);
  //  SkimRECO ctau2000skimmer("input/MC/signal/GMSB/recoskim-pat-ctau2000.root",outdir,"ctau2000",false,batch,jetpt,njets,phpt,phhoe,phsieie,phchgiso,phneuiso,phiso,smajEB,smajEE,sminEB,sminEE,nphs);
  SkimRECO ctau6000skimmer("input/MC/signal/GMSB/recoskim-pat-ctau6000.root",outdir,"ctau6000",false,batch,jetpt,njets,phpt,phhoe,phsieie,phchgiso,phneuiso,phiso,smajEB,smajEE,sminEB,sminEE,nphs);

  // store efficiency in small text file
  std::ofstream effdump;
  effdump.open(Form("%s/efficiency.txt",outdir.Data()),std::ios_base::trunc);

  // Bools to do which plots
  Bool_t doNm1  = false;
  Bool_t doTeff = false;
  Bool_t doAn   = false;

  //deg2016Bskimmer.DoSkim(effdump,doNm1,doTeff,doAn);
  sph2016Bskimmer.DoSkim(effdump,doNm1,doTeff,doAn);
  ctau100skimmer .DoSkim(effdump,doNm1,doTeff,doAn);
  //  ctau2000skimmer.DoSkim(effdump,doNm1,doTeff,doAn);
  ctau6000skimmer.DoSkim(effdump,doNm1,doTeff,doAn);

  effdump.close();
}
