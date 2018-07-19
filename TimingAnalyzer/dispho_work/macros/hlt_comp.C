void doPlot(TTree *& tree, const TString & sample, const Bool_t isHT);

void hlt_comp()
{
  gStyle->SetOptStat(0);

  auto file = TFile::Open("skims/sr_noht_nohlt.root");
  file->cd();

  std::vector<TString> samples = {"Data","GJets","QCD"};

  for (const auto & sample : samples)
  {
    auto tree = (TTree*)file->Get(Form("%s_Tree",sample.Data()));

    doPlot(tree,sample,false);
    doPlot(tree,sample,true);

    delete tree;
  }

  delete file;
}

void doPlot(TTree *& tree, const TString & sample, const Bool_t isHT)
{
  TString cut = (isHT ? "(Sum$(jetpt*(jetpt>30))>400)" : "(1)");
  const TString label = (isHT ? "HT400" : "NoHT");

  if (sample.EqualTo("GJets") || sample.EqualTo("QCD")) cut += " * (evtwgt * puwgt)";
  
  auto h_hltSignal = new TH1F("h_hltSignal","Trigger Bool;Trigger Bool;Events",2,0,2);
  h_hltSignal->SetLineColor(kRed);
  h_hltSignal->SetMarkerColor(kRed);
  h_hltSignal->Sumw2();

  auto h_hltPho200 = new TH1F("h_hltPho200","Trigger Bool;Trigger Bool;Events",2,0,2);
  h_hltPho200->SetLineColor(kBlue);
  h_hltPho200->SetMarkerColor(kBlue);
  h_hltPho200->Sumw2();  

  tree->Draw("hltSignal>>h_hltSignal",cut.Data(),"goff");
  tree->Draw("hltPho200>>h_hltPho200",cut.Data(),"goff");
  
  const auto max = (h_hltSignal->GetMaximum() > h_hltPho200->GetMaximum() ? h_hltSignal->GetMaximum() : h_hltPho200->GetMaximum());
  const auto min = (h_hltSignal->GetMinimum() < h_hltPho200->GetMinimum() ? h_hltSignal->GetMinimum() : h_hltPho200->GetMinimum());

  h_hltSignal->SetMinimum(min / 1.5);
  h_hltSignal->SetMaximum(max * 1.5);

  auto leg = new TLegend(0.12,0.91,0.25,0.99);
  leg->AddEntry(h_hltSignal,"hltSignal","epl");
  leg->AddEntry(h_hltPho200,"hltPho200","epl");

  auto canv = new TCanvas();
  canv->cd();
  canv->SetTicky();
  canv->SetLogy();

  h_hltSignal->Draw("ep");
  h_hltPho200->Draw("ep same");
  leg->Draw("same");

  canv->SaveAs(Form("%s_%s_SR_trigger_comp.png",label.Data(),sample.Data()));

  delete canv;
  delete leg;
  delete h_hltPho200;
  delete h_hltSignal;
}
