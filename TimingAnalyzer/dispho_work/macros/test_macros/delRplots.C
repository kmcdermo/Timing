#include "Common.cpp+"

struct HistInfo
{
  HistInfo(){}
  HistInfo(const TString name, const TString & title, const TString & xtitle, const TString & ytitle, 
	   const Int_t nBinsX, const Float_t xlow, const Float_t xhigh) :
    name(name), title(title), xtitle(xtitle), ytitle(ytitle), nBinsX(nBinsX), xlow(xlow), xhigh(xhigh) {}

  TString name;
  TString title;
  TString xtitle;
  TString ytitle;
  Int_t nBinsX;
  Float_t xlow;
  Float_t xhigh;
};

void MakeHists(std::map<TString,TH1F*> & HistMap, const std::map<TString,Color_t> & LabelMap, const HistInfo & Info)
{
  for (const auto & LabelPair : LabelMap)
  {
    const auto & label = LabelPair.first;
    const auto & color = LabelPair.second;
    
    const TString histname = Info.name+label;
    HistMap[histname] = new TH1F(histname.Data(),Info.title.Data(),Info.nBinsX,Info.xlow,Info.xhigh);
    
    auto & hist = HistMap[histname];
    
    hist->GetXaxis()->SetTitle(Info.xtitle.Data());
    hist->GetYaxis()->SetTitle(Info.ytitle.Data());

    hist->SetLineColor(color);
    hist->SetMarkerColor(color);
    
    hist->Sumw2();
  }
}

void SetMinMax(std::map<TString,TH1F*> & HistMap, const TString & name)
{
  Float_t min = 1e9, max = -1e9;

  for (const auto & HistPair : HistMap)
  {
    const auto & histname = HistPair.first;
    if (!histname.Contains(name,TString::kExact)) continue;

    const auto & hist = HistPair.second;
    for (auto ibinX = 1; ibinX <= hist->GetXaxis()->GetNbins(); ibinX++)
    {
      const auto content = hist->GetBinContent(ibinX);
      
      if (content > max) max = content;
      if (content < min && content > 0.f) min = content;
    }
  }

  for (auto & HistPair : HistMap)
  {
    const auto & histname = HistPair.first;
    if (!histname.Contains(name,TString::kExact)) continue;

    auto & hist = HistPair.second;
    //    hist->SetMinimum(min/1.05f);
    hist->SetMinimum(0);
    hist->SetMaximum(max*1.05f);
  }
}

inline Float_t deltaR(const Float_t phi1, const Float_t eta1, const Float_t phi2, const Float_t eta2)
{
  auto delphi = std::abs(phi1-phi2);
  if (delphi > Common::PI) delphi -= Common::TwoPI;
  return Common::hypot(eta1-eta2,delphi);
}

void delRplots(const TString & filename, const TString & outfiletext)
{
  // set style
  auto tdrStyle = new TStyle("TDRStyle","Style for P-TDR");
  Common::SetTDRStyle(tdrStyle);
  Common::SetupEras();

  // get inputs
  auto file = TFile::Open(filename.Data());
  Common::CheckValidFile(file,filename);
  
  const TString treename = "Data_Tree";
  auto tree = (TTree*)file->Get(treename.Data());
  Common::CheckValidTree(tree,treename,filename);

  // set branches
  Float_t t1pfMETpt = 0.f; TBranch * b_t1pfMETpt = 0; const std::string s_t1pfMETpt = "t1pfMETpt"; tree->SetBranchAddress(s_t1pfMETpt.c_str(), &t1pfMETpt, &b_t1pfMETpt);

  Float_t phoE_0 = 0.f; TBranch * b_phoE_0 = 0; const std::string s_phoE_0 = "phoE_0"; tree->SetBranchAddress(s_phoE_0.c_str(), &phoE_0, &b_phoE_0);
  Float_t phopt_0 = 0.f; TBranch * b_phopt_0 = 0; const std::string s_phopt_0 = "phopt_0"; tree->SetBranchAddress(s_phopt_0.c_str(), &phopt_0, &b_phopt_0);
  Float_t phophi_0 = 0.f; TBranch * b_phophi_0 = 0; const std::string s_phophi_0 = "phophi_0"; tree->SetBranchAddress(s_phophi_0.c_str(), &phophi_0, &b_phophi_0);
  Float_t phoeta_0 = 0.f; TBranch * b_phoeta_0 = 0; const std::string s_phoeta_0 = "phoeta_0"; tree->SetBranchAddress(s_phoeta_0.c_str(), &phoeta_0, &b_phoeta_0);
  Bool_t phoisOOT_0 = 0; TBranch * b_phoisOOT_0 = 0; const std::string s_phoisOOT_0 = "phoisOOT_0"; tree->SetBranchAddress(s_phoisOOT_0.c_str(), &phoisOOT_0, &b_phoisOOT_0);
  Float_t phoseedE_0 = 0.f; TBranch * b_phoseedE_0 = 0; const std::string s_phoseedE_0 = "phoseedE_0"; tree->SetBranchAddress(s_phoseedE_0.c_str(), &phoseedE_0, &b_phoseedE_0);
  Float_t phoseedtime_0 = 0.f; TBranch * b_phoseedtime_0 = 0; const std::string s_phoseedtime_0 = "phoseedtime_0"; tree->SetBranchAddress(s_phoseedtime_0.c_str(), &phoseedtime_0, &b_phoseedtime_0);
  Float_t phoweightedtimeLT120_0 = 0.f; TBranch * b_phoweightedtimeLT120_0 = 0; const std::string s_phoweightedtimeLT120_0 = "phoweightedtimeLT120_0"; tree->SetBranchAddress(s_phoweightedtimeLT120_0.c_str(), &phoweightedtimeLT120_0, &b_phoweightedtimeLT120_0);

  Float_t phoE_1 = 0.f; TBranch * b_phoE_1 = 0; const std::string s_phoE_1 = "phoE_1"; tree->SetBranchAddress(s_phoE_1.c_str(), &phoE_1, &b_phoE_1);
  Float_t phopt_1 = 0.f; TBranch * b_phopt_1 = 0; const std::string s_phopt_1 = "phopt_1"; tree->SetBranchAddress(s_phopt_1.c_str(), &phopt_1, &b_phopt_1);
  Float_t phophi_1 = 0.f; TBranch * b_phophi_1 = 0; const std::string s_phophi_1 = "phophi_1"; tree->SetBranchAddress(s_phophi_1.c_str(), &phophi_1, &b_phophi_1);
  Float_t phoeta_1 = 0.f; TBranch * b_phoeta_1 = 0; const std::string s_phoeta_1 = "phoeta_1"; tree->SetBranchAddress(s_phoeta_1.c_str(), &phoeta_1, &b_phoeta_1);
  Bool_t phoisOOT_1 = 0; TBranch * b_phoisOOT_1 = 0; const std::string s_phoisOOT_1 = "phoisOOT_1"; tree->SetBranchAddress(s_phoisOOT_1.c_str(), &phoisOOT_1, &b_phoisOOT_1);
  Float_t phoseedE_1 = 0.f; TBranch * b_phoseedE_1 = 0; const std::string s_phoseedE_1 = "phoseedE_1"; tree->SetBranchAddress(s_phoseedE_1.c_str(), &phoseedE_1, &b_phoseedE_1);
  Float_t phoseedtime_1 = 0.f; TBranch * b_phoseedtime_1 = 0; const std::string s_phoseedtime_1 = "phoseedtime_1"; tree->SetBranchAddress(s_phoseedtime_1.c_str(), &phoseedtime_1, &b_phoseedtime_1);
  Float_t phoweightedtimeLT120_1 = 0.f; TBranch * b_phoweightedtimeLT120_1 = 0; const std::string s_phoweightedtimeLT120_1 = "phoweightedtimeLT120_1"; tree->SetBranchAddress(s_phoweightedtimeLT120_1.c_str(), &phoweightedtimeLT120_1, &b_phoweightedtimeLT120_1);

  // labels
  const TString common = "isOOT_pho";
  std::map<TString,Color_t> LabelMap = {{"0",kRed},{"1",kBlue},{"01",kGreen}};

  // hist infos
  std::vector<HistInfo> InfoVec = 
  {
    {"delR","#DeltaR","#DeltaR","Events",30,0.f,6.f},
    {"delR_zoom","#DeltaR","#DeltaR","Events",20,0.f,0.02f},
    {"met","MET (#DeltaR<0.3)","MET","Events",20,0.f,1000.f},
    {"delE","#DeltaE (#DeltaR<0.3)","#DeltaE (#DeltaR<0.3)","Events",20,0.f,400.f},
    {"delseedE","#Delta SeedE (#DeltaR<0.3)","#Delta SeedE (#DeltaR<0.3)","Events",20,0.f,400.f},
    {"delseedE_zoom","#Delta SeedE (#DeltaR<0.3)","#Delta SeedE (#DeltaR<0.3)","Events",20,0.f,50.f},
    {"delseedtime","#Delta Seed Time (#DeltaR<0.3)","#Delta Seed Time (#DeltaR<0.3)","Events",20,0.f,10.f},
    {"seedtime0","Seed Time #gamma_{0} (#DeltaR<0.3)","Seed Time #gamma_{0} (#DeltaR<0.3)","Events",30,-5.f,25.f},
    {"seedtime1","Seed Time #gamma_{1} (#DeltaR<0.3)","Seed Time #gamma_{1} (#DeltaR<0.3)","Events",30,-5.f,25.f},
    {"delweightedtimeLT120","#Delta Weighted Time (#DeltaR<0.3)","#Delta Weighted Time (#DeltaR<0.3)","Events",20,0.f,10.f},
    {"weightedtimeLT1200","Weighted Time #gamma_{0} (#DeltaR<0.3)","Weighted Time #gamma_{0} (#DeltaR<0.3)","Events",30,-5.f,25.f},
    {"weightedtimeLT1201","Weighted Time #gamma_{1} (#DeltaR<0.3)","Weighted Time #gamma_{1} (#DeltaR<0.3)","Events",30,-5.f,25.f}
  };
  for (auto & Info : InfoVec) Info.name.Append("_"+common);

  // hists
  std::map<TString,TH1F*> HistMap;
  for (const auto & Info : InfoVec) MakeHists(HistMap,LabelMap,Info);

  const auto nentries = tree->GetEntries();
  for (auto ientry = 0U; ientry < nentries; ientry++)
  {
    b_phoisOOT_0->GetEntry(ientry);
    b_phoisOOT_1->GetEntry(ientry);

    if (!phoisOOT_0 && !phoisOOT_1) continue;

    TString label = common;
    if      ( phoisOOT_0 && !phoisOOT_1) label += "0";
    else if (!phoisOOT_0 &&  phoisOOT_1) label += "1";
    else if ( phoisOOT_0 &&  phoisOOT_1) label += "01";

    // get branches
    b_t1pfMETpt->GetEntry(ientry);

    b_phoE_0->GetEntry(ientry);
    b_phophi_0->GetEntry(ientry);
    b_phoeta_0->GetEntry(ientry);
    b_phoseedE_0->GetEntry(ientry);
    b_phoseedtime_0->GetEntry(ientry);
    b_phoweightedtimeLT120_0->GetEntry(ientry);

    b_phoE_1->GetEntry(ientry);
    b_phophi_1->GetEntry(ientry);
    b_phoeta_1->GetEntry(ientry);
    b_phoseedE_1->GetEntry(ientry);
    b_phoseedtime_1->GetEntry(ientry);
    b_phoweightedtimeLT120_1->GetEntry(ientry);

    const auto dR = deltaR(phophi_0,phoeta_0,phophi_1,phoeta_1);

    HistMap[Form("delR_%s",label.Data())]->Fill(dR);
    HistMap[Form("delR_zoom_%s",label.Data())]->Fill(dR);
    
    if (dR < 0.3) 
    {
      HistMap[Form("met_%s",label.Data())]->Fill(t1pfMETpt);
      HistMap[Form("delE_%s",label.Data())]->Fill(std::abs(phoE_0-phoE_1));
      HistMap[Form("delseedE_%s",label.Data())]->Fill(std::abs(phoseedE_0-phoseedE_1));
      HistMap[Form("delseedE_zoom_%s",label.Data())]->Fill(std::abs(phoseedE_0-phoseedE_1));
      HistMap[Form("delseedtime_%s",label.Data())]->Fill(std::abs(phoseedtime_0-phoseedtime_1));
      HistMap[Form("seedtime0_%s",label.Data())]->Fill(phoseedtime_0);
      HistMap[Form("seedtime1_%s",label.Data())]->Fill(phoseedtime_1);
      HistMap[Form("delweightedtimeLT120_%s",label.Data())]->Fill(std::abs(phoweightedtimeLT120_0-phoweightedtimeLT120_1));
      HistMap[Form("weightedtimeLT1200_%s",label.Data())]->Fill(phoweightedtimeLT120_0);
      HistMap[Form("weightedtimeLT1201_%s",label.Data())]->Fill(phoweightedtimeLT120_1);
    }
  }
  std::cout << "Total integral: " << nentries << std::endl;

  // set min max
  for (const auto & Info : InfoVec) SetMinMax(HistMap,Info.name);

  // draw
  for (const auto & Info : InfoVec) 
  {
    const auto & name = Info.name;

    auto canv = new TCanvas();
    canv->cd();
    auto leg = new TLegend(0.6,0.7,0.8,0.9);

    auto ihist = 0U;
    for (const auto & LabelPair : LabelMap) 
    {
      const auto & label = LabelPair.first;
      auto & hist = HistMap[name+label];

      hist->Draw(ihist>0?"ep same":"ep");
      leg->AddEntry(hist,common+": "+label,"epl");

      std::cout << hist->GetName() << ": " << hist->Integral() << std::endl;

      ihist++;
    }
    leg->Draw("same");

    Common::CMSLumi(canv,0,"Full");
    const auto label = Common::RemoveDelim(std::string(name),std::string("_"+common));
    Common::SaveAs(canv,label+"_"+outfiletext);

    delete leg;
    delete canv;
  }
}
