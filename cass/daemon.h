// Copyright (C) 2010 Nicola Coppola
// Copyright (C) 2010 Lutz Foucar

/**
 * @file daemon.h file contains declaration of unix signals handler
 *
 * @author Nicola Coppola
 */

#ifndef _CASS_DAEMON_H_
#define _CASS_DAEMON_H_

#include <QtCore/QObject>
#include <QtCore/QSocketNotifier>

#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "cass.h"

namespace cass
{
  /** install unix signals.
   *
   * function that will install Unix signal handlers with sigaction(2)
   * This function will setup SIGTERM and SIGQUIT to be handled by
   * QSocketNotifiers. They will then be used to signal our program
   * using the qt signal - slot mechanism. @see UnixSignalDaemon
   * This implemented from the example found at
   * http://qt.nokia.com/doc/4.6/unix-signals.html
   */
  int setup_unix_signal_handlers();


  /** a deamon that will intercept chosen unix signals.

   * This will connect to the selected unix signals and emit
   * Qt Signals to be able to use the Qt Signal - Slot mechanism with them.
   * ideas taken from http://qt.nokia.com/doc/4.6/unix-signals.html.
   * currently it is implemented to intercept SIGQUIT and SIGTERM.
   *
   * @author Nicola Coppola
   */
  class CASSSHARED_EXPORT UnixSignalDaemon : public QObject
  {
    Q_OBJECT;
  public:
    /** constructor.
     *
     * Here we will create the socket notifieres and connect the unix signals
     * to them.
     *
     * @param parent The parent object that we belong to
     */
    UnixSignalDaemon(QObject *parent=0);

    //@{
    /** Unix signal handlers.
     *
     * @param unused this parameter is not used
     */
    static void quitSignalHandler(int unused);
    static void termSignalHandler(int unused);
    //@}

  signals:
    /** signal emitted when unix SIGQUIT has been send*/
    void QuitSignal();

    /** signal emitted when unix SIGTERM has been send*/
    void TermSignal();

  public slots:
    //@{
    /** slots to handle the UNIX signal*/
    void handleSigQuit();
    void handleSigTerm();
    //@}

  private:
    /** the socket descriptor for SIGQUIT*/
    static int sigquitFd[2];

    /** the socket descriptor for SIGTERM*/
    static int sigtermFd[2];

    /** the socket notfier that will handle SIGQUIT*/
    QSocketNotifier *snQuit;

    /** the socket notfier that will handle SIGTERM*/
    QSocketNotifier *snTerm;
  };

}//end namespace cass

#endif

