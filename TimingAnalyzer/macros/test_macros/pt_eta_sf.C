void pt_eta_sf()
{
  auto bkgd_file = TFile::Open("skims/v4/ootVID/dyll.root","UPDATE");
  auto bkgd_tree = (TTree*)bkgd_file->Get("DYLL_Tree");

  auto sign_file = TFile::Open("skims/v4/ootVID/gmsb.root");
  auto sign_tree = (TTree*)sign_file->Get("GMSB_L200_CTau200_Tree");

  //  std::vector<Double_t> ptbins = {0,20,40,60,80,100,125,150,200,250,300,400,500,750,1000,2000};

  auto bkgd_hist = new TH2F("bkgd_hist","",100,0,2000,30,-1.5,1.5);
  auto sign_hist = new TH2F("sign_hist","",100,0,2000,30,-1.5,1.5);

  //  bkgd_tree->Draw("phosceta_0:phopt_0>>bkgd_hist","phosf_0","goff");
  bkgd_tree->Draw("phosceta_0:phopt_0>>bkgd_hist","","goff");
  sign_tree->Draw("phosceta_0:phopt_0>>sign_hist","","goff");
  
  bkgd_hist->Scale(1.f/bkgd_hist->Integral());
  sign_hist->Scale(1.f/sign_hist->Integral());

  auto ratio = (TH2F*)sign_hist->Clone("ratio");
  ratio->Divide(bkgd_hist);
  
  // auto canv = new TCanvas();
  // canv->cd();
  // canv->SetLogz();
  // ratio->Draw("colz");

  Float_t pt, eta, sf;
  TBranch * b_pt, * b_eta, * b_sf;
  bkgd_tree->SetBranchAddress("phopt_0",&pt,&b_pt);
  bkgd_tree->SetBranchAddress("phosceta_0",&eta,&b_eta);
  b_sf = bkgd_tree->Branch("phosf_0",&sf,"phosf_0/F");

  const auto nentries = bkgd_tree->GetEntries();
  for (auto ientry = 0; ientry < nentries; ientry++)
  {
    if (ientry % 10000 == 0) std::cout << "working on entry: " << ientry << std::endl;
    b_pt->GetEntry(ientry);
    b_eta->GetEntry(ientry);

    sf = ratio->GetBinContent(ratio->FindBin(pt,eta));
    b_sf->Fill();
  }
  
  bkgd_file->cd();
  bkgd_tree->Write(bkgd_tree->GetName(),TObject::kWriteDelete);
}
