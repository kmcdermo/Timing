#include "Common.cpp+"

void setToZero(TH2F * hist)
{
  for (auto ix = 1; ix <= hist->GetXaxis()->GetNbins(); ix++)
  {
    for (auto iy = 1; iy <= hist->GetYaxis()->GetNbins(); iy++)
    {
      auto content = hist->GetBinContent(ix,iy);

      if (content < 0.0)
      {
	hist->SetBinContent(ix,iy,0.0);
	hist->SetBinError  (ix,iy,0.0);
      }
    }
  }
}

void pt_eta_sf()
{
  auto tdrStyle = new TStyle("TDRStyle","Style for P-TDR");
  Common::SetTDRStyle(tdrStyle);

  auto bkgd_file = TFile::Open("skims/v4/ootVID/dyll.root","UPDATE");
  auto bkgd_tree = (TTree*)bkgd_file->Get("DYLL_Tree");

  auto sign_file = TFile::Open("skims/v4/ootVID/gmsb.root");
  auto sign_tree = (TTree*)sign_file->Get("GMSB_L200_CTau200_Tree");

  //  std::vector<Double_t> ptbins = {0,20,40,60,80,100,125,150,200,250,300,400,500,750,1000,2000};

  // auto outfile = TFile::Open("pt_eta.root","RECREATE");
  // outfile->cd();

  // auto bkgd_hist = new TH2F("bkgd_hist","#eta vs. p_{T};p_{T} [GeV];#eta",100,0,2000,30,-1.5,1.5);
  // auto sign_hist = new TH2F("sign_hist","#eta vs. p_{T};p_{T} [GeV];#eta",100,0,2000,30,-1.5,1.5);

  // //  bkgd_tree->Draw("phosceta_0:phopt_0>>bkgd_hist","phosf_0","goff");
  // bkgd_tree->Draw("phosceta_0:phopt_0>>bkgd_hist","(phopt_0>70&&phoisEB_0==1&&phoisOOT_0==0&&phohasPixSeed_0==1)*(puwgt*evtwgt)","goff");
  // sign_tree->Draw("phosceta_0:phopt_0>>sign_hist","(phopt_0>70&&phoisEB_0==1&&phoisOOT_0==0&&phoisGen_0==1)*(puwgt*evtwgt)","goff");

  auto outfile = TFile::Open("pt_smaj.root","RECREATE");
  outfile->cd();

  auto bkgd_hist = new TH2F("bkgd_hist","S_{Minor} vs. p_{T};p_{T} [GeV];S_{Minor}",100,0,2000,50,0,2);
  auto sign_hist = new TH2F("sign_hist","S_{Minor} vs. p_{T};p_{T} [GeV];S_{Minor}",100,0,2000,50,0,2);

  //  bkgd_tree->Draw("phosceta_0:phopt_0>>bkgd_hist","phosf_0","goff");
  bkgd_tree->Draw("phosmin_0:phopt_0>>bkgd_hist","(phopt_0>70&&phoisEB_0==1&&phoisOOT_0==0&&phohasPixSeed_0==1)*(puwgt*evtwgt)","goff");
  sign_tree->Draw("phosmin_0:phopt_0>>sign_hist","(phopt_0>70&&phoisEB_0==1&&phoisOOT_0==0&&phoisGen_0==1)*(puwgt*evtwgt)","goff");

  // common stuff
  setToZero(bkgd_hist);
  setToZero(sign_hist);

  bkgd_hist->Scale(1.f/bkgd_hist->Integral());
  sign_hist->Scale(1.f/sign_hist->Integral());

  outfile->cd();
  auto ratio = (TH2F*)sign_hist->Clone("ratio");
  ratio->SetTitle(Form("Sig/Bkgd %s",ratio->GetTitle()));
  ratio->Divide(bkgd_hist);
  
  auto canv = new TCanvas();
  canv->cd();
  canv->SetLogz();
  
  bkgd_hist->SetMinimum(1e-6);
  bkgd_hist->SetMaximum(1);
  bkgd_hist->Draw("colz");
  Common::SaveAs(canv,"bkgd_hist");

  sign_hist->SetMinimum(1e-6);
  sign_hist->SetMaximum(1);
  sign_hist->Draw("colz");
  Common::SaveAs(canv,"sign_hist");
  
  ratio->SetMinimum(1e-3);
  ratio->SetMaximum(1e5);
  ratio->Draw("colz");
  Common::SaveAs(canv,"ratio");
  
  Common::Write(outfile,bkgd_hist);
  Common::Write(outfile,sign_hist);
  Common::Write(outfile,ratio);

  // Float_t pt, eta, sf;
  // TBranch * b_pt, * b_eta, * b_sf;
  // bkgd_tree->SetBranchAddress("phopt_0",&pt,&b_pt);
  // bkgd_tree->SetBranchAddress("phosceta_0",&eta,&b_eta);
  // b_sf = bkgd_tree->Branch("phosf_0",&sf,"phosf_0/F");

  // const auto nentries = bkgd_tree->GetEntries();
  // for (auto ientry = 0; ientry < nentries; ientry++)
  // {
  //   if (ientry % 10000 == 0) std::cout << "working on entry: " << ientry << std::endl;
  //   b_pt->GetEntry(ientry);
  //   b_eta->GetEntry(ientry);

  //   sf = ratio->GetBinContent(ratio->FindBin(pt,eta));
  //   b_sf->Fill();
  // }
  
  // bkgd_file->cd();
  // bkgd_tree->Write(bkgd_tree->GetName(),TObject::kWriteDelete);
}
