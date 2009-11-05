#include <TNtuple.h>
#include <TH1.h>
#include <TClassTable.h>
#include <TSystem.h>
#include "TDirectory.h"
#include <TClass.h>
#include <TDataType.h>
#include <TMapFile.h>

Int_t entries;
Int_t entry;
#include <iostream>
#include "../cass_database/cass_tree.h"
#include "../cass_machinedata/classes/event/machine_event.h"
TMapFile *n = TMapFile::Create("/dev/shm/test_root_copp.map","read",-1);

void h_map_test(){

TTree *T=(TTree*)n->Get("T");

if(!TClassTable::GetDict("cass::MachineData::MachineDataEvent"))
{
  gSystem->Load("/reg/neh/home/ncoppola/diode/trunk/cass_dictionaries/libcass_dictionaries.so");
  gSystem->Load("/reg/neh/home/ncoppola/diode/trunk/cass_machinedata/libcass_machinedata.so");
}

//  T->Print();
entries=(Int_t)T->GetEntries();
T->SetBranchAddress("Nevent",&Nevent);

//#ifdef as
cass::MachineData::MachineDataEvent *MachineEventBranch;
MachineEventBranch = 0;
//TBranch *branchM = T->GetBranch("MachineEventBranch");
T->SetBranchAddress("MachineEventBranch",&MachineEventBranch);
cass::MachineData::MachineDataEvent::EpicsDataMap::iterator ito;

printf("entries read %i\n",entries);
for(entry=0;entry<entries;entry++)
{
  T->GetEntry(entry);

  //  printf("%i %i\n",entry,Nevent);
  printf("mach %i %f %i\n",entry,MachineEventBranch->f_11_ENRC(),
    MachineEventBranch->EpicsData().size());

  for( ito=MachineEventBranch->EpicsData().begin();
          ito!=MachineEventBranch->EpicsData().end();ito++)
  { 
    //    if((*ito).second != 0)
    std::cout << (*ito).first << " ulla " << (*ito).second << std::endl;
  }
  //return;
}
//#endif
}
