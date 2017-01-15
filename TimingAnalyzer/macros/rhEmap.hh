#ifndef _rhEmap_
#define _rhEmap_

#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"
#include "TString.h"

#include <vector>
#include <map>

inline Float_t rad2 (const Float_t x, const Float_t y){return x*x + y*y;}
inline Float_t rad2 (const Float_t x, const Float_t y, const Float_t z){return x*x + y*y + z*z;}
inline Float_t phi  (const Float_t x, const Float_t y){return std::atan2(y,x);}
inline Float_t theta(const Float_t r, const Float_t z){return std::atan2(r,z);}
inline Float_t eta  (const Float_t x, const Float_t y, const Float_t z)
{
  return -1.0f*std::log(std::tan(theta(std::sqrt(rad2(x,y)),z)/2.f));
}

struct epid
{
public:
  epid() {}
  Int_t i1_; // iphi (EB) or ix (EE)
  Int_t i2_; // ieta (EB) or iy (EE)
};

typedef std::map<Int_t,epid> epidMap;

typedef std::map<TString,Int_t> TStrIntMap;
typedef TStrIntMap::iterator    TStrIntMapIter;

class rhEmap
{
public:
  rhEmap();
  ~rhEmap();

  void InitTree();
  void MakeIetaIphiMap();
  void CheckForGoodPhotons();
  void DumpGoodPhotonRHIDs();
  void DoPlotIndices();
  void DoPlotNatural(const Int_t ientry = -1, const Int_t iph = -1, const Bool_t applyrhEcut = false, const Float_t rhEcut = 0.f);
  void DoAllPlotNatural(const Float_t rhEcut = 0.f);

private:
  TFile * fInFile;
  TTree * fInTree;

  // Runtime variables
  epidMap epids;
  TString    fPhVID;
  TStrIntMap fPhVIDMap;

  // input variables
  Int_t                                nphotons;
  std::vector<Int_t>                 * phnrh;
  std::vector<Float_t>               * phE;
  std::vector<Float_t>               * phpt;
  std::vector<Float_t>               * pheta;
  std::vector<Float_t>               * phsmaj;
  std::vector<Float_t>               * phsmin;
  std::vector<Int_t>                 * phVID;
  std::vector<Int_t>                 * phseedpos;
  std::vector<std::vector<Int_t> >   * phrhID;
  std::vector<std::vector<Float_t> > * phrhE;
  std::vector<std::vector<Float_t> > * phrhX;
  std::vector<std::vector<Float_t> > * phrhY;
  std::vector<std::vector<Float_t> > * phrhZ;
  std::vector<std::vector<Float_t> > * phrhtime;
  
  TBranch * b_nphotons;  
  TBranch * b_phnrh;
  TBranch * b_phE;     
  TBranch * b_phpt;     
  TBranch * b_pheta;     
  TBranch * b_phsmaj;     
  TBranch * b_phsmin;     
  TBranch * b_phVID;     
  TBranch * b_phseedpos; 
  TBranch * b_phrhID;
  TBranch * b_phrhE;
  TBranch * b_phrhX;
  TBranch * b_phrhY;
  TBranch * b_phrhZ;
  TBranch * b_phrhtime;
};

#endif
