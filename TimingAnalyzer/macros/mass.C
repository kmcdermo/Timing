void mass()
{
  TFile * file = TFile::Open("input/photondump.root");
  TTree * tree = (TTree*)file->Get("tree/tree");
  
  Float_t pt     = 0; tree->SetBranchAddress("gengr1pt" ,&pt);
  Float_t eta    = 0; tree->SetBranchAddress("gengr1eta",&eta);
  Float_t phi    = 0; tree->SetBranchAddress("gengr1phi",&phi);
  Float_t energy = 0; tree->SetBranchAddress("gengr1E"  ,&energy);  

  TH1F * h_mass = new TH1F("h_mass","mass",100,-1,1);

  for (UInt_t i = 0; i < tree->GetEntries(); i++)
  {
    tree->GetEntry(i);

    if (energy < 0) continue;

    TLorentzVector lorvec; 
    lorvec.SetPtEtaPhiE(pt,eta,phi,energy);  

    h_mass->Fill(lorvec.M());
  }

  TCanvas * canv = new TCanvas();
  canv->cd();
  
  h_mass->Draw();
}
