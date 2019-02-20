#include  "ABCDGenerator.hh"

ABCDGenerator::ABCDGenerator(const Int_t nbinsX, const Int_t nbinsY)
  : fNbinsX(nbinsX), fNbinsY(nbinsY)
{
  // Set max range of NxN
  fNMaxBins = std::max(fNbinsX,fNbinsY);
}

void ABCDGenerator::ProduceABCDConfig()
{
  // produce maps for maximum extent in grid of NxN
  ABCDGenerator::MakeMaxGrid();
  
  // prune maps for only what is needed
  ABCDGenerator::PruneGrid();

  // write out configs
  ABCDGenerator::WriteConfigs();
}

void ABCDGenerator::MakeMaxGrid()
{
  // temps needed for filling maps
  std::vector<Int_t> RatioXVec; // Ratios along x-axis
  std::vector<Int_t> RatioYVec; // Ratios along y-axis

  auto bin = 0;
  auto ratio = 0;
  auto i_X = 0;

  // loop over NxN max grid (where N is the larger of nbinsX and nbinsY), setting global bin number and ratios
  for (auto j = 1; j <= fNMaxBins; j++)
  {
    // increment max x-extent for given y-extent
    if (i_X != fNMaxBins) i_X++;
    
    // increment bins to the right first
    for (auto i = 1; i < i_X; i++)
    {  
      ABCD::BinMap[++bin] = {i,j};

      if (j != 1 && i == 1)
      {
	ABCD::RatioMap[++ratio] = bin;
	RatioYVec.emplace_back(ratio);
      }
    }

    // then increment bins down second
    for (auto k = j; k >= 1; k--)
    {
      ABCD::BinMap[++bin] = {i_X,k};

      if (i_X != 1 && k == 1)
      {
	ABCD::RatioMap[++ratio] = bin;
	RatioXVec.emplace_back(ratio);
      }
    }
  }

  // loop over all bins to set ratios that belong to each bin
  for (const auto & BinPair : ABCD::BinMap)
  {
    const auto bin   = BinPair.first;
    const auto ibinX = BinPair.second.ibinX;
    const auto ibinY = BinPair.second.ibinY;

    for (auto i = 2; i <= ibinX; i++)
    {
      ABCD::BinRatioVecMap[bin].emplace_back(RatioXVec[i-2]);
    }

    for (auto j = 2; j <= ibinY; j++)
    {
      ABCD::BinRatioVecMap[bin].emplace_back(RatioYVec[j-2]);
    }
  }
}

void ABCDGenerator::PruneGrid()
{
  // tmp vector ot bins beyond extent
  std::vector<Int_t> BinsToRemove;
  
  // store which bins to drop
  for (const auto & BinPair : ABCD::BinMap)
  {
    if (BinPair.second.ibinX > fNbinsX) BinsToRemove.emplace_back(BinPair.first);
    if (BinPair.second.ibinY > fNbinsY) BinsToRemove.emplace_back(BinPair.first);
  }

  // now start erasing from maps by key
  for (const auto BinToRemove : BinsToRemove)
  {
    ABCD::BinMap.erase(BinToRemove);
    ABCD::BinRatioVecMap.erase(BinToRemove);
  }

  // erase maps by value
  for (auto iter = ABCD::RatioMap.cbegin(); iter != ABCD::RatioMap.cend();)
  {
    auto toRemove = false;

    for (const auto BinToRemove : BinsToRemove)
    {
      if (iter->second == BinToRemove)
      {
	toRemove = true;
	break;
      }
    }

    if (toRemove)
    {
      ABCD::RatioMap.erase(iter++);
    }
    else
    {
      iter++;
    }
  }
}

void ABCDGenerator::WriteConfigs()
{
  std::ofstream outbins(ABCD::BinInfoName.Data(),std::ios::trunc);
  for (const auto & BinPair : ABCD::BinMap)
  {
    outbins << BinPair.first << " " << BinPair.second.ibinX << " " << BinPair.second.ibinY << std::endl;
  }

  std::ofstream outratios(ABCD::RatioInfoName.Data(),std::ios::trunc);
  for (const auto & RatioPair : ABCD::RatioMap)
  {
    outratios << RatioPair.first << " " << RatioPair.second << std::endl;
  }

  std::ofstream outbinratios(ABCD::BinRatioInfoName.Data(),std::ios::trunc);
  for (const auto & BinRatioVecPair : ABCD::BinRatioVecMap)
  {
    outbinratios << BinRatioVecPair.first << " ";
    for (const auto & BinRatio : BinRatioVecPair.second)
    {
      outbinratios << BinRatio << " ";
    }
    outbinratios << std::endl;
  }
}
