inline float getcgrav(int lamb, const float ctau)
{
  const float cgrav_down = std::sqrt(ctau)*lamb*(-0.0001931*lamb+0.12041);
  const float cgrav_up   = std::sqrt(ctau)*(0.06656*lamb+3.6);

  return (cgrav_down+cgrav_up)/2.f;
}

void gencgrav()
{
  std::vector<float> ctaus = {0.1,10,200,400,600,800,1000,1200}; // cm
  std::vector<int>   lambs = {100,150,200,250,300,350,400}; // tev

  for (auto lamb : lambs)
  {
    std::ofstream output(Form("Lambda%i_cgrav.txt",lamb),std::ios_base::trunc);
    for (auto ctau : ctaus)
    {
      TString CTau;
      if (ctau < 1)
      {
	CTau = Form("%3.1f",ctau);
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
