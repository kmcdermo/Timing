void overplot_cuts()
{
  gStyle->SetOptStat(0);

  const std::vector<Color_t> colors = {kBlack,kBlue,kRed+1,kGreen+1,kMagenta,kOrange+1,kYellow-7,kViolet-1,kAzure+10,kYellow+3,kPink-9}; 

  auto file = TFile::Open("/eos/cms/store/user/kmcdermo/nTuples/skims/2017/rereco_v4/special/qcd_std.root");
  std::vector<TString> labels, cuts;
  
  ifstream cutfile("cut_config/qcd/one_at_a_time/failL_discr9.txt",std::ios::in);
  TString s_label, s_cut;
  while (cutfile >> s_label >> s_cut)
  {
    labels.emplace_back(s_label);
    cuts.emplace_back(s_cut);
  }

  auto tree = (TTree*)file->Get("Data_Tree");
  std::vector<TH1F*> hists(cuts.size());

  auto canv = new TCanvas();
  canv->cd(); 
  auto leg = new TLegend(0.2,0.7,0.4,0.9);   
  leg->SetNColumns(2);

  auto outfile = TFile::Open("outfile.root","RECREATE");

  TString cut = "";

  Float_t min = 1e9, max = -1e9;
  for (int i = 0; i < cuts.size(); i++)
  {
    if (i>0) cut += Form("&&%s",cuts[i].Data());
    else     cut  = Form("%s",cuts[i].Data());

    std::cout << "label: " << labels[i].Data() << " cut: "  << cut.Data() << std::endl;

    outfile->cd();
    hists[i] = new TH1F(Form("h_%s",labels[i].Data()),"",32,-3.2,3.2);
    hists[i]->Sumw2();
    hists[i]->GetXaxis()->SetTitle("photon 0 phi");
    hists[i]->SetLineColor(colors[i]);
    hists[i]->SetMarkerColor(colors[i]);

    tree->Draw(Form("phophi_0>>%s",hists[i]->GetName()),Form("%s",cut.Data()),"goff");
    leg->AddEntry(hists[i],labels[i].Data(),"epl");

    hists[i]->Write();

    hists[i]->Scale(1.f/hists[i]->Integral());

    hists[i]->Write(Form("%s_scaled",hists[i]->GetName()));

    const auto tmpmin = hists[i]->GetMinimum();
    const auto tmpmax = hists[i]->GetMaximum(); 

    if (tmpmin < min) min = tmpmin;  
    if (tmpmax > max) max = tmpmax; 
  }

  for (int i = 0; i < cuts.size(); i++)
  {
    hists[i]->SetMinimum(min/1.1);
    hists[i]->SetMaximum(max*1.1);
    hists[i]->Draw(i>0?"same ep":"ep");
  }

  leg->Draw("same");
  leg->Write("Legend");
  canv->Write("canv");
  
  const std::vector<TString> extensions = {"png","pdf","eps"};
  for (const auto & extension : extensions)
  {
    canv->SaveAs(Form("qcd_std_to_r9.%s",extension.Data()));
  }
}
