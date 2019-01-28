#include "Common.cpp+"

// ROOT includes
#include "TStyle.h"
#include "TString.h"
#include "TFile.h"
#include "TGraph.h"
#include "TCanvas.h"
#include "TLegend.h"

// c++ includes
#include <iostream>
#include <map>

//////////////////
// Sub-routines //
//////////////////

void SetupCommon()
{
  Common::SetupEras();
  Common::SetupSignalSamples();
  Common::SetupSignalGroups();
  Common::SetupSignalSubGroups();
  Common::SetupSignalSubGroupColors();
}

inline Double_t ctau(TString signalsubgroup)
{
  signalsubgroup.ReplaceAll("GMSB_CTau","");
  signalsubgroup.ReplaceAll("cm","");
  signalsubgroup.ReplaceAll("p",".");

  return signalsubgroup.Atof();
}

void savePlot(TCanvas * canv, std::vector<TGraph*> & graphs, const UInt_t nsub,
	      const TString & text, const Bool_t isLogy)
{
  canv->SetLogy(isLogy);
  for (auto isub = 0; isub < nsub; isub++)
  {
    auto & graph = graphs[isub];
    graph->GetYaxis()->SetRangeUser((isLogy?1e-3:0),(isLogy?1.5:1.05));
    graph->Draw(isub>0?"P SAME":"AP");
    canv->Update();
  }
  Common::SaveAs(canv,text+"_"+(isLogy?"log":"lin"));
}

////////////////
// Main macro //
////////////////

void plotSignificances(const TString & signif_list, const TString & sig_outtext)
{
  // set style
  auto tdrStyle = new TStyle("TDRStyle","Style for P-TDR");
  Common::SetTDRStyle(tdrStyle);

  // setup config
  SetupCommon();

  // make output file
  auto outfile = TFile::Open(sig_outtext+".root","RECREATE");

  // dump sig info into map
  std::map<TString,Double_t> SigMap;

  // read list into map
  std::ifstream infile(signif_list.Data(),std::ios::in);
  TString sample, xbin_boundary, ybin_boundary, infilename;
  Double_t significance;

  while (infile >> sample >> significance >> xbin_boundary >> ybin_boundary >> infilename)
  {
    SigMap[sample] = significance;
  }

  // get signals vec
  std::vector<TString> signalsubgroups;
  for (const auto & SignalSubGroupPair : Common::SignalSubGroupMap)
  {
    signalsubgroups.emplace_back(SignalSubGroupPair.first);
  }
  std::sort(signalsubgroups.begin(),signalsubgroups.end(),
	    [](const auto & signalsubgroup1, const auto & signalsubgroup2)
	    {
	      const auto ctau_signalsubgroup1 = ctau(signalsubgroup1);
	      const auto ctau_signalsubgroup2 = ctau(signalsubgroup2);
	      return (ctau_signalsubgroup1 < ctau_signalsubgroup2);
	    });

  // cache size of signal subgroups
  const auto nsub = signalsubgroups.size();

  // draw stuff
  auto canv = new TCanvas();
  canv->cd();
  
  auto leg = new TLegend(0.6,0.75,0.8,0.92);
  leg->SetBorderSize(1);
  leg->SetLineColor(kBlack);

  // make graphs
  std::vector<TGraph*> graphs(nsub);
  for (auto isub = 0U; isub < nsub; isub++)
  {
    const auto & subgroup = signalsubgroups[isub];
    const auto & signals  = Common::SignalSubGroupMap[subgroup];
    const auto nsig       = signals.size();
    const auto color      = Common::SignalSubGroupColorMap[subgroup].color;
    auto & graph          = graphs[isub];
    
    graph = new TGraph(nsig);
    graph->SetName (subgroup+"_"+sig_outtext);
    graph->SetTitle(subgroup+" "+sig_outtext);
    graph->SetLineColor  (color);
    graph->SetMarkerColor(color);

    // fill it
    for (auto isig = 0U; isig < nsig; isig++)
    {
      const auto & signal = signals[isig];

      const TString s_lambda = "_L";
      auto i_lambda = signal.Index(s_lambda);
      auto l_lambda = s_lambda.Length();
      
      const TString s_ctau = "_CTau";
      auto i_ctau = signal.Index(s_ctau);
    
      const TString lambda(signal(i_lambda+l_lambda,i_ctau-i_lambda-l_lambda));

      graph->SetPoint(isig,lambda.Atoi(),SigMap[signal]);
    }

    // draw it
    graph->Draw(isub>0?"P SAME":"AP");
    graph->GetXaxis()->SetTitle("#Lambda");
    graph->GetYaxis()->SetTitle("Significance");

    // add to legend
    auto label = subgroup;
    label.ReplaceAll("GMSB_CTau","c#tau: ");
    label.ReplaceAll("p",".");
    leg->AddEntry(graph,label.Data(),"p");

    // save it
    outfile->cd();
    graph->Write(graph->GetName(),TObject::kWriteDelete);
  }

  // draw legend
  leg->Draw("SAME");
  outfile->cd();
  leg->Write(leg->GetName(),TObject::kWriteDelete);

  // setup canv
  Common::CMSLumi(canv,0,"Full");

  // log plot
  savePlot(canv,graphs,nsub,sig_outtext,true);

  // linear plot
  savePlot(canv,graphs,nsub,sig_outtext,false);

  // save it
  outfile->cd();
  canv->Write(canv->GetName(),TObject::kWriteDelete);

  // delete it all
  for (auto & graph : graphs) delete graph;
  delete leg;
  delete canv;
  delete outfile;
  delete tdrStyle;
}
