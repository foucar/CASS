/*
 *  Database.cpp
 *  diode
 *
 *  Created by Nicola Coppola & lutz foucar .
 *  
 */

#include "database.h"
#include "cass_event.h"

#include <TRandom.h>
#include <TNtuple.h>
#include <TMath.h>

#include <TClassTable.h>
#include <TSystem.h>
//#include <TCollection.h>

// the following may be removed in the end...
#include <qapplication.h>

#include "TGraph.h"
#include "TQtWidget.h"
#include "TCanvas.h"
#include "TDatime.h"
#include "TAxis.h"
#include <QLabel>


// TQtWidget *MyWidget= new TQtWidget(0,"MyWidget");
 TTree *T = new TTree("T","test circ buffers");
 TRandom r;
 Float_t px,py,pz;
 Int_t i;
 
cass::database::Database::Database()
{
  
  Double_t random;

  // this is where I am going to start the tree
  T->Branch("px",&px,"px/F");
  T->Branch("py",&py,"py/F");
  T->Branch("pz",&pz,"pz/F");
  T->Branch("random",&random,"random/D");
  T->Branch("i",&i,"i/i");
  if(!TClassTable::GetDict("Event")) {
    gSystem->Load("$ROOTSYS/test/libEvent.so");
  }
  //  Event *event = new Event();
  //cass::CASSEvent *event = new Event();
  //T->Branch("thiscassevent","CASSEvent",&event);

  T->SetCircular(20000);

  /*int argc;
  char **argv;
  QApplication *app = new QApplication(argc, argv);
  app->connect(app,SIGNAL(lastWindowClosed ()),app,SLOT(quit()));

  MyWidget->resize(1000,1000);
  MyWidget->GetCanvas()->cd();*/
  i=0;
  //printf("init j=%i\n",i);
}

cass::database::Database::~Database()
{
}

void cass::database::Database::add(cass::CASSEvent* cassevent)
{
  Double_t random;

  //MyWidget->GetCanvas()->cd(1);
  i++;
    r.Rannor(px,py);
    pz=px*px+py*py;
    random=r.Rndm();
    T->Fill();
    if(i>2 && (i%500)==0)
    {
      //printf("event j=%i\n",i);
      //printf("j=%i\n",((i-19000)/5000)%12+1);
      //if(i<26000) T->Draw("px");
      //MyWidget->GetCanvas()->cd(((i-19000)/5000)%12+1);

      //T->Draw("px");
      T->Print();
      /*MyWidget->show();
	MyWidget->Refresh();*/
      /*if( ((i-19000)/5000+1)%4 == 3)
      {
        sleep(1);
	}*/
    }
    // maybe if i>20000 i could save the events to file before
    // overwriting them....
    if(i==1) {
      T->Print();
      //      gRoot->GetListOfClasses();
    }
}

cass::CASSEvent* cass::database::Database::nextEvent()
{
    return 0;
}
