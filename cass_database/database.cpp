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

//TFile f("tree_and_histos.root","RECREATE");
//TTree *T = new TTree("T","circ buffer");

#include "cass_tree.h"

//#include "histo_list.h"
//void reset_lastevt_histos();
//#include "reset_histos.h"
//#include "fill_histos.h"
//void   fill_lastevt_histos();
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
//TTree *T=0;

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

  sprintf(Tmap_filename,"%s","/dev/shm/test_root_");
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
  gROOT->cd();
  T= new TTree("T","circ buffer");

  mapfile = TMapFile::Create(Tmap_filename,"RECREATE", 500000000, "");
  mapfile->Add(T,"T");
  //mapfile->Print();
  //mapfile->ls();

  //  Long64_t fgMaxTreeSize=1000000000;
  //T->SetMaxTreeSize(fgMaxTreeSize);
  //printf("Linear Tree allocated with %i bytes\n",fgMaxTreeSize);


  // this is where I am going to start the tree
  T->Branch("Nevent",&Nevent,"Nevent/l");

  // the eventid
  T->Branch("CASS_id",&event_id,"CASS_id/l");

  //machine quantities
  if(!TClassTable::GetDict("cass::MachineData::MachineDataEvent"))
  {
    gSystem->Load("$Dict_LIB/libcass_dictionaries.so");
  }
  else printf("I have got already the MachineData definitions\n");

  cass::MachineData::MachineDataEvent *machinedata = new cass::MachineData::MachineDataEvent();
  T->Branch("MachineEventBranch","cass::MachineData::MachineDataEvent",&machinedata,64000,0);
  delete machinedata;
  //TBranch::SetAutoDelete(kTRUE);

  //REMI
  if(!TClassTable::GetDict("cass::REMI::REMIEvent"))
  {
    gSystem->Load("$Dict_LIB/libcass_dictionaries.so");
  }
  else printf("I have got already the REMI definitions\n");
  cass::REMI::REMIEvent *REMIdata = new cass::REMI::REMIEvent();
  T->Branch("REMIEventBranch","cass::REMI::REMIEvent",&REMIdata,128000,0);
  delete REMIdata;

  //VMI
  if(!TClassTable::GetDict("cass::VMI::VMIEvent"))
  {
    gSystem->Load("$Dict_LIB/libcass_dictionaries.so");
  }
  else printf("I have got already the VMI definitions\n");
  cass::VMI::VMIEvent *VMIdata = new cass::VMI::VMIEvent();
  T->Branch("VMIEventBranch","cass::VMI::VMIEvent",&VMIdata,6000000,0);
  delete VMIdata;

  //pnCCD
  if(!TClassTable::GetDict("cass::pnCCD::pnCCDEvent"))
  {
    gSystem->Load("$Dict_LIB/libcass_dictionaries.so");
  }
  else printf("I have got already the pnCCD definitions\n");
  cass::pnCCD::pnCCDEvent *pnCCDdata = new cass::pnCCD::pnCCDEvent();
  T->Branch("pnCCDEventBranch","cass::pnCCD::pnCCDEvent",&pnCCDdata,6000000,0);
  delete pnCCDdata;

#ifdef sng_pnccd
  //pnCCD (2)
  T->Branch("pnCCD_array_x_size0",&pnCCD_array_x_size0,"pnCCD_array_x_size0/I");
  T->Branch("pnCCD_array_y_size0",&pnCCD_array_y_size0,"pnCCD_array_y_size0/I");

  T->Branch("pnCCD_array_x_size1",&pnCCD_array_x_size1,"pnCCD_array_x_size1/I");
  T->Branch("pnCCD_array_y_size1",&pnCCD_array_y_size1,"pnCCD_array_y_size1/I");

  T->Branch("pnCCD_max_photons_per_event0",&pnCCD_max_photons_per_event0,"pnCCD_max_photons_per_event0/I");
  T->Branch("pnCCD_max_photons_per_event1",&pnCCD_max_photons_per_event1,"pnCCD_max_photons_per_event1/I");

  T->Branch("pnCCD_array_xy_size0",&pnCCD_array_xy_size0,"pnCCD_array_xy_size0/I");
  T->Branch("pnCCD_array_xy_size1",&pnCCD_array_xy_size1,"pnCCD_array_xy_size1/I");
#endif

#ifdef sng_pnccd
  //T->Branch("pnCCD_raw_0",pnCCD_raw_0,"pnCCD_raw_0[1048576]/s");
  //T->Branch("pnCCD_raw_1",pnCCD_raw_1,"pnCCD_raw_1[1048576]/s");
  T->Branch("pnCCD_corr_0",pnCCD_corr_0,"pnCCD_corr_0[1048576]/s");
  T->Branch("pnCCD_corr_1",pnCCD_corr_1,"pnCCD_corr_1[1048576]/s");
#endif

  T->SetAutoSave();
  //T->BranchRef();
  //T->SetCompressionLevel(1);
  T->SetCircular(max_events_in_Buffer);
  printf("Circular buffer allocated with %i events\n",max_events_in_Buffer);

  T->Print();
  Nevent=0;
}

cass::database::Database::~Database()
{
  mapfile->Close("close");
  //delete T;
  //delete all histos, if created
  //delete h_pnCCD1r_lastevt; h_pnCCD1r_lastevt=0;

}

void cass::database::Database::add(cass::CASSEvent* cassevent)
{

  if(!cassevent) return;
  //  std::cout<< " I am in"<<std::endl;
  Nevent++;

  event_id=cassevent->id();

  cass::MachineData::MachineDataEvent *machinedata = &cassevent->MachineDataEvent();
  T->SetBranchAddress("MachineEventBranch",&machinedata);

  cass::REMI::REMIEvent *REMIdata = &cassevent->REMIEvent();
  T->SetBranchAddress("REMIEventBranch",&REMIdata);
#ifdef REMI_DEB
  std::cout << "remi test" << REMIdata->sampleInterval() << std::endl;
#endif
  //  std::cout<< " I am in1"<<std::endl;
  cass::VMI::VMIEvent *VMIdata = &cassevent->VMIEvent();
  T->SetBranchAddress("VMIEventBranch",&VMIdata);
  //  std::cout<< " I am in3"<<std::endl;
#ifdef VMI_DEB
  std::cout<< VMIdata->frame().size() << " a " <<
    VMIdata->cutFrame().size() << " b " <<
    VMIdata->coordinatesOfImpact().size() << std::endl;
#endif

  cass::pnCCD::pnCCDEvent *pnCCDdata = &cassevent->pnCCDEvent();
  T->SetBranchAddress("pnCCDEventBranch",&pnCCDdata);
//  for (size_t ididid=0; ididid<pnCCDdata->detectors().size();++ididid)
//      std::cout <<"Det:"<<ididid<<" size:"<< pnCCDdata->detectors()[ididid].correctedFrame().size()<<std::endl;

#ifdef wide_pnccd
  cass::pnCCD::pnCCDEvent &pnccdevent = cassevent->pnCCDEvent();
  pnCCD_num_pixel_arrays=pnccdevent.getNumPixArrays();

  pnCCD_array_xy_size0=0;
  pnCCD_array_xy_size1=0;

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

    pnCCD_array_xy_size0=pnccdevent.getArrXSize()[0]*pnccdevent.getArrYSize()[0];
    if(pnCCD_num_pixel_arrays==2)
    {
      pnCCD_array_x_size1=pnccdevent.getArrXSize()[1];
      pnCCD_array_y_size1=pnccdevent.getArrYSize()[1];
      pnCCD_max_photons_per_event1=TMath::Min(int(pnccdevent.getMaxPhotPerEvt()[1]),
         MAX_pnCCD_max_photons_per_event);
      pnCCD_max_photons_per_event1=TMath::Min(pnCCD_max_photons_per_event1,max_phot_in_Buffer);
      pnCCD_array_xy_size1=pnccdevent.getArrXSize()[1]*pnccdevent.getArrYSize()[1];
    }
  } 
  if(pnccdevent.rawSignalArrayAddr(1)!=0)
  {
     memcpy(&pnCCD_raw_0,
	    pnccdevent.rawSignalArrayAddr(1), 1024*1024*2);
  }
  if(pnccdevent.rawSignalArrayAddr(2)!=0)
  {
     memcpy(&pnCCD_raw_1,
	    pnccdevent.rawSignalArrayAddr(2), 1024*1024*2);
  }
#endif

#ifdef sgl
  cass::pnCCD::pnCCDEvent &pnccdevent = cassevent->pnCCDEvent();
  size_t pnCCD_num_pixel_arrays=pnccdevent.detectors().size();
  if(pnCCD_num_pixel_arrays>0&&  ( pnccdevent.detectors()[0].correctedFrame()[0]!=0))
  {
     memcpy(&pnCCD_raw_0,
	    &pnccdevent.detectors()[0].rawFrame()[0],
            //	    &pnccdevent.detectors()[0].correctedFrame()[0],
          1024*1024*2);

  if(pnCCD_num_pixel_arrays>1&&  ( pnccdevent.detectors()[1].correctedFrame()[0]!=0))
  {
     memcpy(&pnCCD_raw_1,
	    &pnccdevent.detectors()[1].rawFrame()[0],
            //	    &pnccdevent.detectors()[1].correctedFrame()[0],
          1024*1024*2);
  }
  }
#endif

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

  //  std::cout<< " I am in a"<<std::endl;
  if ( max_events_in_Buffer>99 && (Nevent%10)==0 )
  {
    time(&rawtime);
    timeinfo=localtime(&rawtime);
    strftime(hourmin,11,"%H%M%S",timeinfo);
    printf("done/seen event %i %s\n",int(Nevent)
            , hourmin );
    /*printf("done/seen event %i %u %s\n",int(Nevent), //int(event_id),
          pnCCDdata->detectors()[0].correctedFrame().size()  , hourmin );*/
    //T->Show(i%max_events_in_Buffer-1);
  }

  T->Fill();
  //mapfile->Update(TObject obj = 0);
  if( ( Nevent%(20) )==0 ) mapfile->Update();

#ifdef sng_update
  if ( max_events_in_Buffer>99)
  {

    if( ( Nevent%(10) )==0 )
    {
      printf("updating mapfile\n");
      mapfile->Update();
    }
  }
  else
  {
    if(Nevent%5==0)
    {
      printf("updating now small if\n");
      mapfile->Update();
    }
  }
#endif

//  delete cassevent;
  //printf("I will send the nextEvent signal\n");
  emit nextEvent();
}

/*cass::CASSEvent* cass::database::Database::nextEvent()
{
   return 0;
}*/

 /*cass::CASSEvent* cass::database::Database::OpennextFile(int nof)
{
  return 0;
  }*/
