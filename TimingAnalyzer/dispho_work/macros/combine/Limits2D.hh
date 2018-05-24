#ifndef __Limits2D__
#define __Limits2D__

#include "TROOT.h"
#include "TStyle.h"
#include "TGraph.h"
#include "TLegend.h"
#include "TCanvas.h"

#include "../Common.hh"
#include "Combine.hh"

#include <vector>
#include <algorithm>

struct Bin2D
{
  Bin2D() {}
  Bin2D(const Float_t xlow, const Float_t xcenter, const Float_t xup, 
	const Float_t ylow, const Float_t ycenter, const Float_t yup,
	const std::map<TString,Float_t> && rvalmap)
  : xlow(xlow), xcenter(xcenter), xup(xup), 
    ylow(ylow), ycenter(ycenter), yup(yup),
    rvalmap(rvalmap) {}

  // bin info
  Float_t xlow;
  Float_t xcenter;
  Float_t xup;
  Float_t ylow;
  Float_t ycenter;
  Float_t yup;

  // rvalues for filling (z): copied from GMSBMap
  std::map<TString,Float_t> rvalmap;
};

class Limits2D
{
public:
  Limits2D(const TString & indir, const TString & infilename, const Bool_t doobserved, const TString & outtext);
  ~Limits2D();

  // setup functions
  void Setup();

  // Interpolation functions
  Float_t ZValue(const Float_t x, const Float_t x1, const Float_t x2, 
		 const Float_t y, const Float_t y1, const Float_t y2, 
		 const Float_t fQ11, const Float_t fQ12, const Float_t fQ21, const Float_t fQ22);

  // main plotting routine
  void MakeLimits2D();

private:
  const TString fInDir;
  const TString fInFileName;
  const Bool_t fDoObserved;
  const TString fOutText;

  // style
  TStyle * fTDRStyle;

  // output
  TFile * fOutFile;
};

#endif
