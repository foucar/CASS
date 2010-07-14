// Copyright (C) 2009 Jochen Küpper
// Copyright (C) 2009,2010 Lutz Foucar

/**
 * @file format_converter.cpp file contains definition of the container for all
 *                           format converters
 *
 * @author Lutz Foucar
 */

#include <iostream>

#include <iomanip>
#include <fstream>
#include <QtCore/QMutexLocker>
#include "cass_event.h"
#include "format_converter.h"
#include "acqiris_converter.h"
#include "ccd_converter.h"
#include "machine_converter.h"
#include "pnccd_converter.h"
#include "xtciterator.h"
#include "pdsdata/xtc/Dgram.hh"


//create a black converter for all ids that we are not interested in//
namespace cass
{
  /** Converter that does nothing
   *
   * This converter is a blank that does nothing. It will be used for converting
   * xtc id's that we either don't care about or the user has disabled in .ini
   *
   * @author Lutz Foucar
   */
  class CASSSHARED_EXPORT BlankConverter : public ConversionBackend
  {
  public:
    /** do nothing */
    void operator()(const Pds::Xtc*, cass::CASSEvent*) {}
  };
}



// ===============define static members====================
cass::FormatConverter *cass::FormatConverter::_instance(0);
QMutex cass::FormatConverter::_mutex;

void cass::FormatConverter::destroy()
{
  QMutexLocker locker(&_mutex);
  delete _instance;
  _instance = 0;
}
cass::FormatConverter *cass::FormatConverter::instance()
{
  QMutexLocker locker(&_mutex);
  if(0 == _instance)
    _instance = new FormatConverter();
  return _instance;
}
//==========================================================







cass::FormatConverter::FormatConverter()
  :_configseen(false)
{
  // create all the necessary individual format converters
  _availableConverters[Acqiris]     = new ACQIRIS::Converter();
  _availableConverters[ccd]         = new CCD::Converter();
  _availableConverters[pnCCD]       = new pnCCD::Converter();
  _availableConverters[MachineData] = new MachineData::Converter();
  _availableConverters[Blank]       = new BlankConverter();

  // now initialze all xtc ids with blank converters to be on the save side//
  _usedConverters[Pds::TypeId::Any]                 = _availableConverters[Blank];
  _usedConverters[Pds::TypeId::Id_Xtc]              = _availableConverters[Blank];
  _usedConverters[Pds::TypeId::Id_Frame]            = _availableConverters[Blank];
  _usedConverters[Pds::TypeId::Id_AcqWaveform]      = _availableConverters[Blank];
  _usedConverters[Pds::TypeId::Id_AcqConfig]        = _availableConverters[Blank];
  _usedConverters[Pds::TypeId::Id_TwoDGaussian]     = _availableConverters[Blank];
  _usedConverters[Pds::TypeId::Id_Opal1kConfig]     = _availableConverters[Blank];
  _usedConverters[Pds::TypeId::Id_FrameFexConfig]   = _availableConverters[Blank];
  _usedConverters[Pds::TypeId::Id_EvrConfig]        = _availableConverters[Blank];
  _usedConverters[Pds::TypeId::Id_TM6740Config]     = _availableConverters[Blank];
  _usedConverters[Pds::TypeId::Id_ControlConfig]    = _availableConverters[Blank];
  _usedConverters[Pds::TypeId::Id_pnCCDframe]       = _availableConverters[Blank];
  _usedConverters[Pds::TypeId::Id_pnCCDconfig]      = _availableConverters[Blank];
  _usedConverters[Pds::TypeId::Id_Epics]            = _availableConverters[Blank];
  _usedConverters[Pds::TypeId::Id_FEEGasDetEnergy]  = _availableConverters[Blank];
  _usedConverters[Pds::TypeId::Id_EBeam]            = _availableConverters[Blank];
  _usedConverters[Pds::TypeId::Id_PhaseCavity]      = _availableConverters[Blank];
  _usedConverters[Pds::TypeId::Id_PrincetonFrame]   = _availableConverters[Blank];
  _usedConverters[Pds::TypeId::Id_PrincetonConfig]  = _availableConverters[Blank];
  _usedConverters[Pds::TypeId::Id_EvrData]          = _availableConverters[Blank];
  _usedConverters[Pds::TypeId::Id_FrameFccdConfig]  = _availableConverters[Blank];
  _usedConverters[Pds::TypeId::Id_FccdConfig]       = _availableConverters[Blank];
  _usedConverters[Pds::TypeId::Id_IpimbData]        = _availableConverters[Blank];
  _usedConverters[Pds::TypeId::Id_IpimbConfig]      = _availableConverters[Blank];
  _usedConverters[Pds::TypeId::Id_EncoderData]      = _availableConverters[Blank];
  _usedConverters[Pds::TypeId::Id_EncoderConfig]    = _availableConverters[Blank];

  //now load the converters, that the user want to use//
  loadSettings(0);
}

cass::FormatConverter::~FormatConverter()
{
  // destruct all the individual format converters
  for (availableConverters_t::iterator it=_availableConverters.begin();
       it!=_availableConverters.end();
       ++it)
    delete (it->second);
}

void cass::FormatConverter::addConverter(cass::FormatConverter::Converters converter)
{
  using namespace std;
  //look which converter should be added//
  switch(converter)
  {
  case ccd:
    VERBOSEOUT(cout<<"Use commercial CCD converter"<<endl);
    _usedConverters[Pds::TypeId::Id_Frame] = _availableConverters[ccd];
    break;
  case Acqiris:
    VERBOSEOUT(cout<<"Use acqiris converter"<<endl);
    _usedConverters[Pds::TypeId::Id_AcqWaveform] = _availableConverters[Acqiris];
    _usedConverters[Pds::TypeId::Id_AcqConfig]   = _availableConverters[Acqiris];
    break;
  case pnCCD:
    VERBOSEOUT(cout<<"Use pnCCD converter"<<endl);
    _usedConverters[Pds::TypeId::Id_pnCCDframe]  = _availableConverters[pnCCD];
    _usedConverters[Pds::TypeId::Id_pnCCDconfig] = _availableConverters[pnCCD];
    break;
  case MachineData:
    VERBOSEOUT(cout<<"Use commercial CCD converter"<<endl);
    _usedConverters[Pds::TypeId::Id_Epics]            = _availableConverters[MachineData];
    _usedConverters[Pds::TypeId::Id_FEEGasDetEnergy]  = _availableConverters[MachineData];
    _usedConverters[Pds::TypeId::Id_EBeam]            = _availableConverters[MachineData];
    _usedConverters[Pds::TypeId::Id_PhaseCavity]      = _availableConverters[MachineData];
    _usedConverters[Pds::TypeId::Id_EvrData]          = _availableConverters[MachineData];
    break;
  default:
    break;
  }
}

void cass::FormatConverter::removeConverter(cass::FormatConverter::Converters converter)
{
  using namespace std;
  //look which converter should be removed//
  switch(converter)
  {
  case ccd:
    VERBOSEOUT(cout<<"Don't use commercial CCD converter"<<endl);
    _usedConverters[Pds::TypeId::Id_Frame] = _availableConverters[Blank];
    break;
  case Acqiris:
    VERBOSEOUT(cout<<"Don't use acqiris converter"<<endl);
    _usedConverters[Pds::TypeId::Id_AcqWaveform]  = _availableConverters[Blank];
    _usedConverters[Pds::TypeId::Id_AcqConfig]    = _availableConverters[Blank];
    break;
  case pnCCD:
    VERBOSEOUT(cout<<"Don't use pnCCD converter"<<endl);
    _usedConverters[Pds::TypeId::Id_pnCCDframe]   = _availableConverters[Blank];
    _usedConverters[Pds::TypeId::Id_pnCCDconfig]  = _availableConverters[Blank];
    break;
  case MachineData:
    VERBOSEOUT(cout<<"Don't use machinedata converter"<<endl);
    _usedConverters[Pds::TypeId::Id_Epics]            = _availableConverters[Blank];
    _usedConverters[Pds::TypeId::Id_FEEGasDetEnergy]  = _availableConverters[Blank];
    _usedConverters[Pds::TypeId::Id_EBeam]            = _availableConverters[Blank];
    _usedConverters[Pds::TypeId::Id_PhaseCavity]      = _availableConverters[Blank];
    _usedConverters[Pds::TypeId::Id_EvrData]          = _availableConverters[Blank];
    break;
  default:
    break;
  }
}

void cass::FormatConverter::loadSettings(size_t)
{
  CASSSettings settings;
  settings.sync();
  settings.value("useCommercialCCDConverter",true).toBool()?
      addConverter(ccd): removeConverter(ccd);
  settings.value("useAcqirisConverter",true).toBool()?
      addConverter(Acqiris)     : removeConverter(Acqiris);
  settings.value("usepnCCDConverter",true).toBool()?
      addConverter(pnCCD)       : removeConverter(pnCCD);
  settings.value("useMachineConverter",true).toBool()?
      addConverter(MachineData) : removeConverter(MachineData);
}

enum {NoGoodData=0,GoodData};
bool cass::FormatConverter::processDatagram(cass::CASSEvent *cassevent)
{
  //intialize the return value//
  // the return value reflects the whether the datagram was a L1Transition(an event)
  bool retval(NoGoodData);
  //get the datagram from the cassevent//
  Pds::Dgram *datagram = reinterpret_cast<Pds::Dgram*>(cassevent->datagrambuffer());
/*
  std::cout << "transition \""<< Pds::TransitionId::name(datagram->seq.service())<< "\" ";
  std::cout << "0x"<< std::hex<< datagram->xtc.sizeofPayload()<<std::dec<<"  ";
  std::cout << "0x"<< std::hex<<datagram->xtc.damage.value()<<std::dec<<" ";
  std::cout << "0x"<< std::hex<< datagram->seq.clock().seconds()<<std::dec<<" ";
  std::cout << "0x"<< std::hex<< static_cast<uint32_t>(datagram->seq.stamp().fiducials())<<std::dec<<" ";
  std::cout << std::dec <<std::endl;
*/

  //if datagram is configuration or an event (L1Accept) then we will iterate through it//
  //otherwise we ignore the datagram//
  if ((datagram->seq.service() == Pds::TransitionId::Configure) ||
      (datagram->seq.service() == Pds::TransitionId::L1Accept))
  {
    //when it is a configuration transition then set the flag accordingly//
    if (datagram->seq.service() == Pds::TransitionId::Configure)
      _configseen=true;
    //if the datagram is an event, create the id from time and fiducial//
    if (_configseen && datagram->seq.service() == Pds::TransitionId::L1Accept)
    {
      //extract the bunchId from the datagram//
      uint64_t bunchId = datagram->seq.clock().seconds();
      bunchId = (bunchId<<32) + static_cast<uint32_t>(datagram->seq.stamp().fiducials()<<8);
      //put the id into the cassevent
      cassevent->id() = bunchId;
      //when the datagram was an event we need to tell the caller//
      retval = GoodData;
      //clear the beamline data//
      dynamic_cast<MachineData::MachineDataDevice*>
          (cassevent->devices()[CASSEvent::MachineData])->clear();
    }

    //iterate through the datagram and find the wanted information//
    //if the return value of the iterateor is false, then the transition
    //did not contain all information//
    XtcIterator iter(&(datagram->xtc),_usedConverters,cassevent,0);
    retval = iter.iterate() && retval;
  }
//  std::cout<< std::boolalpha<<retval<<std::endl;
  return retval;
}




// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
