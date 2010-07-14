// Copyright (C) 2010 Nicola Coppola
// Copyright (C) 2010 Lutz Foucar

/**
 * @file daemon.cpp file contains definition of unix signals handler
 *
 * @author Nicola Coppola
 */

#include "daemon.h"

#include <iostream>
#include <stdio.h>
#include <signal.h>


//initialize static memebers
int cass::UnixSignalDaemon::sigquitFd[2];
int cass::UnixSignalDaemon::sigtermFd[2];

//installation of Unix singal handlers//
int cass::setup_unix_signal_handlers()
{
  struct sigaction quit, term;
  VERBOSEOUT(std::cout << "Daemon: what to do in case signal to exit "<<std::endl);

  quit.sa_handler = cass::UnixSignalDaemon::quitSignalHandler;
  sigemptyset(&quit.sa_mask);
  quit.sa_flags = SA_RESTART;

  if (sigaction(SIGQUIT, &quit, 0) > 0)
    return 1;

  VERBOSEOUT(std::cout << "Daemon: what to do in case signal to quit "<<std::endl);
  term.sa_handler = cass::UnixSignalDaemon::termSignalHandler;
  sigemptyset(&term.sa_mask);
  term.sa_flags = SA_RESTART;

  if (sigaction(SIGTERM, &term, 0) > 0)
    return 2;

  return 0;
}

cass::UnixSignalDaemon::UnixSignalDaemon(QObject *parent)
  :QObject(parent)
{
  if (::socketpair(AF_UNIX, SOCK_STREAM, 0, sigquitFd))
      qFatal("Couldn't create QUIT socketpair");

  if (::socketpair(AF_UNIX, SOCK_STREAM, 0, sigtermFd))
      qFatal("Couldn't create TERM socketpair");
  snQuit = new QSocketNotifier(sigquitFd[1], QSocketNotifier::Read, this);
  connect(snQuit, SIGNAL(activated(int)), this, SLOT(handleSigQuit()));
  snTerm = new QSocketNotifier(sigtermFd[1], QSocketNotifier::Read, this);
  connect(snTerm, SIGNAL(activated(int)), this, SLOT(handleSigTerm()));
}


/* See daemon.h for details of that's going on here */
void cass::UnixSignalDaemon::quitSignalHandler(int)
{
  std::cout<<"Daemon::quitSignalHandler(): quit signal seen"<<std::endl;
  char a = 1;
  ssize_t r(::write(sigquitFd[0], &a, sizeof(a)));
  if ( r < 0 ) {
    std::cout << "Daemon::quitSignalHandler(): Quit signal write failed" << std::endl;
  }
}

void cass::UnixSignalDaemon::handleSigQuit()
{
  std::cout<<"Daemon::handleSigQuit(): quit signal handle"<<std::endl;
  snQuit->setEnabled(false);
  char tmp;
  ssize_t r(::read(sigquitFd[1], &tmp, sizeof(tmp)));
  if ( r < 0 ) {
    std::cout << "Daemon::handleSigQuit(): Quit signal read failed" << std::endl;
  }
  // do Qt stuff
  emit QuitSignal();
  snQuit->setEnabled(true);
}


void cass::UnixSignalDaemon::termSignalHandler(int)
{
  std::cout<<"Daemon::termSignalHandler(): Quitterm signal seen"<<std::endl;
  char a = 1;
  ssize_t r(::write(sigtermFd[0], &a, sizeof(a)));
  if ( r < 0 ) {
    std::cout << "Daemon::termSignalHandler(): Term signal write failed" << std::endl;
  }
}


void cass::UnixSignalDaemon::handleSigTerm()
{
  std::cout<<"Daemon::handleSigTerm(): term signal handle"<<std::endl;
  snTerm->setEnabled(false);
  char tmp;
  ssize_t r(::read(sigtermFd[1], &tmp, sizeof(tmp)));
  if ( r < 0 ) {
    std::cout << "Daemon::handleSigTerm(): Term signal read failed" << std::endl;
  }

  // do Qt stuff
  emit TermSignal();

  snTerm->setEnabled(true);
}
