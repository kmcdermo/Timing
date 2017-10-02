#include "../interface/PUReweight.hh"

PUReweight::PUReweight() {
  // save samples for PU weighting
  for (TStrBoolMapIter iter = Config::SampleMap.begin(); iter != Config::SampleMap.end(); ++iter) {
    if ((*iter).second) // isMC == true
    {
      fMCNames.push_back((*iter).first);
    }
    else // data
    {
      fDataNames.push_back((*iter).first);
    }
  }

  // store for later ... would rather have move semantics ... iterators too annoying
  fNData = fDataNames.size();
  fNMC   = fMCNames.size();

  // Initialize output TH1F's for data
  fOutDataNvtx = new TH1F("nvtx_data","",Config::nbinsvtx,0.,Double_t(Config::nbinsvtx));
  fOutDataNvtx->Sumw2();

  // Initialize outputs for MC
  fOutMCNvtx = new TH1F("nvtx_mc","",Config::nbinsvtx,0.,Double_t(Config::nbinsvtx));
  fOutMCNvtx->Sumw2();

  // Intialize Ratio Hist
  fOutDataOverMCNvtx = new TH1F(Form("%s",Config::puplotname.Data()),"",Config::nbinsvtx,0.,Double_t(Config::nbinsvtx));
  fOutDataOverMCNvtx->Sumw2();

  // Make PURW outdirs --> no need to use MakeSubDirs
  fOutDir  = Form("%s/%s",Config::outdir.Data(),Config::pusubdir.Data());
  MakeOutDir(fOutDir);
  MakeOutDir(fOutDir+"/log");
  MakeOutDir(fOutDir+"/lin");
  fOutFile = new TFile(Form("%s/%s",fOutDir.Data(),Config::pufilename.Data()),"RECREATE");
}

PUReweight::~PUReweight() {
  //delete hists
  delete fOutDataNvtx;
  delete fOutMCNvtx;
  delete fOutDataOverMCNvtx;

  //delete
  delete fOutFile;
}

void PUReweight::GetPUWeights(){
  // cut to be used in tree
  TString basecut = Config::selection;
  
  // get vtx distribution for data first
  for (Int_t data = 0; data < fNData; data++){
    TString cut = basecut.Data();
    cut.Append(" && (hltdoubleel33 || hltdoubleel37)");

    // files + trees + tmp hist for data
    TString filename = Form("input/DATA/%s/%s/%s",Config::year.Data(),fDataNames[data].Data(),"skimmedtree.root");
    TFile * file = TFile::Open(filename.Data());
    CheckValidFile(file,filename);

    TString treename = "tree/tree";
    TTree * tree = (TTree*)file->Get(treename.Data());
    CheckValidTree(tree,treename,filename);      
    TH1F * tmpnvtx = new TH1F("tmpnvtx","",Config::nbinsvtx,0.,Double_t(Config::nbinsvtx));
    tmpnvtx->Sumw2();

    // fill each input data nvtx
    std::cout << "Reading data nvtx: " << filename.Data() << " with cut: " << cut.Data() << std::endl;
    tree->Draw("nvtx>>tmpnvtx",Form("%s",cut.Data()),"goff");
    
    // add input data hist to total data hist
    fOutDataNvtx->Add(tmpnvtx);

    // delete objects
    delete tmpnvtx;
    delete tree;
    delete file;
  }

  // get vtx distribution for mc second
  for (Int_t mc = 0; mc < fNMC; mc++){
    // create appropriate selection cut
    TString cut = basecut;
    cut.Prepend("( ");
    cut.Append(Form(" * (%f * %f * wgt / %f)",Config::SampleXsecMap[fMCNames[mc]], Config::lumi, Config::SampleWgtsumMap[fMCNames[mc]])); // make sure to add weights for all mc!
    cut.Append(" )");

    // files + trees for mc + tmp hists
    TString filename = Form("input/MC/%s/%s/%s",Config::year.Data(),fMCNames[mc].Data(),"skimmedtree.root");
    TFile * file = TFile::Open(filename.Data());
    CheckValidFile(file,filename);

    TString treename = "tree/tree";
    TTree * tree = (TTree*)file->Get(treename.Data());
    CheckValidTree(tree,treename,filename);            
    TH1F * tmpnvtx = new TH1F("tmpnvtx","",Config::nbinsvtx,0.,Double_t(Config::nbinsvtx));
    tmpnvtx->Sumw2();

    // fill each input mc nvtx
    std::cout << "Reading MC nvtx: " << filename.Data() << " with cut: " << cut.Data() << std::endl;
    tree->Draw("nvtx>>tmpnvtx",Form("%s",cut.Data()),"goff");

    // add input mc hist to total mc hist
    fOutMCNvtx->Add(tmpnvtx);

    // delete objects
    delete tmpnvtx;
    delete tree;
    delete file;
  }

  // Set line colors
  fOutDataNvtx->SetLineColor(kRed);
  fOutDataNvtx->GetXaxis()->SetTitle("nVertices");
  fOutDataNvtx->GetYaxis()->SetTitle("Events");

  fOutMCNvtx->SetLineColor(kBlue);
  fOutMCNvtx->GetXaxis()->SetTitle("nVertices");
  fOutMCNvtx->GetYaxis()->SetTitle("Events");
  
  // use these for scaling and rescaling
  const Float_t int_DataNvtx = fOutDataNvtx->Integral();
  const Float_t int_MCNvtx   = fOutMCNvtx->Integral();

  TCanvas * c0 = new TCanvas(); // Draw before reweighting --> unscaled
  c0->cd();
  c0->SetTitle("Before PU Reweighting Unnormalized");

  // draw and save in output directory --> appended by what selection we used for this pu reweight
  fOutDataNvtx->Draw("PE");
  fOutMCNvtx->Draw("HIST SAME");

  c0->SetLogy(1); // save log
  c0->SaveAs(Form("%s/log/nvtx_beforePURW_unnorm.%s",fOutDir.Data(),Config::outtype.Data()));

  c0->SetLogy(0); // save lin
  c0->SaveAs(Form("%s/lin/nvtx_beforePURW_unnorm.%s",fOutDir.Data(),Config::outtype.Data()));
  
  /////////////////////////////////////////////
  //       SCALE HERE TO GET REWEIGHTING     //
  /////////////////////////////////////////////
  // scale to unit area to not bias against data
  fOutDataNvtx->Scale(1.0/int_DataNvtx);  
  fOutMCNvtx->Scale(1.0/int_MCNvtx);

  // Draw before reweighting -- scaled
  TCanvas * c1 = new TCanvas();
  c1->cd();
  c1->SetTitle("Before PU Reweighting Normalized");

  // draw and save in output directory --> appended by what selection we used for this pu reweight
  fOutDataNvtx->Draw("PE");
  fOutMCNvtx->Draw("HIST SAME");

  c1->SetLogy(1); // save log
  c1->SaveAs(Form("%s/log/nvtx_beforePURW_norm.%s",fOutDir.Data(),Config::outtype.Data()));

  c1->SetLogy(0); // save lin
  c1->SaveAs(Form("%s/lin/nvtx_beforePURW_norm.%s",fOutDir.Data(),Config::outtype.Data()));

  /////////////////////////////////////////////
  //      DIVIDE HERE TO GET REWEIGHTING     //
  /////////////////////////////////////////////

  // copy fOutDataNvtx to save output of reweights properly
  for (Int_t ibin = 1; ibin <= Config::nbinsvtx; ibin++) {
    fOutDataOverMCNvtx->SetBinContent(ibin,fOutDataNvtx->GetBinContent(ibin));
  }

  // divide Data/MC after copy, now this original hist will be used for reweighting 
  fOutDataOverMCNvtx->Divide(fOutMCNvtx);
  fOutDataOverMCNvtx->GetXaxis()->SetTitle("nVertices");
  fOutDataOverMCNvtx->GetYaxis()->SetTitle("Scale Factor");

  /////////////////////////////////////////////
  //      STORE HERE TO USE REWEIGHTING      //
  /////////////////////////////////////////////

  fOutFile->cd();
  fOutDataOverMCNvtx->Write();

  // scale MC to demonstrate that it works
  for (Int_t ibin = 1; ibin <= Config::nbinsvtx; ibin++) {
    Float_t tmp = fOutMCNvtx->GetBinContent(ibin);
    fOutMCNvtx->SetBinContent(ibin,fOutDataOverMCNvtx->GetBinContent(ibin)*tmp); 
  }

  // Draw after reweighting 
  TCanvas * c2 = new TCanvas();
  c2->cd();
  c2->SetTitle("After PU Reweighting Normalized");

  // draw output and save it, see comment above about selection
  fOutDataNvtx->Draw("PE");
  fOutMCNvtx->Draw("HIST SAME");

  c2->SetLogy(1); // save log
  c2->SaveAs(Form("%s/log/nvtx_afterPURW_norm.%s",fOutDir.Data(),Config::outtype.Data()));

  c2->SetLogy(0); // save lin
  c2->SaveAs(Form("%s/lin/nvtx_afterPURW_norm.%s",fOutDir.Data(),Config::outtype.Data()));

  TCanvas * c3 = new TCanvas(); // Draw before reweighting --> unscaled
  c3->cd();
  c3->SetTitle("After PU Reweighting Unnormalized"); 

  // now that the reweighting is applied, see total events again
  fOutDataNvtx->Scale(int_DataNvtx);
  fOutMCNvtx->Scale(int_MCNvtx);
  
  fOutDataNvtx->Draw("PE");
  fOutMCNvtx->Draw("HIST SAME");

  c3->SetLogy(1); // save log
  c3->SaveAs(Form("%s/log/nvtx_afterPURW_unnorm.%s",fOutDir.Data(),Config::outtype.Data()));

  c3->SetLogy(0); // save lin
  c3->SaveAs(Form("%s/lin/nvtx_afterPURW_unnorm.%s",fOutDir.Data(),Config::outtype.Data()));
  
  delete c0;
  delete c1;
  delete c2;
  delete c3;
}
