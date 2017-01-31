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

void runSkim_VID(Int_t iphpt = 50, TString inovid = "full", Int_t ijetpt = 35, Int_t injets = 0, Bool_t batch = false) 
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

  TString outdir = Form("output/recoskim/phpt%i_%s_jetpt%i_njets%i",iphpt,inovid.Data(),ijetpt,injets);

  Float_t jetpt = ijetpt;
  Int_t   njets = injets;
  
  Float_t phpt  = iphpt;
  Int_t   phvid = 1; // loose id for now
  Int_t   phhoe = phvid, phsieie = phvid, phchgiso = phvid, phneuiso = phvid, phiso = phvid;

  if      (inovid.EqualTo("noPhIso" ,TString::kExact)) {phiso = 0;}
  else if (inovid.EqualTo("noNeuIso",TString::kExact)) {phiso = 0; phneuiso = 0;}
  else if (inovid.EqualTo("noChgIso",TString::kExact)) {phiso = 0; phneuiso = 0; phchgiso = 0;}
  else if (inovid.EqualTo("noSieie" ,TString::kExact)) {phiso = 0; phneuiso = 0; phchgiso = 0; phsieie = 0;}
  else if (inovid.EqualTo("noHoE"   ,TString::kExact)) {phiso = 0; phneuiso = 0; phchgiso = 0; phsieie = 0; phhoe = 0;}

  Float_t smajEB = 10000.f;// 0.6;
  Float_t smajEE = 10000.f;// 0.5;
  Float_t sminEB = 10000.f;// 0.3;
  Float_t sminEE = 10000.f;// 0.3;
  Int_t   nphs   = 1;

  SkimRECO deg2016Bskimmer("input/DATA/doubleeg/2016B/recoskim-doubleeg-2016B.root",outdir,"deg2016B",true,batch,jetpt,njets,phpt,phhoe,phsieie,phchgiso,phneuiso,phiso,smajEB,smajEE,sminEB,sminEE,nphs);
  SkimRECO ctau100skimmer ("input/MC/signal/GMSB/recoskim-pat-ctau100.root" ,outdir,"ctau100" ,false,batch,jetpt,njets,phpt,phhoe,phsieie,phchgiso,phneuiso,phiso,smajEB,smajEE,sminEB,sminEE,nphs);
  SkimRECO ctau2000skimmer("input/MC/signal/GMSB/recoskim-pat-ctau2000.root",outdir,"ctau2000",false,batch,jetpt,njets,phpt,phhoe,phsieie,phchgiso,phneuiso,phiso,smajEB,smajEE,sminEB,sminEE,nphs);
  SkimRECO ctau6000skimmer("input/MC/signal/GMSB/recoskim-pat-ctau6000.root",outdir,"ctau6000",false,batch,jetpt,njets,phpt,phhoe,phsieie,phchgiso,phneuiso,phiso,smajEB,smajEE,sminEB,sminEE,nphs);

  // store efficiency in small text file
  std::ofstream effdump;
  effdump.open(Form("%s/efficiency.txt",outdir.Data()),std::ios_base::trunc);

  // Bools to do which plots
  Bool_t doNm1  = false;
  Bool_t doTeff = true;
  Bool_t doAn   = false;

  deg2016Bskimmer.DoSkim(effdump,doNm1,doTeff,doAn);
  ctau100skimmer .DoSkim(effdump,doNm1,doTeff,doAn);
  ctau2000skimmer.DoSkim(effdump,doNm1,doTeff,doAn);
  ctau6000skimmer.DoSkim(effdump,doNm1,doTeff,doAn);

  effdump.close();
}
