#include "TStyle.h"
#include "TROOT.h"
#include "TFile.h"
#include "TH1F.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TColor.h"
#include "TString.h"

#include <vector>

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

  std::vector<TString> cuts = {"none","R9","Smaj","Smin","HollTkIso","HoE","sieie","EcalIso","HcalIso","phpt"};

  std::vector<TFile*> files = {TFile::Open("dump_dcs_SM/plots.root"),
			       TFile::Open("dump_dcs_SM/plots_jetER.root"),
			       TFile::Open("dump_dcs_SM/plots_jetIdL_jetER.root"),
			       TFile::Open("dump_dcs_SM/plots_nopho_jetIdL_jetER.root")};

  std::vector<TString> labels = {"No Cuts","|#eta_{jet}| < 3.0","jet ID:L","dR_{#gamma,jets} > 0.4"};
  std::vector<Color_t> colors = {kBlack,kRed+1,kGreen+1,kBlue+1};

  std::vector<std::vector<TH1F*> > vhists(cuts.size()); 
  for (Int_t icut = 0; icut < cuts.size(); icut++)
  {
    Float_t max = -1e9;
    Float_t min =  1e9;

    vhists[icut].resize(files.size());
    for (Int_t ifile = 0; ifile < files.size(); ifile++)
    {
      vhists[icut][ifile] = (TH1F*)files[ifile]->Get(Form("ht_%s",cuts[icut].Data()));
      vhists[icut][ifile] ->GetXaxis()->SetTitle("H_{T} = #sum_{i=1}^{n} p_{T}^{i} , p_{T}^{i} > 15");
      vhists[icut][ifile] ->GetYaxis()->SetTitle("Events");
      vhists[icut][ifile] ->SetLineColor(colors[ifile]);
      vhists[icut][ifile] ->SetMarkerColor(colors[ifile]);
      vhists[icut][ifile] ->Scale(1.f/vhists[icut][ifile]->Integral());
      
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
      leg->AddEntry(vhists[icut][ifile],Form("%s: <#mu> = %6.2f",labels[ifile].Data(),vhists[icut][ifile]->GetMean()),"epl");
    }
    leg->Draw("same");
    
    canv->SaveAs(Form("ht_%s.png",cuts[icut].Data()));

    delete leg;
    delete canv;
    for (Int_t ifile = 0; ifile < files.size(); ifile++)
    {
      delete vhists[icut][ifile];
    }
  }

  for (Int_t ifile = 0; ifile < files.size(); ifile++)
  {
    delete files[ifile];
  }
}
