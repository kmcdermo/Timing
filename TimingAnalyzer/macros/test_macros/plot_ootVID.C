#include "Common.cpp+"

struct Observable
{
  Observable(const TString & var, const TString & inequality, const TString & threshold, const TString & name, const TString & label)
    : name(name), label(label) { cut = "(("+var+")"+inequality+threshold+")"; }

  TString name;
  TString label;
  TString cut;
};

struct Category
{
  Category() {}
  Category(const Bool_t isSig, const TString & denom_cut, const TString & numer_cut, const TString & name, const TString & label, const Color_t color)
    : isSig(isSig), denom_cut(denom_cut), numer_cut(numer_cut), name(name), label(label), color(color) {}

  Bool_t  isSig;
  TString denom_cut;
  TString numer_cut;
  TString name;
  TString label;
  Color_t color;
};

struct PlotInfo
{
  PlotInfo() {}
  PlotInfo(const TString & name, const TString & var, const std::vector<Double_t> & bins, const TString & title)
    : name(name), var(var), bins(bins), title(title) {}
  
  TString name;
  TString var;
  std::vector<Double_t> bins;
  TString title;
};

void make_plots(TTree * tree, TFile * outfile, const Bool_t isRatios, const Bool_t isLoose, const Bool_t isSig);
void plot_ootVID()
{
  // style
  gStyle->SetOptStat(0);

  // i/o dirs
  const TString indir  = "skims/v4/ootVID";
  const TString outdir = "/eos/user/k/kmcdermo/www/dispho/plots/ootVID/new_corrs/gen_matched";

  // get inputs
  const auto sign_file_name = indir+"/gmsb.root";
  auto sign_file = TFile::Open(sign_file_name);
  Common::CheckValidFile(sign_file,sign_file_name);
  
  const TString sign_tree_name = "GMSB_L200_CTau200_Tree";  
  auto sign_tree = (TTree*)sign_file->Get(sign_tree_name.Data());
  Common::CheckValidTree(sign_tree,Common::disphotreename,sign_file_name);

  const auto bkgd_file_name = indir+"/gjet.root";
  auto bkgd_file = TFile::Open(bkgd_file_name);
  Common::CheckValidFile(bkgd_file,bkgd_file_name);

  const TString bkgd_tree_name = "GJets_Tree";
  auto bkgd_tree = (TTree*)bkgd_file->Get(bkgd_tree_name.Data());
  Common::CheckValidTree(bkgd_tree,Common::disphotreename,bkgd_file_name);

  // make plots: isRatio, isLoose, isSig
  auto outfile_nm1 = TFile::Open("plots_nm1.root","RECREATE");
  make_plots(sign_tree,outfile_nm1,false,true,true);
  make_plots(sign_tree,outfile_nm1,false,false,true);
  delete outfile_nm1;
  gSystem->Exec("mv *png *pdf *root "+outdir+"/nm1");

  // make full vid (remake outfile)
  auto outfile_vid = TFile::Open("plots_vid.root","RECREATE");
  make_plots(sign_tree,outfile_vid,true,true,true);
  make_plots(sign_tree,outfile_vid,true,false,true);
  delete outfile_vid;
  gSystem->Exec("mv *png *pdf *root "+outdir+"/vid");

  // delete it all
  delete bkgd_tree;
  delete bkgd_file;
  delete sign_tree;
  delete sign_file;
}

void make_plots(TTree * tree, TFile * outfile, const Bool_t isRatios, const Bool_t isLoose, const Bool_t isSig)
{
  // cut observables
  const auto hoe   = Observable("phoHoE_0","<",(isLoose?"0.0185":"0.0165"),"HoE","H/E");
  const auto sieie = Observable("phosieie_0","<",(isLoose?"0.0125":"0.011"),"sieie","#sigma_{i#eta i#eta}");
  const auto ecal  = Observable("max(phoEcalPFClIso_0-(0.001578*phopt_0)-(0.1073*rho)*(abs(phosceta_0)<0.8)-(0.08317*rho)*((abs(phosceta_0)>=0.8)*(abs(phosceta_0)<1.4442)),0.0)","<",(isLoose?"8.0" :"5.0" ),"ecalpfcliso","ECAL PF Cluster Iso");
  const auto hcal  = Observable("max(phoHcalPFClIso_0-((1.691e-5*phopt_0*phopt_0)+(-0.002597*phopt_0))-(0.073*rho)*(abs(phosceta_0)<0.8)-(0.07983*rho)*((abs(phosceta_0)>=0.8)*(abs(phosceta_0)<1.4442)),0.0)","<",(isLoose?"12.0":"10.0"),"hcalpfcliso","HCAL PF Cluster Iso");
  const auto trk   = Observable("max(phoTrkIso_0-(0.01147*rho)*(abs(phosceta_0)<0.8)-(0.005256*rho)*((abs(phosceta_0)>=0.8)*(abs(phosceta_0)<1.4442)),0.0)","<",(isLoose?"8.5":"5.5"),"trkiso","Tracker Iso");
  const auto smaj  = Observable("phosmaj_0","<",(isLoose?"1.0":"0.5"),"smaj","S_{Major}");

  // min max
  const auto min = (isSig && !isRatios) ? 0.65 : 0.0;
  const auto max = (isSig && !isRatios) ? 1.01 : 1.05;

  // label
  const TString label = Form("%s_%s",(isSig?"sig":"bkg"),(isLoose?"loose":"tight"));
  const TString title = Form("%s %s %s Efficiency",(isSig?"Signal":"Background"),(isLoose?"Loose":"Tight"),(isRatios?"VID":"N-1"));

  // common cut
  const TString common_cut = Form("((puwgt%s)*(phoisOOT_0==0)*(phoisEB_0)*(phopt_0>70)*(phoisGen_0==%i))",(isSig?"":"*evtwgt"),(isSig?1:0));

  // categories
  auto icolor = 0;
  std::vector<Category> categories;

  if (isRatios)
  {
    categories =
    {
      {isSig,"1",hoe.cut,hoe.name,hoe.label,Common::ColorVec[icolor++]},
      {isSig,"1",hoe.cut+"&&"+sieie.cut,sieie.name,sieie.label,Common::ColorVec[icolor++]},
      {isSig,"1",hoe.cut+"&&"+sieie.cut+"&&"+ecal.cut,ecal.name,ecal.label,Common::ColorVec[icolor++]},
      {isSig,"1",hoe.cut+"&&"+sieie.cut+"&&"+ecal.cut+"&&"+hcal.cut,hcal.name,hcal.label,Common::ColorVec[icolor++]},
      {isSig,"1",hoe.cut+"&&"+sieie.cut+"&&"+ecal.cut+"&&"+hcal.cut+"&&"+trk.cut,trk.name,trk.label,Common::ColorVec[icolor++]},
      {isSig,"1",hoe.cut+"&&"+sieie.cut+"&&"+ecal.cut+"&&"+hcal.cut+"&&"+trk.cut+"&&"+smaj.cut,smaj.name,smaj.label,Common::ColorVec[icolor++]},
    };
  }
  else
  {
    categories =
    {
      {isSig,sieie.cut+"&&"+ecal.cut+"&&"+hcal.cut+"&&"+trk.cut+"&&"+smaj.cut,hoe.cut,hoe  .name,hoe  .label,Common::ColorVec[icolor++]},
      {isSig,hoe.cut+"&&"+ecal.cut+"&&"+hcal.cut+"&&"+trk.cut+"&&"+smaj.cut,sieie.cut,sieie.name,sieie.label,Common::ColorVec[icolor++]},
      {isSig,hoe.cut+"&&"+sieie.cut+"&&"+hcal.cut+"&&"+trk.cut+"&&"+smaj.cut,ecal.cut,ecal .name,ecal .label,Common::ColorVec[icolor++]},
      {isSig,hoe.cut+"&&"+sieie.cut+"&&"+ecal.cut+"&&"+trk.cut+"&&"+smaj.cut,hcal.cut,hcal .name,hcal .label,Common::ColorVec[icolor++]},
      {isSig,hoe.cut+"&&"+sieie.cut+"&&"+ecal.cut+"&&"+hcal.cut+"&&"+smaj.cut,trk.cut,trk  .name,trk  .label,Common::ColorVec[icolor++]},
      {isSig,hoe.cut+"&&"+sieie.cut+"&&"+ecal.cut+"&&"+hcal.cut+"&&"+trk.cut,smaj.cut,smaj .name,smaj .label,Common::ColorVec[icolor++]},
    };
  }

  // make plot vector
  const std::vector<Double_t> ptbins = {0,20,40,60,80,100,125,150,200,250,300,400,500,750,1000,2000};
  const std::vector<Double_t> etabins = {-1.5,-1.4,-1.3,-1.2,-1.1,-1.0,-0.9,-0.8,-0.7,-0.6,-0.5,-0.4,-0.3,-0.2,-0.1,0.0,0.1,0.2,0.3,0.4,0.5,0.6,0.7,0.8,0.9,1.0,1.1,1.2,1.3,1.4,1.5};
  const std::vector<Double_t> nvtxbins = {0,2,4,6,8,10,12,14,16,18,20,22,24,26,28,30,35,40,45,50,60};
  const std::vector<Double_t> timebins = {-2,-1.5,-1,-0.75,-0.5,-0.25,0,0.25,0.5,0.75,1,1.5,2,3,5,10,15,25};

  std::vector<PlotInfo> plotinfos = 
  {
    {"phopt_0","phopt_0",ptbins,"Leading Photon p_{T} [GeV]"},
    {"phoeta_0","phoeta_0",etabins,"Leading Photon #eta"},
    {"nvtx_0","nvtx",nvtxbins,"Number of Vertices"},
    {"phoweightedtimeLT120_0","phoweightedtimeLT120_0+phoweightedtimeLT120SHIFT_0+phoweightedtimeLT120SMEAR_0",timebins,"Leading Photon Cluster Time [ns]"}
  };

  // loop over plots
  for (const auto & plotinfo : plotinfos)
  {
    std::cout << "Making " << (isSig?"Signal":"Background") 
	      << " " << (isRatios?"VID Eff":"N-1") 
	      << " [" << (isLoose?"Loose":"Tight") << "]"
	      << " plot for: " << plotinfo.var.Data() << std::endl;

    // make canvas
    auto canv = new TCanvas();
    canv->cd();

    TPad * upad = NULL, * lpad = NULL;
    if (isRatios)
    {
      upad = new TPad("upad","", Common::left_up, Common::bottom_up, Common::right_up, Common::top_up);
      upad->SetBottomMargin(Common::merged_margin);
      upad->Draw();
      
      lpad = new TPad("lpad", "", Common::left_lp, Common::bottom_lp, Common::right_lp, Common::top_lp);
      lpad->SetTopMargin(Common::merged_margin);
      lpad->SetBottomMargin(Common::bottom_margin);
      lpad->Draw();
    }

    // make legend
    auto leg = new TLegend(0.85,0.85,1.0,1.0);

    // make hists
    std::vector<TH1F*> hists;
    std::vector<TH1F*> ratios;
    for (auto icat = 0U; icat < categories.size(); icat++)
    {
      const auto & category = categories[icat];
      std::cout << "   working on category: " << category.name.Data() << std::endl;
      
      const auto bins  = &plotinfo.bins[0];
      const auto nbins = plotinfo.bins.size()-1;
      
      auto denom = new TH1F("denom","",nbins,bins); denom->Sumw2();
      auto numer = new TH1F("numer","",nbins,bins); numer->Sumw2();

      const auto denom_cut = common_cut+"*("+category.denom_cut+")";
      const auto numer_cut = common_cut+"*("+category.denom_cut+"&&"+category.numer_cut+")";

      tree->Draw(plotinfo.var+">>denom",denom_cut.Data(),"goff");
      tree->Draw(plotinfo.var+">>numer",numer_cut.Data(),"goff");

      // make efficiency
      outfile->cd();
      hists.emplace_back(new TH1F(category.name.Data(),title+" vs. "+plotinfo.title,nbins,bins));
      auto & hist = hists.back();

      // set values
      for (auto ibin = 1; ibin <= hist->GetXaxis()->GetNbins(); ibin++)
      {
	if ((denom->GetBinContent(ibin) <= 0.0) || (numer->GetBinContent(ibin) <= 0.0)) continue;

	const auto prob = numer->GetBinContent(ibin)/denom->GetBinContent(ibin);
	const auto err  = std::sqrt(prob*(1.0-prob)/denom->GetBinContent(ibin));
	
	hist->SetBinContent(ibin,prob);
	hist->SetBinError  (ibin,err);
      }

      // draw style
      hist->SetMarkerStyle(20);
      hist->SetMarkerSize(0.6);
      hist->SetLineColor(category.color);
      hist->SetMarkerColor(category.color);

      hist->GetXaxis()->SetTitle(plotinfo.title.Data());
      hist->GetYaxis()->SetTitle("Efficiency");
      hist->GetYaxis()->SetRangeUser(min,max);

      if (isRatios)
      {
	hist->GetXaxis()->SetLabelSize(0);
	hist->GetXaxis()->SetTitleSize(0);
	hist->GetYaxis()->SetLabelSize  (Common::LabelSize / Common::height_up); 
	hist->GetYaxis()->SetTitleSize  (Common::TitleSize / Common::height_up);
	hist->GetYaxis()->SetTitleOffset(Common::TitleYOffset * Common::height_up);
      }

      // draw
      canv->cd();
      if (isRatios) upad->cd();
      hist->Draw(icat>0?"ep same":"ep");

      // add to legend
      leg->AddEntry(hist,category.label.Data(),"epl");
      
      // make ratio
      if (icat>0 && isRatios)
      {
	ratios.emplace_back((TH1F*)hist->Clone(category.name+"_ratio"));
	auto & ratio = ratios.back();
	ratio->Divide(hists[icat-1]);
	ratio->SetTitle("");

	// draw style
	ratio->GetXaxis()->SetLabelSize  (Common::LabelSize   / Common::height_lp); 
	ratio->GetXaxis()->SetLabelOffset(Common::LabelOffset / Common::height_lp); 
	ratio->GetXaxis()->SetTitleSize  (Common::TitleSize   / Common::height_lp);
	ratio->GetXaxis()->SetTickLength (Common::TickLength  / Common::height_lp);

	ratio->GetYaxis()->SetTitle("N/N-1");
	ratio->GetYaxis()->SetLabelSize  (Common::LabelSize   / Common::height_lp); 
	ratio->GetYaxis()->SetTitleSize  (Common::TitleSize   / Common::height_lp);
	ratio->GetYaxis()->SetTitleOffset(Common::TitleYOffset * Common::height_lp);
	ratio->GetYaxis()->SetNdivisions(505);

	// draw
	canv->cd();
	lpad->cd();
	ratio->Draw(icat>1?"ep same":"ep");
      }

      // delete
      delete numer;
      delete denom;
    }

    // save it
    canv->cd();
    leg->Draw("same");
    Common::SaveAs(canv,label+"_"+plotinfo.name);
    
    // also to root file
    outfile->cd();
    for (auto & hist : hists) hist->Write(Form("%s_%s_%s",label.Data(),plotinfo.name.Data(),hist->GetName()),TObject::kWriteDelete);

    // delete it all
    for (auto & hist : hists) delete hist;
    delete leg;
    if (isRatios) 
    {
      for (auto & ratio : ratios) delete ratio;
      delete lpad;
      delete upad;
    }
    delete canv;
  }
}
