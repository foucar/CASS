/*
 *  Database.cpp
 *  diode
 *
 *  Created by Nicola Coppola & lutz foucar .
 *  
 */

#include "database.h"
#include "cass_event.h"
//ClassImp(cass::CASSEvent)

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

TTree *T = new TTree("T","circ buffer");
#include "cass_tree.h"
//ClassImp(thisCoordinate)

#include "histo_list.h"

// 1000 seconds at 30 Hz
#define max_events_in_Buffer 30000
//cass::CASSEvent *Theevent;
/*uint64_t od=0;
  cass::CASSEvent *Theevent = new cass::CASSEvent::CASSEvent(od);*/

//cass::CASSEvent::CASSEvent *Theevent = new cass::CASSEvent::CASSEvent(od);

cass::database::Database::Database()
{
  
  Double_t random;

  // this is where I am going to start the tree
  T->Branch("i",&i,"i/i");
  T->Branch("px",&px,"px/F");
  T->Branch("py",&py,"py/F");
  T->Branch("pz",&pz,"pz/F");
  T->Branch("random",&random,"random/D");
  /*if(!TClassTable::GetDict("Event")) {
    gSystem->Load("$ROOTSYS/test/libEvent.so");
    }*/
  /*Event *event = new Event();
    T->Branch("EventBranch","Event",&event,32000,99);*/
  //cass::CASSEvent *event = new cass::CASSEvent::CASSEvent(uint64_t);
  /*  T->Branch("TheCASSevent","cass::CASSEvent::CASSEvent(uint64_t od)",
      &Theevent,32000,99);*/
  // otherwise

  // the eventid
  T->Branch("CASS_id",&event_id,"CASS_id/l");

  //REMI
  T->Branch("REMI_nofChannels",&REMI_nofChannels,"REMI_nofChannels/i");
  T->Branch("REMI_Channel",REMI_Channel,"REMI_Channel[REMI_nofChannels]/i");
  T->Branch("REMI_Channel_nbrPeaks",REMI_Channel_nbrPeaks,"REMI_Channel_nbrPeaks[REMI_nofChannels]/i");
  /*T->Branch("REMI_Channel_Peak",REMI_Channel_Peak,
    "REMI_Channel_Peak[REMI_nofChannels][REMI_Channel_nbrPeaks[REMI_nofChannels]]/i");*/
  T->Branch("REMI_Channel_Peak_time",REMI_Channel_Peak_time,
     "REMI_Channel_Peak_time[REMI_nofChannels][REMI_Channel_nbrPeaks[REMI_nofChannels]]/i");
  T->Branch("REMI_Channel_Peak_com",REMI_Channel_Peak_com,
     "REMI_Channel_Peak_com[REMI_nofChannels][REMI_Channel_nbrPeaks[REMI_nofChannels]]/i");
  T->Branch("REMI_Channel_Peak_cfd",REMI_Channel_Peak_cfd,
     "REMI_Channel_Peak_cfd[REMI_nofChannels][REMI_Channel_nbrPeaks[REMI_nofChannels]]/i");
  T->Branch("REMI_Channel_Peak_integral",REMI_Channel_Peak_integral,
     "REMI_Channel_Peak_integral[REMI_nofChannels][REMI_Channel_nbrPeaks[REMI_nofChannels]]/i");
  T->Branch("REMI_Channel_Peak_height",REMI_Channel_Peak_height,
     "REMI_Channel_Peak_height[REMI_nofChannels][REMI_Channel_nbrPeaks[REMI_nofChannels]]/i");
  T->Branch("REMI_Channel_Peak_width",REMI_Channel_Peak_width,
     "REMI_Channel_Peak_width[REMI_nofChannels][REMI_Channel_nbrPeaks[REMI_nofChannels]]/i");
  T->Branch("REMI_Channel_Peak_fwhm",REMI_Channel_Peak_fwhm,
     "REMI_Channel_Peak_fwhm[REMI_nofChannels][REMI_Channel_nbrPeaks[REMI_nofChannels]]/i");
  T->Branch("REMI_Channel_Peak_startpos",REMI_Channel_Peak_startpos,
     "REMI_Channel_Peak_startpos[REMI_nofChannels][REMI_Channel_nbrPeaks[REMI_nofChannels]]/i");
  T->Branch("REMI_Channel_Peak_stoppos",REMI_Channel_Peak_stoppos,
     "REMI_Channel_Peak_stoppos[REMI_nofChannels][REMI_Channel_nbrPeaks[REMI_nofChannels]]/i");
  T->Branch("REMI_Channel_Peak_maxpos",REMI_Channel_Peak_maxpos,
     "REMI_Channel_Peak_maxpos[REMI_nofChannels][REMI_Channel_nbrPeaks[REMI_nofChannels]]/i");
  T->Branch("REMI_Channel_Peak_maximum",REMI_Channel_Peak_maximum,
     "REMI_Channel_Peak_maximum[REMI_nofChannels][REMI_Channel_nbrPeaks[REMI_nofChannels]]/i");
  T->Branch("REMI_Channel_Peak_polarity",REMI_Channel_Peak_polarity,
     "REMI_Channel_Peak_polarity[REMI_nofChannels][REMI_Channel_nbrPeaks[REMI_nofChannels]]/i");
  printf("should I add isUsed\n");

  T->Branch("REMI_nofDetectors",&REMI_nofDetectors,"REMI_nofDetectors/i");
  T->Branch("REMI_Detector",REMI_Detector,"REMI_Detector[REMI_nofDetectors]/i");
  T->Branch("REMI_Detector_nbrOfHits",REMI_Detector_nbrOfHits,"REMI_Detector_nbrOfHits[REMI_nofDetectors]/i");
  T->Branch("REMI_Detector_Hits_x",REMI_Detector_Hits_x,
     "REMI_Detector_Hits_x[REMI_nofDetectors][REMI_Detector_nbrOfHits[REMI_nofDetectors]]/i");
  T->Branch("REMI_Detector_Hits_y",REMI_Detector_Hits_y,
     "REMI_Detector_Hits_y[REMI_nofDetectors][REMI_Detector_nbrOfHits[REMI_nofDetectors]]/i");
  T->Branch("REMI_Detector_Hits_t",REMI_Detector_Hits_t,
     "REMI_Detector_Hits_t[REMI_nofDetectors][REMI_Detector_nbrOfHits[REMI_nofDetectors]]/i");

  T->Branch("REMI_horpos",REMI_horpos,"REMI_horpos[REMI_nofDetectors]/D");
  T->Branch("REMI_nbrBytes",REMI_nbrBytes,"REMI_nbrBytes[REMI_nofDetectors]/S");
  T->Branch("REMI_sampleInterval",REMI_sampleInterval,"REMI_sampleInterval[REMI_nofDetectors]/D");
  T->Branch("REMI_nbrSamples",REMI_nbrSamples,"REMI_nbrSamples[REMI_nofDetectors]/L");
  T->Branch("REMI_delayTime",REMI_delayTime,"REMI_delayTime[REMI_nofDetectors]/D");
  T->Branch("REMI_trigLevel",REMI_trigLevel,"REMI_trigLevel[REMI_nofDetectors]/D");
  T->Branch("REMI_trigSlope",REMI_trigSlope,"REMI_trigSlope[REMI_nofDetectors]/S");
  T->Branch("REMI_chanCombUsedChannels",REMI_chanCombUsedChannels,"REMI_chanCombUsedChannels[REMI_nofDetectors]/L");
  T->Branch("REMI_nbrConvPerChan",REMI_nbrConvPerChan,"REMI_nbrConvPerChan[REMI_nofDetectors]/S");

  //VMI Pulnix CCD
  T->Branch("VMI_integral",&VMI_integral,"VMI_integral/i");
  T->Branch("VMI_maxPixelValue",&VMI_maxPixelValue,"VMI_maxPixelValue/s");
  T->Branch("VMI_columns",&VMI_columns,"VMI_columns/s");
  T->Branch("VMI_rows",&VMI_rows,"VMI_rows/s");
  T->Branch("VMI_bitsPerPixel",&VMI_bitsPerPixel,"VMI_bitsPerPixel/s");
  T->Branch("VMI_offset",&VMI_offset,"VMI_offset/i");
  // the following are vectors..
  //T->Branch("VMI_frame",VMI_frame,"VMI_frame/s");
  //T->Branch("VMI_cutFrame",VMI_cutFrame,"VMI_cutFrame/s");
  //T->Branch("VMI_coordinatesOfImpact_x",VMI_coordinatesOfImpact_x,"VMI_coordinatesOfImpact_x/s");
  //T->Branch("VMI_coordinatesOfImpact_y",VMI_coordinatesOfImpact_y,"VMI_coordinatesOfImpact_y/s");

  //pnCCD (2)
  T->Branch("pnCCD_num_pixel_arrays",pnCCD_num_pixel_arrays,"pnCCD_num_pixel_arrays[2]/I");
  T->Branch("pnCCD_array_x_size",pnCCD_array_x_size,"pnCCD_array_x_size[2]/I");
  T->Branch("pnCCD_array_y_size",pnCCD_array_y_size,"pnCCD_array_y_size[2]/I");
  T->Branch("pnCCD_max_photons_per_event",pnCCD_max_photons_per_event,"pnCCD_max_photons_per_event[2]/I");
  T->Branch("pnCCD_array_x_size1",&pnCCD_array_x_size1,"pnCCD_array_x_size1/I");
  T->Branch("pnCCD_array_y_size1",&pnCCD_array_y_size1,"pnCCD_array_y_size1/I");
  T->Branch("pnCCD_raw",pnCCD_raw,"pnCCD_raw[pnCCD_array_x_size1][pnCCD_array_y_size1]/s");
  //T->Branch("pnCCD_raw",pnCCD_raw,"pnCCD_raw[pnCCD_array_x_size[0]][pnCCD_array_y_size[0]][2]");
  //T->Branch("pnCCD_corr",pnCCD_corr,"pnCCD_corr[pnCCD_array_x_size[0]][pnCCD_array_y_size[0]][2]");
  //T->Branch();
  //T->Branch();
  // others YAG XFEL intensities...

  T->SetCircular(max_events_in_Buffer);
  printf("Circular buffer allocated with %i events\n",max_events_in_Buffer);

  i=0;

  // I could also create some default histograms
  // like 1 for each pnCCD: last event, all events since beginning of time

}

cass::database::Database::~Database()
{
  // delete all histos
  delete h_pnCCD1_lastevent; h_pnCCD1_lastevent=0;

}

void cass::database::Database::add(cass::CASSEvent* cassevent)
{
  Double_t random;
  Int_t jj,kk;

  if(i==0) {
    /*T->SetCircular(max_events_in_Buffer);
    printf("Circular buffer allocated with %i events\n",max_events_in_Buffer);*/
    T->Print();
  }

  i++;

  if (i<xbins-1) 
  {
    xy[i][i+1]=2*i;
  }

  r.Rannor(px,py);
  pz=px*px+py*py;
  random=r.Rndm();

  event_id=cassevent->id();

  cass::REMI::REMIEvent &remievent = cassevent->REMIEvent();
  REMI_nofChannels=remievent.nbrOfChannels();
  for(jj=0;jj<REMI_nofChannels;jj++)
  {
    REMI_Channel_nbrPeaks[jj]=remievent.channel(jj).nbrPeaks();
    for(kk=0;kk<REMI_Channel_nbrPeaks[jj];kk++)
    {
      REMI_Channel_Peak_time[jj][kk]=remievent.channel(jj).peak(kk).time();
      REMI_Channel_Peak_com[jj][kk]=remievent.channel(jj).peak(kk).com();
      REMI_Channel_Peak_cfd[jj][kk]=remievent.channel(jj).peak(kk).cfd();
      REMI_Channel_Peak_integral[jj][kk]=remievent.channel(jj).peak(kk).integral();
      REMI_Channel_Peak_height[jj][kk]=remievent.channel(jj).peak(kk).height();
      REMI_Channel_Peak_width[jj][kk]=remievent.channel(jj).peak(kk).width();
      REMI_Channel_Peak_fwhm[jj][kk]=remievent.channel(jj).peak(kk).fwhm();
      REMI_Channel_Peak_startpos[jj][kk]=remievent.channel(jj).peak(kk).startpos();
      REMI_Channel_Peak_stoppos[jj][kk]=remievent.channel(jj).peak(kk).stoppos();
      REMI_Channel_Peak_maxpos[jj][kk]=remievent.channel(jj).peak(kk).maxpos();
      REMI_Channel_Peak_maximum[jj][kk]=remievent.channel(jj).peak(kk).maximum();
      REMI_Channel_Peak_polarity[jj][kk]=remievent.channel(jj).peak(kk).polarity();
    }
  }
  REMI_nofDetectors=remievent.nbrOfDetectors();
  for(jj=0;jj<REMI_nofDetectors;jj++)
  { 
    //    REMI_Detector[jj]=remievent.detector(jj);
    REMI_Detector_nbrOfHits[jj]=remievent.detector(jj).nbrOfHits();
    for(kk=0;kk<REMI_Detector_nbrOfHits[jj];kk++)
    {
      REMI_Detector_Hits_x[jj][kk]=remievent.detector(jj).hit(kk).x();
      REMI_Detector_Hits_y[jj][kk]=remievent.detector(jj).hit(kk).y();
      REMI_Detector_Hits_t[jj][kk]=remievent.detector(jj).hit(kk).t();
    }
    REMI_horpos[jj]=remievent.horpos();
    REMI_nbrBytes[jj]=remievent.nbrBytes();
    REMI_sampleInterval[jj]=remievent.sampleInterval();
    REMI_nbrSamples[jj]=remievent.nbrSamples();
    REMI_delayTime[jj]=remievent.delayTime();
    REMI_trigLevel[jj]=remievent.trigLevel();
    REMI_trigSlope[jj]=remievent.trigSlope();
    REMI_chanCombUsedChannels[jj]=remievent.chanCombUsedChannels();
    REMI_nbrConvPerChan[jj]=remievent.nbrConvPerChan();
  }

  cass::VMI::VMIEvent &vmievent = cassevent->VMIEvent();
  VMI_integral=vmievent.integral();
  VMI_maxPixelValue=vmievent.maxPixelValue();
  VMI_columns=vmievent.columns();
  VMI_rows=vmievent.rows();
  VMI_bitsPerPixel=vmievent.bitsPerPixel();
  VMI_offset=vmievent.offset();
  //printf("%i\n",&vmievent.frame());
  //std::vector<uint16_t> VMI_frame= new vmievent.frame();
  //VMI_cutFrame=vmievent.cutFrame();
  //VMI_coordinatesOfImpact[0]=vmievent.coordinatesOfImpact();

  cass::pnCCD::pnCCDEvent &pnccdevent = cassevent->pnCCDEvent();
//  printf("this %i %i %i\n", pnccdevent.num_pixel_arrays,pnccdevent.array_x_size,pnccdevent.array_y_size);
//  printf("thi0 %i %i\n", pnccdevent.max_photons_per_event,pnccdevent.raw_signal_values);
  /*pnCCD_num_pixel_arrays[0]=pnccdevent.num_pixel_arrays;
  pnCCD_array_x_size[0]=pnccdevent.array_x_size;
  pnCCD_array_y_size[0]=pnccdevent.array_y_size;
  pnCCD_max_photons_per_event[0]=pnccdevent.max_photons_per_event;
  pnCCD_array_x_size1=pnccdevent.array_x_size;
  pnCCD_array_y_size1=pnccdevent.array_y_size;
  pnCCD_raw=pnccdevent.raw_signal_values;*/

  //Theevent=cassevent;
  T->Fill();

  if(i>2 && (i%500)==0)
  {
    //T->Draw("px");
    T->Print();
    T->Show(i%max_events_in_Buffer-1);
    /*MyWidget->show();
      MyWidget->Refresh();*/
  }
  // maybe if i>max_events_in_Buffer i could save the events to file before
  // overwriting them....

  //now fill the history histograms,
  h_pnCCD1_history->Fill(float(i),float(i+1),int(xy));

  // we may have sets that overweight the last event
  // if "#events > 5" divide histo by 5 and add last event...

  // the "last event" ones need to be clear each time
  h_pnCCD1_lastevent->Reset();
  h_pnCCD1_lastevent->Fill(float(i),float(i+1),int(xy));
  if(i==xbins-1) {
    printf("saving\n");
    //save the histos instead of draw...
    //h_pnCCD1_lastevent->Draw("Text");
    //h_pnCCD1_history->Draw("Text");
    TFile f("histos.root","new");
    h_pnCCD1_lastevent->Write();
    h_pnCCD1_history->Write();
    T->Write();
    f.Close();
  }
  // I may have to save some histos to be able to reload them again...
  // maybe this need to be done by diode....

  // I need to delock??... But I did not lock
  //emit nextEvent();
}

cass::CASSEvent* cass::database::Database::nextEvent()
{
    return 0;
}
