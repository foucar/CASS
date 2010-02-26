// Copyright (C) 2009 Jochen Küpper
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
  class CASSSHARED_EXPORT BlankConverter : public ConversionBackend
  {
  public:
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



//load the settings
void cass::ConverterParameter::load()
{
  //sync before loading//
  sync();
  _useCCD     = value("useCommercialCCDConverter",true).toBool();
  _useAcqiris = value("useAcqirisConverter",true).toBool();
  _useMachine = value("useMachineConverter",true).toBool();
  _usepnCCD   = value("usepnCCDConverter",true).toBool();
}
//save the settings
void cass::ConverterParameter::save()
{
  setValue("useCommercialCCDConverter",_useCCD);
  setValue("useAcqirisConverter",_useAcqiris);
  setValue("useMachineConverter",_useMachine);
  setValue("usepnCCDConverter",_usepnCCD);
}







cass::FormatConverter::FormatConverter()
{
  // create all the necessary individual format converters
  _availableConverters[Acqiris]     = new ACQIRIS::Converter();
  _availableConverters[ccd]         = new CCD::Converter();
  _availableConverters[pnCCD]       = new pnCCD::Converter();
  _availableConverters[MachineData] = new MachineData::Converter();
  _availableConverters[Blank]       = new BlankConverter();

  // now initialze the uniteresting xtc ids with blank converters//
  _usedConverters[Pds::TypeId::Any]                 = _availableConverters[Blank];
  _usedConverters[Pds::TypeId::Id_Xtc]              = _availableConverters[Blank];
  _usedConverters[Pds::TypeId::Id_TwoDGaussian]     = _availableConverters[Blank];
  _usedConverters[Pds::TypeId::Id_Opal1kConfig]     = _availableConverters[Blank];
  _usedConverters[Pds::TypeId::Id_FrameFexConfig]   = _availableConverters[Blank];
  _usedConverters[Pds::TypeId::Id_EvrConfig]        = _availableConverters[Blank];
  _usedConverters[Pds::TypeId::Id_TM6740Config]     = _availableConverters[Blank];
  _usedConverters[Pds::TypeId::Id_ControlConfig]    = _availableConverters[Blank];
}

cass::FormatConverter::~FormatConverter()
{
  // destruct all the individual format converters
  for (availableConverters_t::iterator it=_converter.begin() ; it != _converter.end(); ++it )
    delete (it->second);
}


void cass::FormatConverter::addConverter(cass::FormatConverter::Converters converter)
{
  //look which converter should be added//
  switch(converter)
  {
  case ccd:
    _usedConverters[Pds::TypeId::Id_Frame]            = _availableConverters[ccd];
    break;
  case Acqiris:
    _usedConverters[Pds::TypeId::Id_AcqWaveform]      = _availableConverters[Acqiris];
    _usedConverters[Pds::TypeId::Id_AcqConfig]        = _availableConverters[Acqiris];
    break;
  case pnCCD:
    _usedConverters[Pds::TypeId::Id_pnCCDframe]       = _availableConverters[pnCCD];
    _usedConverters[Pds::TypeId::Id_pnCCDconfig]      = _availableConverters[pnCCD];
    break;
  case MachineData:
    _usedConverters[Pds::TypeId::Id_Epics]            = _availableConverters[MachineData];
    _usedConverters[Pds::TypeId::Id_FEEGasDetEnergy]  = _availableConverters[MachineData];
    _usedConverters[Pds::TypeId::Id_EBeam]            = _availableConverters[MachineData];
    _usedConverters[Pds::TypeId::Id_PhaseCavity]      = _availableConverters[MachineData];
    break;
  default:
    break;
  }
}

void cass::FormatConverter::removeConverter(cass::FormatConverter::Converters converter)
{
  //look which converter should be removed//
  switch(converter)
  {
  case ccd:
    _usedConverters[Pds::TypeId::Id_Frame]            = _availableConverters[Blank];
    break;
  case Acqiris:
    _usedConverters[Pds::TypeId::Id_AcqWaveform]      = _availableConverters[Blank];
    _usedConverters[Pds::TypeId::Id_AcqConfig]        = _availableConverters[Blank];
    break;
  case pnCCD:
    _usedConverters[Pds::TypeId::Id_pnCCDframe]       = _availableConverters[Blank];
    _usedConverters[Pds::TypeId::Id_pnCCDconfig]      = _availableConverters[Blank];
    break;
  case MachineData:
    _usedConverters[Pds::TypeId::Id_Epics]            = _availableConverters[Blank];
    _usedConverters[Pds::TypeId::Id_FEEGasDetEnergy]  = _availableConverters[Blank];
    _usedConverters[Pds::TypeId::Id_EBeam]            = _availableConverters[Blank];
    _usedConverters[Pds::TypeId::Id_PhaseCavity]      = _availableConverters[Blank];
    break;
  default:
    break;
  }
}

void cass::FormatConverter::loadSettings()
{
  //load the parameters//
  _param.load();
  //install the requested converters//
  _param._useCCD      ? addConverter(ccd)         : removeConverter(ccd);
  _param._useAcqiris  ? addConverter(Acqiris)     : removeConverter(Acqiris);
  _param._usepnCCD    ? addConverter(pnCCD)       : removeConverter(pnCCD);
  _param._useMachine  ? addConverter(MachineData) : removeConverter(MachineData);
}

void cass::FormatConverter::saveSettings()
{
  //save the parameters//
  _param.save();
}


//this slot is called once the eventqueue has new data available//
bool cass::FormatConverter::processDatagram(cass::CASSEvent *cassevent)
{
  //intialize the return value//
  bool retval = false;
  //get the datagram from the cassevent//
  Pds::Dgram *datagram = reinterpret_cast<Pds::Dgram*>(cassevent->datagrambuffer());

/*  std::cout << "transition \""<< Pds::TransitionId::name(datagram->seq.service())<< "\" ";
  std::cout << "0x"<< std::hex<< datagram->xtc.sizeofPayload()<<std::dec<<"  ";
  std::cout << "0x"<< std::hex<<datagram->xtc.damage.value()<<std::dec<<" ";
  std::cout << "0x"<<std::hex<< datagram->seq.clock().seconds()<<" ";
  std::cout << "0x"<<std::hex<< static_cast<uint32_t>(datagram->seq.stamp().fiducials())<<" ";
  std::cout << std::dec <<std::endl;*/

  //if this is the firsttime we run we want to load a config that was stored to disk//
  /*if(_firsttime)
  {
    //reset the flag//
    _firsttime = false;
    //open the file//
    std::ifstream oldconfigtransition;
    oldconfigtransition.open("oldconfig.xtc",std::ios::binary|std::ios::in);
    //if there was such a file then we want to load it//
    if (oldconfigtransition.is_open())
    {
      //read the datagram from the file//
      char * buffer =  new char [0x1000000];
      Pds::Dgram& dg = *reinterpret_cast<Dgram*>(buffer);
      oldconfigtransition.read(buffer,sizeof(dg));
      oldconfigtransition.read(dg.xtc.payload(), dg.xtc.sizeofPayload());
      //done reading.. close file//
      oldconfigtransition.close();
      //now iterate through the config datagram//
      XtcIterator iter(&(dg.xtc),_converter,0,0);
      iter.iterate();
    }
  }*/

  //if datagram is configuration or an event (L1Accept) then we will iterate through it//
  //otherwise we ignore the datagram//
  if ((datagram->seq.service() == Pds::TransitionId::Configure) ||
      (datagram->seq.service() == Pds::TransitionId::L1Accept))
  {
    //if the datagram is an event create the id from time and fiducial//
    if (datagram->seq.service() == Pds::TransitionId::L1Accept)
    {
      //extract the bunchId from the datagram//
      uint64_t bunchId = datagram->seq.clock().seconds();
      bunchId = (bunchId<<32) + static_cast<uint32_t>(datagram->seq.stamp().fiducials()<<8);
      //put the id into the cassevent
      cassevent->id() = bunchId;
      //when the datagram was an event we need to tell the caller//
      retval = true;
    }
    /*//if it is a configure transition we want to store it to file//
    else if(datagram->seq.service() == Pds::TransitionId::Configure)
    {
      //open the file//
      std::ofstream configtransition;
      configtransition.open("oldconfig.xtc",std::ios::binary|std::ios::out);
      if (configtransition.is_open())
      {
        //write the datagram to file//
        configtransition.write(reinterpret_cast<char*>(datagram),sizeof(Pds::Dgram));
        configtransition.write(datagram->xtc.payload(),datagram->xtc.sizeofPayload());
        configtransition.close();
      }
    }*/

    //iterate through the datagram and find the wanted information//
    XtcIterator iter(&(datagram->xtc),_usedConverters,cassevent,0);
    iter.iterate();
  }
  return retval;
}




// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
