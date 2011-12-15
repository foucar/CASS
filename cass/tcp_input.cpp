// Copyright (C) 2011 Lutz Foucar

/**
 * @file tcp_input.cpp contains input that uses tcp as interface
 *
 * @author Lutz Foucar
 */

#include <iostream>
#include <tr1/functional>

#include <QtNetwork/QTcpSocket>

#include "tcp_input.h"

#include "cass_settings.h"
#include "agat_deserializer.h"

using namespace cass;
using namespace std;
using tr1::function;

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

  map<string,function<bool(QDataStream&,CASSEvent&)> > functions;
  functions["agat"] = ACQIRIS::deserializeNormalAgat();

  CASSSettings s;
  s.beginGroup("TCPInput");
  QTcpSocket socket;
  socket.connectToHost(s.value("Server","localhost").toString(),
                       s.value("Port",9090).toUInt());
  string functiontype(s.value("DataType","agat").toString().toStdString());
  if (functions.find(functiontype) == functions.end())
    throw invalid_argument("...");
  function<bool(QDataStream&,CASSEvent&)> deserialize(functions[functiontype]);
  s.endGroup();

  if (!socket.waitForConnected(Timeout))
  {
    cout << "TCPInput::run(): error '" << socket.errorString().toStdString()
         <<"' occurred trying to connect to server '"<<socket.peerName().toStdString()
         <<"' on Port '"<<socket.peerPort()
         <<"'"<<endl;
    return;
  }

  while(_control != _quit)
  {
    pausePoint();

    while (socket.bytesAvailable() < (int)sizeof(quint16))
    {
      if (!socket.waitForReadyRead(Timeout))
        continue;
    }

    quint32 blockSize;
    QDataStream in(&socket);
    in.setVersion(QDataStream::Qt_4_0);
    in >> blockSize;

    while (socket.bytesAvailable() < blockSize)
    {
      if (!socket.waitForReadyRead(Timeout))
      {
        cout << "TCPInput::run(): error '" << socket.errorString().toStdString()
             <<"' occurred trying to get the data from server '"<<socket.peerName().toStdString()
             <<"' on Port '"<<socket.peerPort()
             <<"'"<<endl;
        return;
      }
    }
    while(!in.atEnd())
    {
      CASSEvent *cassevent(0);
      _ringbuffer.nextToFill(cassevent);
      bool isGood(deserialize(in,*cassevent));
      _ringbuffer.doneFilling(cassevent,isGood);
      emit newEventAdded();
    }
  }
}
