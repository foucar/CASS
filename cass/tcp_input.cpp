// Copyright (C) 2011,2013 Lutz Foucar

/**
 * @file tcp_input.cpp contains input that uses tcp as interface
 *
 * @author Lutz Foucar
 */

#include <iostream>

#include <QtNetwork/QTcpSocket>
#include <QtNetwork/QHostAddress>

#include "tcp_input.h"

#include "cass_settings.h"
#include "cass_exceptions.hpp"
#include "tcp_streamer.h"
#include "log.h"

using namespace cass;
using namespace std;


void TCPInput::instance(RingBuffer<CASSEvent> &buffer,
                   Ratemeter &ratemeter,
                   Ratemeter &loadmeter,
                   QObject *parent)
{
  if(_instance)
    throw logic_error("TCPInput::instance(): The instance of the base class is already initialized");
  _instance = shared_pointer(new TCPInput(buffer,ratemeter,loadmeter,parent));
}

TCPInput::TCPInput(RingBuffer<CASSEvent> &ringbuffer,
                   Ratemeter &ratemeter,
                   Ratemeter &loadmeter,
                   QObject *parent)
  :InputBase(ringbuffer,ratemeter,loadmeter,parent)
{}

bool TCPInput::connectToServer(QTcpSocket &socket)
{
  CASSSettings s;
  s.beginGroup("TCPInput");
  bool retval(false);
  /** try to connect until its either connected or the thread is told to quit */
  while (!shouldQuit())
  {
    socket.abort();
    socket.connectToHost(s.value("Server","localhost").toString(),
                         s.value("Port",9090).toUInt());
    if (socket.waitForConnected(s.value("SocketConnectionTimout_ms",1000).toInt()))
    {
      Log::add(Log::INFO,string("TCPInput::connectToServer: (Re)Connected to server '") +
               socket.peerAddress().toString().toStdString() +
               "' on port '" + toString(socket.peerPort()) + "'");
      retval = true;
      break;
    }
    else
    {
      Log::add(Log::WARNING,"TCPInput::connectToServer: Could not connect to server '" +
               s.value("Server","localhost").toString().toStdString() +
               "' on port '" + toString(s.value("Port",9090).toUInt()) +
               "', because of error '" + socket.errorString().toStdString() +
               "'. Retrying");
    }
    sleep(s.value("WaitUntilReconnectionTry_s",5).toUInt());
  }
  s.endGroup();
  return retval;
}

bool TCPInput::dataAvailable(QTcpSocket &socket, qint64 datasize)
{
  /** first check if we're connected to the server. If not, then try to connect */
  if (socket.state() != QAbstractSocket::ConnectedState)
    if(!connectToServer(socket))
      return false;

  /** check if the data is available, if not then wait for a while, if it isn't
   *  available then, check the connection to the server
   */
  while (socket.bytesAvailable() < datasize)
  {
    if (shouldQuit())
      return false;
    if (!socket.waitForReadyRead(_timeout) && (socket.state() != QAbstractSocket::ConnectedState))
    {
      connectToServer(socket);
      return false;
    }
  }
  return true;
}

void TCPInput::runthis()
{
  _status = lmf::PausableThread::running;
  QTcpSocket socket;

  CASSSettings s;
  s.beginGroup("TCPInput");
  string functiontype(s.value("DataType","agat").toString().toStdString());
  TCPStreamer& deserialize(TCPStreamer::instance(functiontype));
  _timeout = s.value("SocketDataTimeout_ms",2 * 1000).toInt();
  s.endGroup();

  /** connect to the server before starting the loop. */
  connectToServer(socket);

  while(!shouldQuit())
  {
    pausePoint();

    if (!dataAvailable(socket,  static_cast<qint64>(sizeof(quint32))))
      continue;

    quint32 payloadSize;
    QDataStream in(&socket);
    in.setVersion(QDataStream::Qt_4_0);
    in.setByteOrder(QDataStream::LittleEndian);
    in >> payloadSize;
    /** check whether the upper bit is set (indicating that data is compressed) */
    const bool dataCompressed(payloadSize & 0x80000000);
    /** reset the upper bit */
    payloadSize &= 0x7FFFFFFF;
    payloadSize -= sizeof(quint32);

    if(!dataAvailable(socket,payloadSize))
      continue;

    /** write received data into a temporary buffer */
    QByteArray buffer;
    if (dataCompressed)
    {
      QByteArray tmp(payloadSize,'0');
      in.readRawData(tmp.data(),payloadSize);
      buffer = qUncompress(tmp);
    }
    else
    {
      buffer.resize(payloadSize);
      in.readRawData(buffer.data(),payloadSize);
    }

    /** use stream to deserialize buffer */
    QDataStream stream(buffer);

    /** deserialize the buffer header */
    deserialize(stream);
    while((!shouldQuit()) && (!stream.atEnd()))
    {
      rbItem_t rbItem(getNextFillable());
      if (rbItem == _ringbuffer.end())
        continue;
      try
      {
        deserialize(stream,*rbItem->element);
        _ringbuffer.doneFilling(rbItem,true);
        newEventAdded(rbItem->element->datagrambuffer().size());
      }
      catch(const DeserializeError& error)
      {
        Log::add(Log::ERROR,string(error.what()) +
                 ": skipping rest of data");
        _ringbuffer.doneFilling(rbItem,false);
        break;
      }
    }
  }
  Log::add(Log::INFO,"TCPInput::run(): Quitting");
}
