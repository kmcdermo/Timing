#include "Common.cpp+"

struct HistInfo
{
  HistInfo() {}
  HistInfo(const Bool_t isSig, const TString & name, const TString & label, const TString & var, const Color_t color)
    : isSig(isSig), name(name), label(label), var(var), color(color) {}

  Bool_t isSig;
  TString name;
  TString label;
  TString var;
  Color_t color;
};

struct Bound
{
  Bound() {}
  Bound(const TString & s_min, const TString & s_max)
    : s_min(s_min), s_max(s_max) {}

  Double_t val(TString var) const
  {
    var.ReplaceAll("p",".");
    return var.Atof();
  }

  TString s_val(TString var) const
  {
    var.ReplaceAll("p",".");
    return var;
  }

  TString s_min;
  TString s_max;

  Double_t min_val() const {return val(s_min);}
  TString s_min_val() const {return s_val(s_min);}
  Double_t max_val() const {return val(s_max);}
  TString s_max_val() const {return s_val(s_max);}
};

void draw(TFile * outfile, std::vector<TH1F*> & hists, TLegend * leg, const Bound & pt, const Bound & eta,
	  const Double_t miny, const Double_t maxy, const Bool_t isLogy);
void make_plots(TTree * sign_tree, TTree * bkgd_tree, const Bound & pt, const Bound & eta);

void plot_corrected()
{
  // style
  gStyle->SetOptStat(0);

  // i/o dirs
  const TString indir  = "skims/v4/ootVID";
  const TString eosdir = "/eos/user/k/kmcdermo/www";
  const TString outdir = "dispho/plots/ootVID_v2/ele_vs_pho_moar";

  // get inputs
  std::cout << "Get Signals" << std::endl;

  const auto sign_file_name = indir+"/gmsb.root";
  auto sign_file = TFile::Open(sign_file_name);
  Common::CheckValidFile(sign_file,sign_file_name);
  
  const TString sign_tree_name = "GMSB_L200_CTau200_Tree";  
  auto sign_tree = (TTree*)sign_file->Get(sign_tree_name.Data());
  Common::CheckValidTree(sign_tree,Common::disphotreename,sign_file_name);

  std::cout << "Get Backgrounds" << std::endl;

  const auto bkgd_file_name = indir+"/dyll.root";
  auto bkgd_file = TFile::Open(bkgd_file_name);
  Common::CheckValidFile(bkgd_file,bkgd_file_name);

  const TString bkgd_tree_name = "DYLL_Tree";
  auto bkgd_tree = (TTree*)bkgd_file->Get(bkgd_tree_name.Data());
  Common::CheckValidTree(bkgd_tree,Common::disphotreename,bkgd_file_name);

  // make plots
  std::vector<Bound> pts =
  {
    {"70","90"},
    {"90","150"},
    {"150","300"},
    {"300","1000"}
  };
  
  std::vector<Bound> etas =
  {
    {"0p0","0p4"},
    {"0p4","0p8"},
    {"0p8","1p1"},
    {"1p1","1p4442"}
  };

  for (const auto & pt : pts)
  {
    for (const auto & eta : etas)
    {
      make_plots(sign_tree,bkgd_tree,pt,eta);
    }
  }

  // move shit
  const auto fulldir = eosdir+"/"+outdir;
  gSystem->Exec("mkdir -p "+fulldir);
  gSystem->Exec("pushd "+eosdir+"; ./copyphp.sh "+fulldir+"; popd;");
  gSystem->Exec("mv *png *pdf *root "+fulldir);

  // delete shit
  delete bkgd_tree;
  delete bkgd_file;
  delete sign_tree;
  delete sign_file;
}

void make_plots(TTree * sign_tree, TTree * bkgd_tree, const Bound & pt, const Bound & eta)
{
  auto outfile = TFile::Open("plots.root","RECREATE");

  const TString commoncut = "(phopt_0>70&&phoisEB_0==1&&phoisOOT_0==0&&phopt_0>"+pt.s_min_val()+"&&phopt_0<="+pt.s_max_val()+"&&abs(phosceta_0)>"+eta.s_min_val()+"&&abs(phosceta_0)<="+eta.s_max_val()+")";
  const TString sign_cut  = "(puwgt*evtwgt)*(phoisGen_0==1)";
  const TString bkgd_cut  = "(puwgt*evtwgt)*(phohasPixSeed_0==1)";

  std::vector<TString> vars = {"phosmaj_0","max(phosmaj_0-(0.311*(abs(phosceta_0)-0.8))*((abs(phosceta_0)>=0.8)*(abs(phosceta_0)<1.4442))-(0.1465*exp(-0.01775*phopt_0+1.948)),0.0)"};

  //  std::vector<TString> vars = {"phopt_0"};
  //  std::vector<TString> vars = {"phosceta_0"};

  std::vector<HistInfo> histInfos =
  {
    {true,"sign_uncor","Sig Uncor",vars[0],kBlue},
    {true,"sign_cor"  ,"Sig Cor",vars[1],kCyan},
    {false,"bkgd_uncor","Bkg Uncor",vars[0],kRed},
    {false,"bkgd_cor"  ,"Bkg Cor",vars[1],kMagenta}
  };

  outfile->cd();
  std::vector<TH1F*> hists;
  auto miny = 1e9, maxy = 1e-9;

  auto leg = new TLegend(0.7,0.7,0.98,0.98);
  leg->SetName("legend");

  for (const auto & histInfo : histInfos)
  {
    std::cout << "working on : " << histInfo.label.Data() << std::endl;
    const auto isSig = histInfo.isSig;
    auto & tree = (isSig? sign_tree : bkgd_tree);
    const auto & cut = (isSig ? sign_cut : bkgd_cut);

    outfile->cd();
    hists.emplace_back(new TH1F(Form("%s_%s_LT_phopt_LTE_%s_and_%s_LT_absphosceta_LTE_%s",histInfo.name.Data(),
				     pt.s_min.Data(),pt.s_max.Data(),eta.s_min.Data(),eta.s_max.Data()),
				"S_{Major};S_{Major};Events",40,0,2));
    //hists.emplace_back(new TH1F(histInfo.name,"SC #eta;SC #eta;Events",30,-1.5,1.5));
    //    hists.emplace_back(new TH1F(histInfo.name,"p_{T};p_{T};Events",100,0,1000));
    auto & hist = hists.back();
      
    hist->Sumw2();
    hist->SetMarkerStyle(20);
    hist->SetMarkerSize(0.6);
    hist->SetLineColor(histInfo.color);
    hist->SetMarkerColor(histInfo.color);

    const auto cuts = commoncut+"*"+cut;
    std::cout << "    cut: " << cuts.Data() << std::endl;
    std::cout << "    var: " << histInfo.var.Data() << std::endl;

    tree->Draw(Form("%s>>%s",histInfo.var.Data(),hist->GetName()),cuts,"goff");
    hist->Scale(1./hist->Integral());

    for (auto ibin = 1; ibin <= hist->GetXaxis()->GetNbins(); ibin++)
    {
      const auto content = hist->GetBinContent(ibin);
      if (content > maxy) maxy = content;
      if (content < miny && content > 0.0) miny = content;
    }

    leg->AddEntry(hist,histInfo.label.Data(),"epl");
  }

  // write it all
  Common::WriteVec(outfile,hists);
  Common::Write(outfile,leg);

  // draw log
  draw(outfile,hists,leg,pt,eta,miny,maxy,true);
  draw(outfile,hists,leg,pt,eta,miny,maxy,false);

  // delete it all
  delete leg;
}

void draw(TFile * outfile, std::vector<TH1F*> & hists, TLegend * leg, const Bound & pt, const Bound & eta,
	  const Double_t miny, const Double_t maxy, const Bool_t isLogy)
{
  // canvas
  outfile->cd();
  auto canv = new TCanvas();

  canv->cd();
  canv->SetName(Form(isLogy?"log":"lin"));
  canv->SetLogy(isLogy);
  canv->SetTickx();
  canv->SetTicky();

  for (auto ihist = 0; ihist < hists.size(); ihist++)
  {
    auto & hist = hists[ihist];
    hist->GetYaxis()->SetRangeUser((isLogy?(miny/1.5):(miny/1.05)),(isLogy?(maxy*1.5):(maxy*1.05)));
    hist->Draw(ihist>0?"ep same":"ep");
  }
  leg->Draw("same");

  Common::Write(outfile,canv);
  Common::SaveAs(canv,Form("smaj_%s_LT_phopt_LTE_%s_and_%s_LT_absphosceta_LTE_%s_%s",
			   pt.s_min.Data(),pt.s_max.Data(),eta.s_min.Data(),eta.s_max.Data(),canv->GetName()));
  //  Common::SaveAs(canv,Form("eta_%s",canv->GetName()));
  //Common::SaveAs(canv,Form("pt_sf_%s",canv->GetName()));
}
