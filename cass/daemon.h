// Copyright (C) 2010 NC POSIX signal treatment
#ifndef CASS_DAEMON_H
#define CASS_DAEMON_H

#include <QtCore/QObject>
#include <QThread>
#include <QMutex>
#include <QtCore/QSocketNotifier>

#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "cass.h"

namespace cass
{
  class CASSSHARED_EXPORT Daemon : public QThread
  {
    Q_OBJECT;
  public:
    Daemon(QObject *parent=0);
    ~Daemon();

    //void run();
    // Unix signal handlers.
    static void quitSignalHandler(int unused);
    static void termSignalHandler(int unused);

  signals:
      void QuitSignal(); 
      void TermSignal(); 

  public slots:
    //void end();
    // Qt signal handlers.
    void handleSigQuit();
    void handleSigTerm();

  private:
    static int                           sigquitFd[2];
    static int                           sigtermFd[2];

    QSocketNotifier                     *snQuit;
    QSocketNotifier                     *snTerm;
  };

}//end namespace cass

#endif

