#include "TTree.h"
#include "TFile.h"

#include <vector>
#include <fstream>

void getruns()
{
  TFile * file = new TFile("input/DATA/2016/doubleeg/skimmedtree.root");
  TTree * tree = (TTree*)file->Get("tree/tree");
  
  Int_t run = 0;
  tree->SetBranchAddress("run",&run);

  Int_t tmprun = 0;
  std::vector<Int_t> runs;
  
  ofstream output;
  output.open("runs2016.txt",std::ios_base::trunc);

  for (UInt_t entry = 0; entry < tree->GetEntries(); entry++)
  {
    tree->GetEntry(entry);
    if (run != tmprun) {
      tmprun = run; //runs.push_back(run);
      output << run << std::endl;
    }
  }

  output.close();

//   for (int i = 0; i < runs.size(); i++)
//   {
//     for (int j = i+1; j < runs.size(); j++)
//     {
//       if (runs[i] == runs[j]) std::cout << runs[i] << std::endl;
//     }
//   }
}
