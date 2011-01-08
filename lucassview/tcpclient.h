//Copyright (C) 2011 Lutz Foucar

/**
 * @file tcpclient.h file contains the classes connect to cass
 *
 * @author Lutz Foucar
 */

#include <string>
#include <list>

namespace cass
{
  class HistogramBackend;
}

namespace lucassview
{
  /** the tcp client that connects to the cass server
   *
   * @author Lutz Foucar
   */
  class TCPClient
  {
  public:
    /** constructor
     *
     * @param server string containing the server ip and port
     */
    TCPClient(const std::string &server);

    /** retrieve the list of available histograms */
    std::list<std::string> operator() ()const;

    /** retrieve a specific histogram
     *
     * @return Histogram for requested key
     * @param histogramkey the key of the requested histogram
     */
    cass::HistogramBackend *operator() (const std::string &histogramkey)const;

  private:
    /** the server to connect to */
    std::string _server;
  };
}
