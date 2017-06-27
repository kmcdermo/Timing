float GetMin(TH1F*& h)
{
  float min = 1e9;
  for (Int_t i = 1; i <= h->GetNbinsX(); i++)
  {
    float tmp = h->GetBinContent(i);
    if (tmp < min && tmp != 0.) min = tmp;
  }
  return min;
}

float GetMax(TH1F*& h)
{
  float max = -1e9;
  for (Int_t i = 1; i <= h->GetNbinsX(); i++)
  {
    float tmp = h->GetBinContent(i);
    if (tmp > max) max = tmp;
  }
  return max;
}


void plot(TH1F *& prompt, TH1F *& oot, TString label, bool isLogy = true, bool scale = true)
{
  TLegend * leg = new TLegend(0.8,0.8,0.98,0.98);
  TCanvas * canv = new TCanvas(); canv->cd(); canv->SetLogy(isLogy);

  if (scale)
  {
    prompt->Scale(1.0/prompt->Integral());
    oot   ->Scale(1.0/oot   ->Integral());
  }

  float min1 = GetMin(prompt);
  float min2 = GetMin(oot);
  float min = (min1<min2) ? min1 : min2;

  float max1 = GetMax(prompt);
  float max2 = GetMax(oot);
  float max = (max1>max2) ? max1 : max2;

  if (isLogy) 
  {
    prompt->SetMinimum(min / 3);
    prompt->SetMaximum(max * 3);
  }
  else 
  {
    prompt->SetMinimum(min / 1.1);
    prompt->SetMaximum(max * 1.1);
  }

  prompt->Draw("ep");
  oot   ->Draw("ep same");

  leg->AddEntry(prompt,Form("ged: %6.3f",prompt->GetMean()),"lep");
  leg->AddEntry(oot   ,Form("oot: %6.3f",oot->GetMean()),"lep");

  leg->Draw("same");
  
  canv->SaveAs(Form("%s/%s.png",label.Data(),oot->GetName()));
  delete canv;
  delete leg;
} 

void doPlots(TString label, TString rootfile)
{
  TFile * file = TFile::Open(rootfile.Data());
  TTree * tree = (TTree*)file->Get("tree/tree");

  int nphotons = 0; TBranch * b_nphotons; tree->SetBranchAddress("nphotons", &nphotons, &b_nphotons);
  std::vector<int>   * phIsOOT = 0;    TBranch * b_phIsOOT;    tree->SetBranchAddress("phIsOOT"   , &phIsOOT   , &b_phIsOOT);
  std::vector<int>   * phIsEB  = 0;    TBranch * b_phIsEB;     tree->SetBranchAddress("phIsEB"    , &phIsEB    , &b_phIsEB);
  std::vector<float> * phr9    = 0;    TBranch * b_phr9;       tree->SetBranchAddress("phr9"      , &phr9      , &b_phr9);
  std::vector<float> * phHoE   = 0;    TBranch * b_phHoE;      tree->SetBranchAddress("phHoE"     , &phHoE     , &b_phHoE);
  std::vector<float> * phsieie = 0;    TBranch * b_phsieie;    tree->SetBranchAddress("phsieie"   , &phsieie   , &b_phsieie);
  std::vector<float> * phseedE = 0;    TBranch * b_phseedE;    tree->SetBranchAddress("phseedE"   , &phseedE   , &b_phseedE);
  std::vector<float> * phseedtime = 0; TBranch * b_phseedtime; tree->SetBranchAddress("phseedtime", &phseedtime, &b_phseedtime);
  std::vector<float> * phpt  = 0; TBranch * b_phpt;  tree->SetBranchAddress("phpt",  &phpt,  &b_phpt);
  std::vector<float> * phphi = 0; TBranch * b_phphi; tree->SetBranchAddress("phphi", &phphi, &b_phphi);
  std::vector<float> * pheta = 0; TBranch * b_pheta; tree->SetBranchAddress("pheta", &pheta, &b_pheta);
  std::vector<float> * phPFClEcalIso = 0; TBranch * b_phPFClEcalIso; tree->SetBranchAddress("phPFClEcalIso", &phPFClEcalIso, &b_phPFClEcalIso);
  std::vector<float> * phPFClHcalIso = 0; TBranch * b_phPFClHcalIso; tree->SetBranchAddress("phPFClHcalIso", &phPFClHcalIso, &b_phPFClHcalIso);
  std::vector<float> * phHollowTkIso = 0; TBranch * b_phHollowTkIso; tree->SetBranchAddress("phHollowTkIso", &phHollowTkIso, &b_phHollowTkIso);

  TH1F * hprompt = new TH1F("hpromptseedtime","seed time [ns]",100,-25,25); hprompt->Sumw2(); hprompt->SetLineColor(kRed);  hprompt->SetMarkerColor(kRed);
  TH1F * hoot    = new TH1F("hootseedtime"   ,"seed time [ns]",100,-25,25); hoot   ->Sumw2(); hoot   ->SetLineColor(kBlue); hoot   ->SetMarkerColor(kBlue);

  TH1F * hnprompt = new TH1F("hnpromptphotons","nPhotons",20,0,20); hnprompt->Sumw2(); hnprompt->SetLineColor(kRed);  hnprompt->SetMarkerColor(kRed);
  TH1F * hnoot    = new TH1F("hnootphotons"   ,"nPhotons",20,0,20); hnoot   ->Sumw2(); hnoot   ->SetLineColor(kBlue); hnoot   ->SetMarkerColor(kBlue);

  TH1F * hngoodprompt = new TH1F("hngoodpromptphotons","nGoodPhotons",10,0,10); hngoodprompt->Sumw2(); hngoodprompt->SetLineColor(kRed);  hngoodprompt->SetMarkerColor(kRed);
  TH1F * hngoodoot    = new TH1F("hngoodootphotons"   ,"nGoodPhotons",10,0,10); hngoodoot   ->Sumw2(); hngoodoot   ->SetLineColor(kBlue); hngoodoot   ->SetMarkerColor(kBlue);

  TH1F * hpromptpt = new TH1F("hpromptpt","Photon p_{T}",100,0,1500); hpromptpt->Sumw2(); hpromptpt->SetLineColor(kRed);  hpromptpt->SetMarkerColor(kRed);
  TH1F * hootpt = new TH1F("hootpt","Photon p_{T}",100,0,1500); hootpt->Sumw2(); hootpt->SetLineColor(kBlue);  hootpt->SetMarkerColor(kBlue);

  TH1F * hpromptphi = new TH1F("hpromptphi","Photon #phi",32,-3.2,3.2); hpromptphi->Sumw2(); hpromptphi->SetLineColor(kRed);  hpromptphi->SetMarkerColor(kRed);
  TH1F * hootphi = new TH1F("hootphi","Photon #phi",32,-3.2,3.2); hootphi->Sumw2(); hootphi->SetLineColor(kBlue);  hootphi->SetMarkerColor(kBlue);

  TH1F * hprompteta = new TH1F("hprompteta","Photon #eta",40,-4.0,4.0); hprompteta->Sumw2(); hprompteta->SetLineColor(kRed);  hprompteta->SetMarkerColor(kRed);
  TH1F * hooteta = new TH1F("hooteta","Photon #eta",40,-4.0,4.0); hooteta->Sumw2(); hooteta->SetLineColor(kBlue);  hooteta->SetMarkerColor(kBlue);

  TH1F * hprompteiso = new TH1F("hpromptecalpfcliso","Photon EcalPFClusterIso",100,0,1.); hprompteiso->Sumw2(); hprompteiso->SetLineColor(kRed);  hprompteiso->SetMarkerColor(kRed);
  TH1F * hooteiso = new TH1F("hootecalpfcliso","Photon EcalPFClusterIso",100,0,1.); hooteiso->Sumw2(); hooteiso->SetLineColor(kBlue);  hooteiso->SetMarkerColor(kBlue);

  TH1F * hprompthiso = new TH1F("hprompthcalpfcliso","Photon HcalPFClusterIso",100,0,1.); hprompthiso->Sumw2(); hprompthiso->SetLineColor(kRed);  hprompthiso->SetMarkerColor(kRed);
  TH1F * hoothiso = new TH1F("hoothcalpfcliso","Photon HcalPFClusterIso",100,0,1.); hoothiso->Sumw2(); hoothiso->SetLineColor(kBlue);  hoothiso->SetMarkerColor(kBlue);

  TH1F * hprompttiso = new TH1F("hprompttkiso","Photon HollowTkIso",100,0,1.); hprompttiso->Sumw2(); hprompttiso->SetLineColor(kRed);  hprompttiso->SetMarkerColor(kRed);
  TH1F * hoottiso = new TH1F("hoottkiso","Photon HollowTkIso",100,0,1.); hoottiso->Sumw2(); hoottiso->SetLineColor(kBlue);  hoottiso->SetMarkerColor(kBlue);

  for (UInt_t ientry = 0; ientry < tree->GetEntries(); ientry++)
  {
    tree->GetEntry(ientry);

    bool promptl = false; int nprompt = 0; int npromptgood = 0;
    bool ootl    = false; int noot =0; int nootgood = 0;
    for (int iph = 0; iph < nphotons; iph++)
    {
      //(*phIsOOT)[iph] ? noot++ : nprompt++; // total only

      if ((*phIsEB)[iph] == 1)
      {
	(*phIsOOT)[iph] ? noot++ : nprompt++;

	if ((*phr9)[iph] < 0.95) continue;
	if ((*phHoE)[iph] > 0.0269) continue;
	if ((*phsieie)[iph] > 0.00994) continue;
      }
      else continue;
       //       if ((*phIsEB)[iph] == 0)
//       {

//      (*phIsOOT)[iph] ? noot++ : nprompt++;
// 	if ((*phr9)[iph] < 0.95) continue;
// 	if ((*phHoE)[iph] > 0.0213) continue;
// 	if ((*phsieie)[iph] > 0.03000) continue;
//       }
//       else continue;

      if ((*phIsOOT)[iph]) 
      {
	nootgood++;
	if (!ootl) 
	{
	  ootl = true;
	  hoot   ->Fill((*phseedtime)[iph]);
	  hootpt->Fill((*phpt)[iph]);
	  hootphi->Fill((*phphi)[iph]);
	  hooteta->Fill((*pheta)[iph]);
	  hooteiso->Fill((*phPFClEcalIso)[iph]);
	  hoothiso->Fill((*phPFClHcalIso)[iph]);
	  hoottiso->Fill((*phHollowTkIso)[iph]);
	}
      }
      else if (!(*phIsOOT)[iph])
      {
	npromptgood++;
	if (!promptl)
	{
	  promptl = true;
	  hprompt->Fill((*phseedtime)[iph]);
	  hpromptpt->Fill((*phpt)[iph]);
	  hpromptphi->Fill((*phphi)[iph]);
	  hprompteta->Fill((*pheta)[iph]);
	  hprompteiso->Fill((*phPFClEcalIso)[iph]);
	  hprompthiso->Fill((*phPFClHcalIso)[iph]);
	  hprompttiso->Fill((*phHollowTkIso)[iph]);
	}
      }
    }

    hnprompt->Fill(nprompt);
    hngoodprompt->Fill(npromptgood);
    hnoot->Fill(noot);
    hngoodoot->Fill(nootgood);
  }

  plot(hprompt,hoot,label,true,false);
  plot(hnprompt,hnoot,label,true,false);
  plot(hngoodprompt,hngoodoot,label,true,false);
  plot(hpromptpt,hootpt,label,true,true);
  plot(hpromptphi,hootphi,label,false,true);
  plot(hprompteta,hooteta,label,false,true);
  plot(hprompteiso,hooteiso,label,true,true);
  plot(hprompthiso,hoothiso,label,false,false);
  plot(hprompttiso,hoottiso,label,true,true);


  delete hprompt; delete hoot;
  delete hnprompt; delete hnoot;
  delete hngoodprompt; delete hngoodoot;
  delete hpromptpt; delete hootpt;
  delete hpromptphi; delete hootphi;
  delete hprompteta; delete hooteta;
  delete hprompteiso; delete hooteiso;
  delete hprompthiso; delete hoothiso;
  delete hprompttiso; delete hoottiso;

  delete tree; 
  delete file;
}

void demo()
{
  gStyle->SetOptStat(0);

  doPlots("legacyMC/lead/EB","input/MC/legacy/patphoton-legacyMC.root");
  doPlots("legacyDATA/lead/EB","input/DATA/legacy/patphoton-legacyDATA.root");
  doPlots("2017DATA/lead/EB","input/DATA/2017/patphoton-2017DATA.root");
}
