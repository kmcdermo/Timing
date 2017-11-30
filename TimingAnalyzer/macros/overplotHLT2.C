struct TestInfo
{
  TString dir;
  TString test;
};
typedef std::vector<TestInfo> TestInfoVec;
typedef std::vector<TEfficiency*> TEffVec;
typedef std::vector<TString> TStrVec;

void Overplot(const TEffVec& teffs, const TStrVec& eras, const TString outname, const Bool_t isLogx,
	      const Float_t xlow, const Float_t xhigh, const Float_t ylow, const Float_t yhigh)
{
  std::vector<Color_t> colors = {kBlue,kRed+1,kGreen+1,kMagenta,kOrange+1,kYellow-7,kViolet-1,kAzure+10,kYellow+3,kBlack};

  TCanvas * canv = new TCanvas();
  canv->cd();
  canv->SetLogx(isLogx);
  canv->SetGrid(1,1);

  TLegend * leg = new TLegend(0.9,0.8,0.99,0.99);
  for (Int_t i = 0; i < eras.size(); i++)
  {
    //teffs[i]->SetLineColor(colors[i]);
    //teffs[i]->SetMarkerColor(colors[i]);
    teffs[i]->SetLineColor(colors[i+1]);
    teffs[i]->SetMarkerColor(colors[i+1]);
    teffs[i]->Draw(i>0?"P same":"AP");
    leg->AddEntry(teffs[i],Form("%s",eras[i].Data()),"epl");
  }

  gPad->Update(); 
  auto graph = teffs[0]->GetPaintedGraph(); 
  graph->GetXaxis()->SetRangeUser(xlow,xhigh);
  graph->GetYaxis()->SetRangeUser(ylow,yhigh);
  gPad->Update();

  leg->Draw("same");
  canv->SaveAs(Form("%s.png",outname.Data()));

  delete leg;
  delete canv;
}

void overplotHLT2()
{
//   TestInfoVec effs = { {"SM","L1"}, {"SM", "L1toHLT"}, {"SM","ET"}, {"SM","PhoID"}, {"SM","DispID"}, {"SM","HT"} };
//   TStrVec eras = {"2017B","2017C","2017D","2017E","2017F"};
  TestInfoVec effs = { {"SP_PhoID","DispID"}, {"SP_DispID","HT"} };
  TStrVec eras = { "2017C", "2017D","2017E","2017F"};
  const Int_t N = eras.size();

  for (auto & eff : effs)
  {
    std::vector<TFile*> files(N); 
    TEffVec effETEBs(N); 
    TEffVec effETEEs(N); 
    TEffVec effetas(N); 
    TEffVec effphis(N); 
    TEffVec efftimes(N); 
    TEffVec effHTs(N);
    
    for (Int_t i = 0; i < N; i++)
    {
      files   [i] = TFile::Open(Form("input/DATA/2017/HLT_CHECK/21_11_17/%s/%s/hltplots-%s.root",eff.dir.Data(),eras[i].Data(),eras[i].Data()));
      effETEBs[i] = (TEfficiency*)files[i]->Get(Form("tree/effETEB_%s",eff.test.Data()));
      effETEEs[i] = (TEfficiency*)files[i]->Get(Form("tree/effETEE_%s",eff.test.Data()));
      effetas [i] = (TEfficiency*)files[i]->Get(Form("tree/effeta_%s",eff.test.Data()));
      effphis [i] = (TEfficiency*)files[i]->Get(Form("tree/effphi_%s",eff.test.Data()));
      efftimes[i] = (TEfficiency*)files[i]->Get(Form("tree/efftime_%s",eff.test.Data()));
      effHTs  [i] = (TEfficiency*)files[i]->Get(Form("tree/effHT_%s",eff.test.Data()));
    }
    
    Overplot(effETEBs,eras,Form("%s_ETEB_%s",eff.dir.Data(),eff.test.Data()),true,1,2000,0.0,1.05);
    Overplot(effETEEs,eras,Form("%s_ETEE_%s",eff.dir.Data(),eff.test.Data()),true,1,2000,0.0,1.05);
    Overplot(effetas,eras,Form("%s_eta_%s",eff.dir.Data(),eff.test.Data()),false,-3.0,3,0.0,1.05);
    Overplot(effphis,eras,Form("%s_phi_%s",eff.dir.Data(),eff.test.Data()),false,-3.5,3.5,0.0,1.05);
    Overplot(efftimes,eras,Form("%s_time_%s",eff.dir.Data(),eff.test.Data()),false,-30,30,0.8,1.005);
    Overplot(effHTs,eras,Form("%s_HT_%s",eff.dir.Data(),eff.test.Data()),true,100,3000,0.0,1.05);

    for (Int_t i = 0; i < N; i++)
    {
      delete files   [i];
      delete effETEBs[i];
      delete effETEEs[i];
      delete effetas [i];
      delete effphis [i];
      delete efftimes[i];
      delete effHTs  [i];
    }
  }
}
