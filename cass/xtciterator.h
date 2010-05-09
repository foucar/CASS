//Copyright (C) 2010 Lutz Foucar

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
  /** Iteration over XTC's.
   *
   * class that will iterate over an xtc using the xtciterator
   * provided by the lcls libary
   *
   * @author Lutz Foucar
   */
  class XtcIterator : public Pds::XtcIterator
  {
  public:
    /** enum for more convenient code*/
    enum {Stop, Continue};
    /** constructor.
     *
     * @param xtc the xtc which contents we iterate over
     * @param converters the map that contains the used converters
     * @param cassevent our event to write the information from the xtc to
     * @param depth The Depth of recursion when called recursivly
     */
    XtcIterator(Pds::Xtc* xtc,
                FormatConverter::usedConverters_t& converters,
                CASSEvent *cassevent,
                unsigned depth)
      :Pds::XtcIterator(xtc),
      _depth(depth),
      _converters(converters),
      _cassevent(cassevent)
    {}


    /** @overload
     * function that is called for each xtc found in the xtc
     *
     * will check whether its an id or another xtc. if its another xtc it will call this
     * with increased recursion depth, otherwise it will call the format converter for the id.
     */
    int process(Pds::Xtc* xtc)
    {
      //if it is another xtc, then iterate through it//
      if (xtc->contains.id() == Pds::TypeId::Id_Xtc)
        iterate(xtc);
      //otherwise use the responsible format converter for this xtc//
      else
      {
        //first check whether datagram is damaged or the xtc id is bigger than we expect//
        //when it contains an incomplete contribution stop iterating and return that it is not good//
        uint32_t damage = xtc->damage.value();
        if (xtc->contains.id() >= Pds::TypeId::NumberOf)
        {
          std::cout << xtc->contains.id() <<" is an unkown xtc id"<<std::endl;
          return Stop;
        }
        else if (damage)
          std::cout <<std::hex<<Pds::TypeId::name(xtc->contains.id())<< " is damaged: 0x" <<xtc->damage.value()<<std::dec<<std::endl;
        else if (damage & ( 0x1 << Pds::Damage::IncompleteContribution))
          return Stop;
        else
          //use the converter that is good for this xtc type//
          (*_converters[xtc->contains.id()])(xtc,_cassevent);
      }
      return Continue;
    }
  private:
    unsigned                           _depth;      //!< counts the recursivness of this
    FormatConverter::usedConverters_t &_converters; //!< reference to the converters
    CASSEvent                         *_cassevent;  //!< pointer to the cassevent to work on
  };
}//end namespace cass

#endif // XTCITERATOR_H
