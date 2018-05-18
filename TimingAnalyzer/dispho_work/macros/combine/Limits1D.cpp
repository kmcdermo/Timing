#include "Limits1D.hh"

Limits1D::Limits1D(const TString & indir, const TString & infilename, const TString & outtext)
  : fInDir(indir), fInFileName(infilename), fOutText(outtext)
{  
  Limits1D::SetupGMSB();
  Limits1D::RemoveGMSBSamples();
  Limits1D::SetupGMSBSubGroups();

  // set style
  fTDRStyle = new TStyle("TDRStyle","Style for P-TDR");
  Config::SetTDRStyle(fTDRStyle);
  gROOT->ForceStyle();

  // make new output file
  fOutFile = TFile::Open(Form("%s.root",fOutText.Data()),"UPDATE");
}

Limits1D::~Limits1D() 
{
  delete fOutFile;
  delete fTDRStyle;
}

void Limits1D::MakeLimits1D()
{
  // loop over GMSB subgroups
  for (const auto & GMSBSubGroupPair : fGMSBSubGroupMap)
  {
    const auto & groupname = GMSBSubGroupPair.first;
    const auto & samples   = GMSBSubGroupPair.second;
    const auto nSamples = samples.size();
    const auto nSigPts  = 2*nSamples;

    // setup graphs
    auto theo_graph = new TGraph(nSamples);
    theo_graph->SetName("Theo_Graph");
    theo_graph->SetLineStyle(6);
    theo_graph->SetLineWidth(2);
    theo_graph->SetLineColor(kBlue);

    auto exp_graph = new TGraph(nSamples);
    exp_graph->SetName("Expected_Graph");
    exp_graph->SetLineStyle(9);
    exp_graph->SetLineWidth(2);
    exp_graph->SetLineColor(kBlack);

    auto sig1_graph = new TGraph(nSigPts);
    sig1_graph->SetName("Sig1_Graph");
    sig1_graph->SetFillStyle(1001);
    sig1_graph->SetFillColor(kGreen+1);
    sig1_graph->SetLineColor(kGreen+1);

    auto sig2_graph = new TGraph(nSigPts);
    sig2_graph->SetName("Sig2_Graph");
    sig2_graph->SetFillStyle(1001);
    sig2_graph->SetFillColor(kOrange);
    sig2_graph->SetLineColor(kOrange);

    // loop over each sample, setting the points appropriately
    for (auto isample = 0U; isample < nSamples; isample++)
    {
      const auto & sample = samples[isample];
      const auto & info = fGMSBMap[sample];
      const auto lambda = info.lambda;
      const auto xsec = info.xsec;
      
      theo_graph->SetPoint(isample,lambda,xsec);
      exp_graph->SetPoint(isample,lambda,info.rexp*xsec);

      sig1_graph->SetPoint(isample,lambda,info.r1sigup*xsec);
      sig1_graph->SetPoint(nSigPts-isample-1,lambda,info.r1sigdown*xsec);

      sig2_graph->SetPoint(isample,lambda,info.r2sigup*xsec);
      sig2_graph->SetPoint(nSigPts-isample-1,lambda,info.r2sigdown*xsec);
    }

    // make legend
    auto leg = new TLegend(0.5,0.65,0.85,0.9);
    leg->SetName("Legend");
    leg->AddEntry(theo_graph,"Theory","l");
    leg->AddEntry(exp_graph,"Expected Limit","l");
    leg->AddEntry(sig1_graph,"#pm 1 std. dev.","f");
    leg->AddEntry(sig2_graph,"#pm 2 std. dev.","f");

    // make canvas
    auto canv = new TCanvas();
    canv->SetName("Canvas");
    canv->cd();
    canv->SetGridx(true);
    canv->SetGridy(true);
    canv->SetLogy(true);

    // label axes
    sig2_graph->GetXaxis()->SetTitle("#Lambda [GeV]");
    sig2_graph->GetYaxis()->SetTitle("95% C.L. #sigma(#chi^{1}_{0} #rightarrow #tilde{G} #gamma) [pb]");

    // start drawing
    sig2_graph->Draw("AF");
    sig1_graph->Draw("F same");
    exp_graph->Draw("C same");
    theo_graph->Draw("C same");
    leg->Draw("same");

    // final touches
    Config::CMSLumi(canv,0);

    // save it!
    canv->SaveAs(Form("%s_%s.png",fOutText.Data(),groupname.Data()));

    // write it!
    fOutFile->cd();
    theo_graph->Write(theo_graph->GetName(),TObject::kWriteDelete);
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
    delete theo_graph;
  }
}

void Limits1D::SetupGMSB()
{
  // read in parameters... 
  std::fstream inparams("signal_config/all_params.txt");
  Float_t mass, width, br;
  TString lambda, ctau;

  while (inparams >> lambda >> ctau >> mass >> width >> br)
  {
    const TString s_ctau = (ctau.EqualTo("0.1") ? "0p1" : ctau);
    const TString name = "GMSB_L"+lambda+"TeV_CTau"+s_ctau+"cm";

    const Int_t i_lambda = lambda.Atoi();
    const Float_t f_ctau = ctau.Atof();

    fGMSBMap[name] = {lambda,i_lambda,s_ctau,f_ctau,mass,width,br};
  }
  
  // read in xsecs...
  std::fstream inxsecs("signal_config/all_xsecs.txt");
  Float_t xsec, exsec;

  while (inxsecs >> lambda >> ctau >> xsec >> exsec)
  {
    const TString s_ctau = (ctau.EqualTo("0.1") ? "0p1" : ctau);
    const TString name = "GMSB_L"+lambda+"TeV_CTau"+s_ctau+"cm";

    fGMSBMap[name].xsec  = xsec;
    fGMSBMap[name].exsec = exsec;
  }

  // read in r-values...
  for (auto & GMSBPair : fGMSBMap)
  {
    const auto & name = GMSBPair.first;
    auto       & info = GMSBPair.second;
    
    // get file
    const TString filename = Form("%s/%s%s.root",fInDir.Data(),fInFileName.Data(),name.Data());
    auto infile = TFile::Open(Form("%s",filename.Data()));
    auto isnull = Config::IsNullFile(infile);

    if (!isnull)
    {
      // get tree
      const TString treename = "limit";
      auto intree = (TTree*)infile->Get(Form("%s",treename.Data()));
      Config::CheckValidTree(intree,treename,filename);

      // get limit branch
      Double_t limit = 0; TBranch * b_limit = 0; TString s_limit = "limit";
      intree->SetBranchAddress(s_limit.Data(),&limit,&b_limit);

      // 5 Entries in tree, one for each quantile 

      // 2sigdown
      b_limit->GetEntry(0);
      info.r2sigdown = limit;

      // 1sigdown
      b_limit->GetEntry(1);
      info.r1sigdown = limit;
     
      // expected
      b_limit->GetEntry(2);
      info.rexp = limit;

      // 1sigup
      b_limit->GetEntry(3);
      info.r1sigup = limit;

      // 2sigup
      b_limit->GetEntry(4);
      info.r2sigup = limit;
      
      // delete once done
      delete intree;
      delete infile;
    }
    else
    {
      info.rexp      = -1.f;
      info.r1sigup   = -1.f;
      info.r1sigdown = -1.f;
      info.r2sigup   = -1.f;
      info.r2sigdown = -1.f;

      std::cout << "skipping this file: " << filename.Data() << std::endl;
    }
  }
}

void Limits1D::RemoveGMSBSamples()
{
  std::vector<TString> keysToRemove;
  for (const auto & GMSBPair : fGMSBMap)
  {
    const auto & name = GMSBPair.first;
    const auto & info = GMSBPair.second;
    
    if (info.rexp < 0.f) keysToRemove.emplace_back(name);
  }

  for (const auto & keyToRemove : keysToRemove) fGMSBMap.erase(keyToRemove);

  // ****************** HACK FOR NOW *************** //
  fGMSBMap.erase("GMSB_L200TeV_CTau400cm");
}

void Limits1D::SetupGMSBSubGroups()
{
  for (const auto & GMSBPair : fGMSBMap)
  {
    const auto & name = GMSBPair.first;
    const auto & info = GMSBPair.second;

    fGMSBSubGroupMap["GMSB_CTau"+info.s_ctau+"cm"].emplace_back(name);
  }
}
