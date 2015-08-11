//Copyright (C) 2011 Lutz Foucar

/**
 * @file jocassview/tcpclient.cpp file contains the classes connect to cass
 *
 * @author Lutz Foucar
 */

#include <QtCore/QDebug>
#include <QtCore/QFuture>

#if QT_VERSION >= 0x050000
#include <QtConcurrent/QtConcurrent>
#include <QtWidgets/QMessageBox>
#else
#include <QtCore/QtConcurrentRun>
#include <QtGui/QMessageBox>
#endif

#include <QtTest/QTest>

#include "tcpclient.h"

#include "id_list.h"
#include "soapCASSsoapProxy.h"
#include "CASSsoap.nsmap"
#include "result.hpp"

using namespace jocassview;
using namespace std;

TCPClient::TCPClient()
  : _transferredBytes(0)
{
}

TCPClient::~TCPClient()
{
}

DataSource::result_t::shared_pointer TCPClient::result(const QString &key, quint64 id)
{
  CASSsoapProxy client;
  client.soap_endpoint = _server.c_str();

  bool ret(false);
  QFuture<int> future = QtConcurrent::run(&client,&CASSsoapProxy::getHistogram,
                                          key.toStdString(), id, &ret);
  while (future.isRunning())
  {
    QCoreApplication::processEvents(QEventLoop::AllEvents);
    QTest::qSleep(10);
  }
  if( (future.result() != SOAP_OK) || !ret)
    return result_t::shared_pointer();
  soap_multipart::iterator attachment(client.dime.begin());
  if(client.dime.end() == attachment)
    return result_t::shared_pointer();
  _transferredBytes += (*attachment).size;
  result_t::shared_pointer result(new result_t());
  cass::Serializer serializer( std::string((char *)(*attachment).ptr, (*attachment).size) );
  serializer >> *result;
  return result;
}

QVector<DataSource::result_t::shared_pointer> TCPClient::results(const QStringList & list)
{
  /** set up the client */
  CASSsoapProxy client;
  client.soap_endpoint = _server.c_str();
  soap_set_dime(&client);
  bool success;
  /** serialize the id list */
  cass::Serializer serializer;
  IdList idlist(list);
  idlist.serialize(serializer);
  string data(serializer.buffer());
  /** add the serialized list to the dime attachment and sent the request */
  soap_set_dime_attachment(&client, (char*)data.data(), data.size(),
                           "application/processorList", "0", 0, NULL);
  int status = client.getResults(false, &success);
  /** create the container for the results */
  QVector<result_t::shared_pointer> results;
  /** when the communication failed return here */
  if (status != SOAP_OK || !success)
    return results;
  /** create a deserializing object and retrieve all results from it */
  soap_multipart::iterator attachment(client.dime.begin());
  if(client.dime.end() == attachment)
    return results;
  cass::Serializer deserializer(std::string((char*)(*attachment).ptr,(*attachment).size));
  for (int i(0); i<list.size(); ++i)
  {
    result_t::shared_pointer result(new result_t());
    deserializer >> *result;
    results.push_back(result);
  }
  return results;
}

QStringList TCPClient::resultNames()
{
  CASSsoapProxy client;
  client.soap_endpoint = _server.c_str();

  bool ret(true);
  QFuture<int> future = QtConcurrent::run(&client,&CASSsoapProxy::getPostprocessorIds, &ret);
  while (future.isRunning())
  {
    QCoreApplication::processEvents(QEventLoop::AllEvents);
    QTest::qSleep(10);
  }
  if( (future.result() != SOAP_OK) || !ret)
  {
    QMessageBox::information(0, tr("TcpClient"),
                             tr("Error resultNames: No communication to '") +
                             QString::fromStdString(_server)+ tr("' possible."));
    return QStringList();
  }
  soap_multipart::iterator attachment (client.dime.begin());
  if(client.dime.end() == attachment)
  {
    QMessageBox::information(0, tr("TcpClient"),
                             tr("Error resultNames: Server '")+
                             QString::fromStdString(_server) + tr("'didn't send data"));
    return QStringList();
  }
//  qDebug() << "TCPClient: DIME attachment:" << endl
//           << "TCPClient: Memory=" << (void*)(*attachment).ptr << endl
//           << "TCPClient: Size=" << (*attachment).size << endl
//           << "TCPClient: Type=" << ((*attachment).type?(*attachment).type:"null") << endl
//           << "TCPClient: ID=" << ((*attachment).id?(*attachment).id:"null") << endl;
  _transferredBytes += (*attachment).size;
  cass::Serializer serializer( std::string((char *)(*attachment).ptr, (*attachment).size) );
  IdList list(serializer);
//  qDebug() << list.getList();
  return list.getList();
}

QString TCPClient::type()const
{
  return QString("TCPClient");
}

size_t TCPClient::receivedBytes()const
{
  return _transferredBytes;
}

void TCPClient::reloadIni() const
{
  CASSsoapProxy client;
  client.soap_endpoint = _server.c_str();

  bool ret(false);
  QFuture<int> future = QtConcurrent::run(&client,&CASSsoapProxy::readini,0,&ret);
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
  CASSsoapProxy client;
  client.soap_endpoint = _server.c_str();

  bool ret(false);
  QFuture<int> future = QtConcurrent::run(&client,&CASSsoapProxy::controlDarkcal,
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
  CASSsoapProxy client;
  client.soap_endpoint = _server.c_str();

  bool ret(false);
  QFuture<int> future = QtConcurrent::run(&client,&CASSsoapProxy::receiveCommand,
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
}

void TCPClient::quitServer() const
{
  CASSsoapProxy client;
  client.soap_endpoint = _server.c_str();

  bool ret(false);
  QFuture<int> future = QtConcurrent::run(&client,&CASSsoapProxy::quit, &ret);
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
  CASSsoapProxy client;
  client.soap_endpoint = _server.c_str();

  bool ret(false);
  QFuture<int> future = QtConcurrent::run(&client,&CASSsoapProxy::clearHistogram, key.toStdString(), &ret);
  while (future.isRunning())
  {
    QCoreApplication::processEvents(QEventLoop::AllEvents);
    QTest::qSleep(10);
  }
  if(!ret)
    QMessageBox::information(0, tr("TcpClient"),
                             tr("Error: Cannot communicate clearHistogram command."));
}
