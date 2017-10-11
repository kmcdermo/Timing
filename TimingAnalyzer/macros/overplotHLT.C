typedef std::vector<TEfficiency*> TEffVec;
typedef std::vector<TString> TStrVec;

void Overplot(const TEffVec& teffs, const TStrVec& eras, const TString cname, const Bool_t isLogx,
	      const Float_t xlow, const Float_t xhigh, const Float_t ylow, const Float_t yhigh)
{
  std::vector<Color_t> colors = {kBlue,kRed+1,kGreen+1,kMagenta,kOrange+1,kYellow-7,kViolet-1,kAzure+10,kYellow+3,kBlack};

  TCanvas * canv = new TCanvas();
  canv->cd();
  canv->SetLogx(isLogx);
  canv->SetGrid(1,1);

  TLegend * leg = new TLegend(0.9,0.9,0.99,0.99);
  for (Int_t i = 0; i < eras.size(); i++)
  {
    teffs[i]->SetLineColor(colors[i]);
    teffs[i]->SetMarkerColor(colors[i]);
    teffs[i]->Draw(i>0?"P same":"AP");
    leg->AddEntry(teffs[i],Form("%s",eras[i].Data()),"epl");
  }

  gPad->Update(); 
  auto graph = teffs[0]->GetPaintedGraph(); 
  graph->GetXaxis()->SetRangeUser(xlow,xhigh);
  graph->GetYaxis()->SetRangeUser(ylow,yhigh);
  gPad->Update();

  leg->Draw("same");
  canv->SaveAs(Form("%s.png",cname.Data()));

  delete leg;
  delete canv;
}

void overplotHLT()
{
  TStrVec effs  = {"EtEff","DispEff","HTEff"};
  TStrVec eras  = {"2017B","2017C","2017DE"};
  const Int_t N = eras.size();

  for (auto & eff : effs)
  {
    std::vector<TFile*> files(N); 
    TEffVec effptEBs(N); 
    TEffVec effptEEs(N); 
    TEffVec effetas(N); 
    TEffVec effphis(N); 
    TEffVec efftimes(N); 
    TEffVec effHTs(N); 
    
    for (Int_t i = 0; i < N; i++)
    {
      files   [i] = TFile::Open(Form("IsTrackCheck_SP_%s/cuts_jetIdL_jetER_%s/plots.root",eras[i].Data(),eff.Data()));
      effptEBs[i] = (TEfficiency*)files[i]->Get("effptEB_0");
      effptEEs[i] = (TEfficiency*)files[i]->Get("effptEE_0");
      effetas [i] = (TEfficiency*)files[i]->Get("effeta_0");
      effphis [i] = (TEfficiency*)files[i]->Get("effphi_0");
      efftimes[i] = (TEfficiency*)files[i]->Get("efftime_0");
      effHTs  [i] = (TEfficiency*)files[i]->Get("effHT_0");
    }
    
    Overplot(effptEBs,eras,Form("cptEB_%s",eff.Data()),true,1,2000,0.0,1.05);
    Overplot(effptEEs,eras,Form("cptEE_%s",eff.Data()),true,1,2000,0.0,1.05);
    Overplot(effetas,eras,Form("ceta_%s",eff.Data()),false,-3.0,3,0.0,1.05);
    Overplot(effphis,eras,Form("cphi_%s",eff.Data()),false,-3.5,3.5,0.0,1.05);
    Overplot(efftimes,eras,Form("ctime_%s",eff.Data()),false,-30,30,0.8,1.005);
    Overplot(effHTs,eras,Form("cHT_%s",eff.Data()),true,100,3000,0.0,1.05);

    for (Int_t i = 0; i < N; i++)
    {
      delete files   [i];
      delete effptEBs[i];
      delete effptEEs[i];
      delete effetas [i];
      delete effphis [i];
      delete efftimes[i];
      delete effHTs  [i];
    }
  }
}
