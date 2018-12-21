constexpr auto nEvs = 100000;
static const TString basedir = "plots";
inline void mkDir(const TString & dir) {gSystem->Exec("mkdir -p "+dir);}

struct CutInfoStruct
{
  CutInfoStruct(){}
  CutInfoStruct(const TString & cut, const TString & label)
    : cut(cut), label(label) {}

  TString cut;
  TString label;
};

void plot_counter(const TString & modifier)
{
  auto file = TFile::Open("counter"+modifier+".root");
  auto tree = (TTree*)file->Get("tree");

  auto canv = new TCanvas("canv","canv",10,45,700,500); canv->Range(-3.9,-0.025,3.3,0.24);
  canv->SetRightMargin(0.025); canv->SetTopMargin(0.028);
  canv->SetTickx(); canv->SetTicky(); canv->cd();

  const std::vector<CutInfoStruct> CutInfos = 
  {
    {"N","None"},
    {"L","Loose"},
    {"T","Tight"}
  }; 
  
  for (const auto & CutInfo : CutInfos)
  {
    const auto & cut = CutInfo.cut;
    const auto & label = CutInfo.label;

    mkDir(basedir+modifier+"/"+label);

    ////////////
    // delphi //
    ////////////
    
    auto h_std = new TH1F("h_std",";#Delta(#phi_{MET}^{GEN}-#phi^_{MET}{RECO});Fraction of Events",32,-3.2f,3.2f); 
    h_std->SetLineColor(kRed+1); h_std->SetMarkerColor(kRed+1); h_std->Sumw2(); 
    tree->Draw("TVector2::Phi_mpi_pi(genMETphi-t1pfMETphi)>>h_std",Form("nOOT_%s>0",cut.Data()),"goff"); h_std->Scale(1.f/nEvs);
    
    auto h_oot = new TH1F("h_oot",";#Delta(#phi_{MET}^{GEN}-#phi_{MET}^{RECO});Fraction of Events",32,-3.2f,3.2f);
    h_oot->SetLineColor(kBlue); h_oot->SetMarkerColor(kBlue); h_oot->Sumw2(); 
    tree->Draw(Form("TVector2::Phi_mpi_pi(genMETphi-ootMETphi_%s)>>h_oot",cut.Data()),Form("nOOT_%s>0",cut.Data()),"goff"); h_oot->Scale(1.f/nEvs);
    
    h_std->SetName("Type-1 PFMET"); h_oot->SetName("Corrected MET"); 
    canv->SetLogy(0); h_oot->SetMinimum(0.f); h_oot->Draw(); h_std->Draw("sames"); canv->Update();
    
    auto s_std = (TPaveStats*)h_std->FindObject("stats"); auto s_oot = (TPaveStats*)h_oot->FindObject("stats");
    s_std->SetX1NDC(0.78); s_std->SetY1NDC(0.78);
    s_std->SetX2NDC(0.95); s_std->SetY2NDC(0.94);
    s_oot->SetX1NDC(s_std->GetX1NDC()); s_oot->SetY1NDC(2.f * s_std->GetY1NDC() - s_std->GetY2NDC() - 0.02);
    s_oot->SetX2NDC(s_std->GetX2NDC()); s_oot->SetY2NDC(s_std->GetY1NDC() - 0.02);
  
    auto leg = new TLegend(0.14,0.84,0.38,0.94); leg->AddEntry(h_std,"Type-1 PFMET","epl"); leg->AddEntry(h_oot,"Corrected MET","epl"); leg->Draw("same");
    canv->SaveAs(basedir+modifier+"/"+label+"/delphi.pdf");
    delete leg; delete s_std; delete h_std; delete s_oot; delete h_oot;

    ///////////
    // delpt //
    ///////////
    
    h_std = new TH1F("h_std",";#Delta(MET^{GEN}-MET^{RECO});Fraction of Events",50,-1000.f,1000.f); 
    h_std->SetLineColor(kRed+1); h_std->SetMarkerColor(kRed+1); h_std->Sumw2(); 
    tree->Draw("genMETpt-t1pfMETpt>>h_std",Form("nOOT_%s>0",cut.Data()),"goff"); h_std->Scale(1.f/nEvs);
    
    h_oot = new TH1F("h_oot",";#Delta(MET^{GEN}-MET^{RECO});Fraction of Events",50,-1000.f,1000.f); 
    h_oot->SetLineColor(kBlue); h_oot->SetMarkerColor(kBlue); h_oot->Sumw2(); 
    tree->Draw(Form("genMETpt-ootMETpt_%s>>h_oot",cut.Data()),Form("nOOT_%s>0",cut.Data()),"goff"); h_oot->Scale(1.f/nEvs);
    
    h_std->SetName("Type-1 PFMET"); h_oot->SetName("Corrected MET");
    canv->SetLogy(0); h_oot->SetMinimum(0.f); h_oot->Draw(); h_std->Draw("sames"); canv->Update();
    
    s_std = (TPaveStats*)h_std->FindObject("stats"); s_oot = (TPaveStats*)h_oot->FindObject("stats");
    s_std->SetX1NDC(0.78); s_std->SetY1NDC(0.78);
    s_std->SetX2NDC(0.95); s_std->SetY2NDC(0.94);
    s_oot->SetX1NDC(s_std->GetX1NDC()); s_oot->SetY1NDC(2.f * s_std->GetY1NDC() - s_std->GetY2NDC() - 0.02);
    s_oot->SetX2NDC(s_std->GetX2NDC()); s_oot->SetY2NDC(s_std->GetY1NDC() - 0.02);
    
    leg = new TLegend(0.14,0.84,0.38,0.94); leg->AddEntry(h_std,"Type-1 PFMET","epl"); leg->AddEntry(h_oot,"Corrected MET","epl"); leg->Draw("same");
    canv->SaveAs(basedir+modifier+"/"+label+"/delpt.pdf");
    delete leg; delete s_std; delete h_std; delete s_oot; delete h_oot;
  
    ///////////////////
    // pt (nOOT > 0) //
    ///////////////////
    
    h_std = new TH1F("h_std",";MET^{RECO};Fraction of Events",30,0.f,3000.f);
    h_std->SetLineColor(kRed+1); h_std->SetMarkerColor(kRed+1); h_std->Sumw2(); 
    tree->Draw("t1pfMETpt>>h_std",Form("nOOT_%s>0",cut.Data()),"goff"); h_std->Scale(1.f/nEvs);

    h_oot = new TH1F("h_oot",";MET^{RECO};Fraction of Events",30,0.f,3000.f);
    h_oot->SetLineColor(kBlue); h_oot->SetMarkerColor(kBlue); h_oot->Sumw2(); 
    tree->Draw(Form("ootMETpt_%s>>h_oot",cut.Data()),Form("nOOT_%s>0",cut.Data()),"goff"); h_oot->Scale(1.f/nEvs);
    
    h_std->SetName("Type-1 PFMET"); h_oot->SetName("Corrected MET"); 
    canv->SetLogy(1); h_oot->SetMinimum(5e-6); h_oot->Draw(); h_std->Draw("sames"); canv->Update();
    
    s_std = (TPaveStats*)h_std->FindObject("stats"); s_oot = (TPaveStats*)h_oot->FindObject("stats");
    s_std->SetX1NDC(0.78); s_std->SetY1NDC(0.78);
    s_std->SetX2NDC(0.95); s_std->SetY2NDC(0.94);
    s_oot->SetX1NDC(s_std->GetX1NDC()); s_oot->SetY1NDC(2.f * s_std->GetY1NDC() - s_std->GetY2NDC() - 0.02);
    s_oot->SetX2NDC(s_std->GetX2NDC()); s_oot->SetY2NDC(s_std->GetY1NDC() - 0.02);

    leg = new TLegend(0.50,0.84,0.74,0.94); leg->AddEntry(h_std,"Type-1 PFMET","epl"); leg->AddEntry(h_oot,"Corrected MET","epl"); leg->Draw("same");
    canv->SaveAs(basedir+modifier+"/"+label+"/pt.pdf");
    delete leg; delete s_std; delete h_std; delete s_oot; delete h_oot;

    ////////////////////
    // pt (nOOT >= 0) //
    ////////////////////

    h_std = new TH1F("h_std",";MET^{RECO};Fraction of Events",50,0.f,5000.f);
    h_std->SetLineColor(kRed+1); h_std->SetMarkerColor(kRed+1); h_std->Sumw2(); 
    tree->Draw("t1pfMETpt>>h_std",Form("nOOT_%s>=0",cut.Data()),"goff"); h_std->Scale(1.f/nEvs);
    
    h_oot = new TH1F("h_oot",";MET^{RECO};Fraction of Events",50,0.f,5000.f);
    h_oot->SetLineColor(kBlue); h_oot->SetMarkerColor(kBlue); h_oot->Sumw2(); 
    tree->Draw(Form("ootMETpt_%s>>h_oot",cut.Data()),Form("nOOT_%s>=0",cut.Data()),"goff"); h_oot->Scale(1.f/nEvs);

    h_std->SetName("Type-1 PFMET"); h_oot->SetName("Corrected MET"); 
    canv->SetLogy(1); h_oot->SetMinimum(5e-6); h_oot->Draw(); h_std->Draw("sames"); canv->Update();
    
    s_std = (TPaveStats*)h_std->FindObject("stats"); s_oot = (TPaveStats*)h_oot->FindObject("stats");
    s_std->SetX1NDC(0.78); s_std->SetY1NDC(0.78);
    s_std->SetX2NDC(0.95); s_std->SetY2NDC(0.94);
    s_oot->SetX1NDC(s_std->GetX1NDC()); s_oot->SetY1NDC(2.f * s_std->GetY1NDC() - s_std->GetY2NDC() - 0.02);
    s_oot->SetX2NDC(s_std->GetX2NDC()); s_oot->SetY2NDC(s_std->GetY1NDC() - 0.02);

    leg = new TLegend(0.50,0.84,0.74,0.94); leg->AddEntry(h_std,"Type-1 PFMET","epl"); leg->AddEntry(h_oot,"Corrected MET","epl"); leg->Draw("same");
    canv->SetLogy(1);
    canv->SaveAs(basedir+modifier+"/"+label+"/pt_full.pdf");
    delete leg; delete s_std; delete h_std; delete s_oot; delete h_oot;

    ///////////////////
    // OOT counters //
    //////////////////
    
    auto h_oot_GT = new TH1F("h_oot_GT",";nPhotons/Event;Fraction of Events",5,0,5);
    h_oot_GT->SetLineColor(kBlue); h_oot_GT->SetMarkerColor(kBlue); h_oot_GT->Sumw2(); h_oot_GT->SetStats(0);
    tree->Draw(Form("nOOT_matchedGTGED_%s>>h_oot_GT",cut.Data()),"","goff"); h_oot_GT->Scale(1.f/nEvs);
    
    auto h_oot_LT = new TH1F("h_oot_LT",";nPhotons/Event;Fraction of Events",5,0,5);
    h_oot_LT->SetLineColor(kRed+1); h_oot_LT->SetMarkerColor(kRed+1); h_oot_LT->Sumw2(); h_oot_LT->SetStats(0);
    tree->Draw(Form("nOOT_matchedLTGED_%s>>h_oot_LT",cut.Data()),"","goff"); h_oot_LT->Scale(1.f/nEvs);
    
    auto h_oot_UN = new TH1F("h_oot_UN",";nPhotons/Event;Fraction of Events",5,0,5);
    h_oot_UN->SetLineColor(kGreen+1); h_oot_UN->SetMarkerColor(kGreen+1); h_oot_UN->Sumw2(); h_oot_UN->SetStats(0);
    tree->Draw(Form("nOOT_unmatchedGED_%s>>h_oot_UN",cut.Data()),"","goff"); h_oot_UN->Scale(1.f/nEvs);

    canv->SetLogy(1); h_oot_GT->SetMinimum(5e-5); h_oot_GT->Draw(); h_oot_LT->Draw("same"); h_oot_UN->Draw("same");

    leg = new TLegend(0.77,0.74,0.94,0.94);
    leg->AddEntry(h_oot_GT,Form("OOT > GED, #mu: %4.2f",h_oot_GT->GetMean()),"epl");
    leg->AddEntry(h_oot_LT,Form("OOT < GED, #mu: %5.3f",h_oot_LT->GetMean()),"epl");
    leg->AddEntry(h_oot_UN,Form("OOT != GED, #mu: %4.2f",h_oot_UN->GetMean()),"epl");
    leg->Draw("same");
    
    canv->SaveAs(basedir+modifier+"/"+label+"/nOOTphotons.pdf");
    delete leg; delete h_oot_GT; delete h_oot_LT; delete h_oot_UN;
  }

  //////////////
  // counters //
  //////////////

  auto h_ged_N = new TH1F("h_ged_N",";nPhotons/Event;Fraction of Events",10,0,10);
  h_ged_N->SetLineColor(kRed+1); h_ged_N->SetMarkerColor(kRed+1); h_ged_N->Sumw2(); h_ged_N->SetStats(0);
  tree->Draw("nGED_N>>h_ged_N","","goff"); h_ged_N->Scale(1.f/nEvs);

  auto h_ged_L = new TH1F("h_ged_L",";nPhotons/Event;Fraction of Events",10,0,10);
  h_ged_L->SetLineColor(kPink-9); h_ged_L->SetMarkerColor(kPink-9); h_ged_L->Sumw2(); h_ged_L->SetStats(0);
  tree->Draw("nGED_L>>h_ged_L","","goff"); h_ged_L->Scale(1.f/nEvs);

  auto h_ged_T = new TH1F("h_ged_T",";nPhotons/Event;Fraction of Events",10,0,10);
  h_ged_T->SetLineColor(kOrange+1); h_ged_T->SetMarkerColor(kOrange+1); h_ged_T->Sumw2(); h_ged_T->SetStats(0);
  tree->Draw("nGED_T>>h_ged_T","","goff"); h_ged_T->Scale(1.f/nEvs);

  auto h_oot_N = new TH1F("h_oot_N",";nPhotons/Event;Fraction of Events",10,0,10);
  h_oot_N->SetLineColor(kBlue); h_oot_N->SetMarkerColor(kBlue); h_oot_N->Sumw2(); h_oot_N->SetStats(0);
  tree->Draw("nOOT_N>>h_oot_N","","goff"); h_oot_N->Scale(1.f/nEvs);

  auto h_oot_L = new TH1F("h_oot_L",";nPhotons/Event;Fraction of Events",10,0,10);
  h_oot_L->SetLineColor(kAzure+10); h_oot_L->SetMarkerColor(kAzure+10); h_oot_L->Sumw2(); h_oot_L->SetStats(0);
  tree->Draw("nOOT_L>>h_oot_L","","goff"); h_oot_L->Scale(1.f/nEvs);

  auto h_oot_T = new TH1F("h_oot_T",";nPhotons/Event;Fraction of Events",10,0,10);
  h_oot_T->SetLineColor(kGreen+1); h_oot_T->SetMarkerColor(kGreen+1); h_oot_T->Sumw2(); h_oot_T->SetStats(0);
  tree->Draw("nOOT_T>>h_oot_T","","goff"); h_oot_T->Scale(1.f/nEvs);

  h_oot_N->SetMinimum(5e-5); h_oot_N->Draw(); h_oot_L->Draw("same"); h_oot_T->Draw("same"); h_ged_N->Draw("same"); h_ged_L->Draw("same"); h_ged_T->Draw("same");
  auto leg = new TLegend(0.60,0.74,0.94,0.94); leg->SetNColumns(2);
  leg->AddEntry(h_ged_N,Form("GED, #mu: %4.2f",h_ged_N->GetMean()),"epl"); leg->AddEntry(h_oot_N,Form("OOT, #mu: %4.2f",h_oot_N->GetMean()),"epl");
  leg->AddEntry(h_ged_L,Form("GED (Loose), #mu: %4.2f",h_ged_L->GetMean()),"epl"); leg->AddEntry(h_oot_L,Form("OOT (Loose), #mu: %4.2f",h_oot_L->GetMean()),"epl");
  leg->AddEntry(h_ged_T,Form("GED (Tight), #mu: %4.2f",h_ged_T->GetMean()),"epl"); leg->AddEntry(h_oot_T,Form("OOT (Tight), #mu: %4.2f",h_oot_T->GetMean()),"epl");
  leg->Draw("same");

  canv->SetLogy(1);
  canv->SaveAs(basedir+modifier+"/nphotons.pdf");
  delete leg; delete h_ged_N; delete h_ged_L; delete h_ged_T; delete h_oot_N; delete h_oot_L; delete h_oot_T;
}
