//Copyright (C) 2011 Lutz Foucar

/**
 * @file histoupdater.cpp file contains the classes that update histograms
 *
 * @author Lutz Foucar
 */

#include <cmath>
#include <limits>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <algorithm>

#include <TObject.h>
#include <TH1.h>
#include <TH2.h>
#include <TDirectory.h>
#include <TROOT.h>
#include <TList.h>
#include <TPad.h>
#include <TCanvas.h>
#include <TCollection.h>
#include <TFile.h>

#include "histo_updater.h"

#include "tcpclient.h"
#include "histogram.h"

using namespace lucassview;
using namespace std;

HistogramUpdater *gCASSClient(0);

namespace lucassview
{
  /**  update all pads in canvases
   *
   * iterate through all canvases and pads and tell them to update
   *
   * @author Lutz Foucar
   */
  void updateCanvases()
  {
    TIter next(gROOT->GetListOfCanvases());
    TObject * canv(0);
    while ((canv = next()))
    {
      TIter next2(static_cast<TCanvas*>(canv)->GetListOfPrimitives());
      TObject * pad(0);
      while ((pad = next2()))
      {
        static_cast<TPad*>(pad)->Modified();
        static_cast<TPad*>(pad)->Update();
      }
    }
  }

  /** create the list of updateable histograms from all available keys
   *
   * First add all cass histograms that are not present as root histograms.
   * Then go through all canvases and add the histograms that are shown in them
   * and add them to the list returned. For this one must go through all
   * canvases and the pads within the canvases. The pads contain the histograms.
   *
   * @return list of keys that need to be updated
   * @param allkeys all available keys on the server
   *
   * @author Lutz Foucar
   */
  list<string> checkList(const list<string> &allkeys)
  {
    list<string> updateList;
    for (list<string>::const_iterator it(allkeys.begin()); it!=allkeys.end(); ++ it)
      if (!gDirectory->FindObjectAny((*it).c_str()))
        updateList.push_back((*it));
    TIter next(gROOT->GetListOfCanvases());
    TObject * canv(0);
    while ((canv = next()))
    {
//      std::cout << "Canv: "<< canv->GetName() <<endl;
      TIter next2(static_cast<TCanvas*>(canv)->GetListOfPrimitives());
      TObject * pad(0);
      while ((pad = next2()))
      {
//        std::cout << " Pad: "<< pad->GetName() <<endl;
        TIter next3(static_cast<TPad*>(pad)->GetListOfPrimitives());
        TObject * hist(0);
        while ((hist = next3()))
        {
          if (hist->InheritsFrom("TH1"))
            updateList.push_back(hist->GetName());
        }
      }
    }
    return updateList;
  }

  /** write the object to file
   *
   * @author Lutz Foucar
   */
  struct writeObject
  {
    /** the file to write the object to */
    TFile * _file;

    /** constructor
     *
     * set the file
     *
     * @param file the file to write the histograms to
     */
    writeObject(const std::string & filename)
      :_file(TFile::Open(filename.c_str(),"RECREATE"))
    {}

    /** destructor
     *
     * saves and closes the file
     */
    ~writeObject()
    {
      _file->SaveSelf();
      _file->Close();
    }

    /** the operator
     *
     * check if the object is a histogram, if so write it to the file
     *
     * @param obj The object that potentially gets written to file
     */
    void operator()(TObject *obj)
    {
      if (obj->InheritsFrom("TH1"))
      {
        cout<<"writeObject(): writing '"<< obj->GetName()<<"' to '"<<_file->GetName()<<"'"<<endl;
        _file->cd();
        obj->Write(0,TObject::kOverwrite);
      }
    }
  };

  /** comapre axis for equalitiy
   *
   * check whether the cass histogram axis and the root histogram axis are the
   * same. Test for number of bins, low and high ends and the title of the axis.
   *
   * @return true when both axis are the same
   * @param ca the axis of the cass histogram
   * @param ra the axis of the root histogram
   *
   * @author Lutz Foucar
   */
  bool operator== (const cass::AxisProperty &ca, const TAxis &ra)
  {
    return (ca.nbrBins() == ra.GetNbins() &&
            fabs(ca.lowerLimit() - ra.GetXmin()) <  sqrt(numeric_limits<double>::epsilon()) &&
            fabs(ca.upperLimit() - ra.GetXmax()) <  sqrt(numeric_limits<double>::epsilon()) &&
            ca.title() == ra.GetTitle());
  }

  /** comapre axis for inequality
   *
   * @return the inverse of opertator==
   * @param ca the axis of the cass histogram
   * @param ra the axis of the root histogram
   *
   * @author Lutz Foucar
   */
  bool operator!= (const cass::AxisProperty &ca, const TAxis &ra)
  {
    return !(ca==ra);
  }

  /** create the list of updateable histograms from all available keys
   *
   * @author Lutz Foucar
   */
  struct updateHist
  {
    /** the client for the server */
    const TCPClient &_client;

    /** constructor
     *
     * @param client the client to connecto to the server
     */
    updateHist(const TCPClient &client)
      :_client(client)
    {}

    /** update the histogram with key
     *
     * retrieve the cass histogram for key. Then try to get the root histogram
     * that has the name "key". Find out what dimension the cass histogram has.
     * Depending on that one treats the histograms differently. In case of the
     * 2D histogram one checks whether the root histogram already exists (the
     * returned pointer is non Zero). If it does exist, check whether the axis
     * properties are the same. When they differ then set the root axis
     * properties to be the same as the properties that the cass histogram has.
     * When the root histogram does not exist, create it from the cass histogram
     * properties.\n
     * Now copy the histogram contents from the cass histogram to the root
     * histogram. After that copy the over and underflow quadrants and the
     * number of fills to the root histogram.
     *
     * In case the histogram is a 1D histogram make the same procedure as you
     * did with the 2D histogram, just only for the x-axis. When then histogram
     * is a 0D histogram or has a different dimension ignore it.
     *
     * @param key The key that the histogram has on the server
     */
    void operator() (const string& key)const
    {
      using namespace cass;
      using namespace tr1;
      try
      {
//        cout << "updateHist(): copy information of "<<key<<endl;
        shared_ptr<HistogramFloatBase> casshist(_client(key));
        TH1 * roothist(dynamic_cast<TH1*>(gDirectory->FindObjectAny(key.c_str())));
        switch (casshist->dimension())
        {
        case 2:
          {
            const AxisProperty &xaxis(casshist->axis()[HistogramBackend::xAxis]);
            const AxisProperty &yaxis(casshist->axis()[HistogramBackend::yAxis]);
            if(!roothist)
            {
              cout << "updateHist(): '"<<key<<"' create roothist with X("<<xaxis.title()<<") '"<<xaxis.nbrBins()<<" "<<xaxis.lowerLimit()<<" "<<xaxis.upperLimit()<<"' "
                  "Y("<<yaxis.title()<<") '"<<yaxis.nbrBins()<<" "<<yaxis.lowerLimit()<<" "<<yaxis.upperLimit()<<"'"<<endl;
              roothist = new TH2F(key.c_str(),key.c_str(),
                                  xaxis.nbrBins(), xaxis.lowerLimit(), xaxis.upperLimit(),
                                  yaxis.nbrBins(), yaxis.lowerLimit(), yaxis.upperLimit());
              roothist->SetOption("colz");
            }
            const TAxis &rxaxis(*roothist->GetXaxis());
            const TAxis &ryaxis(*roothist->GetYaxis());
            if(xaxis != rxaxis || yaxis != ryaxis)
            {
              cout << "updateHist(): '"<<key<<"' resize because axis differs: roothist: "<<xaxis.nbrBins()<<"<->"<<rxaxis.GetNbins()<<" "<<xaxis.lowerLimit()<<"<->"<<rxaxis.GetXmin()<<" "<<xaxis.upperLimit()<<"<->"<<rxaxis.GetXmax()<<" "
                  <<yaxis.nbrBins()<<"<->"<<ryaxis.GetNbins()<<" "<<yaxis.lowerLimit()<<"<->"<<ryaxis.GetXmin()<<" "<<yaxis.upperLimit()<<"<->"<<ryaxis.GetXmax()<<endl;
              roothist->SetBins(xaxis.nbrBins(), xaxis.lowerLimit(), xaxis.upperLimit(),
                                yaxis.nbrBins(), yaxis.lowerLimit(), yaxis.upperLimit());
              roothist->SetXTitle(xaxis.title().c_str());
              roothist->GetXaxis()->CenterTitle(true);
              roothist->SetYTitle(yaxis.title().c_str());
              roothist->GetYaxis()->CenterTitle(true);
              roothist->GetYaxis()->SetTitleOffset(1.5);
            }
            /** copy histogram contents */
            for (size_t iY(0); iY<yaxis.nbrBins();++iY)
              for (size_t iX(0); iX<xaxis.nbrBins();++iX)
                roothist->SetBinContent(roothist->GetBin(iX+1,iY+1),casshist->memory()[iX + iY*xaxis.nbrBins()]);
            /** copy over / underflow */
            roothist->SetBinContent(roothist->GetBin(0,0),casshist->memory()[HistogramBackend::LowerLeft]);
            roothist->SetBinContent(roothist->GetBin(xaxis.nbrBins()+1,0),casshist->memory()[HistogramBackend::LowerRight]);
            roothist->SetBinContent(roothist->GetBin(xaxis.nbrBins()+1,yaxis.nbrBins()+1),casshist->memory()[HistogramBackend::UpperRight]);
            roothist->SetBinContent(roothist->GetBin(0,yaxis.nbrBins()+1),casshist->memory()[HistogramBackend::UpperLeft]);
            roothist->SetBinContent(roothist->GetBin(1,0),casshist->memory()[HistogramBackend::LowerMiddle]);
            roothist->SetBinContent(roothist->GetBin(1,yaxis.nbrBins()+1),casshist->memory()[HistogramBackend::UpperMiddle]);
            roothist->SetBinContent(roothist->GetBin(xaxis.nbrBins()+1,1),casshist->memory()[HistogramBackend::Right]);
            roothist->SetBinContent(roothist->GetBin(0,1),casshist->memory()[HistogramBackend::Left]);
            /** copy number of fills (how many shots have been accumulated) */
            roothist->SetEntries(casshist->nbrOfFills());
          }
          break;
        case 1:
          {
            const AxisProperty &xaxis(casshist->axis()[HistogramBackend::xAxis]);
            if(!roothist)
            {
              cout << "updateHist(): '"<<key<<"' create roothist with X("<<xaxis.title()<<"):'"<<xaxis.nbrBins()<<" "<<xaxis.lowerLimit()<<" "<<xaxis.upperLimit()<<"'"<<endl;
              roothist = new TH1F(key.c_str(),key.c_str(),
                                  xaxis.nbrBins(), xaxis.lowerLimit(), xaxis.upperLimit());
              roothist->GetXaxis()->CenterTitle(true);
            }
            const TAxis &rxaxis(*roothist->GetXaxis());
            if(xaxis != rxaxis)
            {
              cout << "updateHist(): '"<<key<<"' resize because axis differs: roothist: "<<xaxis.nbrBins()<<"<->"<<rxaxis.GetNbins()<<" "<<xaxis.lowerLimit()<<"<->"<<rxaxis.GetXmin()<<" "<<xaxis.upperLimit()<<"<->"<<rxaxis.GetXmax()<<endl;
              roothist->SetBins(xaxis.nbrBins(), xaxis.lowerLimit(), xaxis.upperLimit());
              roothist->SetXTitle(xaxis.title().c_str());
            }
            /** copy histogram contents */
            for (size_t iX(0); iX<xaxis.nbrBins();++iX)
              roothist->SetBinContent(roothist->GetBin(iX+1),casshist->memory()[iX]);
            /** copy over / underflow */
            roothist->SetBinContent(roothist->GetBin(0),casshist->memory()[HistogramBackend::Underflow]);
            roothist->SetBinContent(roothist->GetBin(xaxis.nbrBins()+1),casshist->memory()[HistogramBackend::Overflow]);
            /** copy number of fills (how many shots have been accumulated) */
            roothist->SetEntries(casshist->nbrOfFills());
//            cout <<key<<" underflow '"<< casshist->memory()[HistogramBackend::Underflow] <<"' overflow '"<< casshist->memory()[HistogramBackend::Overflow] <<"'"<<endl;
          }
          break;
        case 0:
          {
            if(!roothist)
            {
              cout << "updateHist(): '"<<key<<"' create roothist with for 0D histogram"<<endl;
              roothist = new TH1F(key.c_str(),key.c_str(),
                                  1,0,1);
            }
            roothist->SetBinContent(1,casshist->memory()[0]);
            roothist->SetEntries(casshist->nbrOfFills());
          }
          break;
        default:
          break;
        }
//        cout << "updateHist(): done with "<<key<<endl;
      }
      catch (const runtime_error& error)
      {
        cout << "updateHist(): "<< error.what()<<endl;
      }
    }
  };
}

HistogramUpdater::HistogramUpdater(const string &server, int port)
  :_server(server),
   _port(port),
   _timer(new TTimer()),
   _updateCanv(false)
{
  _timer->Connect("Timeout()", "HistogramUpdater",this, "syncHistograms()");
}

void HistogramUpdater::autoSync(double freq)
{
  if(freq < sqrt(numeric_limits<double>::epsilon()))
    _timer->Stop();
  else
  {
    _timer->Start(static_cast<int>(1.e3/freq));
  }
}

void HistogramUpdater::syncHistograms()
{
  try
  {
    stringstream serveradress;
    serveradress << _server << ":" << _port;
    TCPClient client (serveradress.str());
    list<string> allkeylist(client());
    list<string> updatableHistsList(checkList(allkeylist));
    for_each(updatableHistsList.begin(),updatableHistsList.end(), updateHist(client));
    if (_updateCanv)
      updateCanvases();
  }
  catch (const runtime_error &error)
  {
    cout << "HistogramUpdater::updateHistograms(): "<<error.what()<<endl;
  }
}

void HistogramUpdater::writeRootFile(const std::string& name)
{
  try
  {
    stringstream serveradress;
    serveradress << _server << ":" << _port;
    TCPClient client (serveradress.str());
    list<string> allkeylist(client());
    for_each(allkeylist.begin(),allkeylist.end(), updateHist(client));
    TIter it(gDirectory->GetList());
    for_each(it.Begin(),TIter::End(),writeObject(name));
    gROOT->cd();
  }
  catch (const runtime_error &error)
  {
    cout << "HistogramUpdater::writeRootFile(): "<<error.what()<<endl;
  }
}

void HistogramUpdater::reloadIni()
{
  try
  {
    stringstream serveradress;
    serveradress << _server << ":" << _port;
    TCPClient client (serveradress.str());
    client.reloadIni();
  }
  catch (const runtime_error &error)
  {
    cout << "HistogramUpdater::reloadIni(): "<<error.what()<<endl;
  }
}
