#include "TFile.h"
#include "TTree.h"
#include "TH1F.h"
#include "TCanvas.h"

#include <iostream>
#include <cmath>

static const Float_t PI    = 3.14159265358979323846;
static const Float_t TWOPI = 2.f*3.14159265358979323846;
static const Float_t delRcut = 1.0;

inline Float_t rad2  (const Float_t x, const Float_t y){return x*x + y*y;}
inline Float_t theta (const Float_t r, const Float_t z){return std::atan2(r,z);}
inline Float_t eta   (const Float_t x, const Float_t y, const Float_t z)
{
  return -1.0f*std::log(std::tan(theta(std::sqrt(rad2(x,y)),z)/2.f));
}
inline Float_t rad2  (const Float_t x, const Float_t y, const Float_t z)
{
  return x*x + y*y + z*z;
}
inline Float_t phi   (const Float_t x, const Float_t y){return std::atan2(y,x);}
inline Float_t mphi  (Float_t phi)
{    
  while (phi >= PI) phi -= TWOPI;
  while (phi < -PI) phi += TWOPI;
  return phi;
}
inline Float_t deltaR(const Float_t eta1, const Float_t phi1, const Float_t eta2, const Float_t phi2)
{
  return std::sqrt(rad2(eta2-eta1,mphi(phi2-phi1)));
}

void countdR()
{
  Bool_t     isMC = false;
  TString   indir = Form("input/%s", (isMC?"MC/2016/dyll":"DATA/2016/doubleeg") );
  TFile   *  file = TFile::Open(Form("%s/skimmedtree.root",indir.Data())); 
  TTree   *  tree = (TTree*)file->Get("tree/tree");
  
  Float_t x1 = 0; TBranch * b_x1 = tree->GetBranch("el1seedX"); b_x1->SetAddress(&x1);
  Float_t y1 = 0; TBranch * b_y1 = tree->GetBranch("el1seedY"); b_y1->SetAddress(&y1);
  Float_t z1 = 0; TBranch * b_z1 = tree->GetBranch("el1seedZ"); b_z1->SetAddress(&z1);
  
  Float_t x2 = 0; TBranch * b_x2 = tree->GetBranch("el2seedX"); b_x2->SetAddress(&x2);
  Float_t y2 = 0; TBranch * b_y2 = tree->GetBranch("el2seedY"); b_y2->SetAddress(&y2);
  Float_t z2 = 0; TBranch * b_z2 = tree->GetBranch("el2seedZ"); b_z2->SetAddress(&z2);

  Int_t n = 0;
  TH1F * hist = new TH1F("hdelr","delta eta of two electrons",100,0,7);
  for (UInt_t entry = 0; entry < tree->GetEntries(); entry++)
  {
    b_x1->GetEvent(entry); b_y1->GetEvent(entry); b_z1->GetEvent(entry);
    b_x2->GetEvent(entry); b_y2->GetEvent(entry); b_z2->GetEvent(entry);

    const Float_t eta1 = eta(x1,y1,z1); const Float_t phi1 = phi(x1,y1);
    const Float_t eta2 = eta(x2,y2,z2); const Float_t phi2 = phi(x2,y2);

    //    if (deltaR(eta1,phi1,eta2,phi2) < delRcut) n++;
    //    hist->Fill(deltaR(eta1,phi1,eta2,phi2));
    hist->Fill(eta2-eta1);
  }

  TCanvas *c1 = new TCanvas();
  c1->cd();
  c1->SetLogy();
  hist->Draw();
  c1->SaveAs("deleta.png");

  std::cout << "n: " << n << " out of " << tree->GetEntries() << std::endl;
}
