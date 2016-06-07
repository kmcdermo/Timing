void comparetrees(){
  gStyle->SetOptStat(0);

  TString  path   = "input/MC/dyll";
  TString  var    = "zmass";
  TString  label1 = "smear";
  TString  label2 = "unsmear";
  Int_t    nbinsx = 100; 
  Double_t xlow   = 76.;
  Double_t xhigh  = 106.;
  Bool_t   isLogY = false;
  
  // get input full trees
  TFile * file1 = TFile::Open(Form("%s/skimmedtree.root",path.Data())); // skimmed + smeared
  TTree * tree1 = (TTree*)file1->Get("tree/tree");
  
  TFile * file2 = TFile::Open(Form("%s/skimmedtree_unsmeared.root",path.Data())); // skimmed + unsmeared
  TTree * tree2 = (TTree*)file2->Get("tree/tree");

  // make hists
  TH1F * hist1 = new TH1F(Form("%s_1",var.Data()),Form("%s_1",var.Data()),nbinsx,xlow,xhigh);
  hist1->SetTitle(Form("%s %s vs. %s (MC)",var.Data(),label1.Data(),label2.Data()));
  hist1->GetXaxis()->SetTitle(var.Data());
  hist1->GetYaxis()->SetTitle("Events");
  hist1->Sumw2();
  TH1F * hist2 = new TH1F(Form("%s_2",var.Data()),Form("%s_2",var.Data()),nbinsx,xlow,xhigh);
  hist2->SetTitle(Form("%s %s vs. %s (MC)",var.Data(),label1.Data(),label2.Data()));
  hist2->GetXaxis()->SetTitle(var.Data());
  hist2->GetYaxis()->SetTitle("Events");
  hist2->Sumw2();

  // Fill histsos
  tree1->Draw(Form("%s>>%s",var.Data(),hist1->GetName()),"","goff");
  tree2->Draw(Form("%s>>%s",var.Data(),hist2->GetName()),"","goff");

  // Get pile-up weights
  TString purwfname = "input/PU/pileupWeights.root";
  TFile * purwfile  = TFile::Open(purwfname.Data());
      
  TH1D * gen_pu     = (TH1D*) purwfile->Get("generated_pu");
  TH1D * puweights  = (TH1D*) purwfile->Get("weights");
  TH1D * weightedPU = (TH1D*) gen_pu->Clone("weightedPU");
  weightedPU->Multiply(puweights);
  TH1D * weights    = (TH1D*) puweights->Clone("rescaledWeights");
  weights->Scale( gen_pu->Integral(1,50) / weightedPU->Integral(1,50) );
  
  std::vector<Double_t> fPUweights;
  for (Int_t i = 1; i <= 50; i++){
    fPUweights.push_back(weights->GetBinContent(i));
  }
  
  // const mc weight
  Double_t mcweight = 2.301 * 1000 * 6025.2 / 4.44757e+11; // for dyll
  
  // set branches 
  Float_t var1; tree1->SetBranchAddress(var.Data(),&var1);
  Float_t var2; tree2->SetBranchAddress(var.Data(),&var2);

  Float_t wgt1; tree1->SetBranchAddress("wgt",&wgt1);
  Float_t wgt2; tree2->SetBranchAddress("wgt",&wgt2);

  Int_t putrue1; tree1->SetBranchAddress("putrue",&putrue1);
  Int_t putrue2; tree2->SetBranchAddress("putrue",&putrue2);

  // loop over trees
  for (UInt_t j = 0; j < tree1->GetEntries(); j++){
    tree1->GetEntry(j);

    Float_t weight = wgt1 * mcweight * fPUweights[putrue1];
    hist1->Fill(var1,weight);
  }

  for (UInt_t j = 0; j < tree2->GetEntries(); j++){
    tree2->GetEntry(j);

    Float_t weight = wgt2 * mcweight * fPUweights[putrue2];
    hist2->Fill(var2,weight);
  }

  TCanvas * c1 = new TCanvas();
  c1->cd();
  c1->SetLogy(isLogY);

  TPad * uppad  = new TPad("uppad","",0,0.3,1.0,1.0);
  uppad->SetBottomMargin(0); // Upper and lower plot are joined
  
  TPad * lowpad = new TPad("lowpad","",0,0.05,1.0,0.3);
  lowpad->SetTopMargin(0);
  lowpad->SetBottomMargin(0.3);
  lowpad->SetGridy(1);
  
  c1->cd();
  uppad->Draw();
  uppad->cd();
  
  hist1->GetYaxis()->SetTitleSize(.045);
  hist1->GetYaxis()->SetTitleOffset(.8);

  Double_t max1 = hist1->GetMaximum();
  Double_t max2 = hist2->GetMaximum();

  hist1->SetMaximum(max1>max2?max1*1.1:max2*1.1);

  hist1->SetLineColor(kBlue);
  hist1->Draw("EP");

  hist2->SetLineColor(kRed);
  hist2->Draw("EP SAME");

  TLegend * leg = new TLegend(.7,.7,.9,.9);
  leg->AddEntry(hist1,label1.Data(),"epl");
  leg->AddEntry(hist2,label2.Data(),"epl");
  leg->Draw("SAME");

  c1->cd();
  lowpad->Draw();
  lowpad->cd();

  TH1F * hist1clone = (TH1F*)hist1->Clone(Form("%s_clone",hist1->GetName()));
  TH1F * hist2clone = (TH1F*)hist2->Clone(Form("%s_clone",hist2->GetName()));

  hist1clone->SetTitle("");
  hist1clone->GetXaxis()->SetTitleSize(.11);
  hist1clone->GetYaxis()->SetTitleSize(.11);
  hist1clone->GetYaxis()->SetTitleOffset(.3);
  hist1clone->GetYaxis()->SetTitle(Form("%s/%s",label1.Data(),label2.Data()));
  hist1clone->GetXaxis()->SetLabelSize(.11);
  hist1clone->GetYaxis()->SetLabelSize(.11);
  hist1clone->SetMinimum(-0.1);
  hist1clone->SetMaximum( 2.1);
  hist1clone->SetStats(0);
  hist1clone->Divide(hist2clone);
  hist1clone->Draw("EP");

  c1->cd();
  c1->SaveAs(Form("%s_%s_v_%s_%s.png",var.Data(), label1.Data(), label2.Data(), (isLogY?"log":"lin") ));


  std::cout << hist1->Integral() << " " << hist2->Integral() << std::endl;
}
