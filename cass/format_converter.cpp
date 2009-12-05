// Copyright (C) 2009 Jochen Küpper
#include <iostream>
#include <iomanip>
#include <fstream>
#include <QtCore/QMutexLocker>
#include "format_converter.h"
#include "remi_converter.h"
#include "vmi_converter.h"
#include "machine_converter.h"
#include "pnccd_converter.h"
#include "xtciterator.h"
#include "pdsdata/xtc/Dgram.hh"


// define static members
cass::FormatConverter *cass::FormatConverter::_instance(0);
QMutex cass::FormatConverter::_mutex;
bool cass::FormatConverter::_firsttime(true);


cass::FormatConverter::FormatConverter()
{
  // create all the necessary individual format converters
  _converter[REMI]        = new REMI::Converter();
  _converter[Pulnix]      = new VMI::Converter();
  _converter[pnCCD]       = new pnCCD::Converter();
  _converter[MachineData] = new MachineData::Converter();
}

cass::FormatConverter::~FormatConverter()
{
  // destruct all the individual format converters
  for (std::map<Converters, ConversionBackend *>::iterator it=_converter.begin() ; it != _converter.end(); ++it )
    delete (it->second);
}



void cass::FormatConverter::destroy()
{
  QMutexLocker locker(&_mutex);
  delete _instance;
  _instance = 0;
}


cass::FormatConverter *cass::FormatConverter::instance()
{
  QMutexLocker locker(&_mutex);
  _firsttime    = true;
  if(0 == _instance)
    _instance = new FormatConverter();
  return _instance;
}



//this slot is called once the eventqueue has new data available//
bool cass::FormatConverter::processDatagram(cass::CASSEvent *cassevent)
{
  //intialize the return value//
  bool retval = false;
  //get the datagram from the cassevent//
  Pds::Dgram *datagram = reinterpret_cast<Pds::Dgram*>(cassevent->datagrambuffer());


  //if this is the firsttime we run we want to load a config that was stored to disk//
  if(_firsttime)
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
      char * buffer =  new char [0x900000];
      Pds::Dgram& dg = *reinterpret_cast<Dgram*>(buffer);
      oldconfigtransition.read(buffer,sizeof(dg));
      oldconfigtransition.read(dg.xtc.payload(), dg.xtc.sizeofPayload());
      //done reading.. close file//
      oldconfigtransition.close();
      //now iterate through the config datagram//
      XtcIterator iter(&(dg.xtc),_converter,0,0);
      iter.iterate();
    }
  }

  //check whether datagram is damaged//
  uint32_t damage = datagram->xtc.damage.value();
  if (!damage)
  {
    //if datagram is configuration or an event (L1Accept) then we will iterate through it//
    //otherwise we ignore the datagram//
    if ((datagram->seq.service() == Pds::TransitionId::Configure) ||
        (datagram->seq.service() == Pds::TransitionId::L1Accept))
    {
      //if the datagram is an event than we create a new cass event first//
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
      //if it is a configure transition we want to store it to file//
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
      }

      //iterate through the datagram and find the wanted information//
      XtcIterator iter(&(datagram->xtc),_converter,cassevent,0);
      iter.iterate();
    }
  }
  else
    std::cout <<std::hex<< "datagram is damaged: 0x" <<datagram->xtc.damage.value()<<std::dec<<std::endl;
  return retval;
}




// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
