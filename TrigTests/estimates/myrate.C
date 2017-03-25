void myrate(Int_t psc, TString mypath)
{
  std::ifstream input;
  input.open("stdpaths.txt",std::ios::in);
  
  TString ipath;
  std::vector<TString> paths;
  while (input >> ipath) { paths.push_back(ipath); }

  TString fullmenu;
  for (UInt_t i = 0; i < paths.size(); i++) { fullmenu += "(" + paths[i] + "==1) || "; }
  
  /////////////////
  // Path To Add //
  /////////////////
  fullmenu += "(" + mypath + "==1)";

  TString spsc = (psc == 1 ? "1p6e34/" : "1p35e34/");
  TString dir  = "GRunDisplacedPhotonHT_Full_V2/" + spsc  + "hlt0/";
  Float_t nls  = (psc == 1 ? 640 : 638); //618;
  Float_t lsl  = 23.3;
  Float_t ps   = 428; // 107 * 4 HLT PS 
  Float_t SF   = ps/(lsl*nls);
  TFile * file = TFile::Open(dir+"hltbits.root");
  TTree * tree = (TTree*)file->Get("HltTree");

  ROOT::v5::TFormula::SetMaxima(3000);

  Int_t   total = 2548707; //2465556; // tree->GetEntries();
  Int_t   npass_stdfull = 38288; /*37052;*/                                Float_t npass_stdfull_unc = std::sqrt(npass_stdfull);
  Int_t   npass_indiv   = tree->GetEntries(Form("%s == 1",mypath.Data())); Float_t npass_indiv_unc   = std::sqrt(npass_indiv);
  Int_t   npass_total   = tree->GetEntries(fullmenu.Data());               Float_t npass_total_unc   = std::sqrt(npass_total);
  //  Int_t   npass_total = npass_stdfull; Float_t npass_total_unc = 0;
  Int_t   npass_pure    = npass_total-npass_stdfull;                       Float_t npass_pure_unc    = std::sqrt(npass_pure);  

  Float_t stdfull_rate = npass_stdfull * SF; Float_t stdfull_rate_unc = npass_stdfull_unc * SF;
  Float_t indiv_rate   = npass_indiv   * SF; Float_t indiv_rate_unc   = npass_indiv_unc   * SF;
  Float_t total_rate   = npass_total   * SF; Float_t total_rate_unc   = npass_total_unc   * SF;
  Float_t pure_rate    = npass_pure    * SF; Float_t pure_rate_unc    = npass_pure_unc    * SF;

  std::cout << "Total entries: " << total << std::endl; 

  std::cout << "Individual nPassed: " << npass_indiv << " +/- " << npass_indiv_unc << std::endl;
  std::cout << "Total nPassed: " << npass_total << " +/- " << npass_total_unc << std::endl;
  std::cout << "Pure nPassed: " << npass_pure << " +/- " << npass_pure_unc << std::endl;

  std::cout << "Individual Rate: " << indiv_rate << " +/- " << indiv_rate_unc << " Hz" <<std::endl;
  std::cout << "Total Rate: " << total_rate << " +/- " << total_rate_unc << " Hz" <<std::endl;
  std::cout << "Pure Rate: " << pure_rate << " +/- " << pure_rate_unc << " Hz" <<std::endl;

  std::ofstream output;
  output.open(Form("%s.txt",mypath.Data()));
  output << indiv_rate << " " << indiv_rate_unc << " "
	 << total_rate << " " << total_rate_unc << " "
	 << pure_rate << " " << pure_rate_unc << std::endl;
}
