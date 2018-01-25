namespace Config
{
  static const Int_t   ipho = 1;
  static const Int_t   npho = ipho+1;
  static const Float_t ptcut = 70.f;
  static const Float_t sol  = 2.99792458e10; // cm/s
  static const Float_t ns_per_s = 1e9; 
  static const Float_t cm_per_m = 1e2; 
};

inline Float_t rad2 (const Float_t x, const Float_t y, const Float_t z = 0.f){return x*x + y*y + z*z;}
inline Float_t hypo (const Float_t x, const Float_t y, const Float_t z = 0.f){return std::sqrt(rad2(x,y,z));}
inline Float_t theta(const Float_t eta){return 2.f*std::atan(std::exp(-eta));}
inline Float_t slope_of_eta(const Float_t eta){return std::tan(theta(eta));}
inline Float_t r_of_eta(const Float_t eta, const Float_t z){return z * slope_of_eta(eta);}
inline Float_t z_of_eta(const Float_t eta, const Float_t r){return r / slope_of_eta(eta);}
inline Float_t gamma(const Float_t p, const Float_t m){return std::sqrt(1.f+std::pow(p/m,2));}
inline Float_t betag(const Float_t p, const Float_t m){return std::abs(p/m);}
inline Float_t tau  (const Float_t ctau, const Float_t gamma){return ctau * gamma / Config::sol;}

namespace ECAL
{
  static const Float_t etaEB    = 1.4442;
  static const Float_t etaEEmin = 1.566;
  static const Float_t etaEEmax = 2.5;

  static const Float_t rEB = 129.f; // 1.29 m
  static const Float_t zEB = z_of_eta(ECAL::etaEB,ECAL::rEB);

  static const Float_t zEE    = 314.f; // 3.14 m 
  static const Float_t rEEmin = r_of_eta(ECAL::etaEEmax,ECAL::zEE);
  static const Float_t rEEmax = r_of_eta(ECAL::etaEEmin,ECAL::zEE);
};

Float_t GetGenPhotonArrivalTime(const Float_t r0, const Float_t z0, const Float_t slope, const Float_t tau0, Float_t& arrival)
{
  Float_t time = -1.f;
  
  const Float_t z = z0 + ( (ECAL::rEB - r0) / slope );
  const Float_t r = r0 + ( slope * (ECAL::zEE - z0) );
  if (std::abs(z) < ECAL::zEB) 
  {
    time = (hypo(ECAL::rEB-r0,z-z0) / Config::sol) - (hypo(ECAL::rEB,z) / Config::sol) + tau0;
    arrival = 0.5;
  }
  else if (r > ECAL::rEEmin && r < ECAL::rEEmax)
  {
    time = (hypo(r-r0,ECAL::zEE-z0) / Config::sol) - (hypo(r0,ECAL::zEE) / Config::sol) + tau0; 
    arrival = 1.5;
  }
  else
  {
    arrival = 2.5;
  }

  return time;
}

void gencalc()
{
  // Get Input
  TFile * file = TFile::Open("dispho.root");
  TTree * tree = (TTree*)file->Get("tree/disphotree");

  // Set branches
  Int_t   nNeutoPhGr  = 0; TBranch * b_nNeutoPhGr  = 0; tree->SetBranchAddress(     "nNeutoPhGr"                  , &nNeutoPhGr , &b_nNeutoPhGr);
  Float_t genNmass    = 0; TBranch * b_genNmass    = 0; tree->SetBranchAddress(Form("genNmass_%i"   ,Config::ipho), &genNmass   , &b_genNmass);
  Float_t genNE       = 0; TBranch * b_genNE       = 0; tree->SetBranchAddress(Form("genNE_%i"      ,Config::ipho), &genNE      , &b_genNE);
  Float_t genNpt      = 0; TBranch * b_genNpt      = 0; tree->SetBranchAddress(Form("genNpt_%i"     ,Config::ipho), &genNpt     , &b_genNpt);
  Float_t genNphi     = 0; TBranch * b_genNphi     = 0; tree->SetBranchAddress(Form("genNphi_%i"    ,Config::ipho), &genNphi    , &b_genNphi);
  Float_t genNeta     = 0; TBranch * b_genNeta     = 0; tree->SetBranchAddress(Form("genNeta_%i"    ,Config::ipho), &genNeta    , &b_genNeta);
  Float_t genNprodvx  = 0; TBranch * b_genNprodvx  = 0; tree->SetBranchAddress(Form("genNprodvx_%i" ,Config::ipho), &genNprodvx , &b_genNprodvx);
  Float_t genNprodvy  = 0; TBranch * b_genNprodvy  = 0; tree->SetBranchAddress(Form("genNprodvy_%i" ,Config::ipho), &genNprodvy , &b_genNprodvy);
  Float_t genNprodvz  = 0; TBranch * b_genNprodvz  = 0; tree->SetBranchAddress(Form("genNprodvz_%i" ,Config::ipho), &genNprodvz , &b_genNprodvz);
  Float_t genNdecayvx = 0; TBranch * b_genNdecayvx = 0; tree->SetBranchAddress(Form("genNdecayvx_%i",Config::ipho), &genNdecayvx, &b_genNdecayvx);
  Float_t genNdecayvy = 0; TBranch * b_genNdecayvy = 0; tree->SetBranchAddress(Form("genNdecayvy_%i",Config::ipho), &genNdecayvy, &b_genNdecayvy);
  Float_t genNdecayvz = 0; TBranch * b_genNdecayvz = 0; tree->SetBranchAddress(Form("genNdecayvz_%i",Config::ipho), &genNdecayvz, &b_genNdecayvz);
  Float_t genphE      = 0; TBranch * b_genphE      = 0; tree->SetBranchAddress(Form("genphE_%i"     ,Config::ipho), &genphE     , &b_genphE);
  Float_t genphpt     = 0; TBranch * b_genphpt     = 0; tree->SetBranchAddress(Form("genphpt_%i"    ,Config::ipho), &genphpt    , &b_genphpt);
  Float_t genphphi    = 0; TBranch * b_genphphi    = 0; tree->SetBranchAddress(Form("genphphi_%i"   ,Config::ipho), &genphphi   , &b_genphphi);
  Float_t genpheta    = 0; TBranch * b_genpheta    = 0; tree->SetBranchAddress(Form("genpheta_%i"   ,Config::ipho), &genpheta   , &b_genpheta);

  // Set hists
  TH1F * ctau = new TH1F("ctau","ctau",100,0,4000);
  ctau->Sumw2();
  TH1F * time = new TH1F("arrival_time","arrival time [ns]",100,0,1000);
  time->Sumw2();
  TH1F * det = new TH1F("detarrival","detector arrival",3,0,3);
  det->Sumw2();

  std::vector<TString> labels = {"EB","EE","Missed"};
  for (int ibin = 1; ibin <= det->GetXaxis()->GetNbins(); ibin++){det->GetXaxis()->SetBinLabel(ibin,labels[ibin-1]);}

  const UInt_t entries = tree->GetEntries();
  std::cout << "Entries: " << entries << std::endl;
  for (UInt_t entry = 0; entry < entries; entry++)
  {
    b_nNeutoPhGr ->GetEntry(entry);
    b_genNmass   ->GetEntry(entry);
    b_genNE      ->GetEntry(entry);
    b_genNpt     ->GetEntry(entry);
    b_genNeta    ->GetEntry(entry);
    b_genNphi    ->GetEntry(entry);
    b_genNprodvx ->GetEntry(entry);
    b_genNprodvy ->GetEntry(entry);
    b_genNprodvz ->GetEntry(entry);
    b_genNdecayvx->GetEntry(entry);
    b_genNdecayvy->GetEntry(entry);
    b_genNdecayvz->GetEntry(entry);
    b_genphE     ->GetEntry(entry);
    b_genphpt    ->GetEntry(entry);
    b_genpheta   ->GetEntry(entry);
    b_genphphi   ->GetEntry(entry);

    if (nNeutoPhGr < Config::npho) continue;
    if (genphpt < Config::ptcut) continue;

    TLorentzVector genN_lorvec; genN_lorvec.SetPtEtaPhiE(genNpt,genNeta,genNphi,genNE);
    const Float_t genNp       = hypo(genN_lorvec.Px(),genN_lorvec.Py(),genN_lorvec.Pz());
    const Float_t genNbg      = betag(genNp,genNmass);
    const Float_t genNtraveld = hypo(genNdecayvx-genNprodvx,genNdecayvy-genNprodvy,genNdecayvz-genNprodvz);
    const Float_t genNctau    = genNtraveld/genNbg; 
    const Float_t genNgamma   = gamma(genNp,genNmass);
    const Float_t genNdecayvr = hypo(genNdecayvx,genNdecayvy);
    const Float_t genphslope  = slope_of_eta(genpheta);
    const Float_t genNlabtime = tau(genNctau,genNgamma);
    
    Float_t arrival = -1.f;
    const Float_t genphtime   = GetGenPhotonArrivalTime(genNdecayvr,genNdecayvz,genphslope,genNlabtime,arrival);

    ctau->Fill(genNctau);
    time->Fill(genphtime*Config::ns_per_s);
    det->Fill(arrival);
  }

  for (int ibin = 1; ibin <= det->GetXaxis()->GetNbins(); ibin++)
  {
    std::cout << labels[ibin-1].Data() << " " << det->GetBinContent(ibin) << std::endl;
  }


//   TCanvas * canv = new TCanvas();
//   canv->cd();
//   canv->SetLogy();
//   rad->Scale(1.f/rad->Integral());
//   rad->Draw();

//  det->Scale(1.f/det->Integral());
//  det->GetYaxis()->SetRangeUser(0.01,1);

//  det->Draw("text");
}
