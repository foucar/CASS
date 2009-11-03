/*
 *  Database.cpp
 *  diode
 *
 *  Created by Nicola Coppola & lutz foucar .
 *  
 */

#include "database.h"
#include "cass_event.h"

#include "machine_event.h"
#include "remi_event.h"
#include "vmi_event.h"
#include "pnccd_event.h"

#include <TROOT.h>
#include <TRandom.h>
#include <TNtuple.h>
#include <TMath.h>
#include <TH2.h>
#include <TFile.h>
#include <TClassTable.h>
#include <TSystem.h>
#include "TDirectory.h"
#include "TProcessID.h"
#include <TCut.h>
#include <TThread.h>
#include <TClass.h>
#include <TDataType.h>
#include <TMapFile.h>

TFile f("tree_and_histos.root","RECREATE");
TTree *T = new TTree("T","circ buffer");

#include "cass_tree.h"

#include "histo_list.h"
void reset_lastevt_histos();
#include "reset_histos.h"
#include "fill_histos.h"
void   fill_lastevt_histos();
#include <time.h>
time_t rawtime;
struct tm * timeinfo;
char hourmin[12];

//I put the map-file on the shm.... device, but maybe there is a better place..
// it was 2000000000

// reduced version
#include <stdlib.h>
#include <string.h>
#include "map_files_name.h"
char username[10];

TMapFile *mapfile;

#ifndef ROOT_Rtypes
#include <Rtypes.h>
#endif

#ifndef ROOT_TObject
#include <TObject.h>
#endif

ClassImp(cass::MachineData::MachineDataEvent);
ClassImp(cass::REMI::REMIEvent);
ClassImp(cass::VMI::VMIEvent);
ClassImp(cass::pnCCD::pnCCDEvent);
//ClassImp(cass::CASSEvent)

cass::database::Database::Database()
{

  Double_t random;

  sprintf(Tmap_filename,"%s","/dev/shm/test_root_");
  //printf("%s\n",Tmap_filename);
  strcpy(username,"");

  if(getenv("USER")!=NULL)
  {
    sprintf(username,"%s", getenv("USER"));
  }
  else sprintf(username,"%s","nobody");
  
  //  printf("user is %s\n",username);
  if(strcmp(username,"ncoppola")==0)
    strcat(Tmap_filename,"copp.map");
  else if(strcmp(username,"lutz")==0)
    strcat(Tmap_filename,"lutz.map");
  else 
  {  
    printf("unknown user\n");
    strcat(Tmap_filename,"nobody.map\0");
  } 
  printf("TMapFile is in %s\n",Tmap_filename);
  // it was 2000000000
  mapfile = TMapFile::Create(Tmap_filename,"RECREATE", 800000000, "");
  mapfile->Add(T,"T");
  //mapfile->Print();
  //mapfile->ls();

  Long64_t fgMaxTreeSize=100000000;
  T->SetMaxTreeSize(fgMaxTreeSize);
  printf("Linear Tree allocated with %i bytes\n",fgMaxTreeSize);

  //should I add a header to the tree??

  // this is where I am going to start the tree
  T->Branch("Nevents",&Nevents,"Nevents/i");
  T->Branch("px",&px,"px/F");
  T->Branch("py",&py,"py/F");
  //T->Branch("pz",&pz,"pz/F");
  T->Branch("random",&random,"random/D");

  // the eventid
  T->Branch("CASS_id",&event_id,"CASS_id/l");

  //machine quantities
  if(!TClassTable::GetDict("cass::MachineData::MachineDataEvent"))
  {
    gSystem->Load("$Dict_LIB/libcass_dictionaries.so");
  }
  else printf("I have got already the MachineData definitions\n");
  cass::MachineData::MachineDataEvent *machinedata = new cass::MachineData::MachineDataEvent();
  T->Branch("MachineEventBranch","cass::MachineData::MachineDataEvent",&machinedata,32000,99);
  //  delete mac_things_event;

  //REMI
  if(!TClassTable::GetDict("cass::REMI::REMIEvent"))
  {
    gSystem->Load("$Dict_LIB/libcass_dictionaries.so");
  }
  else printf("I have got already the REMI definitions\n");
  cass::REMI::REMIEvent *REMIdata = new cass::REMI::REMIEvent();
  T->Branch("REMIEventBranch","cass::REMI::REMIEvent",&REMIdata,32000,99);
  //  delete REMI_things_event;

  //VMI
  if(!TClassTable::GetDict("cass::VMI::VMIEvent"))
  {
    gSystem->Load("$Dict_LIB/libcass_dictionaries.so");
  }
  else printf("I have got already the VMI definitions\n");
  cass::VMI::VMIEvent *VMIdata = new cass::VMI::VMIEvent();
  T->Branch("VMIEventBranch","cass::VMI::VMIEvent",&VMIdata,32000,99);
  //  delete VMI_things_event;

  //pnCCD
  if(!TClassTable::GetDict("cass::pnCCD::pnCCDEvent"))
  {
    gSystem->Load("$Dict_LIB/libcass_dictionaries.so");
  }
  else printf("I have got already the pnCCD definitions\n");
  cass::pnCCD::pnCCDEvent *pnCCDdata = new cass::pnCCD::pnCCDEvent();
  T->Branch("pnCCDEventBranch","cass::pnCCD::pnCCDEvent",&pnCCDdata,32000,99);
  //  delete pnCCD_things_event;

  //pnCCD (2)
  T->Branch("pnCCD_array_x_size0",&pnCCD_array_x_size0,"pnCCD_array_x_size0/I");
  T->Branch("pnCCD_array_y_size0",&pnCCD_array_y_size0,"pnCCD_array_y_size0/I");

  T->Branch("pnCCD_array_x_size1",&pnCCD_array_x_size1,"pnCCD_array_x_size1/I");
  T->Branch("pnCCD_array_y_size1",&pnCCD_array_y_size1,"pnCCD_array_y_size1/I");

  T->Branch("pnCCD_max_photons_per_event0",&pnCCD_max_photons_per_event0,"pnCCD_max_photons_per_event0/I");
  T->Branch("pnCCD_max_photons_per_event1",&pnCCD_max_photons_per_event1,"pnCCD_max_photons_per_event1/I");

  T->SetAutoSave();
  T->BranchRef();
  //T->SetCompressionLevel(0);
  //T->SetCircular(max_events_in_Buffer);
  //printf("Circular buffer allocated with %i events\n",max_events_in_Buffer);

  T->Print();
  Nevents=0;
}

cass::database::Database::~Database()
{
  // delete all histos
  delete h_pnCCD1r_lastevt; h_pnCCD1r_lastevt=0;

}

void cass::database::Database::add(cass::CASSEvent* cassevent)
{
  Double_t random;
#ifdef DEBUG
  Int_t jj,kk;
  UInt_t jj_u;
#endif

  if(!cassevent) return;

  Nevents++;
  //if(Nevents>299) printf("Nevents=%i \n",Nevents);
  // just to have something filled...

  r.Rannor(px,py);
  random=double(r.Rndm());

  event_id=cassevent->id();

  cass::MachineData::MachineDataEvent *machinedata = &cassevent->MachineDataEvent();
  T->SetBranchAddress("MachineEventBranch",&machinedata);

  cass::REMI::REMIEvent *REMIdata = &cassevent->REMIEvent();
  T->SetBranchAddress("REMIEventBranch",&REMIdata);

  cass::VMI::VMIEvent *VMIdata = &cassevent->VMIEvent();
  T->SetBranchAddress("VMIEventBranch",&VMIdata);


  cass::pnCCD::pnCCDEvent *pnCCDdata = &cassevent->pnCCDEvent();
  T->SetBranchAddress("pnCCDEventBranch",&pnCCDdata);

  cass::pnCCD::pnCCDEvent &pnccdevent = cassevent->pnCCDEvent();
  pnCCD_num_pixel_arrays=pnccdevent.getNumPixArrays();

  pnCCD_array_x_size0=0;
  pnCCD_array_y_size0=0;
  pnCCD_array_x_size1=0;
  pnCCD_array_y_size1=0;
  pnCCD_max_photons_per_event0=0;
  pnCCD_max_photons_per_event1=0;
  if(pnCCD_num_pixel_arrays>0)
  {
    pnCCD_array_x_size0=pnccdevent.getArrXSize()[0];
    pnCCD_array_y_size0=pnccdevent.getArrYSize()[0];
    pnCCD_max_photons_per_event0=TMath::Min(int(pnccdevent.getMaxPhotPerEvt()[0]),
         MAX_pnCCD_max_photons_per_event);
    pnCCD_max_photons_per_event0=TMath::Min(pnCCD_max_photons_per_event0,max_phot_in_Buffer_loose);
    if(pnCCD_num_pixel_arrays==2)
    {
      pnCCD_array_x_size1=pnccdevent.getArrXSize()[1];
      pnCCD_array_y_size1=pnccdevent.getArrYSize()[1];
      pnCCD_max_photons_per_event1=TMath::Min(int(pnccdevent.getMaxPhotPerEvt()[1]),
         MAX_pnCCD_max_photons_per_event);
      pnCCD_max_photons_per_event1=TMath::Min(pnCCD_max_photons_per_event1,max_phot_in_Buffer);
    }
  } 

#if DEBUG_pnCCD_raw
  if(pnccdevent.rawSignalArrayAddr(1)!=0)
  {
    size_t idx=0;
    uint16_t* data = (pnccdevent.rawSignalArrayAddr(1));
    for (size_t iy=0;iy<pnCCD_array_y_size0;++iy)
    {
        for (size_t ix=0;ix<pnCCD_array_x_size0;++ix)
	  std::cout <<"m" << data[idx++]<<" "<< pnCCD_raw0[iy][ix];
        std::cout<<std::endl;
    }
  }
#endif


  if ( max_events_in_Buffer>99 && ( Nevents%(max_events_in_Buffer/10) )==0 )
  {
    time(&rawtime);
    timeinfo=localtime(&rawtime);
    strftime(hourmin,11,"%H%M%S",timeinfo);
    printf("done/seen event %i %i %s\n",Nevents,int(event_id), hourmin );
    //T->Show(i%max_events_in_Buffer-1);
  }

  T->Fill();
  //mapfile.Update(TObject obj = 0);

  if ( max_events_in_Buffer>99 && ( Nevents%(10) )==0 )
  {
    printf("updating mapfile\n");
    mapfile->Update();
  }
  else
  {
    if(Nevents%5==0) mapfile->Update();
  }

//  delete cassevent;
  //printf("I will send the nextEvent signal\n");
  emit nextEvent();
}

/*cass::CASSEvent* cass::database::Database::nextEvent()
{
   return 0;
}*/
