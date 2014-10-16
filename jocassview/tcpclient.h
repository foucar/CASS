//Copyright (C) 2011, 2013 Lutz Foucar

/**
 * @file jocassview/tcpclient.h file contains the classes connect to cass
 *
 * @author Lutz Foucar
 */
#ifndef _TCPCLIENT_H
#define _TCPCLIENT_H

#include <list>
#include <tr1/memory>

#include <QtCore/QObject>
#include <QtCore/QString>

#include "data_source.h"

namespace cass
{
class HistogramBackend;
}//end namespace cass

class QStringList;
class CASSsoapProxy;

namespace jocassview
{

/** the tcp client that connects to the cass server
 *
 * @author Lutz Foucar
 */
class TCPClient : public QObject, public DataSource
{
  Q_OBJECT

public:
  /** default constructor */
  TCPClient();

  /** destructor */
  virtual ~TCPClient();

  /** retrieve a specific histogram
   *
   * @return Histogram for requested key
   * @param histogramkey the key of the requested histogram
   */
  cass::HistogramBackend *result(const QString &histogramkey,quint64 id=0);

  /** retrieve the list of available histograms
   *
   * @return the list of names of available histograms
   */
  QStringList resultNames();

  /** return the type of source we are
   *
   * @return "TCPClient"
   */
  QString type() const;

  /** retrieve the transferred bytes */
  size_t receivedBytes()const;

public slots:
  /** reload .ini file */
  void reloadIni() const;

  /** broadcast a command to all postprocessors in the server
   *
   * @param command The command to broadcast
   */
  void broadcastCommand(const QString& command) const;

  /** broadcast a command to all postprocessors in the server
   *
   * @param key The key of the Postprocessor to send the command to
   * @param command The command to broadcast
   */
  void sendCommandTo(const QString &key, const QString& command) const;

  /** change the server to connect to
   *
   * @param serverstring the server name and port as string
   */
  void setServer(const QString &serverstring);

  /** tell the server to quit */
  void quitServer() const;

  /** clear the histogram of a postprocessor
   *
   * @param key The key of the Postprocessor whos histograms should be cleared
   */
  void clearHistogram(QString key) const;

private:
  /** the amount of bytes transferred */
  mutable size_t _transferredBytes;

  /** the interface to cass */
  //CASSsoapProxy *_client;

  /** the server string
   *
   * @note this is needed because the client only hold a pointer to the string
   *       and not the string itself.
   */
  std::string _server;
};
}//end namespace jocassview
#endif
