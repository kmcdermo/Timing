#include "Common.cpp+"

void list_of_events(const TString & filename, const TString & textfilename, const TString & outfilename)
{
  // get inputs
  auto file = TFile::Open(filename.Data());
  Common::CheckValidFile(file,filename);
  
  const TString treename = "Data_Tree";
  auto tree = (TTree*)file->Get(treename.Data());
  Common::CheckValidTree(tree,treename,filename);

  UInt_t run = 0U; TBranch * b_run = 0; const std::string s_run = "run"; tree->SetBranchAddress(s_run.c_str(), &run, &b_run);
  UInt_t lumi = 0U; TBranch * b_lumi = 0; const std::string s_lumi = "lumi"; tree->SetBranchAddress(s_lumi.c_str(), &lumi, &b_lumi);
  ULong64_t event = 0UL; TBranch * b_event = 0; const std::string s_event = "event"; tree->SetBranchAddress(s_event.c_str(), &event, &b_event);

  Float_t rho = 0.f; TBranch * b_rho = 0; const std::string s_rho = "rho"; tree->SetBranchAddress(s_rho.c_str(), &rho, &b_rho);

  Float_t t1pfMETpt = 0.f; TBranch * b_t1pfMETpt = 0; const std::string s_t1pfMETpt = "t1pfMETpt"; tree->SetBranchAddress(s_t1pfMETpt.c_str(), &t1pfMETpt, &b_t1pfMETpt);
  Float_t t1pfMETphi = 0.f; TBranch * b_t1pfMETphi = 0; const std::string s_t1pfMETphi = "t1pfMETphi"; tree->SetBranchAddress(s_t1pfMETphi.c_str(), &t1pfMETphi, &b_t1pfMETphi);
  Float_t t1pfMETsumEt = 0.f; TBranch * b_t1pfMETsumEt = 0; const std::string s_t1pfMETsumEt = "t1pfMETsumEt"; tree->SetBranchAddress(s_t1pfMETsumEt.c_str(), &t1pfMETsumEt, &b_t1pfMETsumEt);

  Float_t phopt_0 = 0.f; TBranch * b_phopt_0 = 0; const std::string s_phopt_0 = "phopt_0"; tree->SetBranchAddress(s_phopt_0.c_str(), &phopt_0, &b_phopt_0);
  Float_t phophi_0 = 0.f; TBranch * b_phophi_0 = 0; const std::string s_phophi_0 = "phophi_0"; tree->SetBranchAddress(s_phophi_0.c_str(), &phophi_0, &b_phophi_0);
  Float_t phoeta_0 = 0.f; TBranch * b_phoeta_0 = 0; const std::string s_phoeta_0 = "phoeta_0"; tree->SetBranchAddress(s_phoeta_0.c_str(), &phoeta_0, &b_phoeta_0);
  Float_t phosceta_0 = 0.f; TBranch * b_phosceta_0 = 0; const std::string s_phosceta_0 = "phosceta_0"; tree->SetBranchAddress(s_phosceta_0.c_str(), &phosceta_0, &b_phosceta_0);
  Float_t phoHoE_0 = 0.f; TBranch * b_phoHoE_0 = 0; const std::string s_phoHoE_0 = "phoHoE_0"; tree->SetBranchAddress(s_phoHoE_0.c_str(), &phoHoE_0, &b_phoHoE_0);
  Float_t phor9_0 = 0.f; TBranch * b_phor9_0 = 0; const std::string s_phor9_0 = "phor9_0"; tree->SetBranchAddress(s_phor9_0.c_str(), &phor9_0, &b_phor9_0);
  Float_t phosieie_0 = 0.f; TBranch * b_phosieie_0 = 0; const std::string s_phosieie_0 = "phosieie_0"; tree->SetBranchAddress(s_phosieie_0.c_str(), &phosieie_0, &b_phosieie_0);
  Float_t phosuisseX_0 = 0.f; TBranch * b_phosuisseX_0 = 0; const std::string s_phosuisseX_0 = "phosuisseX_0"; tree->SetBranchAddress(s_phosuisseX_0.c_str(), &phosuisseX_0, &b_phosuisseX_0);
  Float_t phoseedE_0 = 0.f; TBranch * b_phoseedE_0 = 0; const std::string s_phoseedE_0 = "phoseedE_0"; tree->SetBranchAddress(s_phoseedE_0.c_str(), &phoseedE_0, &b_phoseedE_0);
  Float_t phoseedtime_0 = 0.f; TBranch * b_phoseedtime_0 = 0; const std::string s_phoseedtime_0 = "phoseedtime_0"; tree->SetBranchAddress(s_phoseedtime_0.c_str(), &phoseedtime_0, &b_phoseedtime_0);
  Float_t phoweightedtimeLT120_0 = 0.f; TBranch * b_phoweightedtimeLT120_0 = 0; const std::string s_phoweightedtimeLT120_0 = "phoweightedtimeLT120_0"; tree->SetBranchAddress(s_phoweightedtimeLT120_0.c_str(), &phoweightedtimeLT120_0, &b_phoweightedtimeLT120_0);
  Float_t phoChgHadIso_0 = 0.f; TBranch * b_phoChgHadIso_0 = 0; const std::string s_phoChgHadIso_0 = "phoChgHadIso_0"; tree->SetBranchAddress(s_phoChgHadIso_0.c_str(), &phoChgHadIso_0, &b_phoChgHadIso_0);
  Float_t phoNeuHadIso_0 = 0.f; TBranch * b_phoNeuHadIso_0 = 0; const std::string s_phoNeuHadIso_0 = "phoNeuHadIso_0"; tree->SetBranchAddress(s_phoNeuHadIso_0.c_str(), &phoNeuHadIso_0, &b_phoNeuHadIso_0);
  Float_t phoPhoIso_0 = 0.f; TBranch * b_phoPhoIso_0 = 0; const std::string s_phoPhoIso_0 = "phoPhoIso_0"; tree->SetBranchAddress(s_phoPhoIso_0.c_str(), &phoPhoIso_0, &b_phoPhoIso_0);
  Float_t phoTrkIso_0 = 0.f; TBranch * b_phoTrkIso_0 = 0; const std::string s_phoTrkIso_0 = "phoTrkIso_0"; tree->SetBranchAddress(s_phoTrkIso_0.c_str(), &phoTrkIso_0, &b_phoTrkIso_0);

  Bool_t phoisOOT_0 = 0; TBranch * b_phoisOOT_0 = 0; const std::string s_phoisOOT_0 = "phoisOOT_0"; tree->SetBranchAddress(s_phoisOOT_0.c_str(), &phoisOOT_0, &b_phoisOOT_0);
  Bool_t phoisEB_0 = 0; TBranch * b_phoisEB_0 = 0; const std::string s_phoisEB_0 = "phoisEB_0"; tree->SetBranchAddress(s_phoisEB_0.c_str(), &phoisEB_0, &b_phoisEB_0);
  Bool_t phoisTrk_0 = 0; TBranch * b_phoisTrk_0 = 0; const std::string s_phoisTrk_0 = "phoisTrk_0"; tree->SetBranchAddress(s_phoisTrk_0.c_str(), &phoisTrk_0, &b_phoisTrk_0);
  Bool_t phohasPixSeed_0 = 0; TBranch * b_phohasPixSeed_0 = 0; const std::string s_phohasPixSeed_0 = "phohasPixSeed_0"; tree->SetBranchAddress(s_phohasPixSeed_0.c_str(), &phohasPixSeed_0, &b_phohasPixSeed_0);
  Int_t phogedID_0 = 0; TBranch * b_phogedID_0 = 0; const std::string s_phogedID_0 = "phogedID_0"; tree->SetBranchAddress(s_phogedID_0.c_str(), &phogedID_0, &b_phogedID_0);
  Int_t phoootID_0 = 0; TBranch * b_phoootID_0 = 0; const std::string s_phoootID_0 = "phoootID_0"; tree->SetBranchAddress(s_phoootID_0.c_str(), &phoootID_0, &b_phoootID_0);
  Int_t phoseedisGS6_0 = 0; TBranch * b_phoseedisGS6_0 = 0; const std::string s_phoseedisGS6_0 = "phoseedisGS6_0"; tree->SetBranchAddress(s_phoseedisGS6_0.c_str(), &phoseedisGS6_0, &b_phoseedisGS6_0);
  Int_t phoseedisGS1_0 = 0; TBranch * b_phoseedisGS1_0 = 0; const std::string s_phoseedisGS1_0 = "phoseedisGS1_0"; tree->SetBranchAddress(s_phoseedisGS1_0.c_str(), &phoseedisGS1_0, &b_phoseedisGS1_0);

  Float_t phopt_1 = 0.f; TBranch * b_phopt_1 = 0; const std::string s_phopt_1 = "phopt_1"; tree->SetBranchAddress(s_phopt_1.c_str(), &phopt_1, &b_phopt_1);
  Float_t phophi_1 = 0.f; TBranch * b_phophi_1 = 0; const std::string s_phophi_1 = "phophi_1"; tree->SetBranchAddress(s_phophi_1.c_str(), &phophi_1, &b_phophi_1);
  Float_t phoeta_1 = 0.f; TBranch * b_phoeta_1 = 0; const std::string s_phoeta_1 = "phoeta_1"; tree->SetBranchAddress(s_phoeta_1.c_str(), &phoeta_1, &b_phoeta_1);
  Float_t phosceta_1 = 0.f; TBranch * b_phosceta_1 = 0; const std::string s_phosceta_1 = "phosceta_1"; tree->SetBranchAddress(s_phosceta_1.c_str(), &phosceta_1, &b_phosceta_1);
  Float_t phoHoE_1 = 0.f; TBranch * b_phoHoE_1 = 0; const std::string s_phoHoE_1 = "phoHoE_1"; tree->SetBranchAddress(s_phoHoE_1.c_str(), &phoHoE_1, &b_phoHoE_1);
  Float_t phor9_1 = 0.f; TBranch * b_phor9_1 = 0; const std::string s_phor9_1 = "phor9_1"; tree->SetBranchAddress(s_phor9_1.c_str(), &phor9_1, &b_phor9_1);
  Float_t phosieie_1 = 0.f; TBranch * b_phosieie_1 = 0; const std::string s_phosieie_1 = "phosieie_1"; tree->SetBranchAddress(s_phosieie_1.c_str(), &phosieie_1, &b_phosieie_1);
  Float_t phosuisseX_1 = 0.f; TBranch * b_phosuisseX_1 = 0; const std::string s_phosuisseX_1 = "phosuisseX_1"; tree->SetBranchAddress(s_phosuisseX_1.c_str(), &phosuisseX_1, &b_phosuisseX_1);
  Float_t phoseedE_1 = 0.f; TBranch * b_phoseedE_1 = 0; const std::string s_phoseedE_1 = "phoseedE_1"; tree->SetBranchAddress(s_phoseedE_1.c_str(), &phoseedE_1, &b_phoseedE_1);
  Float_t phoseedtime_1 = 0.f; TBranch * b_phoseedtime_1 = 0; const std::string s_phoseedtime_1 = "phoseedtime_1"; tree->SetBranchAddress(s_phoseedtime_1.c_str(), &phoseedtime_1, &b_phoseedtime_1);
  Float_t phoweightedtimeLT120_1 = 0.f; TBranch * b_phoweightedtimeLT120_1 = 0; const std::string s_phoweightedtimeLT120_1 = "phoweightedtimeLT120_1"; tree->SetBranchAddress(s_phoweightedtimeLT120_1.c_str(), &phoweightedtimeLT120_1, &b_phoweightedtimeLT120_1);
  Float_t phoChgHadIso_1 = 0.f; TBranch * b_phoChgHadIso_1 = 0; const std::string s_phoChgHadIso_1 = "phoChgHadIso_1"; tree->SetBranchAddress(s_phoChgHadIso_1.c_str(), &phoChgHadIso_1, &b_phoChgHadIso_1);
  Float_t phoNeuHadIso_1 = 0.f; TBranch * b_phoNeuHadIso_1 = 0; const std::string s_phoNeuHadIso_1 = "phoNeuHadIso_1"; tree->SetBranchAddress(s_phoNeuHadIso_1.c_str(), &phoNeuHadIso_1, &b_phoNeuHadIso_1);
  Float_t phoPhoIso_1 = 0.f; TBranch * b_phoPhoIso_1 = 0; const std::string s_phoPhoIso_1 = "phoPhoIso_1"; tree->SetBranchAddress(s_phoPhoIso_1.c_str(), &phoPhoIso_1, &b_phoPhoIso_1);
  Float_t phoTrkIso_1 = 0.f; TBranch * b_phoTrkIso_1 = 0; const std::string s_phoTrkIso_1 = "phoTrkIso_1"; tree->SetBranchAddress(s_phoTrkIso_1.c_str(), &phoTrkIso_1, &b_phoTrkIso_1);

  Bool_t phoisOOT_1 = 0; TBranch * b_phoisOOT_1 = 0; const std::string s_phoisOOT_1 = "phoisOOT_1"; tree->SetBranchAddress(s_phoisOOT_1.c_str(), &phoisOOT_1, &b_phoisOOT_1);
  Bool_t phoisEB_1 = 0; TBranch * b_phoisEB_1 = 0; const std::string s_phoisEB_1 = "phoisEB_1"; tree->SetBranchAddress(s_phoisEB_1.c_str(), &phoisEB_1, &b_phoisEB_1);
  Bool_t phoisTrk_1 = 0; TBranch * b_phoisTrk_1 = 0; const std::string s_phoisTrk_1 = "phoisTrk_1"; tree->SetBranchAddress(s_phoisTrk_1.c_str(), &phoisTrk_1, &b_phoisTrk_1);
  Bool_t phohasPixSeed_1 = 0; TBranch * b_phohasPixSeed_1 = 0; const std::string s_phohasPixSeed_1 = "phohasPixSeed_1"; tree->SetBranchAddress(s_phohasPixSeed_1.c_str(), &phohasPixSeed_1, &b_phohasPixSeed_1);
  Int_t phogedID_1 = 0; TBranch * b_phogedID_1 = 0; const std::string s_phogedID_1 = "phogedID_1"; tree->SetBranchAddress(s_phogedID_1.c_str(), &phogedID_1, &b_phogedID_1);
  Int_t phoootID_1 = 0; TBranch * b_phoootID_1 = 0; const std::string s_phoootID_1 = "phoootID_1"; tree->SetBranchAddress(s_phoootID_1.c_str(), &phoootID_1, &b_phoootID_1);
  Int_t phoseedisGS6_1 = 0; TBranch * b_phoseedisGS6_1 = 0; const std::string s_phoseedisGS6_1 = "phoseedisGS6_1"; tree->SetBranchAddress(s_phoseedisGS6_1.c_str(), &phoseedisGS6_1, &b_phoseedisGS6_1);
  Int_t phoseedisGS1_1 = 0; TBranch * b_phoseedisGS1_1 = 0; const std::string s_phoseedisGS1_1 = "phoseedisGS1_1"; tree->SetBranchAddress(s_phoseedisGS1_1.c_str(), &phoseedisGS1_1, &b_phoseedisGS1_1);

  std::vector<Float_t> * jetpt = 0; TBranch * b_jetpt = 0; const std::string s_jetpt = "jetpt"; tree->SetBranchAddress(s_jetpt.c_str(), &jetpt, &b_jetpt);
  std::vector<Float_t> * jetphi = 0; TBranch * b_jetphi = 0; const std::string s_jetphi = "jetphi"; tree->SetBranchAddress(s_jetphi.c_str(), &jetphi, &b_jetphi);
  std::vector<Float_t> * jeteta = 0; TBranch * b_jeteta = 0; const std::string s_jeteta = "jeteta"; tree->SetBranchAddress(s_jeteta.c_str(), &jeteta, &b_jeteta);

  // get text file
  std::ofstream textfile(textfilename.Data(),std::ios_base::trunc);

  // set outputs
  auto outfile = TFile::Open(outfilename.Data(),"RECREATE");
  outfile->cd();

  auto outtree = new TTree("tree","tree");

  UInt_t o_run = 0U; outtree->Branch(s_run.c_str(), &o_run);
  UInt_t o_lumi = 0U; outtree->Branch(s_lumi.c_str(), &o_lumi);
  ULong64_t o_event = 0UL; outtree->Branch(s_event.c_str(), &o_event);

  Float_t o_t1pfMETpt = 0.f; outtree->Branch(s_t1pfMETpt.c_str(), &o_t1pfMETpt);
  Float_t o_t1pfMETphi = 0.f; outtree->Branch(s_t1pfMETphi.c_str(), &o_t1pfMETphi);
  Float_t o_t1pfMETsumEt = 0.f; outtree->Branch(s_t1pfMETsumEt.c_str(), &o_t1pfMETsumEt);
  
  Float_t o_phopt_0 = 0.f; outtree->Branch(s_phopt_0.c_str(), &o_phopt_0);
  Float_t o_phophi_0 = 0.f; outtree->Branch(s_phophi_0.c_str(), &o_phophi_0);
  Float_t o_phoeta_0 = 0.f; outtree->Branch(s_phoeta_0.c_str(), &o_phoeta_0);
  Float_t o_phoHoE_0 = 0.f; outtree->Branch(s_phoHoE_0.c_str(), &o_phoHoE_0);
  Float_t o_phor9_0 = 0.f; outtree->Branch(s_phor9_0.c_str(), &o_phor9_0);
  Float_t o_phosieie_0 = 0.f; outtree->Branch(s_phosieie_0.c_str(), &o_phosieie_0);
  Float_t o_phosuisseX_0 = 0.f; outtree->Branch(s_phosuisseX_0.c_str(), &o_phosuisseX_0);
  Float_t o_phoseedE_0 = 0.f; outtree->Branch(s_phoseedE_0.c_str(), &o_phoseedE_0);
  Float_t o_phoseedtime_0 = 0.f; outtree->Branch(s_phoseedtime_0.c_str(), &o_phoseedtime_0);
  Float_t o_phoweightedtimeLT120_0 = 0.f; outtree->Branch(s_phoweightedtimeLT120_0.c_str(), &o_phoweightedtimeLT120_0);
  Float_t o_phoChgHadIso_0 = 0.f; outtree->Branch(s_phoChgHadIso_0.c_str(), &o_phoChgHadIso_0);
  Float_t o_phoNeuHadIso_0 = 0.f; outtree->Branch(s_phoNeuHadIso_0.c_str(), &o_phoNeuHadIso_0);
  Float_t o_phoPhoIso_0 = 0.f; outtree->Branch(s_phoPhoIso_0.c_str(), &o_phoPhoIso_0);
  Float_t o_phoTrkIso_0 = 0.f; outtree->Branch(s_phoTrkIso_0.c_str(), &o_phoTrkIso_0);

  Bool_t o_phoisOOT_0 = 0; outtree->Branch(s_phoisOOT_0.c_str(), &o_phoisOOT_0);
  Bool_t o_phoisEB_0 = 0; outtree->Branch(s_phoisEB_0.c_str(), &o_phoisEB_0);
  Bool_t o_phoisTrk_0 = 0; outtree->Branch(s_phoisTrk_0.c_str(), &o_phoisTrk_0);
  Bool_t o_phohasPixSeed_0 = 0; outtree->Branch(s_phohasPixSeed_0.c_str(), &o_phohasPixSeed_0);
  Int_t o_phogedID_0 = 0; outtree->Branch(s_phogedID_0.c_str(), &o_phogedID_0);
  Int_t o_phoootID_0 = 0; outtree->Branch(s_phoootID_0.c_str(), &o_phoootID_0);
  Int_t o_phoseedisGS6_0 = 0; outtree->Branch(s_phoseedisGS6_0.c_str(), &o_phoseedisGS6_0);
  Int_t o_phoseedisGS1_0 = 0; outtree->Branch(s_phoseedisGS1_0.c_str(), &o_phoseedisGS1_0);

  Float_t o_phopt_1 = 0.f; outtree->Branch(s_phopt_1.c_str(), &o_phopt_1);
  Float_t o_phophi_1 = 0.f; outtree->Branch(s_phophi_1.c_str(), &o_phophi_1);
  Float_t o_phoeta_1 = 0.f; outtree->Branch(s_phoeta_1.c_str(), &o_phoeta_1);
  Float_t o_phoHoE_1 = 0.f; outtree->Branch(s_phoHoE_1.c_str(), &o_phoHoE_1);
  Float_t o_phor9_1 = 0.f; outtree->Branch(s_phor9_1.c_str(), &o_phor9_1);
  Float_t o_phosieie_1 = 0.f; outtree->Branch(s_phosieie_1.c_str(), &o_phosieie_1);
  Float_t o_phosuisseX_1 = 0.f; outtree->Branch(s_phosuisseX_1.c_str(), &o_phosuisseX_1);
  Float_t o_phoseedE_1 = 0.f; outtree->Branch(s_phoseedE_1.c_str(), &o_phoseedE_1);
  Float_t o_phoseedtime_1 = 0.f; outtree->Branch(s_phoseedtime_1.c_str(), &o_phoseedtime_1);
  Float_t o_phoweightedtimeLT120_1 = 0.f; outtree->Branch(s_phoweightedtimeLT120_1.c_str(), &o_phoweightedtimeLT120_1);
  Float_t o_phoChgHadIso_1 = 0.f; outtree->Branch(s_phoChgHadIso_1.c_str(), &o_phoChgHadIso_1);
  Float_t o_phoNeuHadIso_1 = 0.f; outtree->Branch(s_phoNeuHadIso_1.c_str(), &o_phoNeuHadIso_1);
  Float_t o_phoPhoIso_1 = 0.f; outtree->Branch(s_phoPhoIso_1.c_str(), &o_phoPhoIso_1);
  Float_t o_phoTrkIso_1 = 0.f; outtree->Branch(s_phoTrkIso_1.c_str(), &o_phoTrkIso_1);

  Bool_t o_phoisOOT_1 = 0; outtree->Branch(s_phoisOOT_1.c_str(), &o_phoisOOT_1);
  Bool_t o_phoisEB_1 = 0; outtree->Branch(s_phoisEB_1.c_str(), &o_phoisEB_1);
  Bool_t o_phoisTrk_1 = 0; outtree->Branch(s_phoisTrk_1.c_str(), &o_phoisTrk_1);
  Bool_t o_phohasPixSeed_1 = 0; outtree->Branch(s_phohasPixSeed_1.c_str(), &o_phohasPixSeed_1);
  Int_t o_phogedID_1 = 0; outtree->Branch(s_phogedID_1.c_str(), &o_phogedID_1);
  Int_t o_phoootID_1 = 0; outtree->Branch(s_phoootID_1.c_str(), &o_phoootID_1);
  Int_t o_phoseedisGS6_1 = 0; outtree->Branch(s_phoseedisGS6_1.c_str(), &o_phoseedisGS6_1);
  Int_t o_phoseedisGS1_1 = 0; outtree->Branch(s_phoseedisGS1_1.c_str(), &o_phoseedisGS1_1);

  Int_t o_nJets30 = 0; outtree->Branch("nJets30", &o_nJets30);
  
  Float_t o_jetpt_0 = 0.f; outtree->Branch("jetpt_0", &o_jetpt_0);
  Float_t o_jetphi_0 = 0.f; outtree->Branch("jetphi_0", &o_jetphi_0);
  Float_t o_jeteta_0 = 0.f; outtree->Branch("jeteta_0", &o_jeteta_0);
  Float_t o_jetpt_1 = 0.f; outtree->Branch("jetpt_1", &o_jetpt_1);
  Float_t o_jetphi_1 = 0.f; outtree->Branch("jetphi_1", &o_jetphi_1);
  Float_t o_jeteta_1 = 0.f; outtree->Branch("jeteta_1", &o_jeteta_1);
  Float_t o_jetpt_2 = 0.f; outtree->Branch("jetpt_2", &o_jetpt_2);
  Float_t o_jetphi_2 = 0.f; outtree->Branch("jetphi_2", &o_jetphi_2);
  Float_t o_jeteta_2 = 0.f; outtree->Branch("jeteta_2", &o_jeteta_2);

  Float_t o_dphi_jet0pho0 = 0.f; outtree->Branch("dphi_jet0pho0", &o_dphi_jet0pho0);
  Float_t o_jet1pt_over_pho0pt = 0.f; outtree->Branch("jet1pt_over_pho0pt", &o_jet1pt_over_pho0pt);

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

    // fill text file
    textfile << run << ":" << lumi << ":" << event << std::endl;

    // set output tree
    o_run = run;
    o_lumi = lumi;
    o_event = event;

    // met info
    b_t1pfMETphi->GetEntry(ientry);
    b_t1pfMETsumEt->GetEntry(ientry);

    o_t1pfMETpt = t1pfMETpt;
    o_t1pfMETphi = t1pfMETphi;
    o_t1pfMETsumEt = t1pfMETsumEt;

    // photon info
    b_phopt_0->GetEntry(ientry);
    b_phophi_0->GetEntry(ientry);
    b_phoeta_0->GetEntry(ientry);
    b_phoHoE_0->GetEntry(ientry);
    b_phor9_0->GetEntry(ientry);
    b_phosieie_0->GetEntry(ientry);
    b_phosuisseX_0->GetEntry(ientry);
    b_phoseedE_0->GetEntry(ientry);
    b_phoweightedtimeLT120_0->GetEntry(ientry);

    o_phopt_0 = phopt_0;
    o_phophi_0 = phophi_0;
    o_phoeta_0 = phoeta_0;
    o_phoHoE_0 = phoHoE_0;
    o_phor9_0 = phor9_0;
    o_phosieie_0 = phosieie_0;
    o_phosuisseX_0 = phosuisseX_0;
    o_phoseedE_0 = phoseedE_0;
    o_phoseedtime_0 = phoseedtime_0;
    o_phoweightedtimeLT120_0 = phoweightedtimeLT120_0;

    b_phoisOOT_0->GetEntry(ientry);
    b_phoisEB_0->GetEntry(ientry);
    b_phoisTrk_0->GetEntry(ientry);
    b_phohasPixSeed_0->GetEntry(ientry);
    b_phogedID_0->GetEntry(ientry);
    b_phoootID_0->GetEntry(ientry);
    b_phoseedisGS6_0->GetEntry(ientry);
    b_phoseedisGS1_0->GetEntry(ientry);

    o_phoisOOT_0 = phoisOOT_0;
    o_phoisEB_0 = phoisEB_0;
    o_phoisTrk_0 = phoisTrk_0;
    o_phohasPixSeed_0 = phohasPixSeed_0;
    o_phogedID_0 = phogedID_0;
    o_phoootID_0 = phoootID_0;
    o_phoseedisGS6_0 = phoseedisGS6_0;
    o_phoseedisGS1_0 = phoseedisGS1_0;

    b_phopt_1->GetEntry(ientry);
    b_phophi_1->GetEntry(ientry);
    b_phoeta_1->GetEntry(ientry);
    b_phoHoE_1->GetEntry(ientry);
    b_phor9_1->GetEntry(ientry);
    b_phosieie_1->GetEntry(ientry);
    b_phosuisseX_1->GetEntry(ientry);
    b_phoseedE_1->GetEntry(ientry);
    b_phoseedtime_1->GetEntry(ientry);
    b_phoweightedtimeLT120_1->GetEntry(ientry);

    o_phopt_1 = phopt_1;
    o_phophi_1 = phophi_1;
    o_phoeta_1 = phoeta_1;
    o_phoHoE_1 = phoHoE_1;
    o_phor9_1 = phor9_1;
    o_phosieie_1 = phosieie_1;
    o_phosuisseX_1 = phosuisseX_1;
    o_phoseedE_1 = phoseedE_1;
    o_phoseedtime_1 = phoseedtime_1;
    o_phoweightedtimeLT120_1 = phoweightedtimeLT120_1;

    b_phoisOOT_1->GetEntry(ientry);
    b_phoisEB_1->GetEntry(ientry);
    b_phoisTrk_1->GetEntry(ientry);
    b_phohasPixSeed_1->GetEntry(ientry);
    b_phogedID_1->GetEntry(ientry);
    b_phoootID_1->GetEntry(ientry);
    b_phoseedisGS6_1->GetEntry(ientry);
    b_phoseedisGS1_1->GetEntry(ientry);

    o_phoisOOT_1 = phoisOOT_1;
    o_phoisEB_1 = phoisEB_1;
    o_phoisTrk_1 = phoisTrk_1;
    o_phohasPixSeed_1 = phohasPixSeed_1;
    o_phogedID_1 = phogedID_1;
    o_phoootID_1 = phoootID_1;
    o_phoseedisGS6_1 = phoseedisGS6_1;
    o_phoseedisGS1_1 = phoseedisGS1_1;

    // derived photon info
    b_rho->GetEntry(ientry);

    b_phosceta_0->GetEntry(ientry);
    b_phoChgHadIso_0->GetEntry(ientry);
    b_phoNeuHadIso_0->GetEntry(ientry);
    b_phoPhoIso_0->GetEntry(ientry);
    b_phoTrkIso_0->GetEntry(ientry);

    o_phoChgHadIso_0 = phoChgHadIso_0 - (((std::abs(phosceta_0)<1.0)*(rho*0.0385)) + ((std::abs(phosceta_0)>=1.0&&(std::abs(phosceta_0)<1.479))*(rho*0.0468)));
    o_phoNeuHadIso_0 = phoNeuHadIso_0 - (((std::abs(phosceta_0)<1.0)*(rho*0.0636)) + ((std::abs(phosceta_0)>=1.0&&(std::abs(phosceta_0)<1.479))*(rho*0.1103))) - (phopt_0*0.0126)-(phopt_0*phopt_0*0.000026);
    o_phoPhoIso_0    = phoPhoIso_0    - (((std::abs(phosceta_0)<1.0)*(rho*0.1240)) + ((std::abs(phosceta_0)>=1.0&&(std::abs(phosceta_0)<1.479))*(rho*0.1093))) - (phopt_0*0.0035);
    o_phoTrkIso_0    = phoTrkIso_0    - (rho*0.113) - (phopt_0*0.0056);

    b_phosceta_1->GetEntry(ientry);
    b_phoChgHadIso_1->GetEntry(ientry);
    b_phoNeuHadIso_1->GetEntry(ientry);
    b_phoPhoIso_1->GetEntry(ientry);
    b_phoTrkIso_1->GetEntry(ientry);

    o_phoChgHadIso_1 = phoChgHadIso_1 - (((std::abs(phosceta_1)<1.0)*(rho*0.0385)) + ((std::abs(phosceta_1)>=1.0&&(std::abs(phosceta_1)<1.479))*(rho*0.0468)));
    o_phoNeuHadIso_1 = phoNeuHadIso_1 - (((std::abs(phosceta_1)<1.0)*(rho*0.0636)) + ((std::abs(phosceta_1)>=1.0&&(std::abs(phosceta_1)<1.479))*(rho*0.1103))) - (phopt_1*0.0126)-(phopt_1*phopt_1*0.000026);
    o_phoPhoIso_1    = phoPhoIso_1    - (((std::abs(phosceta_1)<1.0)*(rho*0.1240)) + ((std::abs(phosceta_1)>=1.0&&(std::abs(phosceta_1)<1.479))*(rho*0.1093))) - (phopt_1*0.0035);
    o_phoTrkIso_1    = phoTrkIso_1    - (rho*0.113) - (phopt_1*0.0056);

    // jet info
    b_jetpt->GetEntry(ientry);
    b_jetphi->GetEntry(ientry);
    b_jeteta->GetEntry(ientry);

    const int nj = jetpt->size();

    o_nJets30 = 0;
    
    for (int ij = 0; ij < nj; ij++)
    {
      if ((*jetpt)[ij] > 30) o_nJets30++;
    }

    if (o_nJets30 > 0)
    {
      o_jetpt_0 = (*jetpt)[0];
      o_jetphi_0 = (*jetphi)[0];
      o_jeteta_0 = (*jeteta)[0];

      o_dphi_jet0pho0 = std::abs(TVector2::Phi_mpi_pi((*jetphi)[0]-phophi_0));
    }

    if (o_nJets30 > 1)
    {
      o_jetpt_1 = (*jetpt)[1];
      o_jetphi_1 = (*jetphi)[1];
      o_jeteta_1 = (*jeteta)[1];

      o_jet1pt_over_pho0pt = (*jetpt)[1] / phopt_0;
    }

    if (o_nJets30 > 2)
    {
      o_jetpt_2 = (*jetpt)[2];
      o_jetphi_2 = (*jetphi)[2];
      o_jeteta_2 = (*jeteta)[2];
    }

    // fill outtree
    outtree->Fill();
  }

  // write outtree
  outfile->cd();
  outtree->Write(outtree->GetName(),TObject::kWriteDelete);
}
