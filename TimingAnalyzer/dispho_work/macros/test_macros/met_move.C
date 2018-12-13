#include "Common.cpp+"

struct Photon
{
  Photon(){}

  Float_t pt = 0.f; 
  Float_t phi = 0.f;
  Float_t eta = 0.f;
  Bool_t isOOT = 0;
  
  TBranch * b_pt = 0;
  TBranch * b_phi = 0;
  TBranch * b_eta = 0;
  TBranch * b_isOOT = 0;

  const std::string s_pt = "pt";
  const std::string s_phi = "phi";
  const std::string s_eta = "eta";
  const std::string s_isOOT = "isOOT";
};

inline Float_t deltaR(const Float_t phi1, const Float_t eta1, const Float_t phi2, const Float_t eta2)
{
  auto delphi = std::abs(phi1-phi2);
  if (delphi > Common::PI) delphi -= Common::TwoPI;
  return Common::hypot(eta1-eta2,delphi);
}

inline void resetBoolVec(std::vector<Bool_t> & vec)
{
  for (auto obj : vec) obj = false;
}

void met_move()
{
  // get inputs
  const TString filename = "skims/v2/orig_2phosCR/gjets.root";
  auto file = TFile::Open(filename.Data(),"UPDATE");
  Common::CheckValidFile(file,filename);
  
  const TString treename = "Data_Tree";
  auto tree = (TTree*)file->Get(treename.Data());
  Common::CheckValidTree(tree,treename,filename);

  // set branches
  Float_t t1pfMETpt = 0.f; TBranch * b_t1pfMETpt = 0; const std::string s_t1pfMETpt = "t1pfMETpt"; tree->SetBranchAddress(s_t1pfMETpt.c_str(), &t1pfMETpt, &b_t1pfMETpt);
  Float_t t1pfMETphi = 0.f; TBranch * b_t1pfMETphi = 0; const std::string s_t1pfMETphi = "t1pfMETphi"; tree->SetBranchAddress(s_t1pfMETphi.c_str(), &t1pfMETphi, &b_t1pfMETphi);
  Int_t nphotons = 0; TBranch * b_nphotons = 0; const std::string s_nphotons = "nphotons"; tree->SetBranchAddress(s_nphotons.c_str(), &nphotons, &b_nphotons);
  
  std::vector<Photon> photonVec(Common::nPhotons);
  for (auto ipho = 0; ipho < Common::nPhotons; ipho++)
  {
    auto & photon = photonVec[ipho];

    tree->SetBranchAddress(Form("pho%s_%i",photon.s_pt.c_str(),ipho), &photon.pt, &photon.b_pt);
    tree->SetBranchAddress(Form("pho%s_%i",photon.s_phi.c_str(),ipho), &photon.phi, &photon.b_phi);
    tree->SetBranchAddress(Form("pho%s_%i",photon.s_eta.c_str(),ipho), &photon.eta, &photon.b_eta);
    tree->SetBranchAddress(Form("pho%s_%i",photon.s_isOOT.c_str(),ipho), &photon.isOOT, &photon.b_isOOT);
  }

  // transients: needed for not double counting
  std::vector<Bool_t> isRemovedVec(Common::nPhotons);

  // output
  Float_t hackedMETpt = 0.f; const std::string s_hackedMETpt = "hackedMETpt"; TBranch * b_hackedMETpt = tree->Branch(Form("%s",s_hackedMETpt.c_str()),&hackedMETpt,Form("%s/F",s_hackedMETpt.c_str()));
  Float_t hackedMETphi = 0.f; const std::string s_hackedMETphi = "hackedMETphi"; TBranch * b_hackedMETphi = tree->Branch(Form("%s",s_hackedMETphi.c_str()),&hackedMETphi,Form("%s/F",s_hackedMETphi.c_str()));

  const auto nEntries = tree->GetEntries();
  for (auto entry = 0U; entry < nEntries; entry++)
  {
    // dump status check
    if (entry%Common::nEvCheck == 0 || entry == 0) std::cout << "Processing Entry: " << entry << " out of " << nEntries << std::endl;

    // get branches
    b_t1pfMETpt->GetEntry(entry);
    b_t1pfMETphi->GetEntry(entry);
    b_nphotons->GetEntry(entry);

    // nphos used
    const auto nPhotons = std::min(nphotons,Common::nPhotons);

    // get photon branches
    for (auto ipho = 0; ipho < nPhotons; ipho++)
    {
      auto & photon = photonVec[ipho];

      photon.b_pt->GetEntry(entry);
      photon.b_phi->GetEntry(entry);
      photon.b_eta->GetEntry(entry);
      photon.b_isOOT->GetEntry(entry);
    } // end loop over nphotons get entries

    // reset bools
    resetBoolVec(isRemovedVec);

    // set hacked met to input met
    hackedMETpt = t1pfMETpt;
    hackedMETphi = t1pfMETphi;

    // loop over photons, keep highest pt OOT or GED --> sorted in pt, only increment through once in outer
    // + 1 for double loop foward, and then -1 for backward
    // since GED photons already in MET, skip them for outer loop, while check these in inner loop!
    // if object is already matced, skip it, too

    for (auto ipho = 0; ipho < nPhotons; ipho++)
    {
      const auto & photon_i = photonVec[ipho];
      
      if (!photon_i.isOOT) continue;

      // if OOT is completely unmatched, add it to MET
      Bool_t isMatched = false;

      for (auto jpho = ipho+1; jpho < nPhotons; jpho++)
      {
	const auto & photon_j = photonVec[jpho];

	if (photon_j.isOOT) continue;
	if (isRemovedVec[jpho]) continue;

	const auto dR = deltaR(photon_i.phi,photon_i.eta,photon_j.phi,photon_j.eta);
	if (dR < 0.3)
	{
	  // compute components
	  const Float_t x = hackedMETpt*std::cos(hackedMETphi) - photon_i.pt*std::cos(photon_i.phi) + photon_j.pt*std::cos(photon_j.phi);
	  const Float_t y = hackedMETpt*std::sin(hackedMETphi) - photon_i.pt*std::sin(photon_i.phi) + photon_j.pt*std::sin(photon_j.phi);
	  
	  // make new met
	  hackedMETpt  = Common::hypot(x,y);
	  hackedMETphi = Common::phi  (x,y);

	  // set bools
	  isRemovedVec[jpho] = true;
	  isMatched = true;

	  // only consider the first one matched!
	  break;
	} // end check dR
      } // end loop over j phos

      // backward check for matching --> do NOT add to MET, since OOT photon is lower pt
      for (auto jpho = ipho-1; jpho >= 0; jpho--)
      {
	const auto & photon_j = photonVec[jpho];

	if (photon_j.isOOT) continue;
	if (isRemovedVec[jpho]) continue;

	const auto dR = deltaR(photon_i.phi,photon_i.eta,photon_j.phi,photon_j.eta);
	if (dR < 0.3)
	{
	  isMatched = true;
	  break;
	}
      }

      // ootPhoton non-overlapping and by itself
      if (!isMatched)
      {
	// compute components
	const Float_t x = hackedMETpt*std::cos(hackedMETphi) - photon_i.pt*std::cos(photon_i.phi);
	const Float_t y = hackedMETpt*std::sin(hackedMETphi) - photon_i.pt*std::sin(photon_i.phi);
	
	// make new met
	hackedMETpt  = Common::hypot(x,y);
	hackedMETphi = Common::phi  (x,y);
      } // end check over is not matched ootPhoton

    } // end loop over i phos

    // store new branches
    b_hackedMETpt->Fill();
    b_hackedMETphi->Fill();

  } // end loop over entries

  // write out tree
  tree->Write(tree->GetName(),TObject::kWriteDelete);

  // delete it all
  delete tree;
  delete file;
}
