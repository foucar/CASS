// CASS TCP server
//
// Copyright (C) 2010 Jochen Küpper
// Copyright (C) 2011 - 2013 Lutz Foucar

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
#include <QtCore/QMutex>
#include <QtCore/QRunnable>

#include "cass_event.h"
#include "soapCASSsoapService.h"


namespace cass
{

/** Handle a single SOAP request
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
   * @param port The port that the soap instance is running on. Default is 12321
   */
  static shared_pointer instance(size_t port=12321);

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
   * @param port The port that the soap will run on
   * @param parent The Qt parent object that this class belong to
   */
  SoapServer(size_t port, QObject *parent=0);

  /** Disabled default constructor */
  SoapServer();

  /** Disabled copy constructor */
  SoapServer(const SoapServer&);

  /** Disabled assignment */
  SoapServer& operator=(const SoapServer&);

public:
  /** Destructor
   *
   * Check whether thread is still running. If so terminate it and wait until
   * it finished. Then clean up SOAP by destroying and deleting the soap 
   * instance.
   */
  ~SoapServer();

private:
  /** pointer to the singleton instance */
  static shared_pointer _instance;

  /** Singleton operation locker */
  static QMutex _mutex;
};

} //end namespace cass
#endif
