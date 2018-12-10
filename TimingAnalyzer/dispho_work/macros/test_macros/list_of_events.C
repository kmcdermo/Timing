#include "Common.cpp+"

void list_of_events(const TString & filename, const TString & textfilename)
{
  // get inputs
  auto file = TFile::Open(filename.Data());
  Common::CheckValidFile(file,filename);
  
  const TString treename = "Data_Tree";
  auto tree = (TTree*)file->Get(treename.Data());
  Common::CheckValidTree(tree,treename,filename);

  Float_t t1pfMETpt = 0.f; TBranch * b_t1pfMETpt = 0; const std::string s_t1pfMETpt = "t1pfMETpt"; tree->SetBranchAddress(s_t1pfMETpt.c_str(), &t1pfMETpt, &b_t1pfMETpt);
  Float_t phoseedtime_0 = 0.f; TBranch * b_phoseedtime_0 = 0; const std::string s_phoseedtime_0 = "phoseedtime_0"; tree->SetBranchAddress(s_phoseedtime_0.c_str(), &phoseedtime_0, &b_phoseedtime_0);
  UInt_t run = 0.f; TBranch * b_run = 0; const std::string s_run = "run"; tree->SetBranchAddress(s_run.c_str(), &run, &b_run);
  UInt_t lumi = 0.f; TBranch * b_lumi = 0; const std::string s_lumi = "lumi"; tree->SetBranchAddress(s_lumi.c_str(), &lumi, &b_lumi);
  ULong64_t event; TBranch * b_event = 0; const std::string s_event = "event"; tree->SetBranchAddress(s_event.c_str(), &event, &b_event);

  // get text file
  std::ofstream textfile(textfilename.Data(),std::ios_base::trunc);

  const auto nentries = tree->GetEntries();
  for (auto ientry = 0U; ientry < nentries; ientry++)
  {
    b_phoseedtime_0->GetEntry(ientry);
    if (phoseedtime_0 <  3.f) continue;
    if (phoseedtime_0 > 25.f) continue;

    b_t1pfMETpt->GetEntry(ientry);
    if (t1pfMETpt <  100.f) continue;
    if (t1pfMETpt > 3000.f) continue;

    b_run->GetEntry(ientry);
    b_lumi->GetEntry(ientry);
    b_event->GetEntry(ientry);

    textfile << run << ":" << lumi << ":" << event << std::endl;
  }
}
