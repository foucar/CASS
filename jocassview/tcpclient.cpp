//Copyright (C) 2011 Lutz Foucar

/**
 * @file tcpclient.cpp file contains the classes connect to cass
 *
 * @author Lutz Foucar
 */

#include <QtCore/QDebug>
#include <QtCore/QtConcurrentRun>
#include <QtCore/QFuture>

#include <QtGui/QMessageBox>

#include <QTest>

#include "tcpclient.h"

#include "id_list.h"
#include "soapCASSsoapProxy.h"
#include "CASSsoap.nsmap"
#include "histogram.h"

using namespace jocassview;
using namespace std;

TCPClient::TCPClient()
  : _transferredBytes(0),
    _client(new CASSsoapProxy)
{

}

TCPClient::~TCPClient()
{
  delete _client;
}

QStringList TCPClient::getIdList()const
{
  bool ret(true);
  QFuture<int> future = QtConcurrent::run(_client,&CASSsoapProxy::getPostprocessorIds, &ret);
  while (future.isRunning())
  {
    QCoreApplication::processEvents(QEventLoop::AllEvents);
    QTest::qSleep(10);
  }
  if( (future.result() != SOAP_OK) || !ret)
  {
    QMessageBox::information(0, tr("TcpClient"),
                             tr("Error: Can't retrieve list of keys"));
    return QStringList();
  }
  soap_multipart::iterator attachment (_client->dime.begin());
  if(_client->dime.end() == attachment)
  {
    QMessageBox::information(0, tr("TcpClient"),
                             tr("Error: Can't retrieve list of keys"));
    return QStringList();
  }
  _transferredBytes += (*attachment).size;
  cass::Serializer serializer( std::string((char *)(*attachment).ptr, (*attachment).size) );
  IdList list(serializer);
  return list.getList();

//  cout << "TCPClient: DIME attachment:" << endl
//      << " TCPClient: Memory=" << (void*)(*attachment).ptr << endl
//      << " TCPClient: Size=" << (*attachment).size << endl
//      << " TCPClient: Type=" << ((*attachment).type?(*attachment).type:"null") << endl
//      << " TCPClient: ID=" << ((*attachment).id?(*attachment).id:"null") << endl;
}

cass::HistogramBackend* TCPClient::getData(const QString &histogramkey, quint64 id)const
{
  bool ret(false);
  QFuture<int> future = QtConcurrent::run(_client,&CASSsoapProxy::getHistogram,
                                          histogramkey.toStdString(), id, &ret);
  while (future.isRunning())
  {
    QCoreApplication::processEvents(QEventLoop::AllEvents);
    QTest::qSleep(10);
  }
  if( (future.result() != SOAP_OK) || !ret)
    return 0;
  soap_multipart::iterator attachment(_client->dime.begin());
  if(_client->dime.end() == attachment)
    return 0;
  _transferredBytes += (*attachment).size;
  string mimeType((*attachment).type);
  cass::HistogramBackend* hist(0);
  cass::Serializer serializer( std::string((char *)(*attachment).ptr, (*attachment).size) );
  if(mimeType == "application/cass2Dhistogram")
    hist = new cass::Histogram2DFloat(serializer);
  else if(mimeType == "application/cass1Dhistogram")
    hist = new cass::Histogram1DFloat(serializer);
  else if(mimeType == "application/cass0Dhistogram")
    hist = new cass::Histogram0DFloat(serializer);
  return hist;
}

size_t TCPClient::receivedBytes()const
{
  return _transferredBytes;
}

void TCPClient::reloadIni() const
{
  bool ret(false);
  QFuture<int> future = QtConcurrent::run(_client,&CASSsoapProxy::readini,0,&ret);
  while (future.isRunning())
  {
    QCoreApplication::processEvents(QEventLoop::AllEvents);
    QTest::qSleep(10);
  }
  if(!ret)
    QMessageBox::information(0, tr("TcpClient"),
                             tr("Error: Cannot reload the ini file "));
}

void TCPClient::broadcastCommand(const QString &command) const
{
  bool ret(false);
  QFuture<int> future = QtConcurrent::run(_client,&CASSsoapProxy::controlDarkcal,
                                          command.toStdString(), &ret);
  while (future.isRunning())
  {
    QCoreApplication::processEvents(QEventLoop::AllEvents);
    QTest::qSleep(10);
  }
  if(!ret)
    QMessageBox::information(0, tr("TcpClient"),
                             tr("Error: Cannot broadcast ") + command);
}

void TCPClient::sendCommandTo(const QString &key, const QString &command) const
{
  bool ret(false);
  QFuture<int> future = QtConcurrent::run(_client,&CASSsoapProxy::receiveCommand,
                                          key.toStdString(), command.toStdString(),
                                          &ret);
  while (future.isRunning())
  {
    QCoreApplication::processEvents(QEventLoop::AllEvents);
    QTest::qSleep(10);
  }
  if(!ret)
    QMessageBox::information(0, tr("TcpClient"),
                             tr("Error: Cannot send '") + command + tr("' to '")+
                             key +tr("'"));
}

void TCPClient::setServer(const QString &serverstring)
{
  _server = serverstring.toStdString();
  _client->soap_endpoint = _server.c_str();
}

void TCPClient::quitServer() const
{
  bool ret(false);
  QFuture<int> future = QtConcurrent::run(_client,&CASSsoapProxy::quit, &ret);
  while (future.isRunning())
  {
    QCoreApplication::processEvents(QEventLoop::AllEvents);
    QTest::qSleep(10);
  }
  if(!ret)
    QMessageBox::information(0, tr("TcpClient"),
                             tr("Error: Cannot communicate quitServer command."));
}

void TCPClient::clearHistogram(QString key) const
{
  bool ret(false);
  QFuture<int> future = QtConcurrent::run(_client,&CASSsoapProxy::clearHistogram, key.toStdString(), &ret);
  while (future.isRunning())
  {
    QCoreApplication::processEvents(QEventLoop::AllEvents);
    QTest::qSleep(10);
  }
  if(!ret)
    QMessageBox::information(0, tr("TcpClient"),
                             tr("Error: Cannot communicate clearHistogram command."));
}
