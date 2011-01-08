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
      std::cout << "updateHist(): do something to "<<key<<std::endl;
      cass::HistogramBackend * casshist(_client(key));
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

