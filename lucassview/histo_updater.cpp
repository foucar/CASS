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

#include "histo_updater.h"

#include "tcpclient.h"

namespace lucassview
{
  /** create the list of updateable histograms from all available keys
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
    const TCPClient &_client;

    updateHist(const TCPClient &client)
      :_client(client)
    {}

    void operator() (const std::string& key)const
    {
      std::cout << "updateHist(): do something to "<<key<<std::endl;
    }
  };
}
using namespace lucassview;

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

