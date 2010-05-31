//Copyright (C) 2010 Lutz Foucar

#include <QtCore/QSettings>
#include <cmath>

#include "tais_helper.h"
#include "acqiris_device.h"
#include "pixel_detector.h"
#include "cass.h"
#include "histogram.h"

//initialize static members//
QMutex cass::Tais::TaisHelper::_mutex;
cass::Tais::TaisHelper *cass::Tais::TaisHelper::_instance(0);

cass::Tais::TaisHelper* cass::Tais::TaisHelper::instance()
{
  //lock this//
  QMutexLocker lock(&_mutex);
  //check if an instance of the helper class already exists//
  //return it, otherwise create one and return it//
  if (0 == _instance)
  {
    VERBOSEOUT(std::cout << "creating an instance of the Tais Helper"
               <<std::endl);
    _instance = new TaisHelper();
  }
  return _instance;
}

void cass::Tais::TaisHelper::destroy()
{
  //lock this//
  QMutexLocker lock(&_mutex);
  delete _instance;
  _instance=0;
}


cass::Tais::TaisHelper::TaisHelper()
{
  using namespace std;
  loadSettings();
  for (size_t i=0 ; i<NbrOfWorkers ; ++i)
    _conditionList.push_back(make_pair(0,false));
}

void cass::Tais::TaisHelper::loadSettings()
{
  using namespace std;
  QSettings settings;
  settings.beginGroup("TaisSpecial");
  _conditiontype = static_cast<ConditionType>(settings.value("ConditionType",0).toUInt());
  _tofBound = make_pair(settings.value("TofLowerBound",0).toFloat(),
                        settings.value("TofUpperBound",0).toFloat());
  _tofCond = make_pair(settings.value("TofCondRangeLow",0).toFloat(),
                       settings.value("TofCondRangeUp",0).toFloat());
  _ccdBound = make_pair(make_pair(settings.value("CCDXLowerLeftXBound",0).toFloat(),
                                  settings.value("CCDXLowerLeftYBound",0).toFloat()),
                        make_pair(settings.value("CCDXUpperRightXBound",0).toFloat(),
                                  settings.value("CCDXUpperRightYBound",0).toFloat()));
  _ccdBound2 = make_pair(make_pair(settings.value("CCDXLowerLeftXBoundTwo",0).toFloat(),
                                   settings.value("CCDXLowerLeftYBoundTwo",0).toFloat()),
                         make_pair(settings.value("CCDXUpperRightXBoundTwo",0).toFloat(),
                                   settings.value("CCDXUpperRightYBoundTwo",0).toFloat()));
  _ccdCond = make_pair(settings.value("CCDCondRangeLow",0).toFloat(),
                       settings.value("CCDCondRangeUp",0).toFloat());

}

bool cass::Tais::TaisHelper::process(const CASSEvent& evt)
{
  bool cond(false);
  using namespace std;
  switch(_conditiontype)
  {
  case Tof:
    {
      using namespace cass::ACQIRIS;

      const cass::ACQIRIS::Instruments _instrument (Camp1);
      const size_t _channel(1);

      const Device *dev
          (dynamic_cast<const Device*>(evt.devices().find(CASSEvent::Acqiris)->second));
      //retrieve a reference to the right instument//
      Device::instruments_t::const_iterator instrIt (dev->instruments().find(_instrument));
      //check if instrument exists//
      if (dev->instruments().end() == instrIt)
        throw std::runtime_error(QString("TaisHelper::process(): Data doesn't contain Instrument %1")
                                 .arg(_instrument).toStdString());
      const Instrument &instr(instrIt->second);
      //retrieve a reference to the right channel//
      if (instr.channels().size() <= _channel)
        throw std::runtime_error(QString("TaisHelper: Instrument %1 doesn't contain channel %2")
                                 .arg(_instrument)
                                 .arg(_channel).toStdString());
      const Channel &channel (instr.channels()[_channel]);
      const double gain(channel.gain());
      const double offset(channel.offset());
      const double sampInterval(channel.sampleInterval());
      //retrieve a reference to the waveform of the channel//
      const waveform_t &waveform (channel.waveform());
      float integral (0);
      size_t beginoffset(_tofBound.first / sampInterval );
      size_t endoffset(_tofBound.second / sampInterval );
      waveform_t::const_iterator it(waveform.begin() + beginoffset);
      waveform_t::const_iterator end(waveform.begin() + endoffset);
      for (; it !=end;++it)
        integral += *it * gain - offset;

      cond = ((_tofCond.first < integral) && (integral < _tofCond.second));

//      std::cout << "TaisHelper::process(): integral "<<integral
//          <<" "<<_tofCond.first
//          <<" "<<_tofCond.second
//          <<" "<<(_tofCond.first < integral)
//          <<" "<<(integral < _tofCond.second)
//          <<" "<<cond
//          <<std::endl;
    }
    break;
  case PnCCD:
    {
      cass::CASSEvent::Device _device (CASSEvent::pnCCD);
      size_t _detector(0);
      if (evt.devices().find(_device)->second->detectors()->size() <= _detector)
        throw std::runtime_error(QString("TaisHelper: Detector %1 does not exist in Device %2")
                                 .arg(_detector)
                                 .arg(_device).toStdString());
      const PixelDetector::frame_t &frame
          ((*(evt.devices().find(_device)->second)->detectors())[_detector].frame());

      float integral (0);
      const size_t colwidth ((*(evt.devices().find(_device)->second)->detectors())[_detector].columns());
      const size_t mincol (static_cast<size_t>(_ccdBound.first.first));
      const size_t minrow (static_cast<size_t>(_ccdBound.first.second));
      const size_t maxcol (static_cast<size_t>(_ccdBound.second.first));
      const size_t maxrow (static_cast<size_t>(_ccdBound.second.second));
      for (size_t row(minrow); row <maxrow;++row)
        for (size_t col(mincol); col <maxcol;++col)
        integral += frame[row*colwidth + col];
      float integral2 (0);
      const size_t mincol2 (static_cast<size_t>(_ccdBound2.first.first));
      const size_t minrow2 (static_cast<size_t>(_ccdBound2.first.second));
      const size_t maxcol2 (static_cast<size_t>(_ccdBound2.second.first));
      const size_t maxrow2 (static_cast<size_t>(_ccdBound2.second.second));
      for (size_t row(minrow2); row <maxrow2;++row)
        for (size_t col(mincol2); col <maxcol2;++col)
        integral2 += frame[row*colwidth + col];

      cond = ((_ccdCond.first < (abs(integral)/abs(integral2))) && ((abs(integral)/abs(integral2)) < _ccdCond.second));
      if (cond)std::cout << (abs(integral)/abs(integral2))<<std::endl;
    }
    break;
  case PnCCDPhotonhit:
    {
      cass::CASSEvent::Device _device (CASSEvent::pnCCD);
      size_t _detector(0);
      if (evt.devices().find(_device)->second->detectors()->size() <= _detector)
        throw std::runtime_error(QString("TaisHelper: Detector %1 does not exist in Device %2")
                                 .arg(_detector)
                                 .arg(_device).toStdString());
      const PixelDetector::pixelList_t pixellist
          ((*(evt.devices().find(_device)->second)->detectors())[_detector].pixellist());
      PixelDetector::pixelList_t::const_iterator it
          (pixellist.begin());

      float integral (0);
      float integral2 (0);
      for (;it != pixellist.end() ; ++it)
      {
        if( (it->x() >= _ccdBound.first.first)  &&
            (it->x() <= _ccdBound.second.first) &&
            (it->y() >= _ccdBound.first.second) &&
            (it->y() <= _ccdBound.second.second) )
          integral += 1.;
        if( (it->x() >= _ccdBound2.first.first)  &&
            (it->x() <= _ccdBound2.second.first) &&
            (it->y() >= _ccdBound2.first.second) &&
            (it->y() <= _ccdBound2.second.second) )
          integral2 += 1.;
      }

      cond = integral > integral2;
      if(cond)std::cout << integral << " " << integral2 <<std::endl;
    }
    break;
  case VMI:
    {
      cass::CASSEvent::Device _device (CASSEvent::CCD);
      size_t _detector(0);
      if (evt.devices().find(_device)->second->detectors()->size() <= _detector)
        throw std::runtime_error(QString("TaisHelper: Detector %1 does not exist in Device %2")
                                 .arg(_detector)
                                 .arg(_device).toStdString());
      const PixelDetector::frame_t &frame
          ((*(evt.devices().find(_device)->second)->detectors())[_detector].frame());

      float integral (0);
      const size_t colwidth ((*(evt.devices().find(_device)->second)->detectors())[_detector].columns());
      const size_t mincol (static_cast<size_t>(_ccdBound.first.first));
      const size_t minrow (static_cast<size_t>(_ccdBound.first.second));
      const size_t maxcol (static_cast<size_t>(_ccdBound.second.first));
      const size_t maxrow (static_cast<size_t>(_ccdBound.second.second));
      for (size_t row(minrow); row <maxrow;++row)
        for (size_t col(mincol); col <maxcol;++col)
        integral += frame[row*colwidth + col];

      cond = ((_ccdCond.first < integral) && (integral < _ccdCond.second));
    }
    break;
  default:
    break;
  }

//  std::cout << "TaisHelper::process(): retrun "<<cond<<std::endl;
  return cond;
}





// *** postprocessors 51 calcs integral over a region in 1d histo ***

cass::pp4000::pp4000(PostProcessors& pp, const cass::PostProcessors::key_t &key)
  : PostprocessorBackend(pp, key), _result(new Histogram0DFloat())
{
  _pp.histograms_replace(_key,_result);
}

cass::pp4000::~pp4000()
{
  _pp.histograms_delete(_key);
  _result = 0;
}

void cass::pp4000::operator()(const CASSEvent& evt)
{
  using namespace std;
  _result->lock.lockForWrite();
  *_result = cass::Tais::TaisHelper::instance()->condition(evt);
  _result->lock.unlock();
}

