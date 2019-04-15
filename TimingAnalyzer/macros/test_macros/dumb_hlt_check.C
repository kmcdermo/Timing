#include "Common.cpp+"

struct InputInfo
{
  InputInfo() {}
  InputInfo(const Bool_t isData, const TString & tree_name, const TString & name, const TString & label, const Color_t color)
    : isData(isData), tree_name(tree_name), name(name), label(label), color(color) {}

  Bool_t isData;
  TString tree_name;
  TString name;
  TString label;
  Color_t color;
};

struct HistInfo
{
  HistInfo() {}
  HistInfo(const TString & xtitle, const TString & xvar, const TString & xvarname, const std::vector<Double_t> & xbins)
    : xtitle(xtitle), xvar(xvar), xvarname(xvarname), xbins(xbins) {}

  TString xtitle;
  TString xvar;
  TString xvarname;
  std::vector<Double_t> xbins;
};

void makeEff(TH1F * eff, const TH1F * denom, const TH1F * numer);

void dumb_hlt_check()
{
  gStyle->SetOptStat(0);

  // get inputs
  const TString dir = "skims/v4/test_hlt_wHT";

  const auto data_file_name = dir+"/sr.root";
  auto data_file = TFile::Open(data_file_name.Data());
  Common::CheckValidFile(data_file,data_file_name);

  const auto sign_file_name = dir+"/signals_sr.root";
  auto sign_file = TFile::Open(sign_file_name.Data());
  Common::CheckValidFile(sign_file,sign_file_name);

  // setup input trees
  auto icolor = 0;
  const std::vector<InputInfo> inputInfos = 
  {  
    {true,"Data_Tree","data","Data",kBlack},
    {false,"GMSB_L100_CTau0p1_Tree","gmsb_l100_ct0p1","#Lambda=100, c#tau=0.1",Common::ColorVec[icolor++]},
    {false,"GMSB_L100_CTau200_Tree","gmsb_l100_ct200","#Lambda=100, c#tau=200",Common::ColorVec[icolor++]},
    {false,"GMSB_L100_CTau1000_Tree","gmsb_l100_ct1000","#Lambda=100, c#tau=1000",Common::ColorVec[icolor++]},
    {false,"GMSB_L200_CTau0p1_Tree","gmsb_l200_ct0p1","#Lambda=200, c#tau=0.1",Common::ColorVec[icolor++]},
    {false,"GMSB_L200_CTau200_Tree","gmsb_l200_ct200","#Lambda=200, c#tau=200",Common::ColorVec[icolor++]},
    {false,"GMSB_L200_CTau1000_Tree","gmsb_l200_ct1000","#Lambda=200, c#tau=1000",Common::ColorVec[icolor++]},
    {false,"GMSB_L300_CTau0p1_Tree","gmsb_l300_ct0p1","#Lambda=300, c#tau=0.1",Common::ColorVec[icolor++]},
    {false,"GMSB_L300_CTau200_Tree","gmsb_l300_ct200","#Lambda=300, c#tau=200",Common::ColorVec[icolor++]},
    {false,"GMSB_L300_CTau1000_Tree","gmsb_l300_ct1000","#Lambda=300, c#tau=1000",Common::ColorVec[icolor++]}
  };
  
  // hist info
  const std::vector<HistInfo> histInfos =
  {
    {"p_{T}^{#gamma_{0}} [GeV]","phopt_0","phopt_0",{70,80,90,100,200,300,500,1000}},
    {"SC #eta^{#gamma_{0}}","phosceta_0","phosceta_0",{-1.5,-1.2,-0.9,-0.6,-0.3,0.0,0.3,0.6,0.9,1.2,1.5}},
    {"Weighted Time^{#gamma_{0}} [ns]","phoweightedtimeLT120_0","photime_0",{-2,-1,0,1,2,3,5,10}},
    {"R_{9}^{#gamma_{0}}","phor9_0","phor9_0",{0.9,0.91,0.92,0.93,0.94,0.95,0.96,0.97,0.98,0.99,1.00,1.01}},
    {"Ecal PF Cluster Iso^{#gamma_{0}} [GeV]","max(phoEcalPFClIso_0-(0.003008*phopt_0)-(0.1324*rho)*(abs(phosceta_0)<0.8)-(0.08638*rho)*((abs(phosceta_0)>=0.8)*(abs(phosceta_0)<1.4442)),0.0)","phoEcalPFClIso_0",{0,0.1,0.2,0.3,0.4,0.5,1.0,2.0,3.0,4.0,5.0}},
    {"Hcal PF Cluster Iso^{#gamma_{0}} [GeV]","max(phoHcalPFClIso_0-((2.921e-5*phopt_0*phopt_0)+(-0.005802*phopt_0))-(0.1094*rho)*(abs(phosceta_0)<0.8)-(0.09392*rho)*((abs(phosceta_0)>=0.8)*(abs(phosceta_0)<1.4442)),0.0)","phoHcalPFClIso_0",{0,0.1,0.2,0.3,0.4,0.5,1.0,2.0,3.0,4.0}},
    {"Tracker Iso^{#gamma_{0}} [GeV]","max(phoTrkIso_0-(0.02276*rho)*(abs(phosceta_0)<0.8)-(0.00536*rho)*((abs(phosceta_0)>=0.8)*(abs(phosceta_0)<1.4442)),0.0)","phoTrkIso_0",{0,0.1,0.2,0.3,0.4,0.5,1.0,2.0,3.0,4.0}},
    {"S_{Major}^{#gamma_{0}}","phosmaj_0","phosmaj_0",{0,0.2,0.4,0.6,0.8,1.0,1.2,1.3}},
    {"S_{Minor}^{#gamma_{0}}","phosmin_0","phosmin_0",{0,0.05,0.1,0.15,0.2,0.25,0.3,0.35,0.4}},
    {"isTrk Bool","phoisTrk_0","phoisTrk_0",{-1,0,1,2}},
    {"H_{T}","Sum$(jetpt*(jetpt>30))","HT",{400,500,750,1000,1500,2000,3000}},
  };

  for (const auto & histInfo : histInfos)
  {
    // draw hists
    auto canv = new TCanvas();
    canv->cd();
    canv->SetTickx();
    canv->SetTicky();
    
    // infos
    const auto & xtitle   = "Offline "+histInfo.xtitle;
    auto  xvar            = histInfo.xvar;
    const auto & xvarname = histInfo.xvarname;
    const auto & xbins    = histInfo.xbins;     
    const auto binsx      = &xbins[0];
    const auto & title    = "HLT Eff. vs "+xtitle+";"+xtitle+";#epsilon";

    std::cout << "Working on: " << xvarname.Data() << std::endl;

    // loop over trees
    auto leg  = new TLegend(0.8,0.2,1.0,0.5);
    std::vector<TH1F*> effs;
    for (auto i = 0U; i < inputInfos.size(); i++)
    {
      const auto & inputInfo = inputInfos[i];
      
      std::cout << "   working on: " << inputInfo.name.Data() << std::endl;

      auto denom = new TH1F("denom",title.Data(),xbins.size()-1,binsx); denom->Sumw2();
      auto numer = new TH1F("numer",title.Data(),xbins.size()-1,binsx); numer->Sumw2();
      
      // get tree
      auto & file = (inputInfo.isData ? data_file : sign_file);
      auto tree = (TTree*)file->Get(inputInfo.tree_name.Data());
      
      // cuts
      const TString commoncut = "(1)";
      const TString numer_cut = "(hltSignal)";
      const TString scale     = (inputInfo.isData?"1":"puwgt");
      
      // modify xvar
      if (xvarname.EqualTo("photime_0"))
      {
	if (inputInfo.isData) xvar += "+phoweightedtimeLT120SHIFT_0";
	else                  xvar += "+phoweightedtimeLT120SHIFT_0+phoweightedtimeLT120SMEAR_0";
      }

      // fill hists
      tree->Draw(xvar+">>denom",Form("(%s)*(%s)",commoncut.Data(),scale.Data()),"goff");
      tree->Draw(xvar+">>numer",Form("(%s&&%s)*(%s)",commoncut.Data(),numer_cut.Data(),scale.Data()),"goff");
      
      // make ratio hist
      effs.emplace_back((TH1F*)numer->Clone(inputInfo.name.Data()));
      auto & eff = effs.back();
      eff->Divide(denom);
      eff->SetMarkerStyle(20);
      eff->SetMarkerSize(0.6);
      eff->SetLineColor(inputInfo.color);
      eff->SetMarkerColor(inputInfo.color);
      makeEff(eff,denom,numer);
      eff->GetYaxis()->SetRangeUser(0.0,1.05);
      
      // draw
      canv->cd();
      eff->Draw(i>0?"ep same":"ep");
      
      // add to legend
      leg->AddEntry(eff,inputInfo.label.Data(),"epl");
      
      // delete tmps
      delete tree;
      delete denom;
      delete numer;
    }
    
    // draw leg
    canv->cd();
    leg->Draw("same");
    
    // save as
    Common::SaveAs(canv,"hlteff_v_"+xvarname);

    // delete it all
    for (auto & eff : effs) delete eff;
    delete leg;
    delete canv;
  }
}

void makeEff(TH1F * eff, const TH1F * denom, const TH1F * numer)
{
  for (auto ibin = 1; ibin <= eff->GetXaxis()->GetNbins(); ibin++)
  {
    if ((denom->GetBinContent(ibin) <= 0.0) || (numer->GetBinContent(ibin) <= 0.0)) continue;
    
    const auto prob = numer->GetBinContent(ibin)/denom->GetBinContent(ibin);
    const auto err  = 1.96*std::sqrt(prob*(1.0-prob)/denom->GetBinContent(ibin));
    
    eff->SetBinContent(ibin,prob);
    eff->SetBinError  (ibin,err);
  }
}
