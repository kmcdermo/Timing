#include "Limits1D.hh"

Limits1D::Limits1D(const TString & indir, const TString & infilename, const Bool_t doobserved, const TString & outtext)
  : fInDir(indir), fInFileName(infilename), fDoObserved(doobserved), fOutText(outtext)
{  
  // setup first
  Limits1D::SetupCombine();

  // set style
  fTDRStyle = new TStyle("TDRStyle","Style for P-TDR");
  Common::SetTDRStyle(fTDRStyle);
  gROOT->ForceStyle();

  // make new output file
  fOutFile = TFile::Open(Form("%s.root",fOutText.Data()),"UPDATE");
}

Limits1D::~Limits1D() 
{
  std::cout << "Tidying up in the destructor..." << std::endl;

  delete fOutFile;
  delete fTDRStyle;
}

void Limits1D::MakeLimits1D()
{
  std::cout << "Making limit plots in 1D..." << std::endl;

  // loop over GMSB subgroups
  for (const auto & GMSBSubGroupPair : Combine::GMSBSubGroupMap)
  {
    const auto & groupname = GMSBSubGroupPair.first;
    const auto & samples   = GMSBSubGroupPair.second;
    const auto nSamples = samples.size();
    const auto nSigPts  = 2*nSamples;

    // setup graphs
    auto theo_graph = new TGraph(nSamples);
    theo_graph->SetName(Form("%s_Theo_Graph",groupname.Data()));
    theo_graph->SetLineStyle(8);
    theo_graph->SetLineWidth(2);
    theo_graph->SetLineColor(kBlue);

    auto obs_graph = new TGraph(nSamples);
    obs_graph->SetName(Form("%s_Observed_Graph",groupname.Data()));
    obs_graph->SetLineWidth(2);
    obs_graph->SetLineColor(kBlack);

    auto exp_graph = new TGraph(nSamples);
    exp_graph->SetName(Form("%s_Expected_Graph",groupname.Data()));
    exp_graph->SetLineStyle(7);
    exp_graph->SetLineWidth(2);
    exp_graph->SetLineColor(kBlack);

    auto sig1_graph = new TGraph(nSigPts);
    sig1_graph->SetName(Form("%s_Sig1_Graph",groupname.Data()));
    sig1_graph->SetFillStyle(1001);
    sig1_graph->SetFillColor(kGreen+1);
    sig1_graph->SetLineColor(kGreen+1);

    auto sig2_graph = new TGraph(nSigPts);
    sig2_graph->SetName(Form("%s_Sig2_Graph",groupname.Data()));
    sig2_graph->SetFillStyle(1001);
    sig2_graph->SetFillColor(kOrange);
    sig2_graph->SetLineColor(kOrange);

    // loop over each sample, setting the points appropriately
    for (auto isample = 0U; isample < nSamples; isample++)
    {
      const auto & sample = samples[isample];
      const auto & info = Combine::GMSBMap[sample];
      const auto & vals = info.rvalmap;
      const auto lambda = info.lambda;
      const auto xsec = info.xsec;

      theo_graph->SetPoint(isample,lambda,xsec);
      if (fDoObserved) obs_graph->SetPoint(isample,lambda,vals.at("robs")*xsec);
      exp_graph->SetPoint(isample,lambda,vals.at("rexp")*xsec);

      sig1_graph->SetPoint(isample,lambda,vals.at("r1sigup")*xsec);
      sig1_graph->SetPoint(nSigPts-isample-1,lambda,vals.at("r1sigdown")*xsec);

      sig2_graph->SetPoint(isample,lambda,vals.at("r2sigup")*xsec);
      sig2_graph->SetPoint(nSigPts-isample-1,lambda,vals.at("r2sigdown")*xsec);
    }

    // make legend
    auto leg = new TLegend(0.45,0.65,0.8,0.9);
    leg->SetName(Form("%s_Legend",groupname.Data()));
    leg->AddEntry(theo_graph,"Theory","l");
    if (fDoObserved) leg->AddEntry(obs_graph,"Observed Limit","l");
    leg->AddEntry(exp_graph,"Expected Limit","l");
    leg->AddEntry(sig1_graph,"#pm 1 std. dev.","f");
    leg->AddEntry(sig2_graph,"#pm 2 std. dev.","f");

    // make canvas
    auto canv = new TCanvas();
    canv->SetName(Form("%s_Canvas",groupname.Data()));
    canv->cd();
    canv->SetGridx(true);
    canv->SetGridy(true);
    canv->SetLogy(true);

    // label axes
    sig2_graph->GetXaxis()->SetTitle("#Lambda [GeV]");
    sig2_graph->GetYaxis()->SetTitle("95% C.L. #sigma(#tilde{#chi}^{1}_{0} #rightarrow #tilde{G} #gamma) [pb]");

    // zoom out
    sig2_graph->GetYaxis()->SetRangeUser(0.0001,10);

    // start drawing
    sig2_graph->Draw("AF");
    sig1_graph->Draw("F same");
    exp_graph->Draw("C same");
    if (fDoObserved) obs_graph->Draw("C same");
    theo_graph->Draw("C same");
    leg->Draw("same");

    // final touches
    Common::CMSLumi(canv,0);

    // save it!
    Common::SaveAs(canv,Form("%s_%s",fOutText.Data(),groupname.Data()));
      
    // write it!
    fOutFile->cd();
    theo_graph->Write(theo_graph->GetName(),TObject::kWriteDelete);
    if (fDoObserved) obs_graph->Write(obs_graph->GetName(),TObject::kWriteDelete);
    exp_graph->Write(exp_graph->GetName(),TObject::kWriteDelete);
    sig1_graph->Write(sig1_graph->GetName(),TObject::kWriteDelete);
    sig2_graph->Write(sig2_graph->GetName(),TObject::kWriteDelete);
    leg->Write(leg->GetName(),TObject::kWriteDelete);
    canv->Write(canv->GetName(),TObject::kWriteDelete);

    // delete it all for the next round
    delete canv;
    delete leg;
    delete sig2_graph;
    delete sig1_graph;
    delete exp_graph;
    delete obs_graph;
    delete theo_graph;
  }
}

void Limits1D::SetupCombine()
{
  std::cout << "Setting up Limits1D..." << std::endl;

  Combine::SetupRValVec(fDoObserved);
  Combine::SetupGMSB(fInDir,fInFileName);
  Combine::RemoveGMSBSamples();
  Combine::SetupGMSBSubGroups();
}
