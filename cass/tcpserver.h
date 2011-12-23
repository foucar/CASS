// CASS TCP server
//
// Copyright (C) 2010 Jochen Küpper
// Copyright (C) 2011 Lutz Foucar

/**
 * @file tcpserver.h the soap server implementation
 *
 * @author Jochen Kuepper
 */

#ifndef CASS_TCPSERVER_H
#define CASS_TCPSERVER_H

#include <stdexcept>
#include <tr1/memory>

#include <QtCore/QThread>
#include <QtCore/QThread>
#include <QtCore/QRunnable>

#include "cass_event.h"
#include "event_getter.h"
#include "histogram_getter.h"
#include "soapCASSsoapService.h"


namespace cass
{

/** Handle a single SOAP request
 *
 * @todo setup soap, such that while compiling there are not warnings anymore:
 *        soapC.cpp:457: warning: dereferencing type-punned pointer will break strict-aliasing rules
 *        soapC.cpp:1093: warning: unused parameter 'tt'
 *
 * @author Jochen Küpper
 */
class SoapHandler : public QRunnable
{
public:
  /** constructor
   *
   * @param soap the soap instance to be used be handeld
   */
  SoapHandler(CASSsoapService *soap)
    : _soap(soap)
  {}

  /** destructor
   *
   * destroys the soap instance that was handelt by this
   */
  virtual ~SoapHandler() { delete _soap; }

  /** handle request and terminate*/
  virtual void run();

protected:
  /** the service */
  CASSsoapService *_soap;
};



/** SOAP server
 *
 * @todo Update getImage to actually return an direct (i.e., unscaled) TIFF
 *       image of the float values
 * @todo Provide a multi-content query -- with the SOAP-attachment strategy, we
 *       can very well/easily deliver multiple histograms/images within one
 *       message. Let's do it...(requires a smart updated API).
 *
 * @author Jochen Küpper
 */
class SoapServer : public QThread
{
  /** be friend with the soap instances */
  friend class ::CASSsoapService;

public:
  /** a shared pointer of this class */
  typedef std::tr1::shared_ptr<SoapServer> shared_pointer;

  /** create the instance if not it does not exist already
   *
   * @return the instance to this server
   * @param event The event getter functor
   * @param hist The hist getter functor
   * @param port The port that the soap instance is running on. Default is 12321
   */
  static shared_pointer instance(const EventGetter& event,
                                 const HistogramGetter& hist,
                                 size_t port=12321);

protected:
  /** perform thread-work
   *
   * Sets up and connects to the soap service then waits until the soap service
   * get an information. Copies the soap instance and create an new handler to
   * serve the soap request.
   *
   * The handler is put into a QThreadPool which will automatically destroy it
   * once one is done serving.
   */
  virtual void run();

  /** return existing instance for our friends
   *
   * if it doesn't exist, throw exception
   */
  static shared_pointer instance()
  {
    QMutexLocker locker(&_mutex);
    if(!_instance)
      throw std::logic_error("SoapServer does not exist");
    return _instance;
  }

  /** get_event functor */
  const EventGetter& get_event;

  /** get_histogram functor */
  const HistogramGetter& get_histogram;

  /** the service */
  CASSsoapService *_soap;

  /** maximum backlog of open requests */
  static const size_t _backlog;

  /** server port */
  const size_t _port;

private:
  /** Constructor
   *
   * sets up the initial values
   *
   * @param event The event getter functor
   * @param hist The histogram getter functor
   * @param port The port that the soap will run on
   * @param parent The Qt parent object that this class belong to
   */
  SoapServer(const EventGetter& event, const HistogramGetter& hist, size_t port, QObject *parent=0)
    : QThread(parent), get_event(event), get_histogram(hist),
      _soap(new CASSsoapService), _port(port)
  {
    VERBOSEOUT(std::cout << "SoapServer starting on port " << _port << std::endl);
  }

  /** Disabled default constructor */
  SoapServer();

  /** Disabled copy constructor */
  SoapServer(const SoapServer&);

  /** Disabled assignment */
  SoapServer& operator=(const SoapServer&);

public:
  /** Destructor
   *
   * clean up SOAP by destroying and deleting the soap instance
   */
  ~SoapServer() { _soap->destroy(); delete _soap; }

private:
  /** pointer to the singleton instance */
  static shared_pointer _instance;

  /** Singleton operation locker */
  static QMutex _mutex;
};

} //end namespace cass
#endif



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-style: "Stroustrup"
// c-file-offsets: ((c . 0) (innamespace . 0))
// fill-column: 100
// End:
