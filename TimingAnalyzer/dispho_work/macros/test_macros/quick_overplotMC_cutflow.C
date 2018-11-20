#include "Common.cpp+"

void doQuick(const TString & mc, const TString & cr, const TString & label);
void dump(const TH1F * hist, const TString & samplename);
void setupMC(std::vector<TString> & samples, const TString mc);

void quick()
{
  // doQuick("qcd","qcd","hlt70_pt90");
  // doQuick("gjets","qcd","hlt70_pt90");

  //  doQuick("qcd","gjets","hlt70_pt90");
  doQuick("gjets","gjets","hlt70_pt80");
}

void doQuick(const TString & mc, const TString & cr, const TString & label)
{
  gStyle->SetOptStat(0);

  auto file = TFile::Open(cr+"_2pho_"+label+".root");
  const TString outname = mc+"_mc_"+cr+"_cr_cutflow_"+label+"_split.png";

  auto canv = new TCanvas();
  canv->cd();
  canv->SetLogy();
  canv->SetTickx();
  canv->SetTicky();
  auto leg = new TLegend(0.75,0.75,0.99,0.99);

  std::vector<TString> samples;
  setupMC(samples,mc);
  
  std::vector<TH1F*> hists;
  auto i = 0;
  for (const auto & sample : samples)
  {
    const auto samplename = Common::ReplaceSlashWithUnderscore(sample);

    const TString histname = samplename+"_"+Common::h_cutflow_scaledname;
    hists.emplace_back((TH1F*)file->Get(histname.Data()));
    auto & hist = hists.back();
    
    auto & color = Common::ColorVec[i];
    hist->SetLineColor(color);
    hist->SetMarkerColor(color);

    hist->GetYaxis()->SetRangeUser(1e-5,1e12);
    hist->Draw(i>0?"ep same":"ep");
    leg->AddEntry(hist,samplename.Data(),"epl");

    dump(hist,samplename);
    
    i++;
  }

  leg->Draw("same");
  canv->SaveAs(outname.Data());

  // delete
  for (auto & hist : hists) delete hist;
  delete leg;
  delete canv;
  delete file;
}

void setupMC(std::vector<TString> & samples, const TString mc)
{
  if (mc.EqualTo("qcd"))
  {
    samples = {"MC/QCD_HT/100to200",
	       "MC/QCD_HT/200to300",
	       "MC/QCD_HT/300to500",
	       "MC/QCD_HT/500to700",
	       "MC/QCD_HT/700to1000",
	       "MC/QCD_HT/1000to1500",
	       "MC/QCD_HT/1500to2000",
	       "MC/QCD_HT/2000toInf"};
  }
  else if (mc.EqualTo("gjets"))
  {
    samples = {"MC/GJets_HT/40To100",
	       "MC/GJets_HT/100To200",
	       "MC/GJets_HT/200To400",
	       "MC/GJets_HT/400To600",
	       "MC/GJets_HT/600ToInf"};
  }
  else
  {
    std::cerr << mc.Data() << " is bad! Exiting... " << std::endl;
    exit(1);
  }
}

void dump(const TH1F * hist, const TString & samplename)
{
  std::ofstream output(Form("%s.txt",samplename.Data()),std::ios_base::trunc);

  for (Int_t ibinX = 1; ibinX <= hist->GetXaxis()->GetNbins(); ibinX++)
  {
    const TString label = hist->GetXaxis()->GetBinLabel(ibinX);
    const auto content = hist->GetBinContent(ibinX);
    const auto error   = hist->GetBinError  (ibinX);

    output << "ibinX: " << ibinX << " label: " << label.Data() << " content: "  << content << " +/- " << error << std::endl;
  }
}
