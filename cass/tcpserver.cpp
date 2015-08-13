// Copyright (C) 2010 Jochen Küpper
// Copyright (C) 2011 - 2013 Lutz Foucar

/**
 * @file tcpserver.cpp the soap server implementation
 *
 * @author Jochen Kuepper
 */

#include <stdexcept>
#include <deque>

#include <QtCore/QReadLocker>
#include <QtCore/QBuffer>
#include <QtCore/QByteArray>
#include <QtCore/QQueue>
#include <QtGui/QColor>
#include <QtGui/QImage>
#include <QThreadPool>

#include "tcpserver.h"

#include "CASSsoap.nsmap"
#include "processor_manager.h"
#include "processor.h"
#include "id_list.h"
#include "cass_exceptions.hpp"
#include "input_base.h"
#include "worker.h"
#include "result.hpp"
#include "common_data.h"
#include "log.h"


using namespace cass;
using namespace std;
using namespace tr1;

//==========DEFINE static variables=========================
SoapServer::shared_pointer SoapServer::_instance;
QMutex SoapServer::_mutex;
const size_t SoapServer::_backlog(100);

SoapServer::shared_pointer SoapServer::instance(size_t port)
{
  QMutexLocker locker(&_mutex);
  if(!_instance)
    _instance = shared_pointer(new SoapServer(port));
  return _instance;
}
//==========================================================



void SoapHandler::run()
{
  _soap->serve();   // serve request
  _soap->destroy(); // dealloc C++ data, dealloc data and clean up (destroy + end)
}

SoapServer::SoapServer(size_t port, QObject *parent)
  : QThread(parent),
    _soap(new CASSsoapService),
    _port(port)
{
  Log::add(Log::INFO,"SoapServer starting on port '" + toString(_port) +"'");
}

SoapServer::~SoapServer()
{
  if(isRunning())
    terminate();
  wait();
  _soap->destroy();
  delete _soap;
}

void SoapServer::run()
{
  // define timeouts and such for individual requests
  _soap->send_timeout   =   60; // 60 seconds
  _soap->recv_timeout   =   60; // 60 seconds
  _soap->accept_timeout =    0; // server never stops accepting
  _soap->max_keep_alive = 1000; // max keep-alive sequence
  // allow immediate re-use of address/socket
  _soap->bind_flags = SO_REUSEADDR;
  // start SOAP
  if(SOAP_INVALID_SOCKET == _soap->bind(NULL, _port, _backlog))
  {
    Log::add(Log::ERROR,"SoapServer::run(): Can't connect to socket on port '" +
             toString(_port) +"' for SOAP connection, Quitting the soap server");
    return;
  }
  while(true)
  {
    if(SOAP_INVALID_SOCKET == _soap->accept())
    {
      if(_soap->errnum)
      {
        _soap->soap_stream_fault(cerr);
        Log::add(Log::ERROR,"SoapServer::run(): No valid socket for SOAP connection");
      }
      else
        Log::add(Log::ERROR,"SoapServer::run(): Server timeout for SOAP connection");
      break;
    }
    CASSsoapService *tsoap(_soap->copy()); // make a safe copy
    if(! tsoap)
      break;
    SoapHandler *handler(new SoapHandler(tsoap));
    QThreadPool::globalInstance()->start(handler);  // SoapHandler::setAutoDelete() is set by default.
  }
}

/** quits CASS by telling the input to end */
int CASSsoapService::quit(bool *success)
{
  Log::add(Log::VERBOSEINFO,"CASSsoapService::quit");
  InputBase::reference().end();
  *success = true;;
  return SOAP_OK;
}

/** will call the load settings members of input and the workers
 *
 * first pause the input and the workers. Then call the load settings. then
 * resume the threads.
 */
int CASSsoapService::readini(size_t what, bool *success)
{
  Log::add(Log::VERBOSEINFO,"CASSsoapService::readini(what=" + toString(what) + ")");
  Log::loadSettings();
  QMutexLocker inputLock(&InputBase::reference().lock);
  QMutexLocker workerLock(&Workers::reference().lock);
  InputBase::reference().pause(true);
  Workers::reference().pause();
  InputBase::reference().load();
  QWriteLocker pplock(&ProcessorManager::instance()->lock);
  ProcessorManager::reference().loadSettings(what);
  Workers::reference().resume();
  InputBase::reference().resume();
  *success = true;;
  return SOAP_OK;
}

/** retrieve which processors are running
 *
 * get a list of keys of the running processors. Lock the processors
 * handler lock to prevent that the list changes while its being created.
 */
int CASSsoapService::getPostprocessorIds(bool *success)
{
  static QQueue< shared_ptr<string> > queue;
  QReadLocker lock(&ProcessorManager::reference().lock);
  tr1::shared_ptr<IdList> keys(ProcessorManager::reference().keys());
  shared_ptr<Serializer> ser(new Serializer);
  keys->serialize(*ser);
  soap_set_dime(this);
  shared_ptr<string> datstr(new string(ser->buffer()));
  int result = soap_set_dime_attachment(this, (char*) datstr->data(),datstr->size(), "application/processorList", "0", 0, NULL);
  string output("CASSsoapService::getPostprocessorIds: Sending the following names:");
  for (Processor::names_t::const_iterator it(keys->getList().begin());
       it != keys->getList().end(); ++it)
    output += *it + ", ";
  output += " size of serializer: " + toString(datstr->size());
  Log::add(Log::DEBUG4,output);
  queue.enqueue(datstr);
  if(100 < queue.size())
    queue.dequeue();
  *success = true;
  return result;
}

/** will call savesettings members of workers
 *
 * pause the workers before calling this function then call savesettings for
 * analyzers and processors, then resume the worker threads Prevent others
 * from retrieving data from the processors by locking it
 */
int CASSsoapService::writeini(size_t /*what*/, bool */*success*/)
{
//  Log::add(Log::VERBOSEINFO,"CASSsoapService::readini(what=" + toString(what) + ")");
//  QMutexLocker workerLock(&Workers::reference().lock);
//  Workers::reference().pause();
//  Analyzer::instance()->saveSettings();
//  QWriteLocker pplock(&PostProcessors::instance()->lock);
//  PostProcessors::instance()->saveSettings();
//  Workers::reference().resume();
//  InputBase::reference().resume();
//  *success = false;;
  return SOAP_FATAL_ERROR;
  throw logic_error("CASSsoapService::writeini: should not be used anymore");

}

/** clear the selected processors histogram list
 *
 * lock the processors handler processor list and retrieve the
 * requested processor from it. Tell it to clear its histogram list.
 */
int CASSsoapService::clearHistogram(ProcessorManager::key_t type, bool *success)
{
  Log::add(Log::VERBOSEINFO,"CASSsoapService::clearHistogram(type=" + type + ")");
  QWriteLocker pplock(&ProcessorManager::instance()->lock);
  try
  {
    ProcessorManager::instance()->getProcessor(type).clearHistograms();
    *success = true;;
    return SOAP_OK;
  }
  catch(const InvalidProcessorError&)
  {
    *success = false;
    return SOAP_FATAL_ERROR;
  }
}

/** control the darkcal calibration
 *
 * will tell the map creators of all defined pixeldetectors to start calibrating
 */
int CASSsoapService::controlDarkcal(string controlCommand, bool *success)
{
  try
  {
    pixeldetector::CommonData::controlCalibration(controlCommand);
    QWriteLocker pplock(&ProcessorManager::reference().lock);
    ProcessorManager::processors_t &processors(ProcessorManager::reference().processors());
    ProcessorManager::processors_t::iterator iter(processors.begin());
    ProcessorManager::processors_t::iterator end(processors.end());
    while( iter != end )
      (*iter++)->processCommand(controlCommand);
    *success = true;
    return SOAP_OK;
  }
  catch(const invalid_argument &)
  {
    *success = false;
    return SOAP_FATAL_ERROR;
  }
}

/** pass the string on to the requested processor
 *
 * lock the processor handler list then retrieve the requested processor
 * and pass the string to process to it.
 */
int CASSsoapService::receiveCommand(ProcessorManager::key_t type, string command, bool *success)
{
  Log::add(Log::VERBOSEINFO,"CASSsoapService::receiveCommand: command '" +
           command + "' is for processor " + type );
  QWriteLocker pplock(&ProcessorManager::instance()->lock);
  try
  {
    ProcessorManager::instance()->getProcessor(type).processCommand(command);
    *success = true;;
    return SOAP_OK;
  }
  catch(const InvalidProcessorError&)
  {
    *success = false;
    return SOAP_FATAL_ERROR;
  }
}

int CASSsoapService::getEvent(size_t /*eventID*/, unsigned /*t1*/, unsigned /*t2*/, bool */*success*/)
{
//  Log::add(Log::VERBOSEINFO,"CASSsoapService::getEvent with id "+ toString(eventId));
//  static QQueue<shared_ptr<string> > queue;
//  Serializer serializer;
  // get event somehow
//  event.serialize(serializer);
//  shared_ptr<string> data(new serializer.buffer())
//  queue.enqueue(data);
//  if(10 < queue.size())
//    queue.dequeue();
//  *success = true;
//  soap_set_dime(this); // enable dime
//  return soap_set_dime_attachment(this, (char *)data->data(), data->size(), "application/cassevent",
//                                  QString::number(type).toStdString().c_str(), 0, NULL);
  throw runtime_error("CASSsoapService::getEvent: has not yet been properly implemented");
}

/** get the the requested histogram and return it as dime attachement
 *
 * lock the processor handlers list by using the lock. then get a copy of
 * the requested histogram and serialize it. The mimetype of the dime is
 * determined by the dimension of the histogram.
 *
 * The serialized data is enqueued to make sure that the data is still around
 * also after the scope of this function is left since it might be transferred
 * after this funtion returned.
 */
int CASSsoapService::getHistogram(ProcessorManager::key_t type, ULONG64 eventId, bool *success)
{
  static QQueue<shared_ptr<pair<size_t, string> > > queue;
  QWriteLocker pplock(&ProcessorManager::instance()->lock);
  try
  {
    // get data
    Processor::result_t::shared_pointer result
          (ProcessorManager::reference().getProcessor(type).resultCopy(eventId));
    Serializer serializer;
    const size_t dim(result->dim());
    serializer << *result;
    shared_ptr<pair<size_t, string> >data(
          new pair<size_t, string>(make_pair(dim,serializer.buffer())));
    // MIME type
    string mimetype;
    switch(data->first)
    {
      case 0:
        mimetype = string("application/cass0Dhistogram");
        break;
      case 1:
        mimetype = string("application/cass1Dhistogram");
        break;
      case 2:
        mimetype = string("application/cass2Dhistogram");
        break;
      default:
        mimetype = string("application/casshistogram");
        break;
    }
    // keep bytes around for a while -- this should mitigate the "zeros" problem
    queue.enqueue(data);
    if(200 < queue.size())
      queue.dequeue();
    // answer request
    *success = true;
    soap_set_dime(this);
    Log::add(Log::DEBUG4,"CASSsoapService::getHistogram " + type +
             " from event " + toString(eventId) + ", the serialized size is "
             + toString(data->second.size()));
    return soap_set_dime_attachment(this, (char *)data->second.data(),
                                    data->second.size(), mimetype.c_str(),
                                    type.c_str(), 0, NULL);
  }
  catch(const InvalidResultError& error)
  {
    Log::add(Log::ERROR,string("CASSsoapService::getHistogram: ") + error.what());
    *success = false;
    return SOAP_FATAL_ERROR;
  }
  catch(const InvalidProcessorError& error)
  {
    Log::add(Log::ERROR,string("CASSsoapService::getHistogram: ") + error.what());
    *success = false;
    return SOAP_FATAL_ERROR;
  }
}

int CASSsoapService::getResults(bool sameEventID, bool *success)
{
  static deque<string> cache;
  try
  {
    /** get the list of requested processors */
    soap_multipart::iterator attachment(this->dime.begin());
    cout << "TCPServer: DIME attachment:" << endl
         << " TCPServer: Memory=" << (void*)(*attachment).ptr << endl
         << " TCPServer: Size=" << (*attachment).size << endl
         << " TCPServer: Type=" << ((*attachment).type?(*attachment).type:"null") << endl
         << " TCPServer: ID=" << ((*attachment).id?(*attachment).id:"null") << endl;
    Serializer deserializer(string((char*)(*attachment).ptr, (*attachment).size));
    IdList list(deserializer);

    /** get the result and serialize them */
    Serializer serializer;
    Processor::result_t::shared_pointer result;
    Processor::names_t::const_iterator it(list.getList().begin());
    id_t eventId(0);
    while (it != list.getList().end())
    {
      result = ProcessorManager::reference().getProcessor(*it).resultCopy(eventId);
      /** if the same id is requested for all events and it is the first event
       *  remember the event id of this event
       */
      if (sameEventID && it == list.getList().begin())
        eventId = 0;
      serializer << *result;
      ++it;
    }
    Log::add(Log::VERBOSEINFO,"CASSsoapService::getResults ");

    /** get the serialized data and store it in a cache to prevent the data from
     *  being deleted before completely delivered
     */
    string data(serializer.buffer());
    cache.push_back(data);
    if (cache.size() > 100)
      cache.pop_front();

    /** send the data from the cache as dime attachment */
    *success = true;
    soap_set_dime(this);
    return soap_set_dime_attachment(this, (char *)cache.back().data(),
                                    cache.back().size(), "cass/Result",
                                    "0", 0, NULL);
  }
  catch(const InvalidResultError& error)
  {
    Log::add(Log::ERROR,string("CASSsoapService::getResults: ") + error.what());
    *success = false;
    return SOAP_FATAL_ERROR;
  }
  catch(const InvalidProcessorError& error)
  {
    Log::add(Log::ERROR,string("CASSsoapService::getResults: ") + error.what());
    *success = false;
    return SOAP_FATAL_ERROR;
  }
}
