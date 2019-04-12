#include "Common.cpp+"

void makeHist(TH1F *& hist, const TString & name, const TString & label, const Color_t color);
void fillHists(TTree * tree, TFile * outfile, std::vector<TH1F*> & before_hists, std::vector<TH1F*> & after_hists, 
	       const TString & sample, const Bool_t isOOT);
void getMinMax(const TH1F * hist, Double_t & min, Double_t & max);
void setMinMax(TH1F * hist, const Double_t min, const Double_t max);

void comparePhotonEresiduals(const TString & indir, const TString & outfiletext)
{
  gStyle->SetOptStat("nemMrR");

  const std::vector<TString> samples = {"L200_CTau10", "L200_CTau200", "L200_CTau1000", "L400_CTau10", "L400_CTau200", "L400_CTau1000"};
  auto outfile = TFile::Open(outfiletext+".root","UPDATE");

  for (const auto & sample : samples)
  {
    std::cout << "Working on sample: " << sample.Data() << std::endl;

    // get file
    const auto infilename = indir+"/"+sample+".root";
    auto infile = TFile::Open(infilename.Data());
    Common::CheckValidFile(infile,infilename);

    // get tree
    const TString treename = "tree/disphotree";
    auto tree = (TTree*)infile->Get(treename.Data());
    Common::CheckValidTree(tree,treename,infilename);

    // make histograms
    outfile->cd();
    std::vector<TH1F*> before_ged_hists(Common::nPhotons), after_ged_hists(Common::nPhotons);
    std::vector<TH1F*> before_oot_hists(Common::nPhotons), after_oot_hists(Common::nPhotons);

    // loop photons
    fillHists(tree,outfile,before_ged_hists,after_ged_hists,sample,false);
    fillHists(tree,outfile,before_oot_hists,after_oot_hists,sample,true);
    
    // make final hists
    outfile->cd();
    TH1F * before_ged_hist = NULL, * after_ged_hist = NULL;
    TH1F * before_oot_hist = NULL, * after_oot_hist = NULL;
    makeHist(before_ged_hist,"Uncorrected (GED)","Reco",kRed+1);
    makeHist(after_ged_hist ,"Corrected (GED)","Reco",kBlue);
    makeHist(before_oot_hist,"Uncorrected (OOT)","Reco",kMagenta);
    makeHist(after_oot_hist ,"Corrected (OOT)","Reco",kAzure+10);
    
    // add hists
    for (auto ipho = 0; ipho < Common::nPhotons; ipho++)
    {
      before_ged_hist->Add(before_ged_hists[ipho]);
      after_ged_hist ->Add(after_ged_hists [ipho]);
      before_oot_hist->Add(before_oot_hists[ipho]);
      after_oot_hist ->Add(after_oot_hists [ipho]);
    }
    
    // scale
    before_ged_hist->Scale(1.f/before_ged_hist->Integral());
    after_ged_hist ->Scale(1.f/after_ged_hist ->Integral());
    before_oot_hist->Scale(1.f/before_oot_hist->Integral());
    after_oot_hist ->Scale(1.f/after_oot_hist ->Integral());
      
    // get max / set max
    auto min = 0.0, max = -1.0e9;
    getMinMax(before_ged_hist,min,max);
    getMinMax(after_ged_hist ,min,max);
    getMinMax(before_oot_hist,min,max);
    getMinMax(after_oot_hist ,min,max);

    setMinMax(before_ged_hist,min,max);
    setMinMax(after_ged_hist ,min,max);
    setMinMax(before_oot_hist,min,max);
    setMinMax(after_oot_hist ,min,max);

    // legend
    auto leg = new TLegend(0.14,0.52,0.37,0.86);
    leg->SetName("leg_"+sample);
    leg->AddEntry(before_ged_hist,"Uncorrected (GED)","epl");
    leg->AddEntry(after_ged_hist ,"Corrected (GED)","epl");
    leg->AddEntry(before_oot_hist,"Uncorrected (OOT)","epl");
    leg->AddEntry(after_oot_hist ,"Corrected (OOT)","epl");

    // canvas
    auto canv = new TCanvas();
    canv->SetName("canv_"+sample);
    canv->cd();
    canv->SetTickx();
    canv->SetTicky();

    // draw
    before_ged_hist->Draw("ep");
    after_ged_hist ->Draw("ep sames");
    before_oot_hist->Draw("ep sames");
    after_oot_hist ->Draw("ep sames");
    leg->Draw("same");
    canv->Update();

    // set stats
    auto before_ged_stats = (TPaveStats*)(before_ged_hist->GetListOfFunctions()->FindObject("stats"));
    auto after_ged_stats  = (TPaveStats*)(after_ged_hist ->GetListOfFunctions()->FindObject("stats"));
    auto before_oot_stats = (TPaveStats*)(before_oot_hist->GetListOfFunctions()->FindObject("stats"));
    auto after_oot_stats  = (TPaveStats*)(after_oot_hist ->GetListOfFunctions()->FindObject("stats"));

    // ged stats
    before_ged_stats->SetX1NDC(0.52);
    before_ged_stats->SetX2NDC(0.68);
    after_ged_stats->SetX1NDC(before_ged_stats->GetX1NDC());
    after_ged_stats->SetX2NDC(before_ged_stats->GetX2NDC());

    before_ged_stats->SetY1NDC(0.70); before_ged_stats->SetY2NDC(0.86);
    const auto height = before_ged_stats->GetY2NDC() - before_ged_stats->GetY1NDC();
    const auto h_gap = 0.02;
    after_ged_stats->SetY1NDC(before_ged_stats->GetY1NDC() - height - h_gap);
    after_ged_stats->SetY2NDC(before_ged_stats->GetY1NDC() - h_gap);

    // oot stats
    const auto length = before_ged_stats->GetX2NDC() - before_ged_stats->GetX1NDC();
    const auto l_gap = 0.02;
    before_oot_stats->SetX1NDC(before_ged_stats->GetX2NDC() + l_gap);
    before_oot_stats->SetX2NDC(before_ged_stats->GetX2NDC() + length + l_gap);
    after_oot_stats->SetX1NDC(before_oot_stats->GetX1NDC());
    after_oot_stats->SetX2NDC(before_oot_stats->GetX2NDC());

    before_oot_stats->SetY1NDC(before_ged_stats->GetY1NDC()); before_oot_stats->SetY2NDC(before_ged_stats->GetY2NDC()); 
    after_oot_stats ->SetY1NDC(after_ged_stats ->GetY1NDC()); after_oot_stats ->SetY2NDC(after_ged_stats ->GetY2NDC()); 

    // Save Canvas
    Common::SaveAs(canv,outfiletext+"_"+sample);
    
    // Rename hists (named the same for stats boxes
    before_ged_hist->SetName("before_ged_"+sample);
    after_ged_hist ->SetName("after_ged_" +sample);
    before_oot_hist->SetName("before_oot_"+sample);
    after_oot_hist ->SetName("after_oot_" +sample);

    // write to file
    Common::WriteVec(outfile,before_ged_hists);
    Common::WriteVec(outfile,after_ged_hists);
    Common::WriteVec(outfile,before_oot_hists);
    Common::WriteVec(outfile,after_oot_hists);
    Common::Write(outfile,before_ged_hist);
    Common::Write(outfile,after_ged_hist);
    Common::Write(outfile,before_oot_hist);
    Common::Write(outfile,after_oot_hist);
    Common::Write(outfile,leg);
    Common::Write(outfile,canv);
    
    // delete it all
    delete after_oot_stats;
    delete before_oot_stats;
    delete after_ged_stats;
    delete before_ged_stats;
    delete canv;
    delete leg;
    delete after_oot_hist;
    delete before_oot_hist;
    delete after_ged_hist;
    delete before_ged_hist;
    Common::DeleteVec(after_ged_hists);
    Common::DeleteVec(before_ged_hists);
    Common::DeleteVec(after_oot_hists);
    Common::DeleteVec(before_oot_hists);
    delete tree;
    delete infile;
  }

  // delete it all
  delete outfile;
}

void makeHist(TH1F *& hist, const TString & name, const TString & label, const Color_t color)
{
  hist = new TH1F(name.Data(),"Photon Energy Residual [GeV];E^{#gamma}_{GEN}-E^{#gamma}_{"+label+"} [GeV];Fraction of Events",50,-250,250);
  hist->Sumw2();
  hist->SetLineColor(color);
  hist->SetMarkerColor(color);
}

void fillHists(TTree * tree, TFile * outfile, std::vector<TH1F*> & before_hists, std::vector<TH1F*> & after_hists,
	       const TString & sample, const Bool_t isOOT)
{
  for (auto ipho = 0; ipho < Common::nPhotons; ipho++)
  {
    std::cout << "  Working on photon: " << ipho << " [isOOT: " << Common::PrintBool(isOOT).Data() << "]" << std::endl;
    
    // extra label
    const TString label = (isOOT ? "oot" : "ged");
    const Color_t before_color = (isOOT ? kRed+1: kMagenta);
    const Color_t after_color  = (isOOT ? kBlue : kAzure+10);

    // setup hists
    outfile->cd();
    auto & before_hist = before_hists[ipho];
    auto & after_hist  = after_hists [ipho];
    makeHist(before_hist,Form("%s_pho_%i_before_%s",label.Data(),ipho,sample.Data()),Form("Reco %i (Before)",ipho),before_color);
    makeHist(after_hist ,Form("%s_pho_%i_after_%s" ,label.Data(),ipho,sample.Data()),Form("Reco %i (After)" ,ipho),after_color);

    // read trees : before
    tree->Draw(Form("(((phoE_%i/phoECorrFactor_%i)-genphE_0)*(phoisSignal_%i==1)+(((phoE_%i/phoECorrFactor_%i)-genphE_1)*(phoisSignal_%i==2)))>>%s",
		    ipho,ipho,ipho,ipho,ipho,ipho,before_hist->GetName()),
	       Form("phoisOOT_%i==%s&&(phoisSignal_%i==1||phoisSignal_%i==2)",ipho,(isOOT?"1":"0"),ipho,ipho),"goff");
    
    // read trees : after
    tree->Draw(Form("((phoE_%i-genphE_0)*(phoisSignal_%i==1)+((phoE_%i-genphE_1)*(phoisSignal_%i==2)))>>%s",
		    ipho,ipho,ipho,ipho,after_hist->GetName()),
	       Form("phoisOOT_%i==%s&&(phoisSignal_%i==1||phoisSignal_%i==2)",ipho,(isOOT?"1":"0"),ipho,ipho),"goff");      
  }
}

void getMinMax(const TH1F * hist, Double_t & min, Double_t & max)
{
  for (auto ibin = 1; ibin <= hist->GetXaxis()->GetNbins(); ibin++)
  {
    const auto content = hist->GetBinContent(ibin);
    
    //    if (content < min && content > 0.0) min = content;
    if (content > max && content > 0.0) max = content;
  }
}

void setMinMax(TH1F * hist, const Double_t min, const Double_t max)
{
  hist->GetYaxis()->SetRangeUser(min/1.2f,max*1.2f);
}
