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
void Overlay(std::vector<TH1F*> & hists, const UInt_t N, const std::vector<TString> & labels,
	     const TString & outtext, TFile *& outfile);
void DrawOutput(std::vector<TH1F*> & hists, const UInt_t N, const std::vector<TString> & labels,
		const TString & outtext, const Bool_t isLogY);

void overplot_prediction()
{
  // set style
  auto tdrStyle = new TStyle("TDRStyle","Style for P-TDR");
  Common::SetTDRStyle(tdrStyle);

  /////////////////
  // PREP INPUTS //
  /////////////////

  // vector of file names
  const TString indir = "test_macros/files";
  std::vector<TString> filenames = {"orig_met","deg_met","sph200_met"};
  const auto N = filenames.size();

  // signal hist name
  const TString signalname = "GMSB_L200_CTau600";
  
  // outtext + labels
  const TString outtext = "met_zoom";
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
  std::vector<TH1F*> bkgdhists(N);
  for (auto i = 0U; i < N; i++)
  {
    const auto & label = labels[i];

    auto & infile = infiles[i];
    infile->cd();

    auto & bkgdhist = bkgdhists[i];
    
    bkgdhist = (TH1F*)infile->Get(Common::BkgdHistName.Data());
    Common::CheckValidHist(bkgdhist,Common::BkgdHistName,infile->GetName());

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

    const TString histname = signalname+"_Hist_Plotted";
    signalhist = (TH1F*)infile->Get(histname.Data());
    Common::CheckValidHist(signalhist,histname,infile->GetName());

    signalhist->SetName(Form("%s_%s",histname.Data(),label.Data()));
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

  Overlay(bkgdhists,N,labels,outtext+"_bkgd",outfile);
  Overlay(signalhists,N,labels,outtext+"_"+signalname,outfile);
  Overlay(sbhists,N,labels,outtext+"_sb",outfile);

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

void Overlay(std::vector<TH1F*> & hists, const UInt_t N, const std::vector<TString> & labels, 
	     const TString & outtext, TFile *& outfile)
{
  for (auto i = 0U; i < N; i++)
  {
    const auto & color = Common::ColorVec[i];
    auto & hist = hists[i];

    hist->SetLineColor(color);
    hist->SetMarkerColor(color);

    outfile->cd();
    hist->Write(hist->GetName(),TObject::kWriteDelete);
  }

  // do a drawing for lin then log
  DrawOutput(hists,N,labels,outtext,false);
  DrawOutput(hists,N,labels,outtext,true);
}

void DrawOutput(std::vector<TH1F*> & hists, const UInt_t N, const std::vector<TString> & labels,
		const TString & outtext, const Bool_t isLogY)
{
  const Float_t min = GetMin(hists);
  const Float_t max = GetMax(hists);

  auto canv = new TCanvas();
  canv->cd();
  canv->SetLogy(isLogY);

  auto leg = new TLegend(0.725,0.82,0.825,0.92);
  leg->SetBorderSize(1);
  leg->SetLineColor(kBlack);

  for (auto i = 0U; i < N; i++)
  {
    const auto & label = labels[i];
    auto & hist = hists[i];

    hist->SetMinimum(isLogY?min/1.5f:min/1.05f);
    hist->SetMaximum(isLogY?max*1.5f:max*1.05f);
  
    canv->cd();
    hist->Draw(i>0?"same ep":"ep");
    leg->AddEntry(hist,label.Data(),"epl");
  }

  // final touches and save image
  leg->Draw("same");
  Common::CMSLumi(canv,0);
  Common::SaveAs(canv,outtext+"_"+(isLogY?"log":"lin"));
  
  delete leg;
  delete canv;
}
