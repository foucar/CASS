// Copyright (C) 2011 Lutz Foucar

/**
 * @file tcp_input.cpp contains input that uses tcp as interface
 *
 * @author Lutz Foucar
 */

#include <iostream>

#include <QtNetwork/QTcpSocket>

#include "tcp_input.h"

#include "cass_settings.h"
#include "cass_exceptions.h"
#include "tcp_streamer.h"

using namespace cass;
using namespace std;

TCPInput::TCPInput(RingBuffer<CASSEvent,RingBufferSize>& ringbuffer,
                   Ratemeter &ratemeter,
                   QObject *parent)
  :InputBase(ringbuffer,ratemeter,parent)
{
  loadSettings(0);
}

void TCPInput::run()
{
  _status = lmf::PausableThread::running;
  const int Timeout = 3 * 1000;

  CASSSettings s;
  s.beginGroup("TCPInput");
  QTcpSocket socket;
  socket.connectToHost(s.value("Server","localhost").toString(),
                       s.value("Port",9090).toUInt());
  string functiontype(s.value("DataType","agat").toString().toStdString());
  TCPStreamer& deserialize(TCPStreamer::instance(functiontype));
  s.endGroup();

  if (!socket.waitForConnected(Timeout))
    throw runtime_error("TCPInput::run(): error '" + socket.errorString().toStdString() +
                        "' occurred trying to connect to server '" + socket.peerName().toStdString() +
                        "' on Port '"+ toString(socket.peerPort()) +
                        "'");


  while(_control != _quit)
  {
    pausePoint();

    while (socket.bytesAvailable() < (int)sizeof(quint16))
    {
      if (!socket.waitForReadyRead(Timeout))
        continue;
    }

    quint32 payloadSize;
    QDataStream in(&socket);
    in.setVersion(QDataStream::Qt_4_0);
    in >> payloadSize;
    payloadSize -= sizeof(quint32);

    while (socket.bytesAvailable() < payloadSize)
    {
      if (!socket.waitForReadyRead(Timeout))
        throw runtime_error("TCPInput::run(): error '" + socket.errorString().toStdString() +
                            "' occurred trying to get the data from server '" + socket.peerName().toStdString() +
                            "' on Port '"+ toString(socket.peerPort()) +
                            "'");
    }

    payloadSize -= deserialize(in);
    while(payloadSize > 0)
    {
      CASSEvent *cassevent(0);
      _ringbuffer.nextToFill(cassevent);
      try
      {
        payloadSize -= deserialize(in,*cassevent);
        _ringbuffer.doneFilling(cassevent,true);
        emit newEventAdded();
      }
      catch(const DeserializeError& error)
      {
        cout << error.what() <<": skipping rest of data"<<endl;
        _ringbuffer.doneFilling(cassevent,false);
        break;
      }
    }
  }
}
