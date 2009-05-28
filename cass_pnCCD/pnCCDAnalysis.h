// Copyright (C) 2009 Jochen Küpper


#ifndef PNCCDANALYSIS_H
#define PNCCDANALYSIS_H

#include <map>
#include <string>
#include <vector>
#include "cass_pnccd.h"
#include "AnalysisBackend.h"

#include <QtGui/QImage>

/** @class pnCCD backend parameter sets

 @author Jochen Küpper
 @version 0.1
 */
class pnCCDParameter : BackendParameter {
};



/** @class pnCCD analysis backend

 @author Jochen Küpper
 @version 0.1
 */
class CASS_PNCCDSHARED_EXPORT pnCCDAnalysis : AnalysisBackend
{
public:

    pnCCDAnalysis(const pnCCDParameter& param);

    /** initialize AnalysisBackend with new set of parameters */
    virtual void init(const pnCCDParameter& param);

    /* analyse dataset

    @param data Raw image data to be analysed.
    For memory performance reasons the image could be manipulated in place
    */
    virtual QImage operator()(const QImage& data);
};


#endif // PNCCDANALYSIS_H
