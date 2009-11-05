{
gROOT->ProcessLine(".L /reg/neh/home/ncoppola/diode/trunk/cass_dictionaries/libcass_dictionaries.so");
gROOT->ProcessLine(".L /reg/neh/home/ncoppola/diode/trunk/cass_machinedata/libcass_machinedata.so");
gROOT->ProcessLine(".L /reg/neh/home/ncoppola/diode/trunk/cass_remi/libcass_remi.so");
gROOT->ProcessLine(".L /reg/neh/home/ncoppola/diode/trunk/cass_vmi/libcass_vmi.so");
gROOT->ProcessLine(".L /reg/neh/home/ncoppola/diode/trunk/cass_pnccd/libcass_pnccd.so");

TMapFile *n = TMapFile::Create("/dev/shm/test_root_copp.map","read",-1);
TTree *T=(TTree*)n->Get("T");

cass::MachineData::MachineDataEvent *MachineEventBranch = 0;
TBranch *branchM = T->GetBranch("MachineEventBranch");
T->SetBranchAddress("MachineEventBranch",&MachineEventBranch);

cass::REMI::REMIEvent *REMIEventBranch = 0;
TBranch *branchR = T->GetBranch("REMIEventBranch");
T->SetBranchAddress("REMIEventBranch",&REMIEventBranch);

cass::VMI::VMIEvent *VMIEventBranch = 0;
TBranch *branchV = T->GetBranch("VMIEventBranch");
T->SetBranchAddress("VMIEventBranch",&VMIEventBranch);

cass::pnCCD::pnCCDEvent *pnCCDEventBranch = 0;
TBranch *branchP = T->GetBranch("pnCCDEventBranch");
T->SetBranchAddress("pnCCDEventBranch",&pnCCDEventBranch);

}
