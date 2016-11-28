void getQCD(TH1F*&, TH1F *&);
void getGJets(TH1F*&, TH1F *&);

void shapes()
{
  TH1F * qcdt1pfMETpt;
  TH1F * qcdph1seedtime;
  getQCD(qcdt1pfMETpt,qcdph1seedtime);

  TH1F * gjett1pfMETpt;
  TH1F * gjetph1seedtime;
  getGJets(gjett1pfMETpt,gjetph1seedtime);
}

void getQCD(TH1F*& qcdt1pfMETpt, TH1F *& qcdph1seedtime)
{
  std::vector<TString> qcdHTs = {"100To200","200To300","300To500","500To700","700To1000","1000To1500","1500To2000","2000ToInf"};
  std::vector<Float_t> qcdxss = {27540000,1717000,351300,31630,6802,1206,120.4,25.25}; //pb
  std::vector<TFile*>  qcdfiles(qcdHTs.size());

  std::vector<TH1F*> qcdt1pfMETpts(qcdHTs.size());
  std::vector<TH1F*> qcdph1seedtimes(qcdHTs.size());
  for (UInt_t iqcd = 0; iqcd < qcdHTs.size(); iqcd++)
  {
    qcdfiles[iqcd] = TFile::Open(Form("output/QCD/cuts/%s/cuts_jetpt35.0_phpt100.0_phVIDmedium_rhE1.0_ecalaccept/plots.root",qcdHTs[iqcd].Data()));
    qcdt1pfMETpts[iqcd] = (TH1F*)qcdfiles[iqcd]->Get("t1pfMETpt"); 
    qcdph1seedtimes[iqcd] = (TH1F*)qcdfiles[iqcd]->Get("ph1seedtime"); 
  }

  qcdt1pfMETpt = qcdt1pfMETpts[0];
  qcdph1seedtime = qcdph1seedtimes[0];
  for (UInt_t iqcd = 1; iqcd < qcdHTs.size(); iqcd++)
  {
    Float_t scale = qcdxss[iqcd]/qcdxss[0];
    qcdt1pfMETpt->Add(qcdt1pfMETpts[iqcd],scale);
    qcdph1seedtime->Add(qcdph1seedtimes[iqcd],scale);
  }
  qcdt1pfMETpt->Scale(1.0/qcdt1pfMETpt->Integral());
  qcdph1seedtime->Scale(1.0/qcdph1seedtime->Integral());
}

void getGJets(TH1F*& gjett1pfMETpt, TH1F *& gjetph1seedtime)
{
  std::vector<TString> gjetHTs = {"40To100","100To200","200To400","400To600","600ToInf"};
  std::vector<Float_t> gjetxss = {20730,9226,2300,277.4,93.38}; //pb
  std::vector<TFile*>  gjetfiles(gjetHTs.size());

  std::vector<TH1F*> gjett1pfMETpts(gjetHTs.size());
  std::vector<TH1F*> gjetph1seedtimes(gjetHTs.size());
  for (UInt_t igjet = 0; igjet < gjetHTs.size(); igjet++)
  {
    gjetfiles[igjet] = TFile::Open(Form("output/GJets/cuts/%s/cuts_jetpt35.0_phpt100.0_phVIDmedium_rhE1.0_ecalaccept/plots.root",gjetHTs[igjet].Data()));
    gjett1pfMETpts[igjet] = (TH1F*)gjetfiles[igjet]->Get("t1pfMETpt"); 
    gjetph1seedtimes[igjet] = (TH1F*)gjetfiles[igjet]->Get("ph1seedtime"); 
  }

  gjett1pfMETpt = gjett1pfMETpts[0];
  gjetph1seedtime = gjetph1seedtimes[0];
  for (UInt_t igjet = 1; igjet < gjetHTs.size(); igjet++)
  {
    Float_t scale = gjetxss[igjet]/gjetxss[0];
    gjett1pfMETpt->Add(gjett1pfMETpts[igjet],scale);
    gjetph1seedtime->Add(gjetph1seedtimes[igjet],scale);
  }
  gjett1pfMETpt->Scale(1.0/gjett1pfMETpt->Integral());
  gjetph1seedtime->Scale(1.0/gjetph1seedtime->Integral());
}
