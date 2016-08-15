// run by root -l -b -q macros/wgttime.C+

#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"

#include <iostream>
#include <vector>
#include <utility>
#include <algorithm>

typedef std::pair<Float_t,Float_t> ETpair;

inline bool sortByE(const ETpair & pair1, const ETpair & pair2)
{
  return pair1.first > pair2.first;
}

static const Float_t N  = 33.2; // ns
static const Float_t C  = 0.154; // ns
static const Float_t sn = 0.0513; // GeV

void wgttime()
{
  TFile * file = TFile::Open("input/DATA/doubleeg/skimmedtree.root");
  TTree * tree = (TTree*)file->Get("tree/tree");

  std::vector<Float_t> * el1rhEs; TBranch * b_el1rhEs;   
  tree->SetBranchAddress("el1rhEs", &el1rhEs, &b_el1rhEs);

  std::vector<Float_t> * el1rhtimes; TBranch * b_el1rhtimes;   
  tree->SetBranchAddress("el1rhtimes", &el1rhtimes, &b_el1rhtimes);

  for (UInt_t entry = 0; entry < tree->GetEntries(); entry++)
  {
    if (entry > 10) break;
    tree->GetEntry(entry);
    //    std::cout << "Entry: " << entry << std::endl;

    // sort first, then calculate
    std::vector<ETpair> ETpairsVec;
    for (UInt_t rh = 0; rh < el1rhtimes->size(); rh++)
    {
      Float_t tmpE = (*el1rhEs)[rh];
      Float_t tmpT = (*el1rhtimes)[rh];
      ETpairsVec.push_back(std::make_pair(tmpE,tmpT));
    }
    std::sort(ETpairsVec.begin(),ETpairsVec.end(),sortByE);

    Float_t maxE = -999.0f;
    Float_t maxT = -999.0f;
    Float_t wgtT = 0.0f;
    Float_t sumS = 0.0f;

//     Float_t wgtT_noC = 0.0f;
//     Float_t sumS_noC = 0.0f;
    for (UInt_t rh = 0; rh < ETpairsVec.size(); rh++)
    {
      const ETpair & pair = ETpairsVec[rh];
      Float_t tmpE = pair.first;
      Float_t tmpT = pair.second;
      Float_t tmpS = (N / (tmpE / sn)) + (std::sqrt(2.0f)*C);

      sumS += 1.0f / (tmpS*tmpS);
      wgtT += tmpT / (tmpS*tmpS);
      
      if (tmpE > maxE) { maxT = tmpT; maxE = tmpE; }

//      std::cout << "tmpT: " << tmpT << " tmpE: " << tmpE << " tmpS: " << tmpS << std::endl;
//       Float_t tmpS_noC = (N / (tmpE / sn));
//       sumS_noC += 1.0f / (tmpS_noC * tmpS_noC);
//       wgtT_noC += tmpT / (tmpS_noC * tmpS_noC);
    }
//     std::cout << "==========================" << std::endl << std::endl;
//     std::cout << "maxE: " << maxE << " maxT: " << maxT << " wgT: " << wgtT/sumS << std::endl;
//     //" wgT_noC: " << wgtT_noC/sumS_noC << std::endl;
//     std::cout << "==========================" << std::endl << std::endl;
  }
}
