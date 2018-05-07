#include "TString.h"

#include <fstream>
#include <iostream>

void computeAverageXSec(const TString & infilename, const TString & outfilename)
{
  // input stream
  std::ifstream input(infilename.Data(),std::ios::in); 
  Float_t xsec;
  Float_t exsec;

  // output stream
  Float_t sum_xsec   = 0.f;
  Float_t sum_exsec2 = 0.f;
  Int_t n = 0;

  // read in file
  while (input >> xsec >> exsec)
  {
    sum_xsec   += xsec;
    sum_exsec2 += (exsec*exsec);
    n++;
  }

  // compute final output
  sum_xsec   /= n;
  sum_exsec2  = std::sqrt(sum_exsec2)/n;

  // dump
  std::ofstream output(outfilename.Data(),std::ios_base::trunc);
  output << sum_xsec << " " << sum_exsec2 << std::endl;
}
