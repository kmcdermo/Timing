void eff()
{
  auto file = TFile::Open("/eos/cms/store/user/kmcdermo/nTuples/skims/2017/rereco_v4/Data/SinglePhoton/F/v1/tree.root");
  auto tree = (TTree*)file->Get("disphotree");

  Bool_t hltPho200; TBranch * b_hltPho200;
  tree->SetBranchAddress("hltPho200",&hltPho200,&b_hltPho200);

  Float_t phopt_0; TBranch * b_phopt_0;
  tree->SetBranchAddress("phopt_0",&phopt_0,&b_phopt_0);

  auto effplot = new TEfficiency("effplot","HLT Efficiency vs photon p_{T};Leading photon p_{T};Monophoton Efficiency;",100,0,2000);

  const auto nentries = tree->GetEntries();
  for (auto ientry = 0U; ientry < nentries; ientry++)
  {
    b_hltPho200->GetEntry(ientry);
    b_phopt_0->GetEntry(ientry);

    effplot->Fill(hltPho200,phopt_0);
  }

  auto canv = new TCanvas();
  canv->cd();

  effplot->Draw("AP");
}
