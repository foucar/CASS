// Copyright (C) 2009 Nicola Coppola

//#include <sstream>
//#include <stdlib.h>
//#include <TCanvas.h>
//#include <TROOT.h>
//#include <TFile.h>
#include <TRandom.h>
#include <TNtuple.h>
#include <TMath.h>

#include <qapplication.h>

#include "TGraph.h"
#include "TQtWidget.h"
#include "TCanvas.h"
#include "TDatime.h"
#include "TAxis.h"
#include <QLabel>

int main_root( int argc, char **argv )
{
  QApplication *app = new QApplication(argc, argv);
  app->connect(app,SIGNAL(lastWindowClosed ()),app,SLOT(quit()));

  TQtWidget *MyWidget= new TQtWidget(0,"MyWidget");
  //MyWidget->resize(600,800);
  MyWidget->resize(1000,1000);
  MyWidget->GetCanvas()->cd();

  TTree *T = new TTree("T","test circ buffers");
  TRandom r;
  Float_t px,py,pz;
  Double_t random;
  //UShort_t i;
  Int_t i;
  T->Branch("px",&px,"px/F");
  T->Branch("py",&py,"py/F");
  T->Branch("pz",&pz,"pz/F");
  T->Branch("random",&random,"random/D");
  T->Branch("i",&i,"i/i");
  T->SetCircular(20000);


  //float x[] = {1,2,3,4,5};
  //float y[] = {1.5f, 3.0f, 4.5f, 3.8f,5.2f};
  //TGraph*  m_graph  = new TGraph(sizeof(x)/sizeof(float),x,y);
  MyWidget->GetCanvas()->Divide(3,4);
  MyWidget->GetCanvas()->cd(1);
  //m_graph->Draw("AC*");

  for(i=0;i<125000; i++)
  {
    r.Rannor(px,py);
    pz=px*px+py*py;
    random=r.Rndm();
    T->Fill();
    if(i>20000 && (i%5000)==0)
    {
      printf("j=%i\n",((i-19000)/5000)%12+1);
      //sleep(10);
      
      if(i<26000) T->Draw("px");
      //      MyWidget->GetCanvas()->cd((i-19000)/5000+1);
      
      MyWidget->GetCanvas()->cd(((i-19000)/5000)%12+1);
      T->Draw("px");
      //MyWidget->GetCanvas()->cd(1);
      MyWidget->show();
      //app->exec();
      MyWidget->Refresh();
      if( ((i-19000)/5000+1)%4 == 3)
      {
        //app->exec();
        sleep(1);
      }
    }
  }

  //app->exec();

  return 0;
}
