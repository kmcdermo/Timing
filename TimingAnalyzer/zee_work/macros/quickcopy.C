void quickcopy()
{
  TFile * file = TFile::Open("notof_vtxzcut/DATA/doubleeg/plots.root");
  TFile * out = new TFile("notof_vtxzcut.root","RECREATE");

  file->cd();
  TH1F * td_dseedeta_EBEB = (TH1F*)file->Get("td_dseedeta_EBEB_sigma_gaus2fm");
  TH1F * td_dseedeta_EEEE = (TH1F*)file->Get("td_dseedeta_EEEE_sigma_gaus2fm");
  TH1F * td_dseedeta_EPEP = (TH1F*)file->Get("td_dseedeta_EPEP_sigma_gaus2fm");
  TH1F * td_dseedeta_EMEM = (TH1F*)file->Get("td_dseedeta_EMEM_sigma_gaus2fm");
  TH1F * td_dseedeta_EBEE = (TH1F*)file->Get("td_dseedeta_EBEE_sigma_gaus2fm");
  TH1F * td_dseedeta_EBEE_slice0 = (TH1F*)file->Get("td_dseedeta_EBEE_slice0_sigma_gaus2fm");
  TH1F * td_dseedeta_EBEE_slice1 = (TH1F*)file->Get("td_dseedeta_EBEE_slice1_sigma_gaus2fm");
  TH1F * td_dseedeta_EBEE_slice2 = (TH1F*)file->Get("td_dseedeta_EBEE_slice2_sigma_gaus2fm");
  TH1F * td_dseedeta_EBEE_slice3 = (TH1F*)file->Get("td_dseedeta_EBEE_slice3_sigma_gaus2fm");
  
  out->cd();
  td_dseedeta_EBEB->Write();
  td_dseedeta_EEEE->Write();
  td_dseedeta_EPEP->Write();
  td_dseedeta_EMEM->Write();
  td_dseedeta_EBEE->Write();

  td_dseedeta_EBEE_slice0->Write();
  td_dseedeta_EBEE_slice1->Write();
  td_dseedeta_EBEE_slice2->Write();
  td_dseedeta_EBEE_slice3->Write();
}
