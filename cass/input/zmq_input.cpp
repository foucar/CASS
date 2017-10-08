// Copyright (C) 2017 Lutz Foucar

/**
 * @file zmq_input.cpp contains input that uses ZMQ as interface
 *
 * @author Lutz Foucar
 */

#include <iostream>
#include <zmq.hpp>

#include "zmq_input.h"

#include "cass_settings.h"
#include "cass_exceptions.hpp"
#include "log.h"

using namespace cass;
using namespace std;


void ZMQInput::instance(RingBuffer<CASSEvent> &buffer,
                   Ratemeter &ratemeter,
                   Ratemeter &loadmeter,
                   bool quitwhendone,
                   QObject *parent)
{
  if(_instance)
    throw logic_error("ZMQInput::instance(): The instance of the base class is already initialized");
  _instance = shared_pointer(new ZMQInput(buffer,ratemeter,loadmeter,quitwhendone,parent));
}

ZMQInput::ZMQInput(RingBuffer<CASSEvent> &ringbuffer,
                   Ratemeter &ratemeter,
                   Ratemeter &loadmeter,
                   bool quitwhendone,
                   QObject *parent)
  : InputBase(ringbuffer,ratemeter,loadmeter,parent),
    _quitWhenDone(quitwhendone)
{}


void ZMQInput::runthis()
{
  _status = lmf::PausableThread::running;

  CASSSettings s;
  s.beginGroup("ZMQInput");
  string functiontype(s.value("DataType","agat").toString().toStdString());
  string serverAddress(s.value("ServerAddress","tcp://53.104.0.52:10000").toString().toStdString());
  s.endGroup();

  /** connect to the zmq socket */
  zmq::context_t context (1);
  zmq::socket_t sock (context, ZMQ_SUB);
  sock.connect(serverAddress);


  /** run until the thread is told to quit */
  Log::add(Log::DEBUG0,"ZMQInput::run(): starting loop");
  while(!shouldQuit())
  {
    /** here we can safely pause the execution */
    pausePoint();

    /** retrieve a new element from the ringbuffer, continue with next iteration
     *  in case the retrieved element is the iterator to the last element of the
     *  buffer.
     */
    rbItem_t rbItem(getNextFillable());
    if (rbItem == _ringbuffer.end())
      continue;
    CASSEvent &evt(*rbItem->element);

    /** generate and set variable to keep the size of the retrieved data */
    uint64_t datasize(0);

    /** now retrive new data from the socket */
    zmq::message_t update;
    sock.recv(&update);

    /** now deserialize the data from the socket */

    /** tell the ringbuffer that we're done with the event */
    newEventAdded(datasize);
    _ringbuffer.doneFilling(rbItem, datasize);
  }
  Log::add(Log::INFO,"ZMQInput::run(): Quitting loop");
}
