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
#include "cass_exceptions.h"
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

void TCPInput::connectToServer(QTcpSocket &socket)
{
  CASSSettings s;
  s.beginGroup("TCPInput");
  while (_control != _quit)
  {
    socket.abort();
    socket.connectToHost(s.value("Server","localhost").toString(),
                         s.value("Port",9090).toUInt());
    if (socket.waitForConnected(1000))
    {
      Log::add(Log::INFO,string("TCPInput::connectToServer: (Re)Connected to server '") +
               socket.peerAddress().toString().toStdString() +
               "' on port '" + toString(socket.peerPort()) + "'");
      break;
    }
    else
    {
      Log::add(Log::WARNING,"TCPInput::connectToServer: Could not connect to server '" +
               s.value("Server","localhost").toString().toStdString() +
               "' on port '" + toString(s.value("Port",9090).toUInt()) +
               "', because of error '" + socket.errorString().toStdString() +
               "'. Retrying in 5 s...");
    }
    sleep(5);
  }
  s.endGroup();
  throw RestartInputLoop();
}

void TCPInput::checkSocket(QTcpSocket &socket)
{
  if (_control == _quit)
    throw RestartInputLoop();
  if (socket.state() != QAbstractSocket::ConnectedState)
    connectToServer(socket);
}

void TCPInput::runthis()
{
  _status = lmf::PausableThread::running;
  const int Timeout = 2 * 1000;
  QTcpSocket socket;

  CASSSettings s;
  s.beginGroup("TCPInput");
  string functiontype(s.value("DataType","agat").toString().toStdString());
  TCPStreamer& deserialize(TCPStreamer::instance(functiontype));
  s.endGroup();

  while(_control != _quit)
  {
    try
    {
      pausePoint();

      while (socket.bytesAvailable() < static_cast<qint64>(sizeof(quint32)))
      {
        if (!socket.waitForReadyRead(Timeout))
          checkSocket(socket);
      }

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

      while (socket.bytesAvailable() < payloadSize)
      {
        if (!socket.waitForReadyRead(Timeout))
          checkSocket(socket);
      }

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
      while(!stream.atEnd())
      {
        rbItem_t rbItem(_ringbuffer.nextToFill());
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
    catch(RestartInputLoop&) {}
    {
    }
  }
  Log::add(Log::INFO,"TCPInput::run(): Quitting");
}
