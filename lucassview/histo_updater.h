//Copyright (C) 2011 Lutz Foucar

/**
 * @file histoupdater.h file contains the classes that update histograms
 *
 * @author Lutz Foucar
 */

#ifndef _HISTOUPDATER_H_
#define _HISTOUPDATER_H_

#include <memory>
#include <string>

#include <TTimer.h>

class HistogramUpdater
{
public:
  /** constructor
   *
   * connects the timers Timeout() signal with the updateHistogram() method
   * of this class
   *
   * @param server the server ip or name
   * @param port the server port
   */
  HistogramUpdater(const std::string &server, int port);

  /** update the shown histograms */
  void updateHistograms();

  /** set the server
   *
   * @param server the server ip or name
   */
  void setServer(const std::string & server) {_server = server;}

  /** set server port
   *
   * @param port port that the server listens to
   */
  void setPort(int port) {_port = port;}

  /** automaticly update
   *
   * start autoupdate with requested frequency. If frequency is 0 stop
   * autoupdate.
   *
   * @param freq The frequency in Hz
   */
  void autoUpdate(double freq);

private:
  /** the server */
  std::string _server;

  /** the server port */
  int _port;

  /** the timer for auto update */
  std::auto_ptr<TTimer> _timer;
};
#endif
