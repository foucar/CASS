// Copyright (C) 2011 Lutz Foucar

/**
 * @file rootfile_helper.h contains singleton definition for creating root files
 *
 * @author Lutz Foucar
 */

#include <sstream>

#include <QtCore/QMutexLocker>

#include <TFile.h>

#include "rootfile_helper.h"

using namespace cass;
using namespace std;

std::map<std::string,TFile *> ROOTFileHelper::_rootfiles;
QMutex ROOTFileHelper::_mutex;

TFile* ROOTFileHelper::create(const std::string& rootfilename, const std::string& options)
{
  QMutexLocker lock(&_mutex);
  if (_rootfiles.find(rootfilename) == _rootfiles.end())
  {
    _rootfiles[rootfilename] = TFile::Open(rootfilename.c_str(),options.c_str());
  }
  return _rootfiles[rootfilename];
}

