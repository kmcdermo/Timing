#include "common/common.C"

static const Float_t lumi = 36.56 * 1000; // pb

void getQCD(TH1F*&, TH1F *&);
void getGJets(TH1F*&, TH1F *&);

void shapes()
{
  // set TDR Style (need to force it!)
  TStyle * tdrStyle = new TStyle("tdrStyle","Style for P-TDR");
  SetTDRStyle(tdrStyle);
  gROOT->ForceStyle();

  // BKGs
  TH1F * qcdt1pfMETpt;
  TH1F * qcdph1seedtime;
  getQCD(qcdt1pfMETpt,qcdph1seedtime);

  TH1F * gjett1pfMETpt;
  TH1F * gjetph1seedtime;
  getGJets(gjett1pfMETpt,gjetph1seedtime);

  // Totals 
  TH1F * bkgt1pfMETpt   = qcdt1pfMETpt;
  TH1F * bkgph1seedtime = qcdph1seedtime;
  bkgt1pfMETpt  ->Add(gjett1pfMETpt);
  bkgph1seedtime->Add(gjetph1seedtime);

  // scale that sum == 1.0
  float total     = bkgph1seedtime ->Integral();
  float frac_qcd  = qcdph1seedtime ->Integral() / total;
  float frac_gjet = gjetph1seedtime->Integral() / total;
  
  // scale to 1.0
  bkgt1pfMETpt   ->Scale(1.0/bkgt1pfMETpt->Integral());
  bkgph1seedtime ->Scale(1.0/bkgph1seedtime->Integral());
  qcdt1pfMETpt   ->Scale((1.0/qcdt1pfMETpt   ->Integral())*(frac_qcd));
  qcdph1seedtime ->Scale((1.0/qcdph1seedtime ->Integral())*(frac_qcd));
  gjett1pfMETpt  ->Scale((1.0/gjett1pfMETpt  ->Integral())*(frac_gjet));
  gjetph1seedtime->Scale((1.0/gjetph1seedtime->Integral())*(frac_gjet));
  
  //Make stacks
  THStack * t1pfMETptstack = new THStack("t1pfMETptstack","");
  t1pfMETptstack->Add(gjett1pfMETpt);
  t1pfMETptstack->Add(qcdt1pfMETpt);

  THStack * ph1seedtimestack = new THStack("ph1seedtimestack","");
  ph1seedtimestack->Add(gjetph1seedtime);
  ph1seedtimestack->Add(qcdph1seedtime);

  // Signals
  TFile * file100 = TFile::Open("output/withReReco/cuts/ctau100/cuts_jetpt35.0_phpt100.0_phVIDmedium_rhE1.0_ecalaccept/plots.root");
  ctau100t1pfMETpt   = (TH1F*)file100->Get("t1pfMETpt"); 
  ctau100ph1seedtime = (TH1F*)file100->Get("ph1seedtime"); 

  ctau100t1pfMETpt  ->Scale(1.0/ctau100t1pfMETpt->Integral());
  ctau100ph1seedtime->Scale(1.0/ctau100ph1seedtime->Integral());
  ctau100t1pfMETpt  ->SetLineColor(kViolet-1);
  ctau100ph1seedtime->SetLineColor(kViolet-1);

  TFile * file2000 = TFile::Open("output/withReReco/cuts/ctau2000/cuts_jetpt35.0_phpt100.0_phVIDmedium_rhE1.0_ecalaccept/plots.root");
  ctau2000t1pfMETpt   = (TH1F*)file2000->Get("t1pfMETpt"); 
  ctau2000ph1seedtime = (TH1F*)file2000->Get("ph1seedtime"); 

  ctau2000t1pfMETpt  ->Scale(1.0/ctau2000t1pfMETpt->Integral());
  ctau2000ph1seedtime->Scale(1.0/ctau2000ph1seedtime->Integral());
  ctau2000t1pfMETpt  ->SetLineColor(kRed+1);
  ctau2000ph1seedtime->SetLineColor(kRed+1);

  TFile * file6000 = TFile::Open("output/withReReco/cuts/ctau6000/cuts_jetpt35.0_phpt100.0_phVIDmedium_rhE1.0_ecalaccept/plots.root");
  ctau6000t1pfMETpt = (TH1F*)file6000->Get("t1pfMETpt"); 
  ctau6000ph1seedtime = (TH1F*)file6000->Get("ph1seedtime"); 

  ctau6000t1pfMETpt  ->Scale(1.0/ctau6000t1pfMETpt->Integral());
  ctau6000ph1seedtime->Scale(1.0/ctau6000ph1seedtime->Integral());
  ctau6000t1pfMETpt  ->SetLineColor(kBlue+1);
  ctau6000ph1seedtime->SetLineColor(kBlue+1);

  // Draw it all
  TCanvas * t1pfMETptcanv = new TCanvas();
  t1pfMETptcanv->cd();
  CMSLumi(t1pfMETptcanv,"Simulation");
  //  t1pfMETptstack   ->Draw("HIST");
  bkgt1pfMETpt     ->Draw("HIST");
  ctau100t1pfMETpt ->Draw("HIST same");
  ctau2000t1pfMETpt->Draw("HIST same");
  ctau6000t1pfMETpt->Draw("HIST same");

  TLegend * t1pfMETptleg = new TLegend(0.7,0.65,0.9,0.9);
//   t1pfMETptleg->AddEntry(gjett1pfMETpt,"#gamma+Jets","f");
//   t1pfMETptleg->AddEntry(qcdt1pfMETpt ,"QCD","f");
  t1pfMETptleg->AddEntry(bkgt1pfMETpt,"Background","f");
  t1pfMETptleg->AddEntry(ctau100t1pfMETpt ,"c#tau = 100" ,"l");
  t1pfMETptleg->AddEntry(ctau2000t1pfMETpt,"c#tau = 2000","l");
  t1pfMETptleg->AddEntry(ctau6000t1pfMETpt,"c#tau = 6000","l");
  t1pfMETptleg->Draw("same");

  TCanvas * ph1seedtimecanv = new TCanvas();
  ph1seedtimecanv->cd();
  CMSLumi(ph1seedtimecanv,"Simulation",10);
  //  ph1seedtimestack   ->Draw("HIST");
  bkgph1seedtime     ->Draw("HIST");
  ctau100ph1seedtime ->Draw("HIST same");
  ctau2000ph1seedtime->Draw("HIST same");
  ctau6000ph1seedtime->Draw("HIST same");

  TLegend * ph1seedtimeleg = new TLegend(0.7,0.65,0.9,0.9);
//   ph1seedtimeleg->AddEntry(gjetph1seedtime,"#gamma+Jets","f");
//   ph1seedtimeleg->AddEntry(qcdph1seedtime ,"QCD","f");
  ph1seedtimeleg->AddEntry(bkgph1seedtime,"Background","f");
  ph1seedtimeleg->AddEntry(ctau100ph1seedtime ,"c#tau = 100" ,"l");
  ph1seedtimeleg->AddEntry(ctau2000ph1seedtime,"c#tau = 2000","l");
  ph1seedtimeleg->AddEntry(ctau6000ph1seedtime,"c#tau = 6000","l");
  ph1seedtimeleg->Draw("same");
}

void getQCD(TH1F*& qcdt1pfMETpt, TH1F *& qcdph1seedtime)
{
  std::vector<TString> qcdHTs = {"100To200","200To300","300To500","500To700","700To1000","1000To1500","1500To2000","2000ToInf"};
  std::vector<Float_t> qcdnes = {171217,140932,176621,184492,187035,180495,174838,170585}; //nEvents
  std::vector<Float_t> qcdxss = {27540000,1717000,351300,31630,6802,1206,120.4,25.25}; //pb
  std::vector<TFile*>  qcdfiles(qcdHTs.size());

  std::vector<TH1F*> qcdt1pfMETpts(qcdHTs.size());
  std::vector<TH1F*> qcdph1seedtimes(qcdHTs.size());
  for (UInt_t iqcd = 0; iqcd < qcdHTs.size(); iqcd++)
  {
    qcdfiles[iqcd] = TFile::Open(Form("output/QCD/cuts/%s/cuts_jetpt35.0_phpt100.0_phVIDmedium_rhE1.0_ecalaccept/plots.root",qcdHTs[iqcd].Data()));
    qcdt1pfMETpts[iqcd]   = (TH1F*)qcdfiles[iqcd]->Get("t1pfMETpt"); 
    qcdph1seedtimes[iqcd] = (TH1F*)qcdfiles[iqcd]->Get("ph1seedtime"); 
    qcdt1pfMETpts[iqcd]   ->Scale((1.0/qcdnes[iqcd])*(qcdxss[iqcd]*lumi));
    qcdph1seedtimes[iqcd] ->Scale((1.0/qcdnes[iqcd])*(qcdxss[iqcd]*lumi));
  }

  for (UInt_t iqcd = 0; iqcd < qcdHTs.size(); iqcd++)
  {
    if (iqcd > 0)
    {
      qcdt1pfMETpt  ->Add(qcdt1pfMETpts[iqcd]);
      qcdph1seedtime->Add(qcdph1seedtimes[iqcd]);
    }
    else
    {
      qcdt1pfMETpt   = qcdt1pfMETpts[iqcd];
      qcdph1seedtime = qcdph1seedtimes[iqcd];
    }
  }

  qcdt1pfMETpt->SetLineColor(kGreen+2);
  qcdt1pfMETpt->SetFillColor(kGreen+2);
  qcdph1seedtime->SetLineColor(kGreen+2);
  qcdph1seedtime->SetFillColor(kGreen+2);
}

void getGJets(TH1F*& gjett1pfMETpt, TH1F *& gjetph1seedtime)
{
  std::vector<TString> gjetHTs = {"40To100","100To200","200To400","400To600","600ToInf"};
  std::vector<Float_t> gjetnes = {110327,147189,117893,125644,125788}; //nEvents
  std::vector<Float_t> gjetxss = {20730,9226,2300,277.4,93.38}; //pb
  std::vector<TFile*>  gjetfiles(gjetHTs.size());

  std::vector<TH1F*> gjett1pfMETpts(gjetHTs.size());
  std::vector<TH1F*> gjetph1seedtimes(gjetHTs.size());
  for (UInt_t igjet = 0; igjet < gjetHTs.size(); igjet++)
  {
    gjetfiles[igjet] = TFile::Open(Form("output/GJets/cuts/%s/cuts_jetpt35.0_phpt100.0_phVIDmedium_rhE1.0_ecalaccept/plots.root",gjetHTs[igjet].Data()));
    gjett1pfMETpts[igjet]   = (TH1F*)gjetfiles[igjet]->Get("t1pfMETpt"); 
    gjetph1seedtimes[igjet] = (TH1F*)gjetfiles[igjet]->Get("ph1seedtime"); 
    gjett1pfMETpts[igjet]   ->Scale((1.0/gjetnes[igjet])*(gjetxss[igjet]*lumi));
    gjetph1seedtimes[igjet] ->Scale((1.0/gjetnes[igjet])*(gjetxss[igjet]*lumi));
  }

  for (UInt_t igjet = 0; igjet < gjetHTs.size(); igjet++)
  {
    if (igjet > 0)
    {
      gjett1pfMETpt  ->Add(gjett1pfMETpts[igjet]);
      gjetph1seedtime->Add(gjetph1seedtimes[igjet]);
    }
    else
    {
      gjett1pfMETpt   = gjett1pfMETpts[igjet];
      gjetph1seedtime = gjetph1seedtimes[igjet];
    }
  }

  gjett1pfMETpt->SetLineColor(kYellow-7);
  gjett1pfMETpt->SetFillColor(kYellow-7);
  gjetph1seedtime->SetLineColor(kYellow-7);
  gjetph1seedtime->SetFillColor(kYellow-7);
}
