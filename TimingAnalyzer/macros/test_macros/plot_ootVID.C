#include "plot_ootVID.hh"

void plot_ootVID()
{
  // style
  gStyle->SetOptStat(0);

  // i/o dirs
  const TString indir  = "skims/v4/ootVID";
  const TString outdir = "/eos/user/k/kmcdermo/www/dispho/plots/ootVID/new_corrs_smaj";

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

  // bkgd and signal inclusive effs
  auto outfile_inc = TFile::Open("plots_inc.root","RECREATE");
  make_inclusivePlots(bkgd_tree,sign_tree,outfile_inc);
  delete outfile_inc;
  gSystem->Exec("mv *png *pdf *root "+outdir+"/inc");

  // delete it all
  delete bkgd_tree;
  delete bkgd_file;
  delete sign_tree;
  delete sign_file;
}

void make_plots(TTree * tree, TFile * outfile, const Bool_t isRatios, const Bool_t isLoose, const Bool_t isSig)
{
  // min max
  const auto min = (isSig && !isRatios) ? 0.65 : 0.0;
  const auto max = (isSig && !isRatios) ? 1.01 : 1.05;

  // label
  const TString label = Form("%s_%s",(isSig?"sig":"bkg"),(isLoose?"loose":"tight"));
  const TString title = Form("%s %s %s Efficiency",(isSig?"Signal":"Background"),(isLoose?"Loose":"Tight"),(isRatios?"VID":"N-1"));

  // categories
  auto icolor = 0;
  std::vector<Category> categories;

  if (isRatios)
  {
    categories =
    {
      // hoe
      {isSig,"1",Config::observables.at(CutType::hoe).cut(isLoose),Config::observables.at(CutType::hoe).name,Config::observables.at(CutType::hoe).label,Common::ColorVec[icolor++]},

      // hoe + sieie
      {isSig,"1",Config::observables.at(CutType::hoe).cut(isLoose)+"&&"+Config::observables.at(CutType::sieie).cut(isLoose),Config::observables.at(CutType::sieie).name,Config::observables.at(CutType::sieie).label,Common::ColorVec[icolor++]},

      // hoe + sieie + ecal
      {isSig,"1",Config::observables.at(CutType::hoe).cut(isLoose)+"&&"+Config::observables.at(CutType::sieie).cut(isLoose)+"&&"+Config::observables.at(CutType::ecal).cut(isLoose),Config::observables.at(CutType::ecal).name,Config::observables.at(CutType::ecal).label,Common::ColorVec[icolor++]},

      // hoe + sieie + ecal + hcal
      {isSig,"1",Config::observables.at(CutType::hoe).cut(isLoose)+"&&"+Config::observables.at(CutType::sieie).cut(isLoose)+"&&"+Config::observables.at(CutType::ecal).cut(isLoose)+"&&"+Config::observables.at(CutType::hcal).cut(isLoose),Config::observables.at(CutType::hcal).name,Config::observables.at(CutType::hcal).label,Common::ColorVec[icolor++]},

      // hoe + sieie + ecal + hcal + trk
      {isSig,"1",Config::observables.at(CutType::hoe).cut(isLoose)+"&&"+Config::observables.at(CutType::sieie).cut(isLoose)+"&&"+Config::observables.at(CutType::ecal).cut(isLoose)+"&&"+Config::observables.at(CutType::hcal).cut(isLoose)+"&&"+Config::observables.at(CutType::trk).cut(isLoose),Config::observables.at(CutType::trk).name,Config::observables.at(CutType::trk).label,Common::ColorVec[icolor++]},

      // hoe + sieie + ecal + hcal + trk + smaj
      {isSig,"1",Config::observables.at(CutType::hoe).cut(isLoose)+"&&"+Config::observables.at(CutType::sieie).cut(isLoose)+"&&"+Config::observables.at(CutType::ecal).cut(isLoose)+"&&"+Config::observables.at(CutType::hcal).cut(isLoose)+"&&"+Config::observables.at(CutType::trk).cut(isLoose)+"&&"+Config::observables.at(CutType::smaj).cut(isLoose),Config::observables.at(CutType::smaj).name,Config::observables.at(CutType::smaj).label,Common::ColorVec[icolor++]}
    };
  }
  else
  {
    categories =
    {
      // hoe n-1
      {isSig,Config::observables.at(CutType::sieie).cut(isLoose)+"&&"+Config::observables.at(CutType::ecal).cut(isLoose)+"&&"+Config::observables.at(CutType::hcal).cut(isLoose)+"&&"+Config::observables.at(CutType::trk).cut(isLoose)+"&&"+Config::observables.at(CutType::smaj).cut(isLoose),Config::observables.at(CutType::hoe).cut(isLoose),Config::observables.at(CutType::hoe).name,Config::observables.at(CutType::hoe).label,Common::ColorVec[icolor++]},

      // sieie n-1
      {isSig,Config::observables.at(CutType::hoe).cut(isLoose)+"&&"+Config::observables.at(CutType::ecal).cut(isLoose)+"&&"+Config::observables.at(CutType::hcal).cut(isLoose)+"&&"+Config::observables.at(CutType::trk).cut(isLoose)+"&&"+Config::observables.at(CutType::smaj).cut(isLoose),Config::observables.at(CutType::sieie).cut(isLoose),Config::observables.at(CutType::sieie).name,Config::observables.at(CutType::sieie).label,Common::ColorVec[icolor++]},

      // ecal n-1
      {isSig,Config::observables.at(CutType::hoe).cut(isLoose)+"&&"+Config::observables.at(CutType::sieie).cut(isLoose)+"&&"+Config::observables.at(CutType::hcal).cut(isLoose)+"&&"+Config::observables.at(CutType::trk).cut(isLoose)+"&&"+Config::observables.at(CutType::smaj).cut(isLoose),Config::observables.at(CutType::ecal).cut(isLoose),Config::observables.at(CutType::ecal).name,Config::observables.at(CutType::ecal).label,Common::ColorVec[icolor++]},

      // hcal n-1
      {isSig,Config::observables.at(CutType::hoe).cut(isLoose)+"&&"+Config::observables.at(CutType::sieie).cut(isLoose)+"&&"+Config::observables.at(CutType::ecal).cut(isLoose)+"&&"+Config::observables.at(CutType::trk).cut(isLoose)+"&&"+Config::observables.at(CutType::smaj).cut(isLoose),Config::observables.at(CutType::hcal).cut(isLoose),Config::observables.at(CutType::hcal).name,Config::observables.at(CutType::hcal).label,Common::ColorVec[icolor++]},

      // trk n-1
      {isSig,Config::observables.at(CutType::hoe).cut(isLoose)+"&&"+Config::observables.at(CutType::sieie).cut(isLoose)+"&&"+Config::observables.at(CutType::ecal).cut(isLoose)+"&&"+Config::observables.at(CutType::hcal).cut(isLoose)+"&&"+Config::observables.at(CutType::smaj).cut(isLoose),Config::observables.at(CutType::trk).cut(isLoose),Config::observables.at(CutType::trk).name,Config::observables.at(CutType::trk).label,Common::ColorVec[icolor++]},

      // smaj n-1
      {isSig,Config::observables.at(CutType::hoe).cut(isLoose)+"&&"+Config::observables.at(CutType::sieie).cut(isLoose)+"&&"+Config::observables.at(CutType::ecal).cut(isLoose)+"&&"+Config::observables.at(CutType::hcal).cut(isLoose)+"&&"+Config::observables.at(CutType::trk).cut(isLoose),Config::observables.at(CutType::smaj).cut(isLoose),Config::observables.at(CutType::smaj).name,Config::observables.at(CutType::smaj).label,Common::ColorVec[icolor++]}
    };
  }

  // loop over plots
  for (const auto & plotinfoPair : Config::plotinfos)
  {
    const auto & plotinfo = plotinfoPair.second;

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

      const auto denom_cut = get_commoncut(isSig)+"*("+category.denom_cut+")";
      const auto numer_cut = get_commoncut(isSig)+"*("+category.denom_cut+"&&"+category.numer_cut+")";

      tree->Draw(plotinfo.var+">>denom",denom_cut.Data(),"goff");
      tree->Draw(plotinfo.var+">>numer",numer_cut.Data(),"goff");

      // make efficiency
      outfile->cd();
      hists.emplace_back(new TH1F(label+"_"+plotinfo.name+"_"+category.name,title+" vs. "+plotinfo.title,nbins,bins));
      auto & hist = hists.back();

      // set values
      set_eff(hist,numer,denom);

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
	ratios.emplace_back((TH1F*)hist->Clone(Form("%s_ratio",hist->GetName())));
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
    for (auto & hist : hists) hist->Write(hist->GetName(),TObject::kWriteDelete);

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

void make_inclusivePlots(TTree * bkgd_tree, TTree * sign_tree, TFile * outfile)
{
  // label config
  const TString ytitle = "Efficiency";

  // setup the different tests
  const std::vector<Category> categories =
  {
    // sig loose
    {true,"1",Config::observables.at(CutType::hoe).cut(true)+"&&"+Config::observables.at(CutType::sieie).cut(true)+"&&"+Config::observables.at(CutType::ecal).cut(true)+"&&"+Config::observables.at(CutType::hcal).cut(true)+"&&"+Config::observables.at(CutType::trk).cut(true)+"&&"+Config::observables.at(CutType::smaj).cut(true),"sig_loose","Signal Loose",kCyan},
    
    // sig tight
    {true,"1",Config::observables.at(CutType::hoe).cut(false)+"&&"+Config::observables.at(CutType::sieie).cut(false)+"&&"+Config::observables.at(CutType::ecal).cut(false)+"&&"+Config::observables.at(CutType::hcal).cut(false)+"&&"+Config::observables.at(CutType::trk).cut(false)+"&&"+Config::observables.at(CutType::smaj).cut(false),"sig_tight","Signal Tight",kBlue},

    // sig loose
    {false,"1",Config::observables.at(CutType::hoe).cut(true)+"&&"+Config::observables.at(CutType::sieie).cut(true)+"&&"+Config::observables.at(CutType::ecal).cut(true)+"&&"+Config::observables.at(CutType::hcal).cut(true)+"&&"+Config::observables.at(CutType::trk).cut(true)+"&&"+Config::observables.at(CutType::smaj).cut(true),"bkg_loose","Bkgd Loose",kMagenta},
    
    // sig tight
    {false,"1",Config::observables.at(CutType::hoe).cut(false)+"&&"+Config::observables.at(CutType::sieie).cut(false)+"&&"+Config::observables.at(CutType::ecal).cut(false)+"&&"+Config::observables.at(CutType::hcal).cut(false)+"&&"+Config::observables.at(CutType::trk).cut(false)+"&&"+Config::observables.at(CutType::smaj).cut(false),"bkg_tight","Bkgd Tight",kRed}
  };

  // make plots!!!
  for (const auto & plotinfoPair : Config::plotinfos)
  {
    const auto & plotinfo = plotinfoPair.second;

    std::cout << "Making plot for: " << plotinfo.var.Data() << std::endl;

    // make canvas
    auto canv = new TCanvas();
    canv->cd();

    // make legend
    auto leg = new TLegend(0.85,0.85,1.0,1.0);

    // make hists
    std::vector<TH1F*> hists;
    for (auto icat = 0U; icat < categories.size(); icat++)
    {
      const auto & category = categories[icat];
      std::cout << "   working on category: " << category.name.Data() << std::endl;
      
      
      const auto bins  = &plotinfo.bins[0];
      const auto nbins = plotinfo.bins.size()-1;
      
      auto denom = new TH1F("denom","",nbins,bins); denom->Sumw2();
      auto numer = new TH1F("numer","",nbins,bins); numer->Sumw2();

      const auto denom_cut = get_commoncut(category.isSig)+"*("+category.denom_cut+")";
      const auto numer_cut = get_commoncut(category.isSig)+"*("+category.denom_cut+"&&"+category.numer_cut+")";

      const auto & tree = (category.isSig ? sign_tree : bkgd_tree);
      tree->Draw(plotinfo.var+">>denom",denom_cut.Data(),"goff");
      tree->Draw(plotinfo.var+">>numer",numer_cut.Data(),"goff");

      // make efficiency
      outfile->cd();
      hists.emplace_back(new TH1F(plotinfo.name+"_"+category.name,ytitle+" vs. "+plotinfo.title,nbins,bins));
      auto & hist = hists.back();

      // set values
      set_eff(hist,numer,denom);

      // draw style
      hist->SetMarkerStyle(20);
      hist->SetMarkerSize(0.6);
      hist->SetLineColor(category.color);
      hist->SetMarkerColor(category.color);

      hist->GetXaxis()->SetTitle(plotinfo.title.Data());
      hist->GetYaxis()->SetTitle(ytitle.Data());
      hist->GetYaxis()->SetRangeUser(0.0,1.05);

      // draw
      canv->cd();
      hist->Draw(icat>0?"ep same":"ep");

      // add to legend
      leg->AddEntry(hist,category.label.Data(),"epl");
      
      // delete
      delete numer;
      delete denom;
    }

    // save it
    canv->cd();
    leg->Draw("same");
    Common::SaveAs(canv,plotinfo.name);
    
    // also to root file
    outfile->cd();
    for (auto & hist : hists) hist->Write(hist->GetName(),TObject::kWriteDelete);

    // delete it all
    for (auto & hist : hists) delete hist;
    delete leg;
    delete canv;
  }
}

TString get_commoncut(const Bool_t isSig)
{
  return Form("((puwgt%s)*(phoisOOT_0==0)*(phoisEB_0==1)*(phopt_0>70)*(phoisGen_0==%i))",(isSig?"":"*evtwgt"),(isSig?1:0));
}

void set_eff(TH1F * hist, const TH1F * numer, const TH1F * denom)
{
  for (auto ibin = 1; ibin <= hist->GetXaxis()->GetNbins(); ibin++)
  {
    if ((denom->GetBinContent(ibin) <= 0.0) || (numer->GetBinContent(ibin) <= 0.0)) continue;
    
    const auto prob = numer->GetBinContent(ibin)/denom->GetBinContent(ibin);
    const auto err  = std::sqrt(prob*(1.0-prob)/denom->GetBinContent(ibin));
    
    hist->SetBinContent(ibin,prob);
    hist->SetBinError  (ibin,err);
  }
}
