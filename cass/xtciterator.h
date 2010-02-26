//Copyright (C) 2010 lmf

#ifndef _XTCITERATOR_H_
#define _XTCITERATOR_H_

#include <map>
#include <iostream>

#include "format_converter.h"
#include "conversion_backend.h"

#include "pdsdata/xtc/XtcIterator.hh"
#include "pdsdata/xtc/Xtc.hh"

namespace cass
{
  class XtcIterator : public Pds::XtcIterator
  {
  public:
    enum {Stop, Continue};
    XtcIterator(Pds::Xtc* xtc, FormatConverter::usedConverters_t& converters, CASSEvent *cassevent, unsigned depth) :
        Pds::XtcIterator(xtc),
        _depth(depth),
        _converters(converters),
        _cassevent(cassevent)
    {
    }

    int process(Pds::Xtc* xtc)
    {
      //if it is another xtc, then iterate through it//
      if (xtc->contains.id() == Pds::TypeId::Id_Xtc)
        iterate(xtc);
      //otherwise use the responsible format converter for this xtc//
      else
      {
        //first check whether datagram is damaged or the xtc id is bigger than we expect//
        uint32_t damage = xtc->damage.value();
        if (xtc->contains.id() >= Pds::TypeId::NumberOf)
          std::cout << xtc->contains.id() <<" is an unkown xtc id"<<std::endl;
        else if (damage)
          std::cout <<std::hex<<Pds::TypeId::name(xtc->contains.id())<< " is damaged: 0x" <<xtc->damage.value()<<std::dec<<std::endl;
        else
          //use the converter that is good for this xtc type//
          (*_converters[xtc->contains.id()])(xtc,_cassevent);
      }
      return Continue;
    }
  private:
    unsigned _depth;
    FormatConverter::usedConverters_t& _converters;
    CASSEvent *_cassevent;
  };
}//end namespace cass

#endif // XTCITERATOR_H
