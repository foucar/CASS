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

long long int timeDiff(struct timespec* end, struct timespec* start) {
  long long int diff;
  diff =  (end->tv_sec - start->tv_sec) * 1000000000;
  diff += end->tv_nsec;
  diff -= start->tv_nsec;
  return diff;
}

ClassImp(cass::MachineData::MachineDataEvent);
ClassImp(cass::REMI::REMIEvent);
ClassImp(cass::VMI::VMIEvent);
ClassImp(cass::pnCCD::pnCCDEvent);
//ClassImp(cass::CASSEvent)

cass::database::Database::Database()
{
  struct timespec start, now;
  //maybe the following should be moved somewhere else?? 
  loadSettings();
  if(_param._usejustFile==0)
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

    mapfile = TMapFile::Create(Tmap_filename,"RECREATE", 800000000, "");
    mapfile->Add(T,"T");
    //mapfile->Print();
    //mapfile->ls();
  }
  else
  {
    f= new TFile("tree_and_histos_CASS.root","RECREATE");
    T= new TTree("T","Cass tree");
  }

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

  if(_param._useREMI)
  {
    //REMI
    if(!TClassTable::GetDict("cass::REMI::REMIEvent"))
    {
      gSystem->Load("$Dict_LIB/libcass_dictionaries.so");
    }
    else printf("I have got already the REMI definitions\n");
    cass::REMI::REMIEvent *REMIdata = new cass::REMI::REMIEvent();
    T->Branch("REMIEventBranch","cass::REMI::REMIEvent",&REMIdata,128000,0);
    delete REMIdata;
  }
  else printf("Decided not to store REMI in database structure\n");

  if(_param._useVMI)
  {
    //VMI
    if(!TClassTable::GetDict("cass::VMI::VMIEvent"))
    {
      gSystem->Load("$Dict_LIB/libcass_dictionaries.so");
    }
    else printf("I have got already the VMI definitions\n");
    cass::VMI::VMIEvent *VMIdata = new cass::VMI::VMIEvent();
    T->Branch("VMIEventBranch","cass::VMI::VMIEvent",&VMIdata,2000000,0);
    delete VMIdata;
  }
  else printf("Decided not to store VMI in database structure\n");

  if(_param._usepnCCD)
  {
    //pnCCD
    if(!TClassTable::GetDict("cass::pnCCD::pnCCDEvent"))
    {
      gSystem->Load("$Dict_LIB/libcass_dictionaries.so");
    }
    else printf("I have got already the pnCCD definitions\n");
    cass::pnCCD::pnCCDEvent *pnCCDdata = new cass::pnCCD::pnCCDEvent();
    T->Branch("pnCCDEventBranch","cass::pnCCD::pnCCDEvent",&pnCCDdata,6000000,0);
    delete pnCCDdata;
  }
  else printf("Decided not to store pnCCD in database structure\n");

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
  if(_param._usejustFile==0)
  {
    if(max_events_in_Buffer<_param._number_ofevents)
    {
      T->SetCircular(max_events_in_Buffer);
      printf("Circular buffer allocated with %i events\n",max_events_in_Buffer);
    }
    else
    {
      T->SetCircular(_param._number_ofevents);
      printf("Circular buffer allocated with %i events\n",_param._number_ofevents);
    }

    printf("TMapFile update frequency is %i events\n",_param._updatefrequency);
  }
  else printf("saving to file, no circular tree declared\n");

  T->Print();
  Nevent=0;
#ifdef every
  printf("some more quantities for testing\n");

  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
  sum16u.resize(MAX_pnCCD_array_x_size * MAX_pnCCD_array_y_size);
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &now);
  printf("creating_16u %lld ns\n", timeDiff(&now, &start));

  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
  sum32i.resize(MAX_pnCCD_array_x_size * MAX_pnCCD_array_y_size);
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &now);
  printf("creating_32i %lld ns\n", timeDiff(&now, &start));

  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
  sum32f.resize(MAX_pnCCD_array_x_size * MAX_pnCCD_array_y_size);
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &now);
  printf("creating_32f %lld ns\n", timeDiff(&now, &start));

  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
  sum16u.assign(MAX_pnCCD_array_x_size * MAX_pnCCD_array_y_size,0);
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &now);
  printf("zeroing_16i %lld ns\n", timeDiff(&now, &start));

  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
  sum32i.assign(MAX_pnCCD_array_x_size * MAX_pnCCD_array_y_size,0);
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &now);
  printf("zeroing_32i %lld ns\n", timeDiff(&now, &start));

  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
  sum32f.assign(MAX_pnCCD_array_x_size * MAX_pnCCD_array_y_size,0.);
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &now);
  printf("zeroing_32f %lld ns\n", timeDiff(&now, &start));
#endif

  saveSettings();

}



void cass::database::Parameter::load()
{
  QString s;
  //sync before loading//
  std::cout << "loading database init-settings"<<std::endl;
  sync();
  _updatefrequency = value("UpdateFrequency",10).toUInt();
  _number_ofevents = value("NumberOfEvents",100).toUInt();
  //usejustFile 1: means actually use just File, no TMapFile
  //            0: actually, no File, just TMapFile
  _nofill          = value("DoNotFillTree",0).toUInt();
  _usejustFile     = value("usejustFile",0).toUInt();
  _useREMI         = value("useREMI",1).toUInt();
  _useVMI          = value("useVMI",1).toUInt();
  _usepnCCD        = value("usepnCCD",1).toUInt();
}

void cass::database::Parameter::save()
{
  QString s;
  std::cout << "saving database init-settings"<<std::endl;
  setValue("UpdateFrequency",_updatefrequency);
  setValue("NumberOfEvents",_number_ofevents);
  setValue("DoNotFillTree",_nofill);
  setValue("usejustFile",_usejustFile);
  setValue("useREMI",_useREMI);
  setValue("useVMI",_useVMI);
  setValue("usepnCCD",_usepnCCD);
  std::cout << "saved database running-settings"<<std::endl;
}



cass::database::Database::~Database()
{
  if(_param._usejustFile==0) mapfile->Close("close");
  if(_param._usejustFile!=0)
  {
    //closing tree-file
    f->cd();
    T->Write();
    f->Close();
    delete T;
  }
  //delete all histos, if created
  //delete h_pnCCD1r_lastevt; h_pnCCD1r_lastevt=0;

}

void cass::database::Database::add(cass::CASSEvent* cassevent)
{

  if(!cassevent) return;
  //  std::cout<< " I am in"<<std::endl;
  struct timespec start, now;
  Nevent++;

  event_id=cassevent->id();

  cass::MachineData::MachineDataEvent *machinedata = &cassevent->MachineDataEvent();
  T->SetBranchAddress("MachineEventBranch",&machinedata);
#ifdef debug
  std::cout << "power " << machinedata->f_11_ENRC() << " " << machinedata->f_21_ENRC() << std::endl;
#endif
  cass::MachineData::MachineDataEvent::EpicsDataMap::iterator ito;

  /*for( ito=machinedata->EpicsData().begin();
          ito!=machinedata->EpicsData().end();ito++)
  { 
    //    if((*ito).second != 0)
    std::cout << (*ito).first << " has value " << (*ito).second << std::endl;
    }*/

  if(_param._useREMI)
  {
    cass::REMI::REMIEvent *REMIdata = &cassevent->REMIEvent();
    T->SetBranchAddress("REMIEventBranch",&REMIdata);
#ifdef REMI_DEB
  std::cout << "remi test" << REMIdata->sampleInterval() << std::endl;
#endif
  }

  if(_param._useVMI)
  {
    //  std::cout<< " I am in1"<<std::endl;
    cass::VMI::VMIEvent *VMIdata = &cassevent->VMIEvent();
    T->SetBranchAddress("VMIEventBranch",&VMIdata);
    //  std::cout<< " I am in3"<<std::endl;
#ifdef VMI_DEB
    std::cout<< VMIdata->frame().size() << " a " <<
      VMIdata->cutFrame().size() << " b " <<
      VMIdata->coordinatesOfImpact().size() << std::endl;
#endif
  }

  if(_param._usepnCCD)
  {
    cass::pnCCD::pnCCDEvent *pnCCDdata = &cassevent->pnCCDEvent();
    T->SetBranchAddress("pnCCDEventBranch",&pnCCDdata);
    //  for (size_t ididid=0; ididid<pnCCDdata->detectors().size();++ididid)
    //      std::cout <<"Det:"<<ididid<<" size:"<< pnCCDdata->detectors()[ididid].correctedFrame().size()<<std::endl;
#ifdef every
    for (size_t isiz=0; isiz<pnCCDdata->detectors().size();++isiz)
    {
      cass::pnCCD::pnCCDDetector::frame_t &cf16u = pnCCDdata->detectors()[isiz].correctedFrame16u();
      cass::pnCCD::pnCCDDetector::frame_i32_t &cf32i = pnCCDdata->detectors()[isiz].correctedFrame32i();
      cass::pnCCD::pnCCDDetector::frame_f32_t &cf32f = pnCCDdata->detectors()[isiz].correctedFrame32f();

      cass::pnCCD::pnCCDDetector::frame_t::iterator itCorFrame16u = cf16u.begin();
      cass::pnCCD::pnCCDDetector::frame_i32_t::iterator itCorFrame32i = cf32i.begin();
      cass::pnCCD::pnCCDDetector::frame_f32_t::iterator itCorFrame32f = cf32f.begin();

      std::vector<uint16_t>::iterator itTemp16u = sum16u.begin();
      std::vector<int32_t>::iterator itTemp32i = sum32i.begin();
      std::vector<float>::iterator itTemp32f = sum32f.begin();

      clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
      for ( ; itCorFrame32i != cf32i.end(); ++itCorFrame32i,++itTemp32i )
	*itTemp32i+=*itCorFrame32i;
      clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &now);
      printf("summing_32i %lld ns\n", timeDiff(&now, &start));

      clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
      for ( ; itCorFrame16u != cf16u.end(); ++itCorFrame16u,++itTemp16u )
	*itTemp16u+=*itCorFrame16u;
      clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &now);
      printf("summing_16i %lld ns\n", timeDiff(&now, &start));

      clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &start);
      for ( ; itCorFrame32f != cf32f.end(); ++itCorFrame32f,++itTemp32f )
	*itTemp32f+=*itCorFrame32f;
      clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &now);
      printf("summing_32f %lld ns\n", timeDiff(&now, &start));

    }
#endif


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
  }

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

  if(_param._nofill==0)
  {
    T->Fill();
  //mapfile->Update(TObject obj = 0);
  //  if( ( Nevent%(20) )==0 ) mapfile->Update();
    if(_param._usejustFile==0)
    {
      if( (Nevent%_param._updatefrequency)==0 )
      {
        printf("Going to update the TMapFile, nofill is %i\n",_param._nofill);
        if(_param._nofill==0) mapfile->Update();
      }
    }
    else
    {
      printf("Going to save to file no need to update the TMapFile\n");
    }
  }
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
