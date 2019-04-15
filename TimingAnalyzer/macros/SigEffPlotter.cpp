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
  SigEffPlotter::SetupCommon();
  SigEffPlotter::SetupSignalSubGroups();
}

SigEffPlotter::~SigEffPlotter()
{
  std::cout << "Tidying up in destructor..." << std::endl;
  
  Common::DeleteMap(fEffMap);
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

  // Make input hists
  SigEffPlotter::MakeInputHists();
  
  // Make Legend
  SigEffPlotter::MakeLegend();

  // Make Output
  SigEffPlotter::MakeOutput();

  // Save Metadata
  SigEffPlotter::MakeConfigPave();
}

void SigEffPlotter::MakeInputHists()
{
  std::cout << "Making input hists..." << std::endl;

  // loop over signal groups
  for (auto igroup = 0U; igroup < fSignalSubGroupVec.size(); igroup++)
  {
    const auto & groupname = fSignalSubGroupVec[igroup];
  
    // make hists
    SigEffPlotter::MakeHist(groupname);
  }

  // write hists to outfile (for good measure)
  Common::WriteMap(fOutFile,fEffMap);
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
    const auto & eff = fEffMap[groupname];
    
    // and add to legend!
    auto label = groupname;
    label.ReplaceAll("GMSB_CTau","c#tau: ");
    label.ReplaceAll("p",".");
    fLegend->AddEntry(eff,Form("%s",label.Data()),"lep");
  }

  // write legend to outfile
  Common::Write(fOutFile,fLegend);
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
  Common::Write(fOutFile,fCanvas);
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
  Common::Write(fOutFile,fConfigPave);
}

void SigEffPlotter::MakeHist(const TString & groupname)
{
  std::cout << "Making Hist for: " << groupname.Data() << std::endl;

  // need inputs for axis limits and labels
  const auto & samples = Common::SignalSubGroupMap[groupname];
  const auto nSamples  = samples.size();

  // first create the eff hist
  fOutFile->cd();
  auto & eff = fEffMap[groupname];
  eff = new TH1F(Form("%s_Eff",groupname.Data()),"Signal Sample Efficiency;#Lambda [TeV];#epsilon",nSamples,0,nSamples);
  eff->SetLineColor(Common::SignalSubGroupColorMap[groupname].color);
  eff->SetMarkerColor(Common::SignalSubGroupColorMap[groupname].color);
    
  // loop over samples, set Lambda for GMSB + values
  for (auto isample = 0U; isample < nSamples; isample++)
  {
    // get sample
    const auto sample = samples[isample];

    // get input histname
    const auto & histname = Common::SignalCutFlowHistNameMap[sample];

    // get input hist
    auto hist = (TH1F*)fInFile->Get(Form("%s",histname.Data()));
    Common::CheckValidHist(hist,histname,fInFileName);

    // get input tree for rescaling errors
    const auto treename = Common::TreeNameMap[sample];
    auto tree = (TTree*)fInFile->Get(treename.Data());
    Common::CheckValidTree(tree,treename,fInFile->GetName());

    // get scale factor
    Float_t evtwgt; TBranch * b_evtwgt; tree->SetBranchAddress("evtwgt",&evtwgt,&b_evtwgt);
    tree->GetEntry(0);
    
    // get eff
    const auto denom = hist->GetBinContent(1);
    const auto numer = hist->GetBinContent(hist->GetXaxis()->GetNbins());
    const auto prob  = numer/denom;
    const auto err   = 1.96*std::sqrt((prob*(1.f-prob)/(denom/evtwgt)));

    // set efficiency by bins!
    eff->SetBinContent(isample+1,prob);
    eff->SetBinError  (isample+1,err);

    // set label
    const auto lambda = SigEffPlotter::GetLambda(sample);
    eff->GetXaxis()->SetBinLabel(isample+1,Form("%s",lambda.Data()));

    // delete it all
    delete tree;
    delete hist;
  }
}

void SigEffPlotter::DrawOutput(const Bool_t isLogY)
{
  std::cout << "Drawing for isLogY: " << Common::PrintBool(isLogY).Data() << std::endl;

  // set scale
  fCanvas->cd();
  fCanvas->SetLogy(isLogY);

  // loop over hists and draw
  for (auto igroup = 0U; igroup < fSignalSubGroupVec.size(); igroup++)
  {
    const auto & groupname = fSignalSubGroupVec[igroup];
    auto & eff = fEffMap[groupname];

    // set min/max
    if (igroup == 0)
    {
      eff->SetMinimum( isLogY ? 1e-3 : fMinY/1.05 );
      eff->SetMaximum( isLogY ? 1e0  : fMaxY*1.05 );
    }

    // draw hists
    fCanvas->cd();
    eff->Draw(igroup>0?"ep same":"ep");
  }

  // Draw legend and then save!
  fLegend->Draw("same");
  Common::CMSLumi(fCanvas,0,"Full");
  Common::SaveAs(fCanvas,fOutText+"_"+(isLogY?"log":"lin"));
}

void SigEffPlotter::GetMinYMaxY()
{
  std::cout << "Getting min and max..." << std::endl;

  fMinY =  1e9;
  fMaxY = -1e9;

  // loop over all graphs
  for (const auto & EffPair : fEffMap)
  {
    const auto & eff = EffPair.second;

    // loop over all points to find min, max
    for (auto ibinX = 1; ibinX <= eff->GetXaxis()->GetNbins(); ibinX++)
    {
      const auto content = eff->GetBinContent(ibinX);

      if (content < fMinY && content > 0.0) fMinY = content;
      if (content > fMaxY && content > 0.0) fMaxY = content;
    }
  }
}

void SigEffPlotter::SetupCommon()
{
  std::cout << "Setting up Common..." << std::endl;

  Common::SetupEras();

  Common::SetupSignalSamples();

  Common::SetupGroups();
  Common::SetupSignalGroups();
  Common::SetupTreeNames();
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

TString SigEffPlotter::GetLambda(TString sample)
{
  const TString s_lambda = "_L";
  auto i_lambda = sample.Index(s_lambda);
  auto l_lambda = s_lambda.Length();
  
  const TString s_ctau = "_CTau";
  auto i_ctau = sample.Index(s_ctau);
  
  return TString(sample(i_lambda+l_lambda,i_ctau-i_lambda-l_lambda));
}
