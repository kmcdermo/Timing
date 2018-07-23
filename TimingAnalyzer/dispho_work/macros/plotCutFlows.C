#include "Common.cpp+"

#include "TString.h"
#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TEntryList.h"

#include <vector>

void SetupConfig(const TString & cutflowconfig)
{
  gStyle->SetOptStat(0); // can think about tdrStyle if we need it

  Common::SetupSamples();
  Common::SetupGroups();
  Common::SetupColors();
  Common::SetupGroupCutFlowHistNames();
  Common::SetupCutFlow(cutflowconfig);

  // assign colors for ColorMap
  Common::ColorMap["Bkgd"] = kBlue;
  Common::ColorMap["EWK"]  = kOrange+3; 
}

void GetHists(TFile *& file, std::map<TString,TH1F*> & HistMap)
{
  for (const auto & GroupPair : Common::GroupMap)
  {
    const auto & sample = GroupPair.first;
    const auto group    = GroupPair.second;
    const auto histname = Common::GroupCutFlowHistNameMap[sample];

    HistMap[sample] = (TH1F*)file->Get(Form("%s",histname.Data()));
    Common::CheckValidHist(HistMap[sample],histname,file->GetName());
  }  

  // Make a sum MC bkgd hist and EWK sum hist
  auto bkgd_counter = 0U;
  auto ewk_counter  = 0U;
  for (const auto & GroupPair : Common::GroupMap)
  {
    const auto & sample = GroupPair.first;
    const auto group    = GroupPair.second;

    if (group == isBkgd)
    {
      if   (bkgd_counter == 0) HistMap["Bkgd"] = (TH1F*)HistMap[sample]->Clone(Form("Bkgd_%s",Common::h_cutflowname.Data()));
      else             	       HistMap["Bkgd"]->Add(HistMap[sample]);
      bkgd_counter++;

      if (!Common::IsCRMC(sample)) 
      {
	if   (ewk_counter == 0) HistMap["EWK"] = (TH1F*)HistMap[sample]->Clone(Form("EWK_%s",Common::h_cutflowname.Data()));
	else                    HistMap["EWK"]->Add(HistMap[sample]);
	ewk_counter++;
      }
    }
  }
}

Float_t GetMinimum(const std::vector<TString> & keys, const std::map<TString,TH1F*> & HistMap)
{
  auto min = 1e9f;
  for (const auto & key : keys)
  {
    const auto & hist = HistMap.at(key);
    for (auto bin = 1; bin <= hist->GetNbinsX(); bin++)
    {
      const auto tmpmin = hist->GetBinContent(bin);
      if ((tmpmin < min) && (tmpmin > 0)) min = tmpmin;
    }
  }
  return min;
}

Float_t GetMaximum(const std::vector<TString> & keys, const std::map<TString,TH1F*> & HistMap, const Float_t minX)
{
  auto max = -1e9f;
  for (const auto & key : keys)
  {
    const auto & hist = HistMap.at(key);
    const auto minbin = hist->FindBin(minX);
    for (auto bin = minbin; bin <= hist->GetNbinsX(); bin++)
    {
      const auto tmpmax = hist->GetBinContent(bin);
      if (tmpmax > max) max = tmpmax;
    }
  }
  return max;
}

void plotCutFlows(const TString & infilename, const TString & cutflowconfig, const TString & outfiletext)
{
  // setup sample names and such for looping
  SetupConfig(cutflowconfig);

  // some constants for later: bin starts and ends (all size 1)
  const auto minX = 11.f; // lower edge of cutflow "ph0std"
  const auto maxX = minX + Common::CutFlowPairVec.size(); // upper edge of last bin

  // make local vector of plots we care about
  const std::vector<TString> keys = {"Data","Bkgd","GJets","QCD","EWK"};

  // open input file
  auto file = TFile::Open(Form("%s",infilename.Data()));
  Common::CheckValidFile(file,infilename);
  
  // get hists
  std::map<TString,TH1F*> HistMap;
  GetHists(file,HistMap);
  
  // get min, max range in Y
  const auto minY = GetMinimum(keys,HistMap);
  const auto maxY = GetMaximum(keys,HistMap,minX);
  
  // make canvas and start drawing
  auto Canv = new TCanvas();
  Canv->cd();
  Canv->SetLogy();
  Canv->SetTicky();

  // make legend
  auto Leg = new TLegend(0.77,0.65,0.87,0.88);
  
  // do plotting
  auto plot_counter = 0U;
  for (const auto & key : keys)
  {
    auto & hist = HistMap[key];

    hist->GetXaxis()->SetRangeUser(minX,maxX);
    hist->SetMinimum(minY / 1.5);
    hist->SetMaximum(maxY * 1.5);

    hist->SetLineColor(Common::ColorMap[key]);
    hist->SetMarkerColor(Common::ColorMap[key]);
    hist->Draw(plot_counter>0?"same ep":"ep");
    
    Leg->AddEntry(hist,Form("%s",key.Data()),"epl");

    plot_counter++;
  }

  // draw leg
  Leg->Draw("same");

  // save output
  Common::SaveAs(Canv,outfiletext);

  // delete
  delete Leg;
  delete Canv;
  for (auto & HistPair : HistMap) delete HistPair.second;
  delete file;
}
