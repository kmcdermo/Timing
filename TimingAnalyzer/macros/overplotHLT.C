typedef std::vector<TEfficiency*> TEffVec;

static const Int_t N = 3;

void Overplot(const TEffVec& teffs, const TString cname, const Bool_t isLogx, 
	      const Float_t xlow, const Float_t xhigh, const Float_t ylow, const Float_t yhigh)
{
  std::vector<Color_t> colors = {kRed+1,kGreen+1,kMagenta,kOrange+1,kYellow-7,kViolet-1,kAzure+10,kYellow+3,kBlack};

  TCanvas * canv = new TCanvas();
  canv->cd();
  canv->SetLogx(isLogx);
  canv->SetGrid(1,1);

  TLegend * leg = new TLegend(0.9,0.9,0.99,0.99);
  for (Int_t iv = 0; iv < N; iv++)
  {
    teffs[iv]->SetLineColor(colors[iv]);
    teffs[iv]->SetMarkerColor(colors[iv]);
    teffs[iv]->Draw(iv>0?"P same":"AP");
    leg->AddEntry(teffs[iv],Form("RunC v%i",iv+1),"epl");
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
  std::vector<TFile*> files(N); 
  TEffVec effptEBs(N); 
  TEffVec effptEEs(N); 
  TEffVec effetas(N); 
  TEffVec effphis(N); 
  TEffVec efftimes(N); 

  for (Int_t iv = 0; iv < N; iv++)
  {
    files   [iv] = TFile::Open(Form("runC-v%i-wdenom-noHT/cuts_nopho_jetER_phden_2last/plots.root",iv+1));
    effptEBs[iv] = (TEfficiency*)files[iv]->Get("effptEB_0");
    effptEEs[iv] = (TEfficiency*)files[iv]->Get("effptEE_0");
    effetas [iv] = (TEfficiency*)files[iv]->Get("effeta_0");
    effphis [iv] = (TEfficiency*)files[iv]->Get("effphi_0");
    efftimes[iv] = (TEfficiency*)files[iv]->Get("efftime_0");
  }

  Overplot(effptEBs,"cptEB",true,1,2000,0,1.05);
  Overplot(effptEEs,"cptEE",true,1,2000,0,1.05);
  Overplot(effetas,"ceta",false,-4.0,4,0.995,1.0005);
  Overplot(effphis,"cphi",false,-4.0,4.0,0.995,1.0005);
  Overplot(efftimes,"ctime",false,-30,30,0,1.05);
}
