#include "Common.cpp+"

#include "TStyle.h"
#include "TFile.h"
#include "TH1F.h"
#include "TEfficiency.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TString.h"

void computeSignalEfficiency(const TString & infilename, const TString & outtext)
{
  // set style
  auto tdrStyle = new TStyle();
  Config::SetTDRStyle(tdrStyle);

  // get input
  auto infile = TFile::Open(Form("%s",infilename.Data()));
  Config::CheckValidFile(infile,infilename);
  infile->cd();

  // setup input hist names
  Config::SetupSignalSamples();
  Config::SetupSignalGroups();
  Config::SetupSignalGroupColors();
  Config::SetupSignalCutFlowHistNames();

  // legends, canv and overall map
  auto canv = new TCanvas();
  canv->cd();
  auto leg = new TLegend(0.2,0.75,0.4,0.85);
  std::map<TString,TGraphAsymmErrors*> graphMap;

  // loop over signal groups
  Int_t counter = 0;
  for (const auto & SignalGroupPair : Config::SignalGroupMap)
  {
    const auto & groupname = SignalGroupPair.first;
    const auto & signals   = SignalGroupPair.second;

    // make efficiency object
    const auto nSignals = signals.size();
    auto efficiency = new TEfficiency("signal_efficiency","Signal Sample Efficiency;#Lambda [TeV];#epsilon",nSignals,0,nSignals);

    // loop over signals, get hists, and set the appropriate efficiency
    for (auto isignal = 0; isignal < nSignals; isignal++)
    {
      // get input hist
      const auto & histname = Config::SignalCutFlowHistNameMap[signals[isignal]];
      auto hist = (TH1F*)infile->Get(Form("%s",histname.Data()));
      Config::CheckValidTH1F(hist,histname,infilename);

      // set efficiency by bins!
      efficiency->SetTotalEvents(isignal+1,hist->GetBinContent(1));
      efficiency->SetPassedEvents(isignal+1,hist->GetBinContent(hist->GetXaxis()->GetNbins()));

      delete hist;
    }

    // get underlying graph so we can set labels
    graphMap[groupname] = efficiency->CreateGraph();
    auto & graph = graphMap[groupname];
    graph->SetName(Form("%s_eff_graph",groupname.Data()));
    graph->SetLineColor(Config::SignalGroupColorMap[groupname]);
    graph->SetMarkerColor(Config::SignalGroupColorMap[groupname]);

    // set labels!
    if (counter == 0)
    {
      // absurdity from ROOT
      graph->GetHistogram()->GetXaxis()->Set(nSignals,0,nSignals);
      
      // loop over signals, set Lambda for GMSB
      for (auto isignal = 0; isignal < nSignals; isignal++)
      {
	auto signal = signals[isignal];
	
	const TString s_lambda = "_L";
	auto i_lambda = signal.Index(s_lambda);
	auto l_lambda = s_lambda.Length();

	const TString s_ctau = "_CTau";
	auto i_ctau = signal.Index(s_ctau);
	
	const TString lambda(signal(i_lambda+l_lambda,i_ctau-i_lambda-l_lambda));

	graph->GetHistogram()->GetXaxis()->SetBinLabel(isignal+1,Form("%s",lambda.Data()));
      }
    }

    // draw 
    canv->cd();
    graph->Draw(counter>0?"P same":"AP");

    // and add to legend!
    auto label = groupname;
    label.ReplaceAll("GMSB_","c#tau: ");
    label.ReplaceAll("p",".");
    leg->AddEntry(graph,Form("%s",label.Data()),"lp");

    // step counter up!
    counter++;

    // delete 
    delete efficiency;
  }

  // finish off the rest!
  leg->Draw("same");
  Config::CMSLumi(canv);
  canv->SaveAs(Form("%s.png",outtext.Data()));
  
  // delete
  for (auto & graph : graphMap) delete graph.second;
  delete leg;
  delete canv;
  delete infile;
  delete tdrStyle;
}
