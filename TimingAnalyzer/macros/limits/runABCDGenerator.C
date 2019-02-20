#include "../Common.cpp+"
#include "ABCD.cpp+"
#include "ABCDGenerator.cpp+"

void runABCDGenerator(const Int_t nbinsX, const Int_t nbinsY)
{
  ABCDGenerator Generator(nbinsX,nbinsY);
  Generator.ProduceABCDConfig();
}
