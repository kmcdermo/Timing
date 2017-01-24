void checkRuns()
{
  TFile * file = TFile::Open("input/DATA/doubleeg/skim/recoskim.root");
  TTree * tree = (TTree*)file->Get("tree/tree");

  Int_t run = 0; TBranch * b_run; tree->SetBranchAddress("run", &run, &b_run);

  std::map<Int_t,Int_t> runmap;
  for (UInt_t entry = 0; entry < tree->GetEntries(); entry++)
  {
    tree->GetEntry(entry);
    
    runmap[run]++;
  }

  for (std::map<Int_t,Int_t>::const_iterator iter = runmap.begin(); iter != runmap.end(); ++iter)
  {
    std::cout << iter->first << " " << iter->second << std::endl;
  }

  delete tree;
  delete file;
}
