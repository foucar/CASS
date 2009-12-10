#ifndef XTCITERATOR_H
#define XTCITERATOR_H

#include <map>
#include <iostream>

#include "format_converter.h"
#include "conversion_backend.h"

#include "pdsdata/xtc/XtcIterator.hh"
#include "pdsdata/xtc/Xtc.hh"
#include "pdsdata/xtc/TypeId.hh"

namespace cass
{
  class XtcIterator : public Pds::XtcIterator
  {
  public:
    enum {Stop, Continue};
    XtcIterator(Pds::Xtc* xtc, std::map<FormatConverter::Converters,ConversionBackend*>& converters, CASSEvent *cassevent, unsigned depth) :
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
      {
        iterate(xtc);
      }
      else //otherwise check which format converter is responsible for this xtc//
      {
        //check whether datagram is damaged//
        uint32_t damage = xtc->damage.value();
        if (!damage)
        {
          for (std::map<FormatConverter::Converters,ConversionBackend*>::iterator it=_converters.begin() ; it != _converters.end(); ++it )
          {
            if( it->second->handlesType(xtc->contains.id()))
              (*(it->second))(xtc,_cassevent);
          }
        }
        else
          std::cout <<std::hex<<Pds::TypeId::name(xtc->contains.id())<< " is damaged: 0x" <<xtc->damage.value()<<std::dec<<std::endl;

      }
      return Continue;
    }
      private:
    unsigned _depth;
    std::map<FormatConverter::Converters, ConversionBackend *>& _converters;
    CASSEvent *_cassevent;
  };

}//end namespace

#endif // XTCITERATOR_H
