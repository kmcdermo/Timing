#include "TString.h"
#include "HLTPlots.cc+"

void runHLTPlots(const TString & dataset = "SP", const TString & era = "2017C", const Int_t eff = 1) 
{
  const Bool_t  eteff   = (eff==0?true:false);
  const Bool_t  dispeff = (eff==1?true:false);
  const Bool_t  hteff   = (eff==2?true:false);

  const UInt_t  start = 0;
  const UInt_t  end   = 0;
  const TString infile = Form("input/DATA/2017/HLT_CHECK/%s/hltdump_%s.root",dataset.Data(),era.Data());
  const TString outdir = Form("HLT_Golden_%s_%s",dataset.Data(),era.Data());
  const TString runs = "noruns.txt";
  const Bool_t  isoph = false;
  const Bool_t  isidL = true;
  const Bool_t  iser  = true;
  const Bool_t  applyht = false;
  const Float_t htcut = 400.f;

  HLTPlots plots(infile,start,end,outdir,runs,isoph,isidL,iser,applyht,htcut,eteff,dispeff,hteff);
  plots.DoPlots();
}
