//Copyright (C) 2011, 2013 Lutz Foucar

/**
 * @file tcpclient.h file contains the classes connect to cass
 *
 * @author Lutz Foucar
 */
#ifndef _TCPCLIENT_H
#define _TCPCLIENT_H

#include <list>
#include <tr1/memory>

#include <QtCore/QObject>
#include <QtCore/QString>

namespace cass
{
class HistogramFloatBase;
}

class QStringList;

namespace jocassview
{
/** the tcp client that connects to the cass server
 *
 * @author Lutz Foucar
 */
class TCPClient : public QObject
{
  Q_OBJECT

public:
  /** default constructor */
  TCPClient();

  /** constructor
   *
   * @param server string containing the server ip and port
   */
  TCPClient(const QString &server);

  /** retrieve the list of available histograms */
  QStringList getIdList()const;

  /** retrieve a specific histogram
   *
   * @return Histogram for requested key
   * @param histogramkey the key of the requested histogram
   */
  std::tr1::shared_ptr<cass::HistogramFloatBase> getData(const QString &histogramkey)const;

  /** retrieve the transferred bytes */
  size_t receivedBytes()const;

public slots:
  /** reload .ini file */
  void reloadIni() const;

  /** reload .ini file */
  void controlCalibration(const QString& command) const;

  /** change the server to connect to
   *
   * @param serverstring the server name and port as string
   */
  void setServer(const QString &serverstring);

private:
  /** the server to connect to */
  QString _server;

  /** the amount of bytes transferred */
  mutable size_t _transferredBytes;
};
}//end namespace jocassview
#endif
