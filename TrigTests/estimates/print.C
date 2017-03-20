void print()
{
  std::vector<TString> pathsToRemove =
  {
    "DST_",
    "AlCa_",
    "HLT_EcalCalibration_v",
    "HLT_HcalCalibration_v",
    "HLT_HT410to430_v",
    "HLT_HT430to450_v",
    "HLT_HT450to470_v",
    "HLT_HT470to500_v",
    "HLT_HT500to550_v",
    "HLT_HT550to650_v",
    "HLT_HT650_v",
    "HLT_Physics_v",
    "HLT_L1FatEvents_part0_v",
    "HLT_L1FatEvents_part1_v",
    "HLT_L1FatEvents_part2_v",
    "HLT_L1FatEvents_part3_v",
    "HLT_L1FatEvents_v",
    "HLT_ZeroBias_part0_v",
    "HLT_ZeroBias_part1_v",
    "HLT_ZeroBias_part2_v",
    "HLT_ZeroBias_part3_v",
    "HLT_ZeroBias_part4_v",
    "HLT_Physics_part0_v",
    "HLT_Physics_part1_v",
    "HLT_Physics_part2_v",
    "HLT_Physics_part3_v",
    "HLT_ZeroBias_v",
    "HLT_Random_v",
    "HLT_RsqMR240_Rsq0p09_MR200_4jet_v",
    "HLT_Photon90_CaloIdL_PFHT500_v",
    "HLT_Mu3er_PFHT140_PFMET125_v",
    "HLT_Mu6_PFHT200_PFMET80_BTagCSV_p067_v",
    "HLT_PFMET120_BTagCSV_p067_v",
    "_Prescl",
    "MC_"
  };

  TString dir = "ps_1p6e34/hlt0/";
  TFile * file = TFile::Open(dir+"hltbits.root");
  TTree * tree = (TTree*)file->Get("HltTree");
  
  TObjArray leaves = *(tree->GetListOfLeaves());
  for (Int_t i = 0; i < leaves.GetEntries(); i++)
  {
    TString path = leaves[i]->GetName();
    Bool_t skip = false;
    for (UInt_t j = 0; j < pathsToRemove.size(); j++)
    {
      if (path.Contains(pathsToRemove[j])) skip = true;
    }
    if (!skip) std::cout << path.Data() << std::endl;
  }

  //  tree->Print();
}
