#include "Common.cpp+"

static const TString indir = "hlt_inputs";

struct TestInfo
{
  TestInfo () {}
  TestInfo (const TString & dataset, const TString & reftrig, const TString & leg)
    : dataset(dataset), reftrig(reftrig), leg(leg) {}

  TString dataset;
  TString reftrig;
  TString leg;
};

void GetEff(TEfficiency *& eff, const TString & histname, TFile * file);
void Overplot(const TString & lambda, const std::vector<TString> & ctaus, const std::vector<TEfficiency*> & teffs, const TString outname,
	      const Bool_t isLogx, const Float_t xlow, const Float_t xhigh, const Float_t ylow, const Float_t yhigh);

void overplot_EffHLTPlots()
{
  Common::SetupEras();
  auto tdrStyle = new TStyle("TDRStyle","Style for P-TDR");
  Common::SetTDRStyle(tdrStyle);

  std::vector<TestInfo> testInfos =
  {
    {"SingleMuon","IsoMu27","L1"},
    {"SingleMuon","IsoMu27","L1toHLT"},
    {"SingleMuon","IsoMu27","ET"},
    {"SingleMuon","IsoMu27","PhoID"},
    {"SingleMuon","IsoMu27","DispID"},
    {"SingleMuon","IsoMu27","HT"},
    {"SinglePhoton","PhoId","DispID"},
    {"SinglePhoton","DispId","HT"}
  };

  const std::vector<TString> lambdas = {"100","200","300"};
  const std::vector<TString> ctaus   = {"10","200","1000"};

  std::map<TString,TFile*> signalFiles;
  for (const auto & lambda : lambdas)
  {
    for (const auto & ctau : ctaus)
    {
      const auto filename = indir+"/GMSB_L-"+lambda+"TeV_Ctau-"+ctau+"cm.root";
      const auto key = "L"+lambda+"_CTau"+ctau;
      signalFiles[key] = TFile::Open(filename.Data());
      Common::CheckValidFile(signalFiles[key],filename);
    }
  }

  for (const auto & testInfo : testInfos)
  {
    const auto & dataset = testInfo.dataset;
    const auto & reftrig = testInfo.reftrig;
    const auto & leg = testInfo.leg;

    const auto dataFileName = indir+"/"+dataset+"_"+reftrig+".root";
    const auto dataFile = TFile::Open(dataFileName.Data());
    Common::CheckValidFile(dataFile,dataFileName);

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

	GetEff(effETs[i],"tree/effET_"+leg,file);
	GetEff(effetas[i],"tree/effeta_"+leg,file);
	GetEff(effphis[i],"tree/effphi_"+leg,file);
	GetEff(efftimes[i],"tree/efftime_"+leg,file);
	GetEff(effHTs[i],"tree/effHT_"+leg,file);
      }

      Overplot(lambda,ctaus,effETs  ,outname+"ET"  ,true ,10  ,1000,0.0,1.05);
      Overplot(lambda,ctaus,effetas ,outname+"eta" ,false,-1.5,1.5 ,0.0,1.05);
      Overplot(lambda,ctaus,effphis ,outname+"phi" ,false,-3.2,3.2 ,0.0,1.05);
      Overplot(lambda,ctaus,efftimes,outname+"time",false,-2  ,25  ,0.8,1.005);
      Overplot(lambda,ctaus,effHTs  ,outname+"HT"  ,true ,100 ,3000,0.0,1.05);
      
      Common::DeleteVec(effETs);
      Common::DeleteVec(effetas);
      Common::DeleteVec(effphis);
      Common::DeleteVec(efftimes);
      Common::DeleteVec(effHTs);
    }
    delete dataFile;
  }
  
  Common::DeleteMap(signalFiles);
  delete tdrStyle;
}

void GetEff(TEfficiency *& eff, const TString & histname, TFile * file)
{
  eff = (TEfficiency*)file->Get(histname.Data());
  Common::CheckValidHist(eff,histname.Data(),file->GetName());
}

void Overplot(const TString & lambda, const std::vector<TString> & ctaus, const std::vector<TEfficiency*> & teffs, const TString outname,
	      const Bool_t isLogx, const Float_t xlow, const Float_t xhigh, const Float_t ylow, const Float_t yhigh)
{
  auto canv = new TCanvas();
  canv->cd();
  canv->SetLogx(isLogx);
  canv->SetGrid(1,1);

  auto leg = new TLegend(0.57,0.2,0.825,0.5);

  for (auto i = 0U; i < teffs.size(); i++)
  {
    const auto color = (i > 0 ? Common::ColorVec[i-1] : kBlack);

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

  Common::CMSLumi(canv);
  Common::SaveAs(canv,outname);

  delete leg;
  delete canv;
}
