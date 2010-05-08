// Copyright (C) 2010 Nicola Coppola
// Copyright (C) 2010 Lutz Foucar

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
  VERBOSEOUT(std::cout << "what to do in case signal to exit "<<std::endl;)

  quit.sa_handler = cass::UnixSignalDaemon::quitSignalHandler;
  sigemptyset(&quit.sa_mask);
  quit.sa_flags = 0;
  quit.sa_flags |= SA_RESTART;

  if (sigaction(SIGQUIT, &quit, 0) > 0)
    return 1;

  VERBOSEOUT(std::cout << "what to do in case signal to quit "<<std::endl;)
  term.sa_handler = cass::UnixSignalDaemon::termSignalHandler;
  sigemptyset(&term.sa_mask);
  term.sa_flags |= SA_RESTART;

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


void cass::UnixSignalDaemon::quitSignalHandler(int)
{
  std::cout<<"quit signal seen"<<std::endl;
  char a = 1;
  ::write(sigquitFd[0], &a, sizeof(a));
}

void cass::UnixSignalDaemon::handleSigQuit()
{
  std::cout<<"quit signal handle"<<std::endl;
  snQuit->setEnabled(false);
  char tmp;
  ::read(sigquitFd[1], &tmp, sizeof(tmp));
  // do Qt stuff
  emit QuitSignal();
  snQuit->setEnabled(true);
}


void cass::UnixSignalDaemon::termSignalHandler(int)
{
  std::cout<<"term signal seen"<<std::endl;
  char a = 1;
  ::write(sigtermFd[0], &a, sizeof(a));
}


void cass::UnixSignalDaemon::handleSigTerm()
{
  std::cout<<"term signal handle"<<std::endl;
  snTerm->setEnabled(false);
  char tmp;
  ::read(sigtermFd[1], &tmp, sizeof(tmp));

  // do Qt stuff
  emit TermSignal();

  snTerm->setEnabled(true);
}

