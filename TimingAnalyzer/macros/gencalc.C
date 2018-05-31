#include "gencalc.hh"

void gencalc()
{
  // output stuff
  const TString outname = "sample";
  TFile * outfile = TFile::Open(outname.Data(),"RECREATE");

  // Get Input
  TFile * file = TFile::Open("dispho.root");
  TTree * tree = (TTree*)file->Get("tree/disphotree");

  // start setting variable
  Int_t       nNeutoPhGr = 0; 
  TBranch * b_nNeutoPhGr = 0; 
  TString   s_nNeutoPhGr = "nNeutoPhGr";
  tree->SetBranchAddress(Form("%s",s_nNeutoPhGr.Data()), &nNeutoPhGr, &b_nNeutoPhGr);

  // Set branches
  std::vector<GenStruct> GenInfos(Config::ngen);
  for (Int_t igen = 0; igen < Config::ngen; igen++)
  {
    auto & GenInfo = GenInfos[igen];
    tree->SetBranchAddress(Form("%s_%i",GenInfo.s_genNmass.Data(),igen), &GenInfo.genNmass, &GenInfo.b_genNmass);
    tree->SetBranchAddress(Form("%s_%i",GenInfo.s_genNE.Data(),igen), &GenInfo.genNE, &GenInfo.b_genNE);
    tree->SetBranchAddress(Form("%s_%i",GenInfo.s_genNpt.Data(),igen), &GenInfo.genNpt, &GenInfo.b_genNpt);
    tree->SetBranchAddress(Form("%s_%i",GenInfo.s_genNphi.Data(),igen), &GenInfo.genNphi, &GenInfo.b_genNphi);
    tree->SetBranchAddress(Form("%s_%i",GenInfo.s_genNeta.Data(),igen), &GenInfo.genNeta, &GenInfo.b_genNeta);
    tree->SetBranchAddress(Form("%s_%i",GenInfo.s_genNprodvx.Data(),igen), &GenInfo.genNprodvx, &GenInfo.b_genNprodvx);
    tree->SetBranchAddress(Form("%s_%i",GenInfo.s_genNprodvy.Data(),igen), &GenInfo.genNprodvy, &GenInfo.b_genNprodvy);
    tree->SetBranchAddress(Form("%s_%i",GenInfo.s_genNprodvz.Data(),igen), &GenInfo.genNprodvz, &GenInfo.b_genNprodvz);
    tree->SetBranchAddress(Form("%s_%i",GenInfo.s_genNdecayvx.Data(),igen), &GenInfo.genNdecayvx, &GenInfo.b_genNdecayvx);
    tree->SetBranchAddress(Form("%s_%i",GenInfo.s_genNdecayvy.Data(),igen), &GenInfo.genNdecayvy, &GenInfo.b_genNdecayvy);
    tree->SetBranchAddress(Form("%s_%i",GenInfo.s_genNdecayvz.Data(),igen), &GenInfo.genNdecayvz, &GenInfo.b_genNdecayvz);
    tree->SetBranchAddress(Form("%s_%i",GenInfo.s_genphE.Data(),igen), &GenInfo.genphE, &GenInfo.b_genphE);
    tree->SetBranchAddress(Form("%s_%i",GenInfo.s_genphpt.Data(),igen), &GenInfo.genphpt, &GenInfo.b_genphpt);
    tree->SetBranchAddress(Form("%s_%i",GenInfo.s_genphphi.Data(),igen), &GenInfo.genphphi, &GenInfo.b_genphphi);
    tree->SetBranchAddress(Form("%s_%i",GenInfo.s_genpheta.Data(),igen), &GenInfo.genpheta, &GenInfo.b_genpheta);
  }

  // Set hists
  TH1F * ctau = new TH1F("ctau","Neutralino Travel Distance (c#tau);c#tau (#tilde{#chi}^{1}_{0} Rest Frame) [cm];nNeutralinos",100,0,2);
  ctau->Sumw2();
  TH2F * decay2D = new TH2F("decay2D","Neutralino Decay Vertex;Radius (Lab Frame) [cm];Z (Lab Frame) [cm];nNeutralinos",1000,0,1000,1000,0,1000);
  decay2D->Sumw2();
  TH1F * time = new TH1F("arrival_time","Photon Arrival Time in ECAL;Time (Lab Frame) [ns];nPhotons",100,0,1000);
  time->Sumw2();
  TH1F * det = new TH1F("detarrival","Photon ECAL Arrival;;nNeutralinos",3,0,3);
  det->Sumw2();

  std::vector<TString> labels = {"EB","EE","Missed"};
  for (int ibin = 1; ibin <= det->GetXaxis()->GetNbins(); ibin++){det->GetXaxis()->SetBinLabel(ibin,labels[ibin-1]);}

  const UInt_t entries = tree->GetEntries();
  for (UInt_t entry = 0; entry < entries; entry++)
  {
    b_nNeutoPhGr ->GetEntry(entry);

    for (Int_t igen = 0; igen < nNeutoPhGr; igen++)
    {
      auto & GenInfo = GenInfos[igen];

      GenInfo.b_genNmass   ->GetEntry(entry);
      GenInfo.b_genNE      ->GetEntry(entry);
      GenInfo.b_genNpt     ->GetEntry(entry);
      GenInfo.b_genNeta    ->GetEntry(entry);
      GenInfo.b_genNphi    ->GetEntry(entry);
      GenInfo.b_genNprodvx ->GetEntry(entry);
      GenInfo.b_genNprodvy ->GetEntry(entry);
      GenInfo.b_genNprodvz ->GetEntry(entry);
      GenInfo.b_genNdecayvx->GetEntry(entry);
      GenInfo.b_genNdecayvy->GetEntry(entry);
      GenInfo.b_genNdecayvz->GetEntry(entry);
      GenInfo.b_genphE     ->GetEntry(entry);
      GenInfo.b_genphpt    ->GetEntry(entry);
      GenInfo.b_genpheta   ->GetEntry(entry);
      GenInfo.b_genphphi   ->GetEntry(entry);

      TLorentzVector genN_lorvec; genN_lorvec.SetPtEtaPhiE(GenInfo.genNpt,GenInfo.genNeta,GenInfo.genNphi,GenInfo.genNE);
      const Float_t genNp       = hypo(genN_lorvec.Px(),genN_lorvec.Py(),genN_lorvec.Pz());
      const Float_t genNbg      = betag(genNp,GenInfo.genNmass);
      const Float_t genNtraveld = hypo(GenInfo.genNdecayvx-GenInfo.genNprodvx,GenInfo.genNdecayvy-GenInfo.genNprodvy,GenInfo.genNdecayvz-GenInfo.genNprodvz);
      const Float_t genNctau    = genNtraveld/genNbg;
      const Float_t genNgamma   = gamma(genNp,GenInfo.genNmass);
      const Float_t genNdecayvr = hypo(GenInfo.genNdecayvx,GenInfo.genNdecayvy);
      const Float_t genphslope  = slope_of_eta(GenInfo.genpheta);
      const Float_t genNlabtime = tau(genNctau,genNgamma);
    
      Float_t detector = -1.f;
      const Float_t genphtime   = GetGenPhotonArrivalTime(genNdecayvr,GenInfo.genNdecayvz,genphslope,genNlabtime,detector);

      ctau->Fill(genNctau);
      time->Fill(genphtime*Config::ns_per_s);
      det->Fill(detecor);
      decay2D->Fill(genNdecayvr,GenInfo.genNdecayvz);
    } // end loop neutralinos
  } // end loop entries

  TFormula form("texp","[0]*exp(-x/[1])");
  TF1 * fit = new TF1("texp_fit",form.GetName(),0.f,100.f);
  fit->SetParName(0,"Norm"); fit->SetParameter(0,ctau->GetMaximum());
  fit->SetParName(1,"d");    fit->SetParameter(1,ctau->GetMean());
  ctau->Fit(fit->GetName());
  
  TCanvas * canv = new TCanvas();
  canv->cd();
  canv->SetLogy();
  ctau->Draw();
  fit->Draw("same");
  canv->SaveAs("ctau.png");

  outfile->cd();
  ctau->Write();
  time->Write();
  det->Write();
  decay2D->Write():
}

Float_t GetGenPhotonArrivalTime(const Float_t r0, const Float_t z0, const Float_t slope, const Float_t tau0, Float_t& detector)
{
  Float_t time = -1.f;
  
  const Float_t z = z0 + ( (ECAL::rEB - r0) / slope );
  const Float_t r = r0 + ( slope * (ECAL::zEE - z0) );
  if (std::abs(z) < ECAL::zEB) 
  {
    time = (hypo(ECAL::rEB-r0,z-z0) / Config::sol) - (hypo(ECAL::rEB,z) / Config::sol) + tau0;
    detector = 0.5;
  }
  else if (r > ECAL::rEEmin && r < ECAL::rEEmax)
  {
    time = (hypo(r-r0,ECAL::zEE-z0) / Config::sol) - (hypo(r0,ECAL::zEE) / Config::sol) + tau0; 
    detector = 1.5;
  }
  else
  {
    detector = 2.5;
  }

  return time;
}
