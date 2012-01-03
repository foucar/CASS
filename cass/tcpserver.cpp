// Copyright (C) 2010 Jochen Küpper
// Copyright (C) 2011 Lutz Foucar

/**
 * @file tcpserver.cpp the soap server implementation
 *
 * @author Jochen Kuepper
 */

#include <stdexcept>

#include <QtCore/QReadLocker>
#include <QtCore/QBuffer>
#include <QtCore/QByteArray>
#include <QtCore/QQueue>
#include <QtGui/QColor>
#include <QtGui/QImage>
#include <QThreadPool>

#include "tcpserver.h"

#include "CASSsoap.nsmap"
#include "postprocessor.h"
#include "backend.h"
#include "id_list.h"
#include "cass_exceptions.h"
#include "input_base.h"
#include "worker.h"
#include "analyzer.h"
#include "histogram.h"


using namespace cass;
using namespace std;
using namespace tr1;

//==========DEFINE static variables=========================
SoapServer::shared_pointer SoapServer::_instance;
QMutex SoapServer::_mutex;
const size_t SoapServer::_backlog(100);

SoapServer::shared_pointer SoapServer::instance(const EventGetter& event,
                                                const HistogramGetter& hist,
                                                size_t port)
{
  QMutexLocker locker(&_mutex);
  if(!_instance)
    _instance = shared_pointer(new SoapServer(event, hist, port));
  return _instance;
}
//==========================================================



void SoapHandler::run()
{
  _soap->serve();   // serve request
  _soap->destroy(); // dealloc C++ data, dealloc data and clean up (destroy + end)
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
    throw runtime_error("No valid socket for SOAP server");
  while(true)
  {
    if(SOAP_INVALID_SOCKET == _soap->accept())
    {
      if(_soap->errnum)
      {
        _soap->soap_stream_fault(cerr);
        cerr << "*** No valid socket for SOAP connection ***" << endl;
      }
      else
        cerr << "*** Server timeout for SOAP connection ***" << endl;
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
  VERBOSEOUT(cerr << "CASSsoapService::quit" << endl);
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
  VERBOSEOUT(cerr << "CASSsoapService::readini(what=" << what << ")" << endl);
  QMutexLocker inputLock(&InputBase::reference().lock);
  QMutexLocker workerLock(&Workers::reference().lock);
  InputBase::reference().pause(true);
  Workers::reference().pause();
  InputBase::reference().loadSettings(what);
  Analyzer::instance()->loadSettings(what);
  QWriteLocker pplock(&PostProcessors::instance()->lock);
  PostProcessors::reference().loadSettings(what);
  Workers::reference().resume();
  InputBase::reference().resume();
  *success = true;;
  return SOAP_OK;
}

/** retrieve which postprocessors are running
 *
 * get a list of keys of the running postprocessors. Lock the postprocessors
 * handler lock to prevent that the list changes while its being created.
 */
int CASSsoapService::getPostprocessorIds(bool *success)
{
  static QQueue< shared_ptr<string> > queue;
  QReadLocker lock(&PostProcessors::reference().lock);
  tr1::shared_ptr<IdList> keys(PostProcessors::reference().keys());
  shared_ptr<Serializer> ser(new Serializer);
  keys->serialize(*ser);
  soap_set_dime(this);
  shared_ptr<string> datstr(new string(ser->buffer()));
  queue.enqueue(datstr);
  int result = soap_set_dime_attachment(this, (char*) datstr->data(), ser->buffer().size(), "application/postprocessorList", "0", 0, NULL);
  if(100 < queue.size())
    queue.dequeue();
  *success = true;
  return result;
}

/** will call savesettings members of workers
 *
 * pause the workers before calling this function then call savesettings for
 * analyzers and postprocessors, then resume the worker threads Prevent others
 * from retrieving data from the postprocessors by locking it
 */
int CASSsoapService::writeini(size_t what, bool *success)
{
  VERBOSEOUT(cerr << "CASSsoapService::readini(what=" << what << ")" << endl);
  QMutexLocker workerLock(&Workers::reference().lock);
  Workers::reference().pause();
  Analyzer::instance()->saveSettings();
  QWriteLocker pplock(&PostProcessors::instance()->lock);
  PostProcessors::instance()->saveSettings();
  Workers::reference().resume();
  InputBase::reference().resume();
  *success = true;;
  return SOAP_OK;
}

/** clear the selected postprocessors histogram list
 *
 * lock the postprocessors handler postprocessor list and retrieve the
 * requested postprocessor from it. Tell it to clear its histogram list.
 */
int CASSsoapService::clearHistogram(PostProcessors::key_t type, bool *success)
{
  VERBOSEOUT(cerr << "CASSsoapService::clearHistogram(type=" << type << ")" << endl);
  QWriteLocker pplock(&PostProcessors::instance()->lock);
  try
  {
    PostProcessors::instance()->getPostProcessor(type).clearHistograms();
    *success = true;;
    return SOAP_OK;
  }
  catch(const InvalidPostProcessorError&)
  {
    *success = false;
    return SOAP_FATAL_ERROR;
  }
}

/** pass the string on to the requested postprocessor
 *
 * lock the postprocessor handler list then retrieve the requested postprocessor
 * and pass the string to process to it.
 */
int CASSsoapService::receiveCommand(PostProcessors::key_t type, string command, bool *success)
{
  VERBOSEOUT(cerr << "CASSsoapService::receiveCommand(type=" << type << ")" << endl);
  QWriteLocker pplock(&PostProcessors::instance()->lock);
  try
  {
    PostProcessors::instance()->getPostProcessor(type).processCommand(command);
    *success = true;;
    return SOAP_OK;
  }
  catch(const InvalidPostProcessorError&)
  {
    *success = false;
    return SOAP_FATAL_ERROR;
  }
}

int CASSsoapService::getEvent(size_t type, unsigned t1, unsigned t2, bool *success)
{
//  /** @todo use shared pointer inside the queue */
//  /** @todo get rid of the functor and do everything here */
//  VERBOSEOUT(cerr << "CASSsoapService::getEvent" << endl);
//  static QQueue<string *> queue;
//  string *data(new string(SoapServer::instance()->get_event(EventParameter(type, t1, t2))));
//  queue.enqueue(data);
//  if(10 < queue.size())
//    queue.dequeue();
//  *success = true;
//  soap_set_dime(this); // enable dime
//  return soap_set_dime_attachment(this, (char *)data->c_str(), data->size(), "application/cassevent",
//                                  QString::number(type).toStdString().c_str(), 0, NULL);
  throw runtime_error("CASSsoapService::getEvent: has not yet been properly implemented");
}

/** get the the requested histogram and return it as dime attachement
 *
 * lock the postprocessor handlers list by using the lock. then get a copy of
 * the requested histogram and serialize it. The mimetype of the dime is
 * determined by the dimension of the histogram.
 *
 * The serialized data is enqueued to make sure that the data is still around
 * also after the scope of this function is left since it might be transferred
 * after this funtion returned.
 */
int CASSsoapService::getHistogram(PostProcessors::key_t type, ULONG64 eventId, bool *success)
{
  VERBOSEOUT(cerr << "CASSsoapService::getHistogram" << endl);
  static QQueue<shared_ptr<pair<size_t, string> > > queue;
  QWriteLocker pplock(&PostProcessors::instance()->lock);
  try
  {
    // get data
    shared_ptr<HistogramBackend> hist(
          PostProcessors::reference().getPostProcessor(type).getHistCopy(eventId));
    Serializer serializer;
    size_t dim(hist->dimension());
    hist->serialize(serializer);
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
    return soap_set_dime_attachment(this, (char *)data->second.data(), data->second.size(), mimetype.c_str(),
                                    type.c_str(), 0, NULL);
  }
  catch(InvalidHistogramError)
  {
    *success = false;
    return SOAP_FATAL_ERROR;
  }
  catch(InvalidPostProcessorError)
  {
    *success = false;
    return SOAP_FATAL_ERROR;
  }
}









// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
