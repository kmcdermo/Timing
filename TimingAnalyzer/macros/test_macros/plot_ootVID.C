#include "plot_ootVID.hh"

void plot_ootVID()
{
  // style
  gStyle->SetOptStat(0);

  // i/o dirs
  const TString indir  = "skims/v4/ootVID";
  const TString eosdir = "/eos/user/k/kmcdermo/www";
  const TString outdir = "dispho/plots/ootVID_v3/efficiency";

  // get inputs
  const auto sign_file_name = indir+"/gmsb_isGen.root";
  auto sign_file = TFile::Open(sign_file_name);
  Common::CheckValidFile(sign_file,sign_file_name);
  
  const TString sign_tree_name = "GMSB_L200_CTau200_Tree";  
  auto sign_tree = (TTree*)sign_file->Get(sign_tree_name.Data());
  Common::CheckValidTree(sign_tree,Common::disphotreename,sign_file_name);

  const auto bkgd_file_name = indir+"/bkgd.root";
  auto bkgd_file = TFile::Open(bkgd_file_name);
  Common::CheckValidFile(bkgd_file,bkgd_file_name);

  const TString bkgd_tree_name = "GJets_Tree";
  auto bkgd_tree = (TTree*)bkgd_file->Get(bkgd_tree_name.Data());
  Common::CheckValidTree(bkgd_tree,Common::disphotreename,bkgd_file_name);

  //  make cut-flow efficiency plots: isGEDVID, isLoose, isSig
  auto outfile_CutFlows = TFile::Open("plots_CutFlows.root","RECREATE");
  
  // GED VID
  make_CutFlows(sign_tree,outfile_CutFlows,true,true,true);
  make_CutFlows(bkgd_tree,outfile_CutFlows,true,true,false);
  make_CutFlows(sign_tree,outfile_CutFlows,true,false,true);
  make_CutFlows(bkgd_tree,outfile_CutFlows,true,false,false);

  // OOT VID
  make_CutFlows(sign_tree,outfile_CutFlows,false,true,true);
  make_CutFlows(bkgd_tree,outfile_CutFlows,false,true,false);
  make_CutFlows(sign_tree,outfile_CutFlows,false,false,true);
  make_CutFlows(bkgd_tree,outfile_CutFlows,false,false,false);

  delete outfile_CutFlows;
  move_output(eosdir,outdir,"CutFlows");

  // bkgd and signal inclusive effs: isGEDVID
  auto outfile_InclusiveEffs = TFile::Open("plots_InclusiveEffs.root","RECREATE");

  make_InclusiveEffs(bkgd_tree,sign_tree,outfile_InclusiveEffs,true);
  make_InclusiveEffs(bkgd_tree,sign_tree,outfile_InclusiveEffs,false);

  delete outfile_InclusiveEffs;
  move_output(eosdir,outdir,"InclusiveEffs");

  // copy index.php
  gSystem->Exec("pushd "+eosdir+"; ./copyphp.sh "+outdir+"; popd;");

  // delete it all
  delete bkgd_tree;
  delete bkgd_file;
  delete sign_tree;
  delete sign_file;
}

void make_CutFlows(TTree * tree, TFile * outfile, const Bool_t isGEDVID, const Bool_t isLoose, const Bool_t isSig)
{
  // min max
  const auto min = 0.0;
  const auto max = 1.05;

  // label
  const TString label = Form("%s_%s_%s",(isSig?"sig":"bkg"),(isLoose?"loose":"tight"),(isGEDVID?"ged":"oot"));
  const TString title = Form("%s %s #sigma_{i#eta i#eta} %s VID Efficiency",(isSig?"Signal":"Background"),(isLoose?"Loose":"Tight"),(isGEDVID?"GED":"OOT"));

  // categories
  auto icolor = 0;
  std::vector<Category> categories;
  if (isGEDVID)
  {
    setup_CutFlowCategories(categories,isLoose,isSig,CutType::hoe,icolor++);
    setup_CutFlowCategories(categories,isLoose,isSig,CutType::chghad,icolor++);
    setup_CutFlowCategories(categories,isLoose,isSig,CutType::neuhad,icolor++);
    setup_CutFlowCategories(categories,isLoose,isSig,CutType::pho,icolor++);
    setup_CutFlowCategories(categories,isLoose,isSig,CutType::smaj,icolor++);
    setup_CutFlowCategories(categories,isLoose,isSig,CutType::smin,icolor++);
    setup_CutFlowCategories(categories,isLoose,isSig,CutType::sieie,icolor++);
  }
  else
  {
    setup_CutFlowCategories(categories,isLoose,isSig,CutType::hoe,icolor++);
    setup_CutFlowCategories(categories,isLoose,isSig,CutType::ecal,icolor++);
    setup_CutFlowCategories(categories,isLoose,isSig,CutType::hcal,icolor++);
    setup_CutFlowCategories(categories,isLoose,isSig,CutType::trk,icolor++);
    setup_CutFlowCategories(categories,isLoose,isSig,CutType::smaj,icolor++);
    setup_CutFlowCategories(categories,isLoose,isSig,CutType::smin,icolor++);
    setup_CutFlowCategories(categories,isLoose,isSig,CutType::sieie,icolor++);
  }

  // loop over plots
  for (const auto & plotinfoPair : Config::plotinfos)
  {
    const auto & plotinfo = plotinfoPair.second;

    std::cout << "Making " << (isSig?"Signal":"Background") 
	      << " " << (isLoose?"Loose":"Tight") << " Sieie"
	      << " " << (isGEDVID?"GED":"OOT") << " VID cut-flow"
	      << " plot for: " << plotinfo.var.Data() << std::endl;

    // make canvas
    auto canv = new TCanvas();
    canv->cd();

    auto upad = new TPad("upad","",Common::left_up,Common::bottom_up,Common::right_up,Common::top_up);
    upad->SetBottomMargin(Common::merged_margin);
    upad->Draw();
    upad->SetTickx();
    upad->SetTicky();
    
    auto lpad = new TPad("lpad","",Common::left_lp,Common::bottom_lp,Common::right_lp,Common::top_lp);
    lpad->SetTopMargin(Common::merged_margin);
    lpad->SetBottomMargin(Common::bottom_margin);
    lpad->Draw();
    lpad->SetTickx();
    lpad->SetTicky();

    // make legend
    auto leg = new TLegend(0.85,0.75,1.0,1.0);

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

      const auto denom_cut = get_commoncut(isSig,isGEDVID);
      const auto numer_cut = denom_cut+"*("+category.cut+")";

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

      hist->GetXaxis()->SetLabelSize(0);
      hist->GetXaxis()->SetTitleSize(0);
      hist->GetYaxis()->SetLabelSize  (Common::LabelSize / Common::height_up); 
      hist->GetYaxis()->SetTitleSize  (Common::TitleSize / Common::height_up);
      hist->GetYaxis()->SetTitleOffset(Common::TitleYOffset * Common::height_up);

      // draw
      canv->cd();
      upad->cd();
      hist->Draw(icat>0?"ep same":"ep");

      // add to legend
      leg->AddEntry(hist,category.label.Data(),"epl");
      
      // make ratio
      if (icat>0)
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
    for (auto & ratio : ratios) delete ratio;
    delete lpad;
    delete upad;
    delete canv;
  }
}

void make_InclusiveEffs(TTree * bkgd_tree, TTree * sign_tree, TFile * outfile, const Bool_t isGEDVID)
{
  // label config
  const TString ytitle = "Efficiency";

  // setup the different tests
  std::vector<Category> categories; 
  
  // isGEDVID, isLoose, isSig
  setup_InclusiveEffCategories(categories,isGEDVID,true,true);
  setup_InclusiveEffCategories(categories,isGEDVID,false,true);
  setup_InclusiveEffCategories(categories,isGEDVID,true,false);
  setup_InclusiveEffCategories(categories,isGEDVID,false,false);

  // make plots!!!
  for (const auto & plotinfoPair : Config::plotinfos)
  {
    const auto & plotinfo = plotinfoPair.second;

    std::cout << "Making plot for: " << plotinfo.var.Data() << std::endl;
    
    // make canvas
    auto canv = new TCanvas();
    canv->cd();
    canv->SetTickx();
    canv->SetTicky();
    
    // make legend
    auto leg = new TLegend(0.80,0.80,1.0,1.0);

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

      const auto denom_cut = get_commoncut(category.isSig,isGEDVID);
      const auto numer_cut = denom_cut+"*("+category.cut+")";

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
    Common::SaveAs(canv,Form("%s_%s",plotinfo.name.Data(),(isGEDVID?"ged":"oot")));
    
    // also to root file
    outfile->cd();
    for (auto & hist : hists) hist->Write(hist->GetName(),TObject::kWriteDelete);

    // delete it all
    for (auto & hist : hists) delete hist;
    delete leg;
    delete canv;
  }
}

TString get_commoncut(const Bool_t isSig, const Bool_t isGEDVID)
{
  return Form("((puwgt%s)*(%s(phoisEB_0==1)&&(phopt_0>70)&&(phopt_0<500)&&(phoisGen_0==%i)))",(isSig?"":"*evtwgt"),(isGEDVID?"(phoisOOT_0==0)&&":""),(isSig?1:0));
}

void setup_CutFlowCategories(std::vector<Category> & categories, const Bool_t isLoose, const Bool_t isSig, const CutType cutType, const Int_t icolor)
{
  categories.emplace_back(isSig,Form("%s%s",(categories.size()>0?Form("%s&&",categories.back().cut.Data()):""),Config::observables.at(cutType).cut(isLoose).Data()),Config::observables.at(cutType).name,Config::observables.at(cutType).label,Common::ColorVec[icolor]);
}

void setup_InclusiveEffCategories(std::vector<Category> & categories, const Bool_t isGEDVID, const Bool_t isLoose, const Bool_t isSig)
{
  const TString name  = Form("%s_%s_%s",(isSig?"sig":"bkg"),(isLoose?"loose":"tight"),(isGEDVID?"ged":"oot"));
  const TString label = Form("%s %s #sigma_{i#eta i#eta}",(isSig?"Signal":"Background"),(isLoose?"Loose":"Tight"));
  const auto color = (isSig?(isLoose?kAzure+10:kBlue):(isLoose?kMagenta:kRed+1));

  if (isGEDVID)
  {
    categories.emplace_back(isSig,Config::observables.at(CutType::hoe).cut(isLoose)+"&&"+Config::observables.at(CutType::chghad).cut(isLoose)+"&&"+Config::observables.at(CutType::neuhad).cut(isLoose)+"&&"+Config::observables.at(CutType::pho).cut(isLoose)+"&&"+Config::observables.at(CutType::smaj).cut(isLoose)+"&&"+Config::observables.at(CutType::smin).cut(isLoose)+"&&"+Config::observables.at(CutType::sieie).cut(isLoose),name,label,color);
  }
  else
  {
    categories.emplace_back(isSig,Config::observables.at(CutType::hoe).cut(isLoose)+"&&"+Config::observables.at(CutType::ecal).cut(isLoose)+"&&"+Config::observables.at(CutType::hcal).cut(isLoose)+"&&"+Config::observables.at(CutType::trk).cut(isLoose)+"&&"+Config::observables.at(CutType::smaj).cut(isLoose)+"&&"+Config::observables.at(CutType::smin).cut(isLoose)+"&&"+Config::observables.at(CutType::sieie).cut(isLoose),name,label,color);
  }
}

void set_eff(TH1F * hist, const TH1F * numer, const TH1F * denom)
{
  for (auto ibin = 1; ibin <= hist->GetXaxis()->GetNbins(); ibin++)
  {
    if ((denom->GetBinContent(ibin) <= 0.0) || (numer->GetBinContent(ibin) <= 0.0)) continue;
    
    const auto prob = numer->GetBinContent(ibin)/denom->GetBinContent(ibin);
    const auto err  = 1.96*std::sqrt(prob*(1.0-prob)/denom->GetBinContent(ibin));
    
    hist->SetBinContent(ibin,prob);
    hist->SetBinError  (ibin,err);
  }
}

void move_output(const TString & eosdir, const TString & outdir, const TString & dir)
{
  const auto fulldir = eosdir+"/"+outdir+"/"+dir;
  gSystem->Exec("mkdir -p "+fulldir);
  gSystem->Exec("mv *png *pdf *root "+fulldir);
}
