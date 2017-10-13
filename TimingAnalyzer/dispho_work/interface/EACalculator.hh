#ifndef _eacalculator_
#define _eacalculator_

#include "CommonTypes.hh"

class EACalculator
{
public:
  EACalculator(const TString & sample, const Bool_t isMC);
  ~EACalculator();

  void ExtractEA();

private:
  const TString fSample;
  const Bool_t fIsMC;
  
};

#endif
