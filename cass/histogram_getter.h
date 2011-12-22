// Copyright (C) 2010 Lutz Foucar
// Copyright (C) 2010 Jochen Küpper
// Copyright (C) 2011 Stephan Kassemeyer

/**
 * @file histogram_getter.h file contains declaration retriever classes
 *
 * @author Lutz Foucar
 */

#ifndef __HISTOGRAM_GETTER_H__
#define __HISTOGRAM_GETTER_H__

#include <string>

#include <QtGui/QImage>

#include "cass.h"
#include "postprocessor.h"

#ifdef JPEG_CONVERSION
#include <jpeglib.h>
#endif //JPEG_CONVERSION

namespace cass
{

/** Histogram retrieval parameters.
 *
 * @author Jochen Küpper
 */
struct HistogramParameter
{
  explicit HistogramParameter(const PostProcessors::key_t& _key, uint64_t _eventId=0)
    : key(_key), eventId(_eventId)
  {}

  PostProcessors::key_t key;
  uint64_t eventId;
};



/** retrieve a histogram from the histogram container
 *
 * @author Lutz Foucar
 * @author Jochen Küpper
 * @author Stephan Kassemeyer
 */
class CASSSHARED_EXPORT HistogramGetter
{
public:

    /** constructor */
    HistogramGetter()
      :_postprocessors(*PostProcessors::instance())
    {}

    /** Serialize histogram.
     *
     * will serialize the requested histogram to a string and return it
     *
     * @return
     * @param
     */
    const std::pair<size_t, std::string> operator()(const HistogramParameter&) const;

    /** clear histogram.
     * function that will clear a requested histogram.
     * @return void
     * @param hp Histogram parameter, contains info which histogram to clear
     */
    void clear(const HistogramParameter& hp)const;

    /** Create an QImage from the histogram */
    QImage qimage(const HistogramParameter&) const;

#ifdef JPEG_CONVERSION
    /** Create a jpeg compressed image from the histogram */
    std::vector<JOCTET>*  jpegImage(const HistogramParameter&);
#endif //JPEG_CONVERSION

protected:
    /** pointer to the postprocessors class. will be retrieved using singleton */
    PostProcessors &_postprocessors;

#ifdef JPEG_CONVERSION
    std::map<PostProcessors::key_t, std::vector<JOCTET>*> jpeg_cache;
    std::map<PostProcessors::key_t, double> jpeg_cache_times;
#endif //JPEG_CONVERSION

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
