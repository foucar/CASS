//Copyright (C) 2010-2014 Lutz Foucar

/**
 * @file xtciterator.hpp file contains iterator to iterate through a xtc datagram
 *
 * @author Lutz Foucar
 */

#ifndef _XTCITERATOR_H_
#define _XTCITERATOR_H_

#include <map>
#include <iostream>
#include <string>

#include "format_converter.h"
#include "conversion_backend.h"
#include "log.h"

#include "pdsdata/xtc/XtcIterator.hh"
#include "pdsdata/xtc/Xtc.hh"
#include "pdsdata/compress/CompressedXtc.hh"
#include "pdsdata/xtc/BldInfo.hh"
#include "pdsdata/xtc/DetInfo.hh"

namespace cass
{
/** overwrite the destruction of xtc pointers */
static void Destroy(Xtc*) {}

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
    : Pds::XtcIterator(xtc),
      _depth(depth),
      _converters(converters),
      _cassevent(cassevent)
  {}

  /** @overload
   * function that is called for each xtc found in the xtc
   *
   * will check whether its an id or another xtc. if its another xtc it will
   * call this with increased recursion depth, otherwise it will call the
   * format converter for the id.
   */
  int process(Pds::Xtc* xtc_orig)
  {
    /** output information about the xtc */
    using std::string;
    Log::add(Log::DEBUG4,string("XTC Type '") + TypeId::name(xtc_orig->contains.id()) + "'(" + toString(xtc_orig->contains.id()) + ")");
    Log::add(Log::DEBUG4,string("XTC Version '") + toString(xtc_orig->contains.version()) + "'");
    Log::add(Log::DEBUG4,string("XTC Compressed '") + (xtc_orig->contains.compressed() ? "true":"false") + "'");
    Log::add(Log::DEBUG4,string("XTC CompressedVersion '") + toString(xtc_orig->contains.compressed_version()) + "'");
    Log::add(Log::DEBUG4,string("XTC Damage value '") + toString(xtc_orig->damage.value()) + "'");
    Log::add(Log::DEBUG4,string("XTC Level '") + Level::name(xtc_orig->src.level()) + toString(xtc_orig->src.level()) + "'");
    switch (xtc_orig->src.level())
    {
    case Level::Source :
      Log::add(Log::DEBUG4,string("XTC DetInfo: ") + DetInfo::name(reinterpret_cast<const DetInfo&>(xtc_orig->src)));
      break;
    case Level::Reporter :
      Log::add(Log::DEBUG4,string("XTC BldInfo: ") + BldInfo::name(reinterpret_cast<const BldInfo&>(xtc_orig->src)));
      break;
    default :
      Log::add(Log::DEBUG4,string("XTC Proc '") + toString(xtc_orig->src.log()) + ":" + toString(xtc_orig->src.phy()) + "'");
      break;
    }

    /** if it is another xtc, then iterate through it */
    if (xtc_orig->contains.id() == Pds::TypeId::Id_Xtc)
    {
      if (!iterate(xtc_orig))
        return Stop;
    }
    /** otherwise use the responsible format converter for this xtc */
    else
    {
      /** need to check wether the xtc is compressed */
      std::tr1::shared_ptr<Xtc> xtc = xtc_orig->contains.compressed() ?
            Pds::CompressedXtc::uncompress(*xtc_orig) :
            std::tr1::shared_ptr<Xtc>(xtc_orig,Destroy);

      /** first check whether datagram is damaged or the xtc id is bigger than
       *  we expect when it contains an incomplete contribution stop iterating
       *  and return that it is not good
       */
      uint32_t damage = xtc->damage.value();
      if (xtc->contains.id() >= Pds::TypeId::NumberOf)
      {
        Log::add(Log::ERROR, toString(xtc->contains.id()) +
                 " is an unkown xtc id. Skipping Event");
        return Stop;
      }
      else if (damage)
      {
        Log::add(Log::WARNING,std::string(Pds::TypeId::name(xtc->contains.id())) +
                 " is damaged: " + toString(xtc->damage.value()) );
        if (damage & ( 0x1 << Pds::Damage::DroppedContribution))
        {
          Log::add(Log::ERROR,"'"+ toString(xtc->damage.value()) +
                   "' is dropped Contribution. Skipping Event");
          return Stop;
        }
        else if(damage & (0x1 <<Pds::Damage::UserDefined))
        {
          (*_converters[xtc->contains.id()])(xtc.get(),_cassevent);
        }
        else
        {
          Log::add(Log::ERROR,"'"+ toString(xtc->damage.value()) +
                   "' is unkown Damage. Skipping Event");
          return Stop;
        }
      }
      else
      {
        (*_converters[xtc->contains.id()])(xtc.get(),_cassevent);
      }
    }
    return Continue;
  }

private:
  /** counts the recursivness of this */
  unsigned _depth;

  /** reference to the converters */
  FormatConverter::usedConverters_t &_converters;

  /** pointer to the cassevent to work on */
  CASSEvent *_cassevent;
};
}//end namespace cass

#endif // XTCITERATOR_H
