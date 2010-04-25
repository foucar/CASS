// Copyright (C) 2010 Lutz Foucar
// Copyright (C) 2010 Jochen Küpper

#ifndef __HISTOGRAM_GETTER_H__
#define __HISTOGRAM_GETTER_H__

#include <string>
#include <QtGui/QImage>

#include "cass.h"
#include "postprocessing/postprocessor.h"

namespace cass
{

/** Histogram retrievel parameters.

@author Jochen Küpper
*/
struct HistogramParameter
{
    explicit HistogramParameter(PostProcessors::id_t _type)
        : type(_type)
        {};

    explicit HistogramParameter(size_t _type)
        : type(PostProcessors::id_t(_type))
        {};

    PostProcessors::id_t type;
};



/** retrive a histogram from the histogram container
 *
 * @author Lutz Foucar
 * @author Jochen Küpper
 *
 * @todo Make independent of global PostProcessors singleton (by passing it in here)
 */
class CASSSHARED_EXPORT HistogramGetter
{
public:

    /** constructor.
     *
     * @param histograms container of all histogram, we will retrieve the requested histograms from
     *        there
     */
    HistogramGetter() {};

    /** Serialize histogram.
     * function that will serialize the requested histogram to a string and return it
     */
    const std::string operator()(const HistogramParameter&) const;

    /** clear histogram.
     * function that will clear a requested histogram.
     * @return void
     * @param hp Histogram parameter, contains info which histogram to clear
     */
    void clear(const HistogramParameter& hp)const;

    /** Create an QImage from the histogram */
    QImage qimage(const HistogramParameter&) const;
};

} //end namespace cass

#endif



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
