static const float tolerance = 1e-7;

inline float getcgrav(const int lamb, const float ctau)
{
  return std::sqrt(ctau)*(-10.7968+3.51482*std::pow(lamb,0.3871)); // old: original tuning for 100-400 TeV
  //  return std::sqrt(ctau)*(-5.857+1.893*std::pow(lamb,0.4652)); // new: better for L>400 TeV
}

void gencgrav()
{
  // new points for tuning
  //  std::vector<float> ctaus = {0.001,0.01,0.1,1,5,10,25,50,100,200,300,500,750,1000,1500,2000,2500,3000}; // cm
  //  std::vector<int>   lambs = {50,100,200,300,400,500,600,700,800,900,1000,1100,1200,1300,1400,1500}; // tev

  // points for livia
  //  std::vector<float> ctaus = {0.1,3,6,15,30,200,300,500,1000,1500};
  //  std::vector<int>   lambs = {1500,2000};

  // submitted points
  std::vector<float> ctaus = {0.001,0.1,0.5,1,5,10,50,100,200,400,600,800,1000,1200,5000,10000};
  std::vector<int>   lambs = {100,150,200,250,300,350,400,500,600};
  
  // points for zchia
//   std::vector<float> ctaus = {100};
//   std::vector<int>   lambs = {100,150,200,250,300,350,400};

  TString outdir = "Lambda";
  FileStat_t dummyFileStat;
  if (gSystem->GetPathInfo(outdir.Data(), dummyFileStat) == 1)
  {
    TString mkDir = "mkdir -p ";
    mkDir += outdir.Data();
    gSystem->Exec(mkDir.Data());
  }                                
  for (auto lamb : lambs)
  {
    std::ofstream output(Form("Lambda/Lambda%iTeV_cgrav.txt",lamb),std::ios_base::trunc);
    for (auto ctau : ctaus)
    {
      TString CTau;
      if (ctau < 1)
      {
	int n = -1;
	float ictau = ctau;
	while (ictau < (1.f-tolerance))
	{
	  ictau *= 10;
	  n++;
	}
	const TString sub = Form("%i.%i",3+n,1+n);
	CTau = Form(Form("%%%sf",sub.Data()),ctau);
      }
      else
      {
	int ictau = ctau;
	CTau = Form("%i",ictau);
      }
      output << CTau.Data() << " " << getcgrav(lamb,ctau) << std::endl;
    }
    output.close();
  }
}
