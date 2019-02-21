#include "ABCD.hh"

namespace ABCD
{
  Int_t nbinsX;
  Int_t nbinsY;
  Int_t nMaxBins;
  Int_t nbinsXY;
  void SetupNBins(const TH2F * hist)
  {
    ABCD::nbinsX = hist->GetXaxis()->GetNbins();
    ABCD::nbinsY = hist->GetYaxis()->GetNbins();
    
    // Set max range of NxN
    ABCD::nMaxBins = std::max(nbinsX,nbinsY);

    // Set total nBins
    ABCD::nbinsXY = ABCD::nbinsX*ABCD::nbinsY;
  }

  std::map<Int_t,BinXY> BinMap;
  void SetupBinMap(const TString & binInfoName)
  {
    std::ifstream inbins(binInfoName.Data(),std::ios::in);
    auto bin = 0, ibinX = 0, ibinY = 0;
  
    while (inbins >> bin >> ibinX >> ibinY)
    {
      ABCD::BinMap[bin] = {ibinX,ibinY};
    }
  }

  std::map<Int_t,Int_t> RatioMap;
  void SetupRatioMap(const TString & ratioInfoName)
  {
    std::ifstream inratios(ratioInfoName.Data(),std::ios::in);
    auto bin = 0, ratio = 0;
  
    while (inratios >> ratio >> bin)
    {
      ABCD::RatioMap[ratio] = bin;
    }
  }

  std::map<Int_t,std::vector<Int_t> > BinRatioVecMap;
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
