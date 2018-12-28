void qcdNVTX()
{
  const std::vector<TString> htbins = {"100to200","200to300","300to500","500to700","700to1000","1000to1500","1500to2000","2000toInf"};
  
  TH1F * hist_nvtx_wopu = new TH1F();
  TH1F * hist_nvtx_wpu  = new TH1F();
  TH1F * h_cutflow      = new TH1F();

  for (auto iht = 0U; iht < htbins.size(); iht++)
  {
    auto file = TFile::Open(Form("/eos/cms/store/user/kmcdermo/nTuples/skims/2017/rereco_v4_tmp_nvtx/MC/QCD_HT/%s/tree.root",htbins[iht].Data()));
    auto tmp_wopu = (TH1F*)file->Get("nvtx_wopu");
    auto tmp_wpu  = (TH1F*)file->Get("nvtx_wpu");
    auto tmp_cutf = (TH1F*)file->Get("h_cutflow");

    if (iht == 0) 
    {
      hist_nvtx_wopu = (TH1F*)tmp_wopu->Clone("sum_nvtx_wopu");
      hist_nvtx_wpu  = (TH1F*)tmp_wpu ->Clone("sum_nvtx_wpu");
      h_cutflow      = (TH1F*)tmp_cutf->Clone("sum_cutflow");
    }
    else 
    {
      hist_nvtx_wopu->Add(tmp_wopu);
      hist_nvtx_wpu ->Add(tmp_wpu);
      h_cutflow     ->Add(tmp_cutf);
    }
  }

  std::cout << "W/O PU Integral: " << hist_nvtx_wopu->Integral() << std::endl;
  std::cout << "W/  PU Integral: " << hist_nvtx_wpu ->Integral() << std::endl;
  std::cout << "===========================" << std::endl;
  
  for (auto ibin = 1; ibin < h_cutflow->GetXaxis()->GetNbins(); ibin++)
  {
    std::cout << h_cutflow->GetXaxis()->GetBinLabel(ibin) << " : " << h_cutflow->GetBinContent(ibin) << std::endl;
  }

}
