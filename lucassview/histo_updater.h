//Copyright (C) 2011 Lutz Foucar

/**
 * @file histoupdater.h file contains the classes that update histograms
 *
 * @author Lutz Foucar
 */

#include <string>

class HistogramUpdater
{
public:
  /** constructor */
  HistogramUpdater();

  /** update the shown histograms */
  void updateHistograms();

  /** set the server
   *
   * @param server the server ip or name
   */
  void setServer(const std::string & server);

  /** set server port
   *
   * @param port port that the server listens to
   */
  void setPort(int port);

private:
  /** the server */
  std::string _server;

  /** the server port */
  int port;
};
