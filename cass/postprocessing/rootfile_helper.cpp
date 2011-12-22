// Copyright (C) 2011 Lutz Foucar

/**
 * @file rootfile_helper.h contains singleton definition for creating root files
 *
 * @author Lutz Foucar
 */

#include <sstream>
#include <algorithm>

#include <QtCore/QMutexLocker>

#include <TFile.h>

#include "rootfile_helper.h"

using namespace cass;
using namespace std;

namespace cass
{
template<class T>
struct map_data_compare :
    public binary_function<typename T::value_type,
                           typename T::mapped_type,
                           bool>
{
public:
  bool operator() (typename T::value_type &pair,
                   typename T::mapped_type i) const
  {
    return pair.second == i;
  }
};
}

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
  /** @todo use bind here */
  rootfiles_t::iterator iFile
      (find_if(_rootfiles.begin(),_rootfiles.end(),
               bind2nd(map_data_compare<rootfiles_t>(),rootfile)));
  if (iFile != _rootfiles.end())
  {
    rootfile->SaveSelf();
    rootfile->Close();
    iFile->second = 0;
  }
}
