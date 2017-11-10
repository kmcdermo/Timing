#include "TString.h"

#include <fstream>
#include <iostream>

void computeAverageXSec(const TString & filename)
{
  // input stream
  std::ifstream input(filename.Data(),std::ios::in); 
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
  std::cout << sum_xsec << " " << sum_exsec2 << std::endl;
}
