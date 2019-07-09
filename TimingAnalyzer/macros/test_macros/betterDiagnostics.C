void dumpInfo(std::ofstream & outfile, const TString & label, const TString & start);
TString PrintValueAndError(const Double_t val, const Double_t err);

void betterDiagnostics(const TString & fullC, const TString & maskC, const TString & category)
{
  std::ofstream outfile(category+".txt",std::ios::trunc);

  // full binC info
  dumpInfo(outfile,fullC,"& & \\Npostfit");

  // mask binC info
  dumpInfo(outfile,maskC,"& & \\Npostfitmask");
}

void dumpInfo(std::ofstream & outfile, const TString & label, const TString & start)
{
  // get file + main dir
  auto file = TFile::Open(label+".root");
  auto main_dir = (TDirectory*)file->Get("shapes_fit_b");

  // file start of line
  outfile << start.Data();

  // loop over bin labels and dump
  const std::vector<TString> bin_labels = {"1","2","3","4"};
  for (const auto & bin_label : bin_labels)
  {
    // move into correct channel
    main_dir->cd();
    auto sub_dir = (TDirectory*)main_dir->Get("ch1_Bin"+bin_label+"_2017");
    sub_dir->cd();
    
    // get histogram + values
    auto hist = (TH1F*)sub_dir->Get("bkg");
    const auto content = hist->GetBinContent(1);
    const auto error   = hist->GetBinError  (1);

    // fill outfile
    outfile << Form(" & %s",PrintValueAndError(content,error).Data());

    // delete it all
    delete hist;
    delete sub_dir;
  }
  
  // fill end of line
  outfile << " \\\\" << std::endl;
}

TString PrintValueAndError(const Double_t val, const Double_t err)
{
  TString output = "";
  
  if      (err >= 5.0)               output = Form("$%i \\pm %i$",Int_t(std::round(val)),Int_t(std::round(err)));
  else if (err <  5.0 && err >= 1.0) output = Form("$%.1f \\pm %.1f$",val,err);
  else
  {
    const TString tmp_err = Form("%.10f",err);
    Bool_t is_decimal = false;
    Int_t precision = 0;
    
    for (auto i = 0; i < tmp_err.Length(); i++)
    {
      const TString tmp = tmp_err[i];
      if (!is_decimal)
      {
	if (tmp.EqualTo(".")) is_decimal = true;
	continue;
      }
      
      if (is_decimal)
      { 
	precision++;
	if (!tmp.EqualTo("0"))
	{
	  const auto x = tmp.Atoi();
	  if (x >= 5) break;
	  else
	  {
	    precision += 1;
	    break;
	  }
	}
      }
    }
    
    output = Form("$%.*f \\pm %.*f$",precision,val,precision,err);
  }
  
  return output;
}
