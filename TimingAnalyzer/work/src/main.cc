// hname must match name string in histo map! too lazy to pass extra parameter
#include "../interface/Common.hh"
#include "../interface/Analysis.hh"

#include "TROOT.h"
#include "TString.h"

#include <map>

typedef std::map<TString,bool> TStrBoolMap;
typedef  TStrBoolMap::iterator TStrBoolMapIter;

int main(){
  TString outdir  = "timeres0";
  TString outtype = "png";
  const Float_t lumi = 2.301; // brilcalc lumi --normtag /afs/cern.ch/user/l/lumipro/public/normtag_file/moriond16_normtag.json -i rereco2015D.txt -u /fb
  TString extratext = "Preliminary";
  
  // to do:
  // mc vs data stacking
  // mc vs data output in histos
    
  TStrBoolMap SampleMap;
  SampleMap["demo"] = false;
  
  for (TStrBoolMapIter mapiter = SampleMap.begin(); mapiter != SampleMap.end(); ++mapiter) {
    Analysis analysis((*mapiter).first,(*mapiter).second,outdir,outtype,lumi,extratext);
    analysis.TimeResPlots();
  }
  

}
