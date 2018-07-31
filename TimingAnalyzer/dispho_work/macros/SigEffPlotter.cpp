#include "SigEffPlotter.hh"

SigEffPlotter::SigEffPlotter(const TString & infilename, const TString & outtext) :
  fInFileName(infilename), fOutText(outtext)
{
  std::cout << "Initializing SigEffPlotter..." << std::endl;

  // Get input file
  fInFile = TFile::Open(Form("%s",fInFileName.Data()));
  Common::CheckValidFile(fInFile,fInFileName);
 
  // set style
  fTDRStyle = new TStyle("TDRStyle","Style for P-TDR");
  Common::SetTDRStyle(fTDRStyle);

  // output root file for quick inspection
  fOutFile = TFile::Open(Form("%s.root",fOutText.Data()),"UPDATE");

  // setup config
  SigEffPlotter::SetupConfig();
  SigEffPlotter::SetupSignalSubGroups();
}

SigEffPlotter::~SigEffPlotter()
{
  std::cout << "Tidying up in destructor..." << std::endl;
  
  for (auto & graph : fGraphMap) delete graph.second;

  delete fLegend;
  delete fCanvas;
  delete fConfigPave;
  delete fOutFile;
  delete fInFile;
  delete fTDRStyle;
}

void SigEffPlotter::MakeSigEffPlot()
{
  std::cout << "Making Signal Efficiency Plot..." << std::endl;

  // Make input graphs
  SigEffPlotter::MakeInputGraphs();
  
  // Make Legend
  SigEffPlotter::MakeLegend();

  // Make Output
  SigEffPlotter::MakeOutput();

  // Save Metadata
  SigEffPlotter::MakeConfigPave();
}

void SigEffPlotter::MakeInputGraphs()
{
  std::cout << "Making input graphs..." << std::endl;

  // loop over signal groups
  for (auto igroup = 0U; igroup < fSignalSubGroupVec.size(); igroup++)
  {
    const auto & groupname = fSignalSubGroupVec[igroup];

    // make efficiency object
    auto efficiency = SigEffPlotter::MakeEfficiency(groupname);
  
    // make a graph so we mess with the labels, axis limits
    SigEffPlotter::MakeGraph(efficiency,groupname);

    // save efficiency object to outfile
    fOutFile->cd();
    efficiency->Write(efficiency->GetName(),TObject::kWriteDelete);
    
    // delete tmp efficiency 
    delete efficiency;
  }

  // write graphs to outfile (for good measure)
  for (const auto & GraphPair : fGraphMap)
  {
    const auto & graph = GraphPair.second;

    fOutFile->cd();
    graph->Write(graph->GetName(),TObject::kWriteDelete);
  }
}

void SigEffPlotter::MakeLegend()
{
  std::cout << "Making Legend..." << std::endl;

  // make new legend
  fLegend = new TLegend(0.2,0.75,0.4,0.92);
  fLegend->SetBorderSize(1);
  fLegend->SetLineColor(kBlack);

  for (const auto & groupname : fSignalSubGroupVec)
  {
    // get inputs
    const auto & graph = fGraphMap[groupname];
    
    // and add to legend!
    auto label = groupname;
    label.ReplaceAll("GMSB_CTau","c#tau: ");
    label.ReplaceAll("p",".");
    fLegend->AddEntry(graph,Form("%s",label.Data()),"lep");
  }

  // write legend to outfile
  fOutFile->cd();
  fLegend->Write(fLegend->GetName(),TObject::kWriteDelete);
}
  
void SigEffPlotter::MakeOutput()
{
  std::cout << "Making canvas output..." << std::endl;
  
  // setup the canvas
  fCanvas = new TCanvas("Canvas","");
  fCanvas->cd();

  // get min and max
  SigEffPlotter::GetMinYMaxY();

  // once for lin
  SigEffPlotter::DrawOutput(true);

  // once for log
  SigEffPlotter::DrawOutput(false);

  // write canv to outfile
  fOutFile->cd();
  fCanvas->Write(fCanvas->GetName(),TObject::kWriteDelete);
}

void SigEffPlotter::MakeConfigPave()
{
  std::cout << "Dumping config to a pave..." << std::endl;

  // create the pave, copying in old info
  fOutFile->cd();
  fConfigPave = new TPaveText();
  fConfigPave->SetName(Form("%s",Common::pavename.Data()));

  // give grand title
  fConfigPave->AddText("***** Input Config *****");

  // save name of input file
  fConfigPave->AddText(Form("InFile name: %s",fInFileName.Data()));

  // padding
  Common::AddPaddingToPave(fConfigPave,1);

  // dump in skim config
  Common::AddTextFromInputPave(fConfigPave,fInFile);

  // save to output file
  fOutFile->cd();
  fConfigPave->Write(fConfigPave->GetName(),TObject::kWriteDelete);
}

TEfficiency * SigEffPlotter::MakeEfficiency(const TString & groupname)
{
  std::cout << "Making TEfficiency for: " << groupname.Data() << std::endl;
  
  // get inputs
  const auto & samples = Common::SignalSubGroupMap[groupname];
  const auto nSamples  = samples.size();
  
  // new efficiency object
  auto eff = new TEfficiency(Form("%s_Eff",groupname.Data()),"Signal Sample Efficiency;#Lambda [TeV];#epsilon",nSamples,0,nSamples);

  // loop over samples, get hists, and set the appropriate efficiency
  for (auto isample = 0U; isample < nSamples; isample++)
  {
    // get input histname
    const auto & histname = Common::SignalCutFlowHistNameMap[samples[isample]];

    // get input hist
    fInFile->cd();
    auto hist = (TH1F*)fInFile->Get(Form("%s",histname.Data()));
    Common::CheckValidHist(hist,histname,fInFileName);
    
    // set efficiency by bins!
    eff->SetTotalEvents(isample+1,hist->GetBinContent(1));
    eff->SetPassedEvents(isample+1,hist->GetBinContent(hist->GetXaxis()->GetNbins()));
    
    delete hist;
  }

  return eff;
}

void SigEffPlotter::MakeGraph(const TEfficiency * efficiency, const TString & groupname)
{
  std::cout << "Making Graph for: " << groupname.Data() << std::endl;

  // first create the graph from the efficiency object
  fGraphMap[groupname] = efficiency->CreateGraph();
  auto & graph = fGraphMap[groupname];

  // set names and colors
  graph->SetName(Form("%s_Graph",groupname.Data()));
  graph->SetLineColor(Common::SignalSubGroupColorMap[groupname].color);
  graph->SetMarkerColor(Common::SignalSubGroupColorMap[groupname].color);
  
  // need inputs for axis limits and labels
  const auto & samples = Common::SignalSubGroupMap[groupname];
  const auto nSamples  = samples.size();
  
  // absurdity from ROOT
  graph->GetHistogram()->GetXaxis()->Set(nSamples,0,nSamples);
  
  // loop over samples, set Lambda for GMSB
  for (auto isample = 0U; isample < nSamples; isample++)
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

void SigEffPlotter::DrawOutput(const Bool_t isLogY)
{
  std::cout << "Drawing for isLogY: " << Common::PrintBool(isLogY).Data() << std::endl;

  // set scale
  fCanvas->cd();
  fCanvas->SetLogy(isLogY);

  // loop over groups and draw
  for (auto igroup = 0U; igroup < fSignalSubGroupVec.size(); igroup++)
  {
    const auto & groupname = fSignalSubGroupVec[igroup];
    auto & graph = fGraphMap[groupname];

    // set min/max
    if (igroup == 0)
    {
      graph->GetHistogram()->SetMinimum( isLogY ? fMinY/1.5 : fMinY/1.05 );
      graph->GetHistogram()->SetMaximum( isLogY ? fMaxY*1.5 : fMaxY*1.05 );
    }

    // draw graph
    fCanvas->cd();
    graph->Draw(igroup>0?"P same":"AP");
  }

  // Draw legend and then save!
  fLegend->Draw("same");
  Common::CMSLumi(fCanvas,0);
  Common::SaveAs(fCanvas,fOutText+"_"+(isLogY?"log":"lin"));
}

void SigEffPlotter::GetMinYMaxY()
{
  std::cout << "Getting min and max..." << std::endl;

  fMinY =  1e9;
  fMaxY = -1e9;

  // loop over all graphs
  for (const auto & GraphPair : fGraphMap)
  {
    const auto & graph = GraphPair.second;

    // loop over all points to find min, max
    for (auto ipoint = 0; ipoint < graph->GetN(); ipoint++)
    {
      Double_t x,y;
      graph->GetPoint(ipoint,x,y);
      
      if (y < fMinY && y > 0.) fMinY = y;
      if (y > fMaxY)           fMaxY = y;
    }
  }
}

void SigEffPlotter::SetupConfig()
{
  std::cout << "Setting up Config..." << std::endl;

  Common::SetupSignalSamples();

  //// ************** HACK FOR NOW ********************* ////
  Common::SampleMap.erase("MC/GMSB/L200TeV_CTau400cm");

  Common::SetupGroups();
  Common::SetupSignalGroups();
  Common::SetupSignalCutFlowHistNames();

  Common::SetupSignalSubGroups();
  Common::SetupSignalSubGroupColors();

  Common::SetTDRStyle(fTDRStyle);
}

void SigEffPlotter::SetupSignalSubGroups()
{
  std::cout << "Setting up signal sub groups internally..." << std::endl;

  // first store all signal groups we are using
  for (const auto & SignalSubGroupPair : Common::SignalSubGroupMap)
  {
    const auto & subgroup = SignalSubGroupPair.first;
    fSignalSubGroupVec.emplace_back(subgroup);
  }

  // then sort them by ctau
  std::sort(fSignalSubGroupVec.begin(),fSignalSubGroupVec.end(),
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
}
