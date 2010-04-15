// Copyright (C) 2010 NC POSIX signal treatment
#include "daemon.h"
int cass::Daemon::sigquitFd[2];
int cass::Daemon::sigtermFd[2];
#include <iostream>

cass::Daemon::Daemon(QObject *parent)
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

cass::Daemon::~Daemon()
{
}

void cass::Daemon::quitSignalHandler(int)
{
  std::cout<<"quit signal seen"<<std::endl;
  char a = 1;
  ::write(sigquitFd[0], &a, sizeof(a));
}

void cass::Daemon::handleSigQuit()
{
  std::cout<<"quit signal handle"<<std::endl;
  snQuit->setEnabled(false);
  char tmp;
  ::read(sigquitFd[1], &tmp, sizeof(tmp));
  // do Qt stuff
  emit QuitSignal();
  snQuit->setEnabled(true);
}


void cass::Daemon::termSignalHandler(int)
{
  std::cout<<"term signal seen"<<std::endl;
  char a = 1;
  ::write(sigtermFd[0], &a, sizeof(a));
}


void cass::Daemon::handleSigTerm()
{
  std::cout<<"term signal handle"<<std::endl;
  snTerm->setEnabled(false);
  char tmp;
  ::read(sigtermFd[1], &tmp, sizeof(tmp));

  // do Qt stuff
  emit TermSignal();

  snTerm->setEnabled(true);
}

