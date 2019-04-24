struct TestInfo
{
  TestInfo () {}
  TestInfo (const TString & dataset, const TString & reftrig, const TString & leg)
    : dataset(dataset), reftrig(reftrig), leg(leg) {}

  TString dataset;
  TString reftrig;
  TString leg;
};

void Overplot(const TString & lambda, const std::vector<TString> & ctaus, const std::vector<TEfficiency*> & teffs, const TString outname,
	      const Bool_t isLogx, const Float_t xlow, const Float_t xhigh, const Float_t ylow, const Float_t yhigh);

void overplot_EffHLTPlots()
{
  const TString indir = "hlt_inputs";

  std::vector<TestInfo> testInfos =
  {
    {"SingleMuon","IsoMu27","L1"},
    {"SingleMuon","IsoMu27","L1toHLT"},
    {"SingleMuon","IsoMu27","ET"},
    {"SingleMuon","IsoMu27","PhoID"},
    {"SingleMuon","IsoMu27","DispID"},
    {"SingleMuon","IsoMu27","HT"},
    {"SinglePhoton","PhoID","DispID"},
    {"SinglePhoton","DispID","HT"}
  };

  const std::vector<TString> lambdas = {"100","200","300"};
  const std::vector<TString> ctaus   = {"10","200","1000"};

  std::map<TString,TFile*> signalFiles;
  for (const auto & lambda : lambdas)
    for (const auto & ctau : ctaus)
      signalFiles["L"+lambda+"_CTau"+ctau] = TFile::Open(indir+"/GMSB_L-"+lambda+"TeV_Ctau-"+ctau+"cm.root");

  for (const auto & testInfo : testInfos)
  {
    const auto & dataset = testInfo.dataset;
    const auto & reftrig = testInfo.reftrig;
    const auto & leg = testInfo.leg;
    const auto dataFile = TFile::Open(indir+"/"+dataset+"_"+reftrig+".root");

    for (const auto & lambda : lambdas)
    {
      const auto outname = dataset+"_and_L"+lambda+"_RefPath-"+reftrig+"_Leg-"+leg+"_vs_";

      const auto N = ctaus.size()+1;
      std::vector<TEfficiency*> effETs(N);
      std::vector<TEfficiency*> effetas(N); 
      std::vector<TEfficiency*> effphis(N); 
      std::vector<TEfficiency*> efftimes(N); 
      std::vector<TEfficiency*> effHTs(N);
    
      for (auto i = 0U; i < N; i++)
      {
	auto & file = (i > 0 ? signalFiles["L"+lambda+"_CTau"+ctaus[i-1]] : dataFile);

	effETs  [i] = (TEfficiency*)file->Get("tree/effET_"+leg);
	effetas [i] = (TEfficiency*)file->Get("tree/effeta_"+leg);
	effphis [i] = (TEfficiency*)file->Get("tree/effphi_"+leg);
	efftimes[i] = (TEfficiency*)file->Get("tree/efftime_"+leg);
	effHTs  [i] = (TEfficiency*)file->Get("tree/effHT_"+leg);
      }

      Overplot(lambda,ctaus,effETs  ,outname+"ET"  ,true ,10  ,1000,0.0,1.05);
      Overplot(lambda,ctaus,effetas ,outname+"eta" ,false,-1.5,1.5 ,0.0,1.05);
      Overplot(lambda,ctaus,effphis ,outname+"phi" ,false,-3.2,3.2 ,0.0,1.05);
      Overplot(lambda,ctaus,efftimes,outname+"time",false,-2  ,25  ,0.8,1.005);
      Overplot(lambda,ctaus,effHTs  ,outname+"HT"  ,true ,100 ,3000,0.0,1.05);
      
      for (auto i = 0U; i < N; i++)
      {
	delete effETs  [i];
	delete effetas [i];
	delete effphis [i];
	delete efftimes[i];
	delete effHTs  [i];
      }
    }
    delete dataFile;
  }
  
  for (auto & signalFile : signalFiles) delete signalFile.second;
}

void Overplot(const TString & lambda, const std::vector<TString> & ctaus, const std::vector<TEfficiency*> & teffs, const TString outname,
	      const Bool_t isLogx, const Float_t xlow, const Float_t xhigh, const Float_t ylow, const Float_t yhigh)
{
  const std::vector<Color_t> colors = {kBlack,kBlue,kRed+1,kGreen+1};

  auto canv = new TCanvas();
  canv->cd();
  canv->SetLogx(isLogx);
  canv->SetGrid(1,1);

  auto leg = new TLegend(0.6,0.2,0.86,0.5);

  for (auto i = 0U; i < teffs.size(); i++)
  {
    const auto color = colors[i];

    teffs[i]->SetLineColor(color);
    //    teffs[i]->SetLineWidth(2);
    teffs[i]->SetMarkerColor(color);
    teffs[i]->SetMarkerStyle(20);
    teffs[i]->SetMarkerSize(0.6);
    teffs[i]->Draw(i>0?"PZ same":"APZ");

    leg->AddEntry(teffs[i],(i>0?Form("#Lambda=%sTeV, c#tau=%scm",lambda.Data(),ctaus[i-1].Data()):"Data"),"epl");
  }

  gPad->Update(); 
  auto graph = teffs[0]->GetPaintedGraph(); 
  graph->GetXaxis()->SetRangeUser(xlow,xhigh);
  graph->GetYaxis()->SetRangeUser(ylow,yhigh);
  gPad->Update();

  leg->Draw("same");
  canv->SaveAs(Form("%s.png",outname.Data()));
  canv->SaveAs(Form("%s.pdf",outname.Data()));

  delete leg;
  delete canv;
}
