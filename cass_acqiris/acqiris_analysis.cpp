// Copyright (C) 2010 lmf

#include "acqiris_analysis.h"


//void cass::ACQIRIS::Parameter::load()
//{
// /* //string for the container index//
//  QString s;
//  //sync before loading//
//  sync();
//
//  //delete the previous detector parameters//
//  for (AcqirisDevice::detectors_t::iterator it=_detectors.begin();it!=_detectors.end();++it)
//    delete (*it);
//  _detectors.clear();
//
//  //the detector parameters//
//  beginGroup("DetectorContainer");
//  for (size_t i = 0; i < value("size",1).toUInt();++i)
//  {
//    beginGroup(s.setNum(static_cast<uint32_t>(i)));
//    //find out which type the detector is//
//    DetectorType dettype = static_cast<DetectorType>(value("DetectorType",Delayline).toInt());
//    //create a new detector according to the detectortype//
//    switch(dettype)
//    {
//    case Delayline : _detectors.push_back(new DelaylineDetector()); break;
//    default:
//      std::cerr<<"Acqris Analyzer: Detectortype \""<<dettype<<"\" is unknown"<<std::endl;
//      endGroup();
//      continue;
//      break;
//    }
//    //load the parameters of the detector//
//    _detectors[i]->loadParameters(this);
//    endGroup(); //QString(i)
//  }
// */ endGroup();//detectorcontainer
//}
//
//void cass::ACQIRIS::Parameter::save()
//{
///*  //string for the container index//
//  QString s;
//  //the detector parameters//
//  beginGroup("DetectorContainer");
//  //how many detectors are there//
//  setValue("size",static_cast<uint32_t>(_detectors.size()));
//  for (size_t i = 0; i < _detectors.size();++i)
//  {
//    beginGroup(s.setNum(static_cast<uint32_t>(i)));
//    _detectors[i]->saveParameters(this);
//    endGroup();
//  }
//*/  endGroup();//detectorcontainer
//}











cass::ACQIRIS::Analysis::Analysis()
{
//  //create the map with the waveform analyzers//
//  _waveformanalyzer[cfd8]  = new CFD8Bit();
//  _waveformanalyzer[cfd16] = new CFD16Bit();
//  _waveformanalyzer[com8]  = new CoM8Bit();
//  _waveformanalyzer[com16] = new CoM16Bit();
//
//  //create the map with the detector analyzers//
//  _detectoranalyzer[DelaylineSimple] = new DelaylineDetectorAnalyzerSimple(&_waveformanalyzer);
}


void cass::ACQIRIS::Analysis::operator()(cass::CASSEvent* cassevent)
{
/*  //get the remievent from the cassevent//
  AcqirisDevice* dev =
      dynamic_cast<AcqirisDevice*>(cassevent->devices()[cass::CASSEvent::Acqiris]);

  //get a reference to the detector and channel container//
  const AcqirisDevice::channels_t &chans = dev->channels();
  AcqirisDevice::detectors_t &dets       = dev->detectors();

  //ignore event if it is not initialized (there are no channels present)//
  if (chans.size())
  {
    //copy the parameters to the event//
    if(dev->detectors().size() !=_param._detectors.size())
    {
      //when both have not the same size, then delete all detectors in//
      //the device and create them new from the parameters//
      for (AcqirisDevice::detectors_t::iterator it=dets.begin();it!=dets.end();++it)
        delete (*it);
      dets.clear();
      for (AcqirisDevice::detectors_t::iterator it=_param._detectors.begin();
           it!=_param._detectors.end();
           ++it)
      {
        switch((*it)->type())
        {
        case Delayline : dets.push_back(new DelaylineDetector(*dynamic_cast<DelaylineDetector*>(*it))); break;
        default: break;
        }
      }
    }
    else
    {
      //otherwise go through all detectors and check whether they are//
      //the same type//
      AcqirisDevice::detectors_t::const_iterator iParDet= _param._detectors.begin();
      AcqirisDevice::detectors_t::iterator       iDevDet= dets.begin();
      for (;iParDet != _param._detectors.end();++iParDet,++iDevDet)
      {
        if ( (*iParDet)->type() != (*iDevDet)->type() )
        {
          //if they are not the same type, then delete the det in the device//
          //and create a new one from the parameter//
          delete (*iDevDet);

          switch((*iParDet)->type())
          {
          case Delayline : (*iDevDet) 
                             = new DelaylineDetector(*dynamic_cast<DelaylineDetector*>(*iParDet)); break;
          default: break;
          }
        }
        else
        {
          //otherwise just copy the info from the parameter detector//
          //to the device detector//
          (*(*iDevDet)) = (*(*iParDet));
        }
      }
    }


    //analyze the detectors//
    //this has to be done for each detektor individually//
    for (size_t i=0; i<dev->detectors().size();++i)
    {
      //retrieve reference to the detector//
      DetectorBackend &det = *dev->detectors()[i];
      //analyze the detector using the requested analyzer//
      _detectoranalyzer[det.analyzerType()]->analyze(det, dev->channels());
    }
  }
*/
}



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
