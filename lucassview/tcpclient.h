//Copyright (C) 2011 Lutz Foucar

/**
 * @file lucassview/tcpclient.h file contains the classes connect to cass
 *
 * @author Lutz Foucar
 */
#ifndef _TCPCLIENT_H
#define _TCPCLIENT_H

#include <string>
#include <list>
#include <tr1/memory>

#include "result.hpp"

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
    cass::Result<float>::shared_pointer operator() (const std::string &histogramkey)const;

    /** reload .ini file */
    void reloadIni() const;

    /** reload .ini file */
    void controlCalibration(const std::string& command) const;

    /** retrieve the transferred bytes */
    size_t receivedBytes()const {return _transferredBytes;}

  private:
    /** the server to connect to */
    std::string _server;

    /** the amount of bytes transferred */
    mutable size_t _transferredBytes;
  };
}
#endif
