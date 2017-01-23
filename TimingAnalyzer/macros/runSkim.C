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

void runSkim() 
{
  setupcpp11(); 

  gROOT->LoadMacro("SkimRECO.cc++g");

  // config is:
  // filename, outdir, dump cuts/histnames
  // jetptcut, njetscut, 
  // phptcut
  // id cuts: phHoEcut, phsieiecut, phchgisocut, phneuisocut, phisocut --> 3 =tight, 2 =medium, 1 =loose, 0 =nothing
  // phsmajcut, phsmincut, 
  // nphscut
  // applyEBonly, applyEEonly

  TString outdir = "output/skim-loose-phpt20";  

  Float_t jetpt = 20.f;
  Int_t   njets = 2;
  
  Float_t phpt     = 20.f;
  Int_t   phhoe    = 1;
  Int_t   phsieie  = 1;
  Int_t   phchgiso = 1;
  Int_t   phneuiso = 1;
  Int_t   phiso    = 1;
  Float_t smajEB   = 1.f;
  Float_t smajEE   = 1.f;
  Float_t sminEB   = 1.f;
  Float_t sminEE   = 1.f;
  Int_t   nphs     = 1;

  SkimRECO dataskimmer    ("input/DATA/doubleeg/skim/recoskim.root"         ,outdir,"data"    ,true ,jetpt,njets,phpt,phhoe,phsieie,phchgiso,phneuiso,phiso,smajEB,smajEE,sminEB,sminEE,nphs);
  SkimRECO ctau100skimmer ("input/MC/signal/skim/recoskim-pat-ctau100.root" ,outdir,"ctau100" ,false,jetpt,njets,phpt,phhoe,phsieie,phchgiso,phneuiso,phiso,smajEB,smajEE,sminEB,sminEE,nphs);
  SkimRECO ctau2000skimmer("input/MC/signal/skim/recoskim-pat-ctau2000.root",outdir,"ctau2000",false,jetpt,njets,phpt,phhoe,phsieie,phchgiso,phneuiso,phiso,smajEB,smajEE,sminEB,sminEE,nphs);
  SkimRECO ctau6000skimmer("input/MC/signal/skim/recoskim-pat-ctau6000.root",outdir,"ctau6000",false,jetpt,njets,phpt,phhoe,phsieie,phchgiso,phneuiso,phiso,smajEB,smajEE,sminEB,sminEE,nphs);

  // store efficiency in small text file
  std::ofstream effdump;
  effdump.open(Form("%s/efficiency.txt",outdir.Data()),std::ios_base::trunc);

  dataskimmer    .DoSkim(effdump);
  ctau100skimmer .DoSkim(effdump);
  ctau2000skimmer.DoSkim(effdump);
  ctau6000skimmer.DoSkim(effdump);

  effdump.close();
}
