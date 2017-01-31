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

void runSkim(Int_t iphpt = 50, Int_t iphvid = 1, Int_t ijetpt = 35, Int_t injets = 0, Bool_t batch = false) 
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

  TString outdir = Form("output/skim_phpt%i_phvid%i_jetpt%i_njets%i",iphpt,iphvid,ijetpt,injets);

  Float_t jetpt = ijetpt;
  Int_t njets = injets;
  
  Float_t phpt     = iphpt;
  Int_t   phvid    = iphvid;
  Int_t   phhoe    = phvid;
  Int_t   phsieie  = phvid;
  Int_t   phchgiso = phvid;
  Int_t   phneuiso = phvid;
  Int_t   phiso    = phvid;
  Float_t smajEB   = 1.f;// 0.6;
  Float_t smajEE   = 1.f;// 0.5;
  Float_t sminEB   = 1.f;// 0.3;
  Float_t sminEE   = 1.f;// 0.3;
  Int_t   nphs     = 1;

  SkimRECO dataskimmer    ("input/DATA/doubleeg/2016B/recoskim-doubleeg-2016B.root",outdir,"data",true ,batch,jetpt,njets,phpt,phhoe,phsieie,phchgiso,phneuiso,phiso,smajEB,smajEE,sminEB,sminEE,nphs);
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

  dataskimmer    .DoSkim(effdump,doNm1,doTeff,doAn);
  ctau100skimmer .DoSkim(effdump,doNm1,doTeff,doAn);
  ctau2000skimmer.DoSkim(effdump,doNm1,doTeff,doAn);
  ctau6000skimmer.DoSkim(effdump,doNm1,doTeff,doAn);

  effdump.close();
}
