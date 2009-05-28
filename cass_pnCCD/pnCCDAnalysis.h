// Copyright (C) 2009 Jochen Küpper

#ifndef PNCCDANALYSIS_H
#define PNCCDANALYSIS_H

#include <map>
#include <string>
#include <vector>
#include "cass_pnccd.h"
#include "AnalysisBackend.h"

#include <QtGui/QImage>


namespace cass {
namespace pnCCD {

/** @class pnCCD backend parameter sets

@author Jochen Küpper
@version 0.1
*/
class Parameter : cass::BackendParameter {
};



/** @class pnCCD analysis backend

@author Jochen Küpper
@version 0.1
*/
class CASS_PNCCDSHARED_EXPORT Analysis : cass::AnalysisBackend
{
public:

    Analysis(const Parameter& param);

    /** initialize AnalysisBackend with new set of parameters */
    virtual void init(const Parameter& param);

    /* analyse dataset

    @param data Raw image data to be analysed.
    For memory performance reasons the image could be manipulated in place
    */
    virtual QImage operator()(const QImage& data);
};


}
}

#endif



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
