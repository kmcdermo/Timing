#include "ABCDGenerator.hh"

ABCDGenerator::ABCDGenerator(const TString & infilename, const TString & bininfoname,
			     const TString & ratioinfoname, const TString & binratioinfoname)
  : fInFileName(infilename), fBinInfoName(bininfoname),
    fRatioInfoName(ratioinfoname), fBinRatioInfoName(binratioinfoname)
{
  std::cout << "Initializing ABCDGenerator..." << std::endl;

  // Setup
  ABCDGenerator::SetupCommon();
  ABCDGenerator::SetupBinningScheme();
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
  std::cout << "Making generic maximally sized grid..." << std::endl;

  // temps needed for filling maps
  std::vector<Int_t> RatioXVec; // Ratios along x-axis
  std::vector<Int_t> RatioYVec; // Ratios along y-axis

  auto bin = 0;
  auto ratio = 0;
  auto i_X = 0;

  // loop over NxN max grid (where N is the larger of nbinsX and nbinsY), setting global bin number and ratios
  for (auto j = 1; j <= ABCD::nMaxBins; j++)
  {
    // increment max x-extent for given y-extent
    if (i_X != ABCD::nMaxBins) i_X++;
    
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
  std::cout << "Pruning grid down to requested size..." << std::endl;

  // tmp vector ot bins beyond extent
  std::vector<Int_t> BinsToRemove;
  
  // store which bins to drop
  for (const auto & BinPair : ABCD::BinMap)
  {
    if (BinPair.second.ibinX > ABCD::nbinsX) BinsToRemove.emplace_back(BinPair.first);
    if (BinPair.second.ibinY > ABCD::nbinsY) BinsToRemove.emplace_back(BinPair.first);
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
  std::cout << "Writing ABCD configuration to text files..." << std::endl;

  std::ofstream outbins(fBinInfoName.Data(),std::ios::trunc);
  for (const auto & BinPair : ABCD::BinMap)
  {
    outbins << BinPair.first << " " << BinPair.second.ibinX << " " << BinPair.second.ibinY << std::endl;
  }

  std::ofstream outratios(fRatioInfoName.Data(),std::ios::trunc);
  for (const auto & RatioPair : ABCD::RatioMap)
  {
    outratios << RatioPair.first << " " << RatioPair.second << std::endl;
  }

  std::ofstream outbinratios(fBinRatioInfoName.Data(),std::ios::trunc);
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

void ABCDGenerator::SetupCommon()
{
  std::cout << "Setup Common..." << std::endl;

  Common::SetupSamples();
  Common::SetupGroups();
  Common::SetupHistNames();
}

void ABCDGenerator::SetupBinningScheme()
{
  std::cout << "Getting binning from data hist..." << std::endl;

  // get file + hist
  auto infile = TFile::Open(fInFileName.Data());
  Common::CheckValidFile(infile,fInFileName);
  infile->cd();
  
  const TString inhistname = Common::HistNameMap["Data"]+"_Plotted";
  auto inhist = (TH2F*)infile->Get(inhistname.Data());
  Common::CheckValidHist(inhist,inhistname,fInFileName);

  // set the values in ABCD
  ABCD::SetupNBins(inhist);

  // delete tmps
  delete inhist;
  delete infile;
}
