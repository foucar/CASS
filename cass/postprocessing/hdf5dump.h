// Copyright (C) 2010 Thomas White

#ifndef _HDF5_POSTPROCESSOR_H_
#define _HDF5_POSTPROCESSOR_H_

#include "postprocessing/backend.h"
#include "cass_event.h"
#include "cass_acqiris.h"

namespace cass
{

/** HDF5 Dump
 *
 * This postprocessor will dump the pnCCD images and other relevant
 * data to an HDF5 file.  One HDF5 file will be created for each
 * event.
 *
 * @author Thomas White
 */
class pp1001 : public PostprocessorBackend
{
public:
    /** constructor.*/
    pp1001(PostProcessors&, PostProcessors::id_t);

    /** destructor */
    virtual ~pp1001();

    /** copy image from CASS event to HDF5 */
    virtual void operator()(const CASSEvent&);

};

}

#endif




// Local Variables:
// coding: utf-8
// mode: C++
// c-file-style: "Stroustrup"
// c-file-offsets: ((c . 0) (innamespace . 0))
// fill-column: 100
// End:
