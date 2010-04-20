// Copyright (C) 2010 Jochen Küpper
// Copyright (C) 2010 Lutz Foucar

#include "histogram.h"
#include "histogram_getter.h"
#include "serializer.h"
#include "postprocessing/postprocessor.h"


using namespace cass;


const std::string HistogramGetter::operator()(const HistogramParameter& hp) const
{
    Serializer serializer;
    // serialize the wanted histogram using the serializer
    PostProcessors::histograms_t::const_iterator iter(_histograms.find(hp.type));
    // lock access
    QMutex *mutex(iter->second->mutex());
    QMutexLocker lock(mutex);
    dynamic_cast<HistogramFloatBase*>(iter->second)->serialize(serializer);
    //return the buffer (std::string) of the serializer
    std::cerr << "HistogramGetter::operator() -- string size = " << serializer.buffer().size() << std::endl;
    return serializer.buffer();
}


QImage HistogramGetter::qimage(const HistogramParameter& hp) const
{
    std::cerr << "HistogramGetter::qimage" << std::endl;
    // get an iterator to the requested histogram
    PostProcessors::histograms_t::const_iterator iter(_histograms.find(hp.type));
    // and return the QImage of that histogram
    QImage image(dynamic_cast<Histogram2DFloat *>(iter->second)->qimage());
    std::cerr << "    got image" << std::endl;
    return image;
}



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
