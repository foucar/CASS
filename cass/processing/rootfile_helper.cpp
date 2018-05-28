// Copyright (C) 2011 Lutz Foucar

/**
 * @file rootfile_helper.cpp contains singleton definition for creating root files
 *
 * @author Lutz Foucar
 */

#include <sstream>
#include <algorithm>
#include <tr1/functional>

#include <QtCore/QMutexLocker>

#include <TFile.h>

#include "rootfile_helper.h"

using namespace cass;
using namespace std;
using std::tr1::bind;
using std::tr1::placeholders::_1;

ROOTFileHelper::rootfiles_t ROOTFileHelper::_rootfiles;
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

void ROOTFileHelper::close(TFile *rootfile)
{
  QMutexLocker lock(&_mutex);
  rootfiles_t::iterator iFile(
      find_if(_rootfiles.begin(),_rootfiles.end(),
              std::tr1::bind<bool>(equal_to<TFile*>(),rootfile,
                         std::tr1::bind<TFile*>(&rootfiles_t::value_type::second,_1))));
  if (iFile != _rootfiles.end())
  {
    rootfile->SaveSelf();
    rootfile->Close();
    iFile->second = 0;
  }
}
