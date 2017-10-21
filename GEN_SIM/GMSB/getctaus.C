static const float tolerance = 1e-7;
typedef std::map<TString,float> sfmap;

void readIn(const TString & file, sfmap& ctaumap)
{
  const float hbarc = 1.973e-14; // GeV * cm/s 

  std::ifstream input(file.Data(),std::ios_base::in);
  float ctau;
  float width;
  while (input >> ctau >> width)
  {
    const float genctau = hbarc/width;

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

    ctaumap[CTau] = (ctau-genctau)/ctau;
  }
  input.close();
}

void getctaus(const int lamb, const TString & file)
{
  sfmap ctaumap;
  readIn(file,ctaumap);

  TString outdir = "Diffs";
  FileStat_t dummyFileStat;
  if (gSystem->GetPathInfo(outdir.Data(), dummyFileStat) == 1)
  {
    TString mkDir = "mkdir -p ";
    mkDir += outdir.Data();
    gSystem->Exec(mkDir.Data());
  }                                
  
  std::ofstream output(Form("%s/Lambda%iTeV_diffs.txt",outdir.Data(),lamb),std::ios::trunc);
  for (const auto & ctaupair : ctaumap)
  {
    output << ctaupair.first.Data() << " " << ctaupair.second << std::endl;
  }
  output.close();
}
