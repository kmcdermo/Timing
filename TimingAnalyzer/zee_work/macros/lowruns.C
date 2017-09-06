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

  Int_t bigs = 0;
  for (std::map<Int_t,Int_t>::iterator iter = runcount.begin(); iter != runcount.end(); ++iter){
    Int_t count = (*iter).second;
    Int_t run   = (*iter).first;

    if ( count < 1000 ) {
      std::cout << "run: " << run << " nEvents: " << count << std::endl;
    }
    if ( count > 1000 ) {
      bigs++;
    }
  }

  std::cout << "nEvents: " << tree->GetEntries() << " nRuns: " << runcount.size() << " Average: " << Float_t(tree->GetEntries()) / runcount.size() << std::endl;
  std::cout << "nRuns > 1000 events: " << bigs << std::endl;

}
