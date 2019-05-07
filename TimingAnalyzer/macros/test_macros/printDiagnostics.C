enum Param {bkg1, c1, c2};

static const std::map<Param,TString> paramMap = 
{
  {Param::bkg1,"bkg1_2017"},
  {Param::c1,"c1_2017"},
  {Param::c2,"c2_2017"},
};

struct Var
{
  Var() {}

  Double_t val;
  Double_t unc;
};

void prepVars(TFile * file, std::map<Param,Var> & varMap, std::map<std::pair<Param,Param>,Double_t> & covMap)
{
  const auto & fit_b = (RooFitResult*)file->Get("fit_b");
  const auto & pars_list = fit_b->floatParsFinal();

  for (const auto & paramPair : paramMap)
  {
    const auto param = paramPair.first;
    const auto & param_name = paramPair.second;
    const auto idx = pars_list.index(param_name.Data());
    const auto var = (RooRealVar*)pars_list.at(idx);

    varMap[param].val = var->getVal();
    varMap[param].unc = var->getError();
    //    std::cout << param_name.Data() << ": " << varMap.at(param).val << " +/- " << varMap.at(param).unc << std::endl;
  }
  
  for (const auto & paramPair_i : paramMap)
  {
    const auto param1 = paramPair_i.first;
    const auto & param_name1 = paramPair_i.second;
    
    for (const auto & paramPair_j : paramMap)
    {
      const auto param2 = paramPair_j.first;
      const auto & param_name2 = paramPair_j.second;
      covMap[{param1,param2}] = fit_b->correlation(param_name1,param_name2)*varMap[param1].unc*varMap[param2].unc;
      //      std::cout << param_name1.Data() << " " << param_name2.Data() << " : " << fit_b->correlation(param_name1,param_name2) << " = " << covMap.at({param1,param2}) << std::endl;
    }
  }

  // const auto & covMat = fit_b->covarianceMatrix();
  // for (auto i = 0; i < covMat.GetNcols(); i++)
  //   for (auto j = 0; j < covMat.GetNrows(); j++)
  //     std::cout << i << "," << j << " : " << covMat[i][j] << std::endl;

  // delete it all
  delete fit_b;
}

void printDiagnostics(const TString & filename_stat = "", const TString & filename_tot = "", const TString & label = "")
{
  // initialize it all
  auto file_stat = TFile::Open(filename_stat.Data());
  auto file_tot  = TFile::Open(filename_tot .Data());

  std::map<Param,Var> varMap_stat;
  std::map<Param,Var> varMap_tot;

  std::map<std::pair<Param,Param>,Double_t> covMap_stat;
  std::map<std::pair<Param,Param>,Double_t> covMap_tot;

  // get vars
  prepVars(file_stat,varMap_stat,covMap_stat);
  prepVars(file_tot ,varMap_tot ,covMap_tot);

  // compute yield: A
  const auto predA          = varMap_tot.at(Param::bkg1).val;
  const auto predA_unc_stat = varMap_stat.at(Param::bkg1).unc;
  const auto predA_unc_tot  = varMap_tot.at(Param::bkg1).unc;
  const auto predA_unc_syst = std::sqrt(std::pow(predA_unc_tot,2)-std::pow(predA_unc_stat,2));

  // compute yield: B
  const auto predB          = varMap_tot.at(Param::bkg1).val*varMap_tot.at(Param::c1).val;
  const auto predB_unc_stat = varMap_stat.at(Param::bkg1).val*varMap_stat.at(Param::c1).val*
                              std::sqrt(std::pow(varMap_stat.at(Param::bkg1).unc/varMap_stat.at(Param::bkg1).val,2)+
					std::pow(varMap_stat.at(Param::c1).unc/varMap_stat.at(Param::c1).val,2)+
					(2*covMap_stat.at({Param::bkg1,Param::c1})/(varMap_stat.at(Param::bkg1).val*varMap_stat.at(Param::c1).val)));
  const auto predB_unc_tot  = varMap_tot.at(Param::bkg1).val*varMap_tot.at(Param::c1).val*
                              std::sqrt(std::pow(varMap_tot.at(Param::bkg1).unc/varMap_tot.at(Param::bkg1).val,2)+
					std::pow(varMap_tot.at(Param::c1).unc/varMap_tot.at(Param::c1).val,2)+
					(2*covMap_tot.at({Param::bkg1,Param::c1})/(varMap_tot.at(Param::bkg1).val*varMap_tot.at(Param::c1).val)));
  const auto predB_unc_syst = std::sqrt(std::pow(predB_unc_tot,2)-std::pow(predB_unc_stat,2));

  // compute yield: D
  const auto predD          = varMap_tot.at(Param::bkg1).val*varMap_tot.at(Param::c2).val;
  const auto predD_unc_stat = varMap_stat.at(Param::bkg1).val*varMap_stat.at(Param::c2).val*
                              std::sqrt(std::pow(varMap_stat.at(Param::bkg1).unc/varMap_stat.at(Param::bkg1).val,2)+
					std::pow(varMap_stat.at(Param::c2).unc/varMap_stat.at(Param::c2).val,2)+
					(2*covMap_stat.at({Param::bkg1,Param::c2})/(varMap_stat.at(Param::bkg1).val*varMap_stat.at(Param::c2).val)));
  const auto predD_unc_tot  = varMap_tot.at(Param::bkg1).val*varMap_tot.at(Param::c2).val*
                              std::sqrt(std::pow(varMap_tot.at(Param::bkg1).unc/varMap_tot.at(Param::bkg1).val,2)+
					std::pow(varMap_tot.at(Param::c2).unc/varMap_tot.at(Param::c2).val,2)+
					(2*covMap_tot.at({Param::bkg1,Param::c2})/(varMap_tot.at(Param::bkg1).val*varMap_tot.at(Param::c2).val)));
  const auto predD_unc_syst = std::sqrt(std::pow(predD_unc_tot,2)-std::pow(predD_unc_stat,2));

  // compute yield: C
  const auto predC          = varMap_tot.at(Param::bkg1).val*varMap_tot.at(Param::c1).val*varMap_tot.at(Param::c2).val;
  const auto predC_unc_stat = varMap_stat.at(Param::bkg1).val*varMap_stat.at(Param::c1).val*varMap_stat.at(Param::c2).val*
                              std::sqrt(std::pow(varMap_stat.at(Param::bkg1).unc/varMap_stat.at(Param::bkg1).val,2)+
					std::pow(varMap_stat.at(Param::c1).unc/varMap_stat.at(Param::c1).val,2)+
					std::pow(varMap_stat.at(Param::c2).unc/varMap_stat.at(Param::c2).val,2)+
					(2*covMap_stat.at({Param::bkg1,Param::c1})/(varMap_stat.at(Param::bkg1).val*varMap_stat.at(Param::c1).val))+
					(2*covMap_stat.at({Param::bkg1,Param::c2})/(varMap_stat.at(Param::bkg1).val*varMap_stat.at(Param::c2).val))+
					(2*covMap_stat.at({Param::c1  ,Param::c2})/(varMap_stat.at(Param::c1).val*varMap_stat.at(Param::c2).val)));
  const auto predC_unc_tot  = varMap_tot.at(Param::bkg1).val*varMap_tot.at(Param::c1).val*varMap_tot.at(Param::c2).val*
                              std::sqrt(std::pow(varMap_tot.at(Param::bkg1).unc/varMap_tot.at(Param::bkg1).val,2)+
					std::pow(varMap_tot.at(Param::c1).unc/varMap_tot.at(Param::c1).val,2)+
					std::pow(varMap_tot.at(Param::c2).unc/varMap_tot.at(Param::c2).val,2)+
					(2*covMap_tot.at({Param::bkg1,Param::c1})/(varMap_tot.at(Param::bkg1).val*varMap_tot.at(Param::c1).val))+
					(2*covMap_tot.at({Param::bkg1,Param::c2})/(varMap_tot.at(Param::bkg1).val*varMap_tot.at(Param::c2).val))+
					(2*covMap_tot.at({Param::c1  ,Param::c2})/(varMap_tot.at(Param::c1).val*varMap_tot.at(Param::c2).val)));
  const auto predC_unc_syst = std::sqrt(std::pow(predC_unc_tot,2)-std::pow(predC_unc_stat,2));
  
  // print it out
  std::ofstream output(label+".txt",std::ios::trunc);
  output << "bin: val +/- stat +/- syst" << std::endl;
  output << "A: " << predA << " +/- " << predA_unc_stat << " +/- " << predA_unc_syst << " = " << predA_unc_tot << std::endl;
  output << "B: " << predB << " +/- " << predB_unc_stat << " +/- " << predB_unc_syst << " = " << predB_unc_tot << std::endl;
  output << "C: " << predC << " +/- " << predC_unc_stat << " +/- " << predC_unc_syst << " = " << predC_unc_tot << std::endl;
  output << "D: " << predD << " +/- " << predD_unc_stat << " +/- " << predA_unc_syst << " = " << predD_unc_tot << std::endl;

  // delete it all
  delete file_tot;
  delete file_stat;
}

