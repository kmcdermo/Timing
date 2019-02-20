#include "ABCD.hh"

namespace ABCD
{

  void SetupBinMap(const TString & binInfoName)
  {
    std::ifstream inbins(binInfoName.Data(),std::ios::in);
    auto bin = 0, ibinX = 0, ibinY = 0;
  
    while (inbins >> bin >> ibinX >> ibinY)
    {
      ABCD::BinMap[bin] = {ibinX,ibinY};
    }
  }

  void SetupRatioMap(const TString & ratioInfoName)
  {
    std::ifstream inratios(ratioInfoName.Data(),std::ios::in);
    auto bin = 0, ratio = 0;
  
    while (inratios >> ratio >> bin)
    {
      ABCD::RatioMap[ratio] = bin;
    }
  }

  void SetupBinRatioVecMap(const TString & binRatioInfoName)
  {
    std::ifstream inbinratios(binRatioInfoName.Data(),std::ios::in);
    std::string line;
  
    while (std::getline(inbinratios,line))
    {
      std::stringstream linestream(line);
      auto i = 0, bin = 0;
      auto isBin = true;
      
      while (linestream >> i)
      {
	if (!isBin)
	{
	  ABCD::BinRatioVecMap[bin].emplace_back(i);
	}
	else
        {
	  bin = i;
	  isBin = false;
	}
      }
    }
  }
  
  void DumpInfo()
  {
    for (const auto & BinPair : ABCD::BinMap)
    {
      std::cout << BinPair.first << ": " << BinPair.second.ibinX << "," << BinPair.second.ibinY << std::endl;
    }

    std::cout << "---------------------------------------------------" << std::endl;

    for (const auto & RatioPair : ABCD::RatioMap)
    {
      std::cout << RatioPair.first << ": " << RatioPair.second << std::endl;
    }

    std::cout << "---------------------------------------------------" << std::endl;
  
    for (const auto & BinRatioVecPair : ABCD::BinRatioVecMap)
    {
      std::cout << BinRatioVecPair.first << ": ";
      for (const auto & BinRatio : BinRatioVecPair.second)
      {
	std::cout << BinRatio << " ";
      }
      std::cout << std::endl;
    }
  }
};
