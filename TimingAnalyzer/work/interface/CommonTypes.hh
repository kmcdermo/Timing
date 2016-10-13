#ifndef _commontypes_
#define _commontypes_

#include "TString.h"
#include "TColor.h"

#include <list>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <map>

// global typedefs

typedef std::list<std::string> lStr_t;

typedef std::map<TString,TString>  TStrMap;
typedef std::map<TString,Bool_t>   TStrBoolMap;
typedef std::map<TString,Color_t>  ColorMap;
typedef std::map<TString,Int_t>    TStrIntMap;
typedef std::map<TString,Float_t>  TStrFltMap;
typedef std::map<TString,Double_t> TStrDblMap;
typedef std::unordered_map<Int_t,Int_t> IntMap;

typedef std::pair<Float_t,Float_t> FFPair;

typedef std::vector<TString>  TStrVec;
typedef std::vector<Int_t>    IntVec;
typedef std::vector<Float_t>  FltVec;
typedef std::vector<Double_t> DblVec;
typedef std::vector<FFPair>   FFPairVec;

typedef std::map<TString,DblVec> TStrDblVMap;

typedef lStr_t::iterator      lStr_i;
typedef TStrMap::iterator     TStrMapIter;
typedef TStrBoolMap::iterator TStrBoolMapIter;
typedef TStrFltMap::iterator  TStrFltMapIter;

#endif
