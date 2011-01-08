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
#include <TH1.h>
#include <TH2.h>
#include <TDirectory.h>

#include "histo_updater.h"

#include "tcpclient.h"
#include "histogram.h"

using namespace lucassview;

namespace lucassview
{
  /** create the list of updateable histograms from all available keys
   *
   * @return list of keys that need to be updated
   * @param allkeys all available keys on the server
   *
   * @author Lutz Foucar
   */
  std::list<std::string> checkList(const std::list<std::string> &allkeys)
  {
    using namespace std;
    list<string> updateList(allkeys);
    cout << "checkList(): create the list of histograms that need to be updated"<<endl;
    return updateList;
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
     * if the histogram is not yet on the list of histograms, create it first
     *
     * @param key The key that the histogram has on the server
     */
    void operator() (const std::string& key)const
    {
      using namespace cass;
      std::cout << "updateHist(): copy information of "<<key<<std::endl;
      HistogramFloatBase * casshist(dynamic_cast<HistogramFloatBase*>(_client(key)));
      TH1 * roothist(dynamic_cast<TH1*>(gDirectory->FindObjectAny(key.c_str())));
      switch (casshist->dimension())
      {
      case 2:
        {
          const AxisProperty &xaxis(casshist->axis()[HistogramBackend::xAxis]);
          const AxisProperty &yaxis(casshist->axis()[HistogramBackend::yAxis]);
          if(roothist)
          {
            const TAxis & rxaxis (*roothist->GetXaxis());
            const TAxis & ryaxis (*roothist->GetYaxis());
            if(xaxis.nbrBins() != rxaxis.GetNbins() &&
               xaxis.lowerLimit() != rxaxis.GetXmin() &&
               xaxis.upperLimit() != rxaxis.GetXmax() &&
               yaxis.nbrBins() != ryaxis.GetNbins() &&
               yaxis.lowerLimit() != ryaxis.GetXmin() &&
               yaxis.upperLimit() != ryaxis.GetXmax())
            {
              roothist->SetBins(xaxis.nbrBins(), xaxis.lowerLimit(), xaxis.upperLimit(),
                                yaxis.nbrBins(), yaxis.lowerLimit(), yaxis.upperLimit());
            }
          }
          else
          {
            roothist = new TH2F(key.c_str(),key.c_str(),
                                xaxis.nbrBins(), xaxis.lowerLimit(), xaxis.upperLimit(),
                                yaxis.nbrBins(), yaxis.lowerLimit(), yaxis.upperLimit());
          }
          for (size_t iY(0); iY<xaxis.nbrBins();++iY)
            for (size_t iX(0); iX<yaxis.nbrBins();++iX)
              roothist->SetBinContent(roothist->GetBin(iX+1,iY+1),casshist->memory()[iX + iY*xaxis.nbrBins()]);
          roothist->SetBinContent(roothist->GetBin(0,0),casshist->memory()[HistogramBackend::LowerLeft]);
          roothist->SetBinContent(roothist->GetBin(xaxis.nbrBins()+1,0),casshist->memory()[HistogramBackend::LowerRight]);
          roothist->SetBinContent(roothist->GetBin(xaxis.nbrBins()+1,yaxis.nbrBins()+1),casshist->memory()[HistogramBackend::UpperRight]);
          roothist->SetBinContent(roothist->GetBin(0,yaxis.nbrBins()+1),casshist->memory()[HistogramBackend::UpperLeft]);
          roothist->SetBinContent(roothist->GetBin(1,0),casshist->memory()[HistogramBackend::LowerMiddle]);
          roothist->SetBinContent(roothist->GetBin(1,yaxis.nbrBins()+1),casshist->memory()[HistogramBackend::UpperMiddle]);
          roothist->SetBinContent(roothist->GetBin(xaxis.nbrBins()+1,1),casshist->memory()[HistogramBackend::Right]);
          roothist->SetBinContent(roothist->GetBin(0,1),casshist->memory()[HistogramBackend::Left]);
        }
        break;
      case 1:
        {
          const AxisProperty &xaxis(casshist->axis()[HistogramBackend::xAxis]);
          if(roothist)
          {
            const TAxis & rxaxis (*roothist->GetXaxis());
            if(xaxis.nbrBins() != rxaxis.GetNbins() &&
               xaxis.lowerLimit() != rxaxis.GetXmin() &&
               xaxis.upperLimit() != rxaxis.GetXmax())
            {
              roothist->SetBins(xaxis.nbrBins(), xaxis.lowerLimit(), xaxis.upperLimit());
            }
          }
          else
          {
            roothist = new TH1F(key.c_str(),key.c_str(),
                                xaxis.nbrBins(), xaxis.lowerLimit(), xaxis.upperLimit());
          }
          for (size_t iX(0); iX<casshist->axis()[HistogramBackend::xAxis].nbrBins();++iX)
            roothist->SetBinContent(roothist->GetBin(iX+1),casshist->memory()[iX]);
          roothist->SetBinContent(roothist->GetBin(0),casshist->memory()[HistogramBackend::Underflow]);
          roothist->SetBinContent(roothist->GetBin(xaxis.nbrBins()+1),casshist->memory()[HistogramBackend::Overflow]);
        }
        break;
      case 0:
      default:
        break;
      }
      std::cout << "updateHist(): done with "<<key<<std::endl;
    }
  };
}

HistogramUpdater::HistogramUpdater(const std::string &server, int port)
  :_server(server),
   _port(port),
   _timer(new TTimer())
{
  _timer->Connect("Timeout()", "HistogramUpdater",this, "updateHistograms()");
}

void HistogramUpdater::autoUpdate(double freq)
{
  using namespace std;
  if(freq < sqrt(numeric_limits<double>::epsilon()))
    _timer->Stop();
  else
  {
    _timer->Start(static_cast<int>(1.e3/freq));
  }
}

void HistogramUpdater::updateHistograms()
{
  using namespace std;
  try
  {
    stringstream serveradress;
    serveradress << _server << ":" << _port;
    TCPClient client (serveradress.str());
    list<string> allkeylist(client());
    list<string> updatableHistsList(checkList(allkeylist));
    for_each(updatableHistsList.begin(),updatableHistsList.end(), updateHist(client));
  }
  catch (const runtime_error &error)
  {
    cout << error.what()<<endl;
  }
}

