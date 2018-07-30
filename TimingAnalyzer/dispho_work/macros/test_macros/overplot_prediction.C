#include "Common.cpp+"

#include "TStyle.h"
#include "TFile.h"
#include "TH1F.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TString.h"
#include "TColor.h"

#include <vector>

Float_t GetMin(const std::vector<TH1F*> & hists);
Float_t GetMax(const std::vector<TH1F*> & hists);
void overlay(std::vector<TH1F*> & hists, const UInt_t N, const std::vector<Color_t> & colors,
	     const std::vector<TString> & labels, const TString & outtext, TFile *& outfile);

void overplot_prediction()
{
  // set style
  auto tdrStyle = new TStyle("TDRStyle","Style for P-TDR");
  Common::SetTDRStyle(tdrStyle);

  /////////////////
  // PREP INPUTS //
  /////////////////

  // colors
  std::vector<Color_t> colors = {kBlack,kRed+1,kBlue,kGreen};

  // vector of file names
  std::vector<TString> filenames = {""};
  const auto N = filenames.size();

  // signal hist name
  const TString signalname = "GMSB_L200_CTau600";
  
  // outtext + labels
  const TString outtext = "";
  const TString labels = {"Orig","DEG","SPH200"};

  // outfile
  auto outfile = TFile::Open(Form("%s.root",outtext.Data()),"RECREATE");

  // vector of files
  std::vector<TFile*> infiles(N);
  for (auto i = 0U; i < N; i++)
  {
    const auto & filename = filenames[i];
    auto & infile = infiles[i];

    infile = TFile::Open(filename.Data());
  }

  // vector of Bkgd Prediction hists
  std::vector<TH1F*> bkgdhists(N);
  for (auto i = 0U; i < N; i++)
  {
    const auto & label = labels[i];

    auto & infile = infiles[i];
    infile->cd();

    auto & bkgdhist = bkgdhists[i];
    
    bkgdhist = (TH1F*)infile->Get(Common::BkgdHistName.Data());
    bkgdhist->SetName(Form("%s_%s",Common::BkgdHistName.Data(),label.Data()));
  }
  
  // vector of signal hists
  std::vector<TH1F*> signalhists(N);
  for (auto i = 0U; i < N; i++)
  {
    const auto & label = labels[i];

    auto & infile = infiles[i];
    infile->cd();

    auto & signalhist = signalhists[i];

    signalhist = (TH1F*)infile->Get(Form("%s_Hist_Plotted",signalname.Data()));
    signalhist->SetName(Form("%s_%s",signalhist->GetName(),label.Data()));
  }

  //////////////////
  // PREP OUTPUTS //
  //////////////////

  // vector of hists for s / sqrt (s + b)
  std::vector<TH1F*> sbhists(N);
  for (auto i = 0U; i < N; i++)
  {
    const auto & bkgdhist = bkgdhists[i];
    const auto & signalhist = signalhists[i];
    const auto & label = labels[i];
    
    auto & sbhist = sbhists[i];

    // tmp s + b hist
    auto tmphist = (TH1F*)bkgdhist->Clone("tmp_hist");
    tmphist->Add(signalhist);

    // sqrt tmp hist
    for (auto ibin = 0; ibin <= tmphist->GetXaxis()->GetNbins() + 1; ibin++)
    {
      const auto orig_content = tmphist->GetBinContent(ibin);
      const auto orig_error   = tmphist->GetBinError(ibin);
      
      tmphist->SetBinContent(ibin,std::sqrt(orig_content));
      tmphist->SetBinError  (ibin,(orig_error/(2.f*std::sqrt(orig_content)))); // https://en.wikipedia.org/wiki/Propagation_of_uncertainty#Example_formulas
    }

    // s / sqrt(s+b)
    outfile->cd();
    sbhist = (TH1F*)signalhist->Clone(Form("SB_Hist_%s",label.Data()));
    sbhist->Divide(tmphist);
  
    delete tmphist;
  }

  /////////////
  // OVERLAY //
  /////////////

  Overlay(bkgdhists,N,colors,labels,outfile);
  Overlay(signalhists,N,colors,labels,outfile);
  Overlay(sbhists,N,colors,labels,outfile);

  ////////////
  // DELETE //
  ////////////

  for (auto & sbhist : sbhists) delete sbhists;
  for (auto & signalhist : signalhists) delete signalhists;
  for (auto & bkgdhist : bkgdhists) delete bkgdhists;
  
  delete outfile;
  for (auto & infile : infiles) delete infile;

  delete tdrStyle;
}

Float_t GetMin(const std::vector<TH1F*> & hists)
{
  Float_t min = 1e9;
  for (auto & hist : hists)
  {
    for (auto ibin = 1; ibin <= hist->GetXaxis()->GetNbins(); ibin++)
    {
      const Float_t tmpmin = hist->GetBinContent(ibin);
      if ((tmpmin > 0.f) && (tmpmin < min)) min = tmpmin;
    }
  }
  return min;
}

Float_t GetMax(const std::vector<TH1F*> & hists)
{
  Float_t max = -1e9;
  for (auto & hist : hists)
  {
    const Float_t tmpmax = hist->GetBinContent(hist->GetMaximumBin());
    if (tmpmax > max) max = tmpmax;
  }
  return max;
}

void Overlay(std::vector<TH1F*> & hists, const UInt_t N, const std::vector<Color_t> & colors, 
	     const std::vector<TString> & labels, const TString & outtext, TFile *& outfile)
{
  const Float_t min = GetMin(hists);
  const Float_t max = GetMax(hists);

  auto canv = new TCanvas();
  auto leg = new TLegend(0.55,0.65,0.825,0.92);
  
  for (auto i = 0U; i < N; i++)
  {
    const auto & color = colors[i];
    const auto & label = labels[i];
    auto & hist = hists[i];

    hist->SetLineColor(color);
    hist->SetMarkerColor(color);

    hist->SetMinimum(min/1.05f);
    hist->SetMaximum(max*1.05f);

    outfile->cd();
    hist->Write(hist->GetName(),TObject::kWriteDelete);

    canv->cd();
    hist->Draw(i>0?"same ep":"ep");
    leg->AddEntry(hist,label.Data(),"epl");
  }

  // final touches and save image
  Common::CMSLumi(canv,0);
  Common::SaveAs(canv,outtext);
  
  delete leg;
  delete canv;
}
