#include "Common.cpp+"

#include "TStyle.h"
#include "TFile.h"
#include "TH2F.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TString.h"

#include <vector>

Float_t GetMin(const TH2F * hist);
Float_t GetMin(const std::vector<TH2F*> & hists);
Float_t GetMax(const std::vector<TH2F*> & hists);
void DrawOutput(std::vector<TH2F*> & hists, const UInt_t N, const std::vector<TString> & labels,
		const TString & outtext, TFile *& outfile);

void overplot_prediction2D()
{
  // set style
  auto tdrStyle = new TStyle("TDRStyle","Style for P-TDR");
  Common::SetTDRStyle(tdrStyle);

  /////////////////
  // PREP INPUTS //
  /////////////////

  // vector of file names
  const TString indir = "test_macros/files";
  std::vector<TString> filenames = {"orig_2D","deg_2D","sph200_2D"};
  const auto N = filenames.size();

  // signal hist name
  const TString signalname = "GMSB_L200_CTau600";
  
  // outtext
  const TString outtext = "met_vs_time";
  const std::vector<TString> labels = {"Orig","DEG","SPH200"};

  // outfile
  auto outfile = TFile::Open(Form("%s.root",outtext.Data()),"RECREATE");

  // vector of files
  std::vector<TFile*> infiles(N);
  for (auto i = 0U; i < N; i++)
  {
    const auto & filename = indir+"/"+filenames[i]+".root";
    auto & infile = infiles[i];

    infile = TFile::Open(filename.Data());
    Common::CheckValidFile(infile,filename);
  }

  // vector of Bkgd Prediction hists
  std::vector<TH2F*> bkgdhists(N);
  for (auto i = 0U; i < N; i++)
  {
    const auto & label = labels[i];

    auto & infile = infiles[i];
    infile->cd();

    auto & bkgdhist = bkgdhists[i];
    
    const TString histname = "BkgdHist";
    bkgdhist = (TH2F*)infile->Get(histname.Data());
    Common::CheckValidHist(bkgdhist,histname,infile->GetName());

    bkgdhist->SetName(Form("%s_%s",histname.Data(),label.Data()));
  }
  
  // vector of signal hists
  std::vector<TH2F*> signalhists(N);
  for (auto i = 0U; i < N; i++)
  {
    const auto & label = labels[i];

    auto & infile = infiles[i];
    infile->cd();

    auto & signalhist = signalhists[i];

    const TString histname = signalname+"_SignHist";
    signalhist = (TH2F*)infile->Get(histname.Data());
    Common::CheckValidHist(signalhist,histname,infile->GetName());

    signalhist->SetName(Form("%s_%s",histname.Data(),label.Data()));
  }

  //////////////////
  // PREP OUTPUTS //
  //////////////////

  // vector of hists for s / sqrt (s + b)
  std::vector<TH2F*> sbhists(N);
  for (auto i = 0U; i < N; i++)
  {
    const auto & bkgdhist = bkgdhists[i];
    const auto & signalhist = signalhists[i];
    const auto & label = labels[i];
    
    auto & sbhist = sbhists[i];

    // tmp s + b hist
    auto tmphist = (TH2F*)bkgdhist->Clone("tmp_hist");
    tmphist->Add(signalhist);

    // sqrt tmp hist
    for (auto xbin = 0; xbin <= tmphist->GetXaxis()->GetNbins() + 1; xbin++)
    {
      for (auto ybin = 0; ybin <= tmphist->GetYaxis()->GetNbins() + 1; ybin++)
      {
	const auto orig_content = tmphist->GetBinContent(xbin,ybin);
	const auto orig_error   = tmphist->GetBinError  (xbin,ybin);
      
	tmphist->SetBinContent(xbin,ybin,std::sqrt(orig_content));
	tmphist->SetBinError  (xbin,ybin,(orig_error/(2.f*std::sqrt(orig_content)))); // https://en.wikipedia.org/wiki/Propagation_of_uncertainty#Example_formulas
      }
    }

    // s / sqrt(s+b)
    outfile->cd();
    sbhist = (TH2F*)signalhist->Clone(Form("SB_Hist_%s",label.Data()));
    sbhist->Divide(tmphist);
  
    delete tmphist;
  }

  /////////////
  // OVERLAY //
  /////////////

  DrawOutput(bkgdhists,N,labels,outtext+"_bkgd",outfile);
  DrawOutput(signalhists,N,labels,outtext+"_"+signalname,outfile);
  DrawOutput(sbhists,N,labels,outtext+"_sb",outfile);

  ////////////
  // DELETE //
  ////////////

  for (auto & sbhist : sbhists) delete sbhist;
  for (auto & signalhist : signalhists) delete signalhist;
  for (auto & bkgdhist : bkgdhists) delete bkgdhist;
  
  delete outfile;
  for (auto & infile : infiles) delete infile;

  delete tdrStyle;
}

Float_t GetMin(const TH2F * hist)
{
  Float_t min = 1e9;
  for (auto xbin = 1; xbin <= hist->GetXaxis()->GetNbins(); xbin++)
  {
    for (auto ybin = 1; ybin <= hist->GetYaxis()->GetNbins(); ybin++)
    {
      const Float_t tmpmin = hist->GetBinContent(xbin,ybin);
      if ((tmpmin > 0.f) && (tmpmin < min)) min = tmpmin;
    }
  }
  return min;
}

Float_t GetMin(const std::vector<TH2F*> & hists)
{
  Float_t min = 1e9;
  for (auto & hist : hists)
  {
    for (auto xbin = 1; xbin <= hist->GetXaxis()->GetNbins(); xbin++)
    {
      for (auto ybin = 1; ybin <= hist->GetYaxis()->GetNbins(); ybin++)
      {
      	const Float_t tmpmin = hist->GetBinContent(xbin,ybin);
	if ((tmpmin > 0.f) && (tmpmin < min)) min = tmpmin;
      }
    }
  }
  return min;
}

Float_t GetMax(const std::vector<TH2F*> & hists)
{
  Float_t max = -1e9;
  for (auto & hist : hists)
  {
    const Float_t tmpmax = hist->GetBinContent(hist->GetMaximumBin());
    if (tmpmax > max) max = tmpmax;
  }
  return max;
}

void DrawOutput(std::vector<TH2F*> & hists, const UInt_t N, const std::vector<TString> & labels, 
	     const TString & outtext, TFile *& outfile)
{
  // const Float_t min = GetMin(hists);
  // const Float_t max = GetMax(hists);

  for (auto i = 0U; i < N; i++)
  {
    const auto & label = labels[i];
    auto & hist = hists[i];

    outfile->cd();
    hist->Write(hist->GetName(),TObject::kWriteDelete);

    // hist->SetMinimum(min);
    // hist->SetMaximum(max);

    // const Float_t min = GetMin(hist);
    // hist->SetMinimum(min);

    auto canv = new TCanvas();
    canv->cd();

    hist->Draw("colz");

    Common::CMSLumi(canv);
    Common::SaveAs(canv,outtext+"_"+label);
		   
    delete canv;
  }
}
