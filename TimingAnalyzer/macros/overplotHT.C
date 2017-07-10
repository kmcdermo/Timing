#include "TStyle.h"
#include "TROOT.h"
#include "TFile.h"
#include "TH1F.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TColor.h"
#include "TString.h"

#include <vector>

void plotter(std::vector<TFile*>&, const TString&, const TString&, const Bool_t, const TString&);

inline Float_t GetMax(const TH1F * hist)
{
  Float_t max = -1e9;
  for (Int_t ibin = 1; ibin <= hist->GetNbinsX(); ibin++)
  {
    Float_t tmp = hist->GetBinContent(ibin);
    if (tmp > max) max = tmp;
  }
  return max;
}

inline Float_t GetMin(const TH1F * hist)
{
  Float_t min = 1e9;
  for (Int_t ibin = 1; ibin <= hist->GetNbinsX(); ibin++)
  {
    Float_t tmp = hist->GetBinContent(ibin);
    if (tmp < min && tmp != 0.f) min = tmp;
  }
  return min;
}

void overplotHT()
{
  gStyle->SetOptStat(0);
  const TString dir = "dump_dcs_SM_jetpt30";
  std::vector<TFile*> files = {TFile::Open(Form("%s/plots_jetpt.root",dir.Data())),
			       TFile::Open(Form("%s/plots_jetER_jetpt.root",dir.Data())),
			       TFile::Open(Form("%s/plots_jetIdL_jetER_jetpt.root",dir.Data())),
			       TFile::Open(Form("%s/plots_nopho_jetIdL_jetER_jetpt.root",dir.Data()))};

  plotter(files,"ht","H_{T} = #sum_{i=1}^{n} p_{T}^{i} , p_{T}^{i} > 15",false,dir);
  plotter(files,"nJets","nJets, p_{T} > 15",false,dir);
  plotter(files,"deltaRs","#DeltaR(#gamma,jets)",false,dir);

  for (Int_t ifile = 0; ifile < files.size(); ifile++)
  {
    delete files[ifile];
  }
}

void plotter(std::vector<TFile*> & files, const TString & hname, const TString & xlabel, const Bool_t scale, const TString & dir)
{
  const std::vector<TString> cuts = {"none","R9","Smaj","Smin","HollTkIso","HoE","sieie","EcalIso","HcalIso","phpt"};
  const std::vector<TString> labels = {"No Cuts","|#eta_{jet}| < 3.0","jet ID:L","#DeltaR(#gamma,jets) > 0.4"};
  const std::vector<Color_t> colors = {kBlack,kRed+1,kGreen+1,kBlue+1};

  std::vector<std::vector<TH1F*> > vhists(cuts.size()); 
  for (Int_t icut = 0; icut < cuts.size(); icut++)
  {
    Float_t max = -1e9;
    Float_t min =  1e9;

    vhists[icut].resize(files.size());
    for (Int_t ifile = 0; ifile < files.size(); ifile++)
    {
      vhists[icut][ifile] = (TH1F*)files[ifile]->Get(Form("%s_%s",hname.Data(),cuts[icut].Data()));
      vhists[icut][ifile] ->GetXaxis()->SetTitle(xlabel.Data());
      vhists[icut][ifile] ->GetYaxis()->SetTitle("Events");
      vhists[icut][ifile] ->SetLineColor(colors[ifile]);
      vhists[icut][ifile] ->SetMarkerColor(colors[ifile]);
      if (scale) vhists[icut][ifile] ->Scale(1.f/vhists[icut][ifile]->Integral());
      
      const Float_t tmpmax = GetMax(vhists[icut][ifile]);
      const Float_t tmpmin = GetMin(vhists[icut][ifile]);

      if (tmpmax > max) max = tmpmax;
      if (tmpmin < min) min = tmpmin;
    }

    TCanvas * canv = new TCanvas(); canv->cd(); canv->SetLogy(1);
    TLegend * leg  = new TLegend(0.6,0.6,0.9,0.9);

    for (Int_t ifile = 0; ifile < files.size(); ifile++)
    {
      vhists[icut][ifile] -> SetMinimum(min/2.f);
      vhists[icut][ifile] -> SetMaximum(max*2.f);
      vhists[icut][ifile] -> Draw(ifile>0?"same ep":"ep");
      leg->AddEntry(vhists[icut][ifile],Form("%s: <#mu> = %5.1f",labels[ifile].Data(),vhists[icut][ifile]->GetMean()),"epl");
    }
    leg->Draw("same");
    
    canv->SaveAs(Form("%s/%s_%s.png",dir.Data(),hname.Data(),cuts[icut].Data()));

    delete leg;
    delete canv;
    for (Int_t ifile = 0; ifile < files.size(); ifile++)
    {
      delete vhists[icut][ifile];
    }
  }

}
