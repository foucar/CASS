// Copyright (C) 2010 lmf
// Copyright (C) 2010 Jochen Küpper

#include <QtCore/QMutex>
#include <QtCore/QSettings>

#include <cassert>
#include <stdexcept>
#include <algorithm>

#include "postprocessing/postprocessor.h"
#include "postprocessing/ccd.h"
#include "postprocessing/waveform.h"
#include "postprocessing/alignment.h"


namespace cass
{


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
//===============================================================



/**! Internal helper function to  convert QVariant to id_t */
static inline PostProcessors::id_t QVarianttoId_t(QVariant i)
{
    return PostProcessors::id_t(i.toInt());
}






PostProcessors::PostProcessors()
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


void PostProcessors::loadSettings(size_t)
{
    QSettings settings;
    settings.beginGroup("postprocessors");
    QVariantList list(settings.value("active").toList());
    _active.resize(list.size());
    std::transform(list.begin(), list.end(), _active.begin(), QVarianttoId_t);
    // remove duplicates (keep first occurence)
    _active.unique();
    setup();
}


void cass::PostProcessors::setup()
{
    using namespace std;
    // delete all unused PostProcessors
    for(postprocessors_t::iterator iter = _postprocessors.begin(); iter != _postprocessors.end(); ++iter)
        if(_active.end() != find(_active.begin(), _active.end(), iter->first))
            _postprocessors.erase(iter);
    // Add newly added PostProcessors -- for histograms we simply make sure the pointer is 0 and let
    // the postprocessor correctly initialize it whenever it wants to
    list<id_t>::iterator iter(_active.begin());
    while(iter != _active.end()) {
        // check that the postprocessor is not already implemented
        if(_postprocessors.end() == _postprocessors.find(*iter)) {
            // create postprocessor
            _histograms[*iter] = 0;
            _postprocessors[*iter] = create(_histograms, *iter);
            // check for dependencies; if there are any open dependencies put all of them in front
            // of us
            bool update(false);
            list<id_t> deps(_postprocessors[*iter]->dependencies());
            for(list<id_t>::iterator d=deps.begin(); d!=deps.end(); ++d) {
                if(_postprocessors.end() != _postprocessors.find(*d)) {
                    _active.insert(iter, *d);
                    list<id_t>::iterator remove(find(iter, _active.end(), *d));
                    if(_active.end() != remove)
                        _active.erase(remove);
                    update = true;
                }
            }
            // if we have updated _active, start over again
            if(update) {
                // start over
                iter = _active.begin();
                continue;
            }
        }
        ++iter;
    }
}


cass::PostprocessorBackend * cass::PostProcessors::create(histograms_t hs, id_t id)
{
  PostprocessorBackend * processor(0);
  switch(id)
  {
  case Pnccd1LastImage:
  case Pnccd2LastImage:
    processor = new pp1(hs, id);
    break;
  case Pnccd1BinnedRunningAverage:
  case Pnccd1BackgroundCorrectedBinnedRunnngAverage:
    processor = new pp101(hs, id);
    break;
  case CampChannel00LastWaveform:
  case CampChannel01LastWaveform:
  case CampChannel02LastWaveform:
  case CampChannel03LastWaveform:
  case CampChannel04LastWaveform:
  case CampChannel05LastWaveform:
  case CampChannel06LastWaveform:
  case CampChannel07LastWaveform:
  case CampChannel08LastWaveform:
  case CampChannel09LastWaveform:
  case CampChannel10LastWaveform:
  case CampChannel11LastWaveform:
  case CampChannel12LastWaveform:
  case CampChannel13LastWaveform:
  case CampChannel14LastWaveform:
  case CampChannel15LastWaveform:
  case CampChannel16LastWaveform:
  case CampChannel17LastWaveform:
  case CampChannel18LastWaveform:
  case CampChannel19LastWaveform:
    processor = new pp4(hs,id);
    break;
  case CampChannel00AveragedWaveform:
  case CampChannel01AveragedWaveform:
  case CampChannel02AveragedWaveform:
  case CampChannel03AveragedWaveform:
  case CampChannel04AveragedWaveform:
  case CampChannel05AveragedWaveform:
  case CampChannel06AveragedWaveform:
  case CampChannel07AveragedWaveform:
  case CampChannel08AveragedWaveform:
  case CampChannel09AveragedWaveform:
  case CampChannel10AveragedWaveform:
  case CampChannel11AveragedWaveform:
  case CampChannel12AveragedWaveform:
  case CampChannel13AveragedWaveform:
  case CampChannel14AveragedWaveform:
  case CampChannel15AveragedWaveform:
  case CampChannel16AveragedWaveform:
  case CampChannel17AveragedWaveform:
  case CampChannel18AveragedWaveform:
  case CampChannel19AveragedWaveform:
    processor = new pp500(hs,id);
    break;
  default:
    throw std::invalid_argument("Postprocessor id not available");
  }
  return processor;
}

} // end namespace cass



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
