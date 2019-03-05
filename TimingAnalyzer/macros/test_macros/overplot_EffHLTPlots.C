#include "Common.cpp+"

struct TestInfo
{
  TString dir;
  TString test;
};

struct GroupInfo
{
  GroupInfo() {}
  GroupInfo(const std::vecotr<TestInfo> & effs, const std::vector<TString> & eras)
    : effs(effs), eras(eras) {}

  std::vector<TestInfo> effs;
  std::vector<TString>  eras;
};

void Overplot(const std::vector<TEfficiency*> & teffs, const std::vector<TString> & eras, const TString outname,
	      const Bool_t isLogx, const Float_t xlow, const Float_t xhigh, const Float_t ylow, const Float_t yhigh);

void overplot_EffHLTPlots()
{
  std::vector<GroupInfo> groups =
  {
    { { {"SM","L1"}, {"SM", "L1toHLT"}, {"SM","ET"}, {"SM","PhoID"}, {"SM","DispID"}, {"SM","HT"} }, {"2017B","2017C","2017D","2017E","2017F"} },
    { { {"SP_PhoID","DispID"}, {"SP_DispID","HT"} }, { "2017C", "2017D","2017E","2017F"} }
  };

  for (const auto & group : groups)
  {
    const auto & effs = group.effs;
    const auto & eras = group.eras;

    const auto N = eras.size();

    for (auto & eff : effs)
    {
      std::vector<TFile*> files(N); 
      std::vector<TEffiency*> effETEBs(N); 
      std::vector<TEffiency*> effETEEs(N); 
      std::vector<TEffiency*> effetas(N); 
      std::vector<TEffiency*> effphis(N); 
      std::vector<TEffiency*> efftimes(N); 
      std::vector<TEffiency*> effHTs(N);
    
      for (auto i = 0U; i < N; i++)
      {
	files   [i] = TFile::Open(Form("input/%s/%s/hltplots-%s.root",eff.dir.Data(),eras[i].Data(),eras[i].Data()));
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
      
      for (auto i = 0U; i < N; i++)
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
}

void Overplot(const std::vector<TEfficiency*> & teffs, const std::vector<TString> & eras, const TString outname,
	      const Bool_t isLogx, const Float_t xlow, const Float_t xhigh, const Float_t ylow, const Float_t yhigh)
{
  auto canv = new TCanvas();
  canv->cd();
  canv->SetLogx(isLogx);
  canv->SetGrid(1,1);

  auto leg = new TLegend(0.9,0.8,0.99,0.99);
  for (auto i = 0U; i < eras.size(); i++)
  {
    const auto color = Common::ColorVec[i];

    teffs[i]->SetLineColor(color);
    teffs[i]->SetMarkerColor(color);
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
