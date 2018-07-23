#include "Common.cpp+"

#include "TStyle.h"
#include "TFile.h"
#include "TH1F.h"
#include "TEfficiency.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TString.h"
#include "TColor.h"

#include <map>
#include <vector>

void Setup(TStyle *& tdrStyle)
{
  Common::SetupSignalSamples();

  //// ************** HACK FOR NOW ********************* //
  Common::SampleMap.erase("MC/GMSB/L200TeV_CTau400cm");

  Common::SetupGroups();
  Common::SetupSignalGroups();
  Common::SetupSignalCutFlowHistNames();

  Common::SetupSignalSubGroups();
  Common::SetupSignalSubGroupColors();

  Common::SetTDRStyle(tdrStyle);
}

void computeSignalEfficiency(const TString & infilename, const TString & outtext)
{
  // set things up
  auto tdrStyle = new TStyle();
  Setup(tdrStyle);
  
  // get input
  auto infile = TFile::Open(Form("%s",infilename.Data()));
  Common::CheckValidFile(infile,infilename);
  infile->cd();

  // use vector to put things in sorted order...
  std::vector<TString> SignalSubGroupVec;
  for (const auto & SignalSubGroupPair : Common::SignalSubGroupMap)
  {
    SignalSubGroupVec.emplace_back(SignalSubGroupPair.first);
  }
  std::sort(SignalSubGroupVec.begin(),SignalSubGroupVec.end(),
	    [](const auto & group1, const auto & group2)
	    {
	      const TString s_ctau = "_CTau";
	      auto i_ctau = group1.Index(s_ctau);
	      auto l_ctau = s_ctau.Length();
	      
	      TString s_ctau1(group1(i_ctau+l_ctau,group1.Length()-i_ctau-l_ctau));
	      TString s_ctau2(group2(i_ctau+l_ctau,group2.Length()-i_ctau-l_ctau));
  
	      s_ctau1.ReplaceAll("p",".");
	      s_ctau2.ReplaceAll("p",".");

	      const auto ctau1 = s_ctau1.Atof();
	      const auto ctau2 = s_ctau2.Atof();
	      
	      return ctau1 < ctau2;
	    });
  
  // legends, canv and overall map
  auto canv = new TCanvas();
  canv->cd();
  auto leg = new TLegend(0.2,0.7,0.5,0.9);
  std::map<TString,TGraphAsymmErrors*> graphMap;

  // loop over signal groups
  for (auto igroup = 0U; igroup < SignalSubGroupVec.size(); igroup++)
  {
    const auto & groupname = SignalSubGroupVec[igroup];
    const auto & samples   = Common::SignalSubGroupMap[groupname];

    // make efficiency object
    const auto nSamples = samples.size();
    auto efficiency = new TEfficiency("signal_efficiency","Signal Sample Efficiency;#Lambda [TeV];#epsilon",nSamples,0,nSamples);

    // loop over samples, get hists, and set the appropriate efficiency
    for (auto isample = 0; isample < nSamples; isample++)
    {
      // get input hist
      const auto & histname = Common::SignalCutFlowHistNameMap[samples[isample]];
      auto hist = (TH1F*)infile->Get(Form("%s",histname.Data()));
      Common::CheckValidHist(hist,histname,infilename);

      // set efficiency by bins!
      efficiency->SetTotalEvents(isample+1,hist->GetBinContent(1));
      efficiency->SetPassedEvents(isample+1,hist->GetBinContent(hist->GetXaxis()->GetNbins()));

      delete hist;
    }

    // get underlying graph so we can set labels
    graphMap[groupname] = efficiency->CreateGraph();
    auto & graph = graphMap[groupname];
    graph->SetName(Form("%s_eff_graph",groupname.Data()));
    graph->SetLineColor(Common::SignalSubGroupColorMap[groupname].color);
    graph->SetMarkerColor(Common::SignalSubGroupColorMap[groupname].color);

    // set labels!
    if (igroup == 0)
    {
      // absurdity from ROOT
      graph->GetHistogram()->GetXaxis()->Set(nSamples,0,nSamples);
      graph->GetHistogram()->GetYaxis()->SetRangeUser(0.0,0.5);
      
      // loop over samples, set Lambda for GMSB
      for (auto isample = 0; isample < nSamples; isample++)
      {
	auto sample = samples[isample];
	
	const TString s_lambda = "_L";
	auto i_lambda = sample.Index(s_lambda);
	auto l_lambda = s_lambda.Length();

	const TString s_ctau = "_CTau";
	auto i_ctau = sample.Index(s_ctau);
	
	const TString lambda(sample(i_lambda+l_lambda,i_ctau-i_lambda-l_lambda));

	graph->GetHistogram()->GetXaxis()->SetBinLabel(isample+1,Form("%s",lambda.Data()));
      }
    }

    // draw 
    canv->cd();
    graph->Draw(igroup>0?"P same":"AP");

    // and add to legend!
    auto label = groupname;
    label.ReplaceAll("GMSB_CTau","c#tau: ");
    label.ReplaceAll("p",".");
    leg->AddEntry(graph,Form("%s",label.Data()),"lp");

    // delete 
    delete efficiency;
  }

  // finish off the rest!
  leg->Draw("same");
  Common::CMSLumi(canv,0);
  Common::SaveAs(canv,outtext);
  
  // delete
  for (auto & graph : graphMap) delete graph.second;
  delete leg;
  delete canv;
  delete infile;
  delete tdrStyle;
}
