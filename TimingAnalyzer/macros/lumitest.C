#include <map>

void lumitest()
{
  TFile * file = TFile::Open("ootrecotree.root");
  TTree * tree = (TTree*)file->Get("tree/countingtree");

  Int_t run;  tree->SetBranchAddress("run",&run);
  Int_t lumi; tree->SetBranchAddress("lumi",&lumi);
  Int_t event; tree->SetBranchAddress("event",&event);

  Int_t nphotons; tree->SetBranchAddress("nphotons",&nphotons);

  Int_t trun = 0, tlumi = 0, nevents = 0, nbadevs = 0;
  std::map<int,int> tevent;
  for (UInt_t i = 0; i < tree->GetEntries(); i++)
  {
    tree->GetEntry(i);

    if (nphotons < 1)
    {
      nbadevs++;
      //std::cout << event << " : " << nphotons << std::endl;
    }

    if (run > trun)
    { 
      trun = run;
      std::cout << "run: " << trun << std::endl;
    }
    if (lumi > tlumi)
    { 
      if (nevents != 0) std::cout << " nevents: " << nevents << std::endl;
      nevents = 0;
      tlumi = lumi;
      std::cout << "lumi: " << tlumi << std::endl;
    }
    if (tevent.count(event) == 0)
    {
      nevents++;
      tevent[event]++;
    }
  }
  std::cout << " nevents: " << nevents << " nbadevs: " << nbadevs << std::endl;
}
