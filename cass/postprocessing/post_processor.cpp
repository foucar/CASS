// Copyright (C) 2010 lmf
// Copyright (C) 2010 Jochen Küpper

#include <QtCore/QMutex>
#include <QtCore/QSettings>

#include <cassert>
#include <stdexcept>
#include <algorithm>

#include "post_processor.h"
#include "ccd_postprocessor.h"
#include "waveform_postprocessor.h"
#include "alignment_postprocessor.h"


// ============define static members (do not touch)==============
cass::PostProcessors *cass::PostProcessors::_instance(0);
QMutex cass::PostProcessors::_mutex;

// create an instance of the singleton
cass::PostProcessors *cass::PostProcessors::instance()
{
  QMutexLocker locker(&_mutex);
  if(0 == _instance)
    _instance = new PostProcessors();
  return _instance;
}
// destroy the instance of the singleton
void cass::PostProcessors::destroy()
{
  QMutexLocker locker(&_mutex);
  delete _instance;
  _instance = 0;
}
// file-local helper function -- convert QVariant to id_t
static inline cass::PostProcessors::id_t QVarianttoId_t(QVariant i)
{
  return cass::PostProcessors::id_t(i.toInt());
}
//===============================================================





cass::PostProcessors::PostProcessors()
{
  // set up list of all active postprocessors/histograms
  // and fill maps of histograms and postprocessors
  loadSettings(0);
}


void cass::PostProcessors::process(cass::CASSEvent& event)
{
  for(std::list<id_t>::iterator iter(_active.begin()); iter != _active.end(); ++iter)
    (*(_postprocessors[*iter]))(event);
}


void cass::PostProcessors::loadSettings(size_t)
{
  QSettings settings;
  settings.beginGroup("postprocessors");
  QVariantList list(settings.value("active").toList());
  _active.resize(list.size());
  std::transform(list.begin(), list.end(), _active.begin(), QVarianttoId_t);
  setup();
}


void cass::PostProcessors::setup()
{
  // delete all unused PostProcessors
  for(postprocessors_t::iterator iter = _postprocessors.begin(); iter != _postprocessors.end(); ++iter)
    if(_active.end() != find(_active.begin(), _active.end(), iter->first))
      _postprocessors.erase(iter);
  // Add newly added PostProcessors -- for histograms we simply make sure the pointer is 0 and let
  // the postprocessor correctly initialize it whenever it wants to
  for(std::list<id_t>::iterator iter = _active.begin(); iter != _active.end(); ++iter) 
  {
    if(_postprocessors.end() == _postprocessors.find(*iter)) 
    {
      _histograms[*iter] = 0;
      _postprocessors[*iter] = create(_histograms, *iter);
    }
  }
}


cass::PostprocessorBackend * cass::PostProcessors::create(histograms_t hs, id_t id)
{
  PostprocessorBackend * processor(0);
  switch(id) 
  {
  case Pnccd1LastImage:
  case Pnccd2LastImage:
    processor = new PostprocessorPnccdLastImage(hs, id);
    break;
  default:
    throw std::invalid_argument("Impossible postprocessor id for PostprocessorPnccdLastImage");
  }
  return processor;
}

// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
