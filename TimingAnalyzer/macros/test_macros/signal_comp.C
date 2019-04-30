#include "Common.cpp+"

struct HistInfo
{
  HistInfo () {}
  HistInfo (const TString & name, const TString & title, const TString & var, const std::vector<Double_t> & xbins) 
    : name(name), title(title), var(var), xbins(xbins) {}

  TString name;
  TString title;
  TString var;
  std::vector<Double_t> xbins;
};

void setupHist(TH1F * hist, const Color_t color);
void setupRatioHist(TH1F * ratiohist);
void getMinYMaxY(const TH1F * hist, Double_t & minY, Double_t & maxY);
void drawCanv(TLegend * leg, TH1F * centhist, TH1F * privhist, TH1F * ratiohist, TFile * outfile,
	      const TString & name, const Double_t minY, const Double_t maxY, const Bool_t isLogY);

void signal_comp()
{
  const TString outfiletext = "signal_comp";

  // style
  auto tdrStyle = new TStyle("TDRStyle","Style for P-TDR");
  Common::SetTDRStyle(tdrStyle);

  // signal name
  const TString signal = "GMSB_L200_CTau200";

  // in file
  const auto infilename = "skims/v4p1/sig_comp/"+signal+".root";
  auto infile = TFile::Open(infilename.Data());
  Common::CheckValidFile(infile,infilename);
  
  // central tree
  const auto centtreename = signal+"_Central_Tree";
  auto centtree = (TTree*)infile->Get(centtreename.Data());
  Common::CheckValidTree(centtree,centtreename,infilename);

  // private tree
  const auto privtreename = signal+"_Private_Tree";
  auto privtree = (TTree*)infile->Get(privtreename.Data());
  Common::CheckValidTree(privtree,privtreename,infilename);

  // out file
  auto outfile = TFile::Open(outfiletext+".root","recreate");

  // hist infos
  std::vector<HistInfo> histInfos = 
  {
    {"nvtx","nVertices","nvtx",{0,10,15,20,25,30,35,40,50,60,70,80,90,100}},
    {"phopt_0","Leading Photon p_{T} [GeV]","phopt_0",{70,80,90,100,125,150,175,200,250,300,350,400,450,500,600,700,800,900,1000}},
    {"phophi_0","Leading Photon #phi","phophi_0",{-3.2,-2.8,-2.4,-2.0,-1.6,-1.2,-0.8,-0.4,0.0,0.4,0.8,1.2,1.6,2.0,2.4,2.8,3.2}},
    {"phosceta_0","Leading Photon SC #eta","phoeta_0",{-1.6,-1.4,-1.2,-1.0,-0.8,-0.6,-0.4,-0.2,0.0,0.2,0.4,0.6,0.8,1.0,1.2,1.4,1.6}},
    {"photime_0","Leading Photon Weighted Time [ns]","phoweightedtimeLT120_0+phoweightedtimeLT120SHIFT_0+phoweightedtimeLT120SMEAR_0",
     {-2,-1.5,-1,-0.75,-0.5,-0.25,0.0,0.25,0.5,0.75,1.0,1.5,2,3,5,10,25}},
    {"t1pfMETpt","p_{T}^{Reco miss} [GeV]","t1pfMETpt",{0,20,40,60,80,100,150,200,250,300,500,700,1000}},
    {"genMETpt","p_{T}^{Gen miss} [GeV]","genMETpt",{0,20,40,60,80,100,150,200,250,300,500,700,1000}},
    {"genMETphi","#phi^{Gen miss}","genMETphi",{-3.2,-2.8,-2.4,-2.0,-1.6,-1.2,-0.8,-0.4,0.0,0.4,0.8,1.2,1.6,2.0,2.4,2.8,3.2}},
    {"residualMETpt","#Delta(p_{T}^{reco miss}-p_{T}^{gen miss}) [GeV]","t1pfMETpt-genMETpt",{-1000,-700,-500,-300,-250,-200,-150,-100,-80,-60,-40,-20,0.0,20,40,60,80,100,150,200,250,300,500,700,1000}},
    {"gendist","Gen Distance","(sqrt(pow(genNdecayvx_0-genNprodvx_0,2)+pow(genNdecayvy_0-genNprodvy_0,2)+pow(genNdecayvz_0-genNprodvz_0,2))*genNmass_0)/(genphpt_0*abs(cosh(genpheta_0)))*(genphpt_0>0)+(genphpt_0*(genphpt_0<0))",{0,1000,2000,3000,4000,5000,6000,7000,8000,9000,10000}}


 };

  // ytitle
  const TString ytitle = "Fraction of Events";
  
  // cuts
  const TString cut = "evtwgt * puwgt";

  // loop over histinfos
  for (const auto & histInfo : histInfos)
  {
    const auto & name   = histInfo.name;
    const auto & title  = histInfo.title;
    const auto & var    = histInfo.var;
    const auto & xbins  = histInfo.xbins;
    const auto   nbinsX = xbins.size()-1;
    const auto & binsX  = &xbins[0];

    // in outfile dir
    outfile->cd();
    
    // make hists
    auto centhist = new TH1F("cent_"+name,title+";"+title+";"+ytitle,nbinsX,binsX);
    setupHist(centhist,kBlue);

    auto privhist = new TH1F("priv_"+name,title+";"+title+";"+ytitle,nbinsX,binsX);
    setupHist(privhist,kRed+1);    

    // fill hists
    centtree->Draw(Form("%s>>%s",var.Data(),centhist->GetName()),cut.Data(),"goff");
    privtree->Draw(Form("%s>>%s",var.Data(),privhist->GetName()),cut.Data(),"goff");

    // scale
    const auto isUp = false;
    Common::Scale(centhist,isUp);
    Common::Scale(privhist,isUp);
    
    // fraction of events
    centhist->Scale(1.f/centhist->Integral());
    privhist->Scale(1.f/privhist->Integral());

    // ratio hist
    auto ratiohist = (TH1F*)privhist->Clone("ratio_"+name);
    ratiohist->Divide(centhist);
    setupRatioHist(ratiohist);

    // get min,max
    auto minY = 1e9, maxY = -1e9;
    getMinYMaxY(centhist,minY,maxY);
    getMinYMaxY(privhist,minY,maxY);

    // legend
    auto leg = new TLegend(0.7,0.8,0.82,0.91);
    leg->SetName("leg_"+name);
    leg->SetBorderSize(1);
    leg->SetLineColor(kBlack);
    leg->AddEntry(centhist,"Central","epl");
    leg->AddEntry(privhist,"Private","epl");

    // draw and save
    drawCanv(leg,centhist,privhist,ratiohist,outfile,name,minY,maxY,false);
    drawCanv(leg,centhist,privhist,ratiohist,outfile,name,minY,maxY,true);

    // delete it all
    delete leg;
    delete ratiohist;
    delete privhist;
    delete centhist;
  }

  // delete it all
  delete outfile;
  delete privtree;
  delete centtree;
  delete infile;
  delete tdrStyle;
}

void setupHist(TH1F * hist, const Color_t color)
{
  hist->Sumw2();
  hist->SetLineColor  (color);
  hist->SetMarkerColor(color);

  hist->GetXaxis()->SetLabelSize(0);
  hist->GetXaxis()->SetTitleSize(0);
  hist->GetYaxis()->SetLabelSize  (Common::LabelSize / Common::height_up); 
  hist->GetYaxis()->SetTitleSize  (Common::TitleSize / Common::height_up);
  hist->GetYaxis()->SetTitleOffset(Common::TitleFF * Common::TitleYOffset * Common::height_up);
}

void setupRatioHist(TH1F * ratiohist)
{
  // set style for ratio plot
  ratiohist->GetYaxis()->SetTitle("Priv/Cent");
  ratiohist->SetMinimum(-0.1); // Define Y ..
  ratiohist->SetMaximum( 2.1); // .. range
  ratiohist->SetLineColor(kBlack);
  ratiohist->SetMarkerColor(kBlack);
  ratiohist->SetStats(0);      // No statistics on lower plot

  // rescale titles and labels
  ratiohist->GetYaxis()->SetNdivisions(505);
  ratiohist->GetXaxis()->SetLabelSize  (Common::LabelSize   / Common::height_lp); 
  ratiohist->GetXaxis()->SetLabelOffset(Common::LabelOffset / Common::height_lp); 
  ratiohist->GetXaxis()->SetTitleSize  (Common::TitleSize   / Common::height_lp);
  ratiohist->GetXaxis()->SetTickLength (Common::TickLength  / Common::height_lp);
  ratiohist->GetYaxis()->SetLabelSize  (Common::LabelSize   / Common::height_lp); 
  ratiohist->GetYaxis()->SetTitleSize  (Common::TitleSize   / Common::height_lp);
  ratiohist->GetYaxis()->SetTitleOffset(Common::TitleFF * Common::TitleYOffset * Common::height_lp);
}

void getMinYMaxY(const TH1F * hist, Double_t & minY, Double_t & maxY)
{
  for (auto ibinX = 1; ibinX <= hist->GetXaxis()->GetNbins(); ibinX++)
  {
    const auto content = hist->GetBinContent(ibinX);
    const auto error   = hist->GetBinError  (ibinX);

    if (content < minY && content > 0.0) minY = content;
    if (content > maxY && content > 0.0) maxY = content;
  }
}

void drawCanv(TLegend * leg, TH1F * centhist, TH1F * privhist, TH1F * ratiohist, TFile * outfile,
	      const TString & name, const Double_t minY, const Double_t maxY, const Bool_t isLogY)
{
  // set min / max
  const auto min = (isLogY ? minY / 2.f : minY / 1.2f);
  const auto max = (isLogY ? maxY * 2.f : maxY * 1.2f);

  centhist->GetYaxis()->SetRangeUser(min,max);
  privhist->GetYaxis()->SetRangeUser(min,max);

  // make ratio line
  auto ratioline = new TLine();
  ratioline->SetLineColor(kRed);
  ratioline->SetLineWidth(2);
  ratioline->SetX1(ratiohist->GetXaxis()->GetXmin());
  ratioline->SetX2(ratiohist->GetXaxis()->GetXmax());
  ratioline->SetY1(1.0);
  ratioline->SetY2(1.0);

  // make canvas + pads
  auto canv = new TCanvas("canv_"+name,"");
  canv->cd();
  
  auto upperpad = new TPad("upperpad_"+name,"", Common::left_up, Common::bottom_up, Common::right_up, Common::top_up);
  upperpad->SetBottomMargin(Common::merged_margin);

  auto lowerpad = new TPad("lowerpad_"+name, "", Common::left_lp, Common::bottom_lp, Common::right_lp, Common::top_lp);
  lowerpad->SetTopMargin(Common::merged_margin);
  lowerpad->SetBottomMargin(Common::bottom_margin);

  // draw upper
  canv->cd();
  upperpad->Draw();
  upperpad->cd();
  upperpad->SetLogy(isLogY);

  centhist->Draw("ep");
  privhist->Draw("ep same");
  leg->Draw("same");
  
  // draw lower
  canv->cd(); 
  lowerpad->Draw();
  lowerpad->cd(); 

  ratiohist->Draw("ep");
  ratioline->Draw("same");
  ratiohist->Draw("ep same");

  // save if log only
  if (isLogY)
  {
    Common::Write(outfile,centhist);
    Common::Write(outfile,privhist);
    Common::Write(outfile,ratiohist);
    Common::Write(outfile,leg);
    Common::Write(outfile,canv);
    Common::Write(outfile,upperpad);
    Common::Write(outfile,lowerpad);
  }

  // save
  Common::SaveAs(canv,Form("%s_%s",name.Data(),(isLogY?"log":"lin")));

  // delete it all
  delete lowerpad;
  delete upperpad;
  delete canv;
  delete ratioline;
}
