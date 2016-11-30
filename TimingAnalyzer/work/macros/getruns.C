#include "TTree.h"
#include "TFile.h"

#include <vector>
#include <fstream>
#include <unordered_map>

void getruns()
{
  TFile * file = new TFile("input/DATA/2016/doubleeg/skimmedtree.root");
  TTree * tree = (TTree*)file->Get("tree/tree");
  
  Int_t run = 0;
  tree->SetBranchAddress("run",&run);
  std::unordered_map<Int_t,Int_t> runs;

  ofstream output;
  output.open("runs2016.txt",std::ios_base::trunc);

  for (UInt_t entry = 0; entry < tree->GetEntries(); entry++)
  {
    tree->GetEntry(entry);
    if (runs.count(run) == 0)
    {
      runs[run]++;
      output << run << std::endl;
    }
  }

  output.close();
}
