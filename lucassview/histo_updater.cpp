//Copyright (C) 2011 Lutz Foucar

/**
 * @file histo_updater.cpp file contains the classes that update histograms
 *
 * @author Lutz Foucar
 */

#include <cmath>
#include <limits>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <algorithm>
#include <list>

#include <TObject.h>
#include <TH1.h>
#include <TH2.h>
#include <TDirectory.h>
#include <TROOT.h>
#include <TList.h>
#include <TSeqCollection.h>
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
  /** update the contents of the canvases
   *
   * recursively iterate through lists if object is a pad, update it and
   * iterate through its primitives.
   *
   * @param list the list of TObjects to iterate through
   *
   * @author Lutz Foucar
   */
  void updateCanvases(TSeqCollection* list)
  {
    TIter next(list);
    TObject *obj(0);
    while ((obj = next()))
    {
//      cout << "Obj: "<< obj->GetName() <<" " << obj->InheritsFrom("TPad")<<endl;
      if (obj->InheritsFrom("TPad"))
      {
        static_cast<TPad*>(obj)->Modified();
        static_cast<TPad*>(obj)->Update();
        updateCanvases(static_cast<TPad*>(obj)->GetListOfPrimitives());
      }
    }
  }

  /** delete all root histograms that are not on the list
   *
   * go through the list of objects in memory and erase it if it not on the list
   * @author Lutz Foucar
   */
  struct deleteObsoleteHistogram
  {
    /** constructor
     *
     * @param allkeys all available keys on the server
     */
    deleteObsoleteHistogram(const list<string> &allkeys)
      :_allkeys(allkeys)
    {}

    /** the operator
     *
     * if the object is a TH1 and its name is not on the allkeys list, check if
     * the title is the same as the name of the object. If so it is most likely
     * a cass histogram, that is not on the list anymore, so delete it.
     *
     * @param obj The object that potentially will be deleted
     */
    void operator()(TObject *obj) const
    {
      if (obj->InheritsFrom("TH1"))
      {
        if (find(_allkeys.begin(),_allkeys.end(),obj->GetName()) == _allkeys.end())
        {
          if (string(obj->GetName()) == string(obj->GetTitle()))
          {
            cout<<"deleteObsoleteHistogram(): delete '"<< obj->GetName()<<"' it is not on the casshistogram list"<<endl;
            obj->Delete();
          }
        }
      }
    }

    /** the list with all cass histogram keys */
    const list<string> &_allkeys;
  };

  /** iteratively go through canvas list and find histgrams
   *
   * when a histogram is found add it to the updateList, if it is another pad,
   * iterate through its primatives
   *
   * @param list The list of TObjects to iterate through
   * @param updateList The list containg the names of the found histgrams
   *
   * @author Lutz Foucar
   */
  void iterateListAndAddDisplayedHistograms(TSeqCollection* list, std::list<string>& updateList)
  {
    TIter next(list);
    TObject *obj(0);
    while ((obj = next()))
    {
//      cout << "Obj: "<< obj->GetName() <<" " << obj->InheritsFrom("TPad")<<endl;
      if (obj->InheritsFrom("TPad"))
        iterateListAndAddDisplayedHistograms(static_cast<TCanvas*>(obj)->GetListOfPrimitives(),
                                             updateList);
      else if(obj->InheritsFrom("TH1"))
      {
        updateList.push_back(obj->GetName());
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
    iterateListAndAddDisplayedHistograms(gROOT->GetListOfCanvases(),updateList);
//    TIter next(gROOT->GetListOfCanvases());
//    TObject * canv(0);
//    while ((canv = next()))
//    {
//      cout << "Canv: "<< canv->GetName() <<" " << canv->InheritsFrom("TPad")<<endl;
//      TIter next2(static_cast<TCanvas*>(canv)->GetListOfPrimitives());
//      TObject * pad(0);
//      while ((pad = next2()))
//      {
//        cout << " Pad: "<< pad->GetName()<<" " << pad->InheritsFrom("TPad") <<endl;
//        TIter next3(static_cast<TPad*>(pad)->GetListOfPrimitives());
//        TObject * hist(0);
//        while ((hist = next3()))
//        {
//          if (hist->InheritsFrom("TH1"))
//            updateList.push_back(hist->GetName());
//        }
//      }
//    }
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
     * @param filename filename of the file to write the histograms to
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
    return (static_cast<int>(ca.nbrBins()) == ra.GetNbins() &&
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
              {
                const double histvalue(casshist->memory()[iX + iY*xaxis.nbrBins()]);
                const float value = (isnan(histvalue)) ?
                              0. : casshist->memory()[iX + iY*xaxis.nbrBins()];
                roothist->SetBinContent(roothist->GetBin(iX+1,iY+1),value);
              }
            /** copy over / underflow */
            size_t OverUnderFlowStart (xaxis.nbrBins()*yaxis.nbrBins());
            roothist->SetBinContent(roothist->GetBin(0,0),casshist->memory()[OverUnderFlowStart+HistogramBackend::LowerLeft]);
            roothist->SetBinContent(roothist->GetBin(xaxis.nbrBins()+1,0),casshist->memory()[OverUnderFlowStart+HistogramBackend::LowerRight]);
            roothist->SetBinContent(roothist->GetBin(xaxis.nbrBins()+1,yaxis.nbrBins()+1),casshist->memory()[OverUnderFlowStart+HistogramBackend::UpperRight]);
            roothist->SetBinContent(roothist->GetBin(0,yaxis.nbrBins()+1),casshist->memory()[OverUnderFlowStart+HistogramBackend::UpperLeft]);
            roothist->SetBinContent(roothist->GetBin(1,0),casshist->memory()[OverUnderFlowStart+HistogramBackend::LowerMiddle]);
            roothist->SetBinContent(roothist->GetBin(1,yaxis.nbrBins()+1),casshist->memory()[OverUnderFlowStart+HistogramBackend::UpperMiddle]);
            roothist->SetBinContent(roothist->GetBin(xaxis.nbrBins()+1,1),casshist->memory()[OverUnderFlowStart+HistogramBackend::Right]);
            roothist->SetBinContent(roothist->GetBin(0,1),casshist->memory()[OverUnderFlowStart+HistogramBackend::Left]);
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
            {
              const double histvalue(casshist->memory()[iX]);
              const float value = (isnan(histvalue)) ? 0. : casshist->memory()[iX];
              roothist->SetBinContent(roothist->GetBin(iX+1),value);
            }
            /** copy over / underflow */
            size_t OverUnderFlowStart (xaxis.nbrBins());
            roothist->SetBinContent(roothist->GetBin(0),casshist->memory()[OverUnderFlowStart+HistogramBackend::Underflow]);
            roothist->SetBinContent(roothist->GetBin(xaxis.nbrBins()+1),casshist->memory()[OverUnderFlowStart+HistogramBackend::Overflow]);
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
              roothist = new TH1F(key.c_str(),key.c_str(),1,0,1);
            }
            const double histvalue(casshist->memory()[0]);
            const float value = (isnan(histvalue)) ? 0. : casshist->memory()[0];
            roothist->SetBinContent(1,value);
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
//    TIter it(gDirectory->GetList());
//    for_each(it.Begin(),TIter::End(),deleteObsoleteHistogram(allkeylist));
    TIter next(gDirectory->GetList());
    const deleteObsoleteHistogram removeUnusedHist(allkeylist);
    TObject *obj(0);
    while ((obj = next()))
      removeUnusedHist(obj);
    if (_updateCanv)
      updateCanvases(gROOT->GetListOfCanvases());
  }
  catch (const runtime_error &error)
  {
    cout << "HistogramUpdater::updateHistograms(): "<<error.what()<<endl;
  }
}

void HistogramUpdater::syncHistogram(const string &name)
{
  try
  {
    stringstream serveradress;
    serveradress << _server << ":" << _port;
    TCPClient client (serveradress.str());
    updateHist updater = updateHist(client);
    updater(name);
  }
  catch (const runtime_error &error)
  {
    cout << "HistogramUpdater::updateHistogram(): "<<error.what()<<endl;
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
//    TIter it(gDirectory->GetList());
//    for_each(it.Begin(),TIter::End(),writeObject(name));
    TIter next(gDirectory->GetList());
    writeObject writeObj(name);
    TObject *obj(0);
    while ((obj = next()))
      writeObj(obj);
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

void HistogramUpdater::controlDarkcal(const std::string& command)
{
  try
  {
    stringstream serveradress;
    serveradress << _server << ":" << _port;
    TCPClient client (serveradress.str());
    client.controlDarkcal(command);
  }
  catch (const runtime_error &error)
  {
    cout << "HistogramUpdater::controlDarkcal(): "<<error.what()<<endl;
  }
}
