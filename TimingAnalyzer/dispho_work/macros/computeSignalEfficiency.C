#include "computeSignalEfficiency.hh"

void computeSignalEfficiency(const TString & infilename, const TString & outtext)
{
  // set things up
  auto tdrStyle = new TStyle();
  Setup(tdrStyle);
  
  // get input
  auto infile = TFile::Open(Form("%s",infilename.Data()));
  Common::CheckValidFile(infile,infilename);
  infile->cd();

  // legends, canv and overall map
  auto canv = new TCanvas();
  canv->cd();
  auto leg = new TLegend(0.2,0.75,0.4,0.85);
  std::map<TString,TGraphAsymmErrors*> graphMap;

  // loop over signal groups
  Int_t counter = 0;
  for (const auto & SignalSubGroupPair : Common::SignalSubGroupMap)
  {
    const auto & groupname = SignalSubGroupPair.first;
    const auto & samples   = SignalSubGroupPair.second;

    // make efficiency object
    const auto nSamples = samples.size();
    auto efficiency = new TEfficiency("signal_efficiency","Signal Sample Efficiency;#Lambda [TeV];#epsilon",nSamples,0,nSamples);

    // loop over samples, get hists, and set the appropriate efficiency
    for (auto isample = 0; isample < nSamples; isample++)
    {
      // get input hist
      const auto & histname = Common::SignalCutFlowHistNameMap[samples[isample]];
      auto hist = (TH1F*)infile->Get(Form("%s",histname.Data()));
      Common::CheckValidTH1F(hist,histname,infilename);

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
    if (counter == 0)
    {
      // absurdity from ROOT
      graph->GetHistogram()->GetXaxis()->Set(nSamples,0,nSamples);
      graph->GetHistogram()->GetYaxis()->SetRangeUser(0.0,1.0);
      
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
    graph->Draw(counter>0?"P same":"AP");

    // and add to legend!
    auto label = groupname;
    label.ReplaceAll("GMSB_CTau","c#tau: ");
    label.ReplaceAll("p",".");
    leg->AddEntry(graph,Form("%s",label.Data()),"lp");

    // step counter up!
    counter++;

    // delete 
    delete efficiency;
  }

  // finish off the rest!
  leg->Draw("same");
  Common::CMSLumi(canv);
  canv->SaveAs(Form("%s.png",outtext.Data()));
  
  // delete
  for (auto & graph : graphMap) delete graph.second;
  delete leg;
  delete canv;
  delete infile;
  delete tdrStyle;
}

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
