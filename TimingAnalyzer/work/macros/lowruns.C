#include <fstream>
#include <iostream>
#include <map>
#include <utility>
#include <vector>
#include <algorithm>

void lowruns(){

  TFile * file = TFile::Open("input/DATA/doubleeg/skimmedtree.root");
  TTree * tree = (TTree*)file->Get("tree/tree");

  ifstream input;
  input.open("config/runs2015D.txt",std::ios::in);

  Int_t runno;
  std::map<Int_t,Int_t> runcount;
  while(input >> runno){
    runcount[runno] = 0;
  }
  input.close();

  Int_t run; 
  tree->SetBranchAddress("run",&run);

  for (UInt_t i = 0; i < tree->GetEntries(); i++){
    tree->GetEntry(i);
    runcount[run]++;
  }

  for (std::map<Int_t,Int_t>::iterator iter = runcount.begin(); iter != runcount.end(); ++iter){
    Int_t count = (*iter).second;
    Int_t run   = (*iter).first;

    if ( count < 100 ) {
      std::cout << "run: " << run << " nEvents: " << count << std::endl;
    }
  }

  std::cout << "Average: " << Float_t(tree->GetEntries()) / runcount.size() << std::endl;

  std::vector<Int_t> counts;
  for (std::map<Int_t,Int_t>::iterator iter = runcount.begin(); iter != runcount.end(); ++iter){
    Int_t count = (*iter).second;
    counts.push_back(count);
  }
}
