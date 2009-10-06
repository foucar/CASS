#ifndef XTCITERATOR_H
#define XTCITERATOR_H

#include <map>

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
        XtcIterator(Pds::Xtc* xtc, std::map<FormatConverter::Converters,ConversionBackend*>& converters, CASSEvent *cassevent, unsigned depth) :
                Pds::XtcIterator(xtc),
                _depth(depth),
                _converters(converters),
                _cassevent(cassevent)
        {
        }

        int process(Pds::Xtc* xtc)
        {
            //find out what type this xtc is//
            switch (xtc->contains.id())
            {
            case (Pds::TypeId::Id_Xtc) ://if it is another xtc we iterate through it recursively
                {
                    XtcIterator iter(xtc, _converters, _cassevent, _depth+1);
                    iter.iterate();
                    break;
                }
            case (Pds::TypeId::Id_Frame) :
                (*(_converters[FormatConverter::Pulnix]))(xtc,_cassevent);
                break;
            case (Pds::TypeId::Id_AcqWaveform) :
                (*(_converters[FormatConverter::REMI]))(xtc,_cassevent);
                break;
            case (Pds::TypeId::Id_AcqConfig) :
                {
                    unsigned version = xtc->contains.version();
                    switch (version)
                    {
                    case 1:
                        (*(_converters[FormatConverter::REMI]))(xtc,_cassevent);
                        break;
                    default:
                        break;
                    }
                }
                break;
            case (Pds::TypeId::Id_pnCCDconfig) :
	      //(*(_converters[FormatConverter::pnCCD]))(xtc,_cassevent);
                break;
            case (Pds::TypeId::Id_pnCCDframe) :
              //  (*(_converters[FormatConverter::pnCCD]))(xtc,_cassevent);
                break;
            default :
                    break;
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
