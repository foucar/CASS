// Copyright (C) 2010 Jochen Küpper

#include <algorithm>
#include <cassert>
#include <list>
#include <map>

#include "postprocessing/alignment.h"
#include "cass_event.h"
#include "histogram.h"



namespace cass
{
// *** postprocessors 143 and 144 -- Gussian height and width from averaged VMI CCD image ***


/*! Helper for postprocessors 143-144

This class actually does all the work; see individual postprocessors for details.
*/
class pp143_144_helper
{
public:

    /*! create the instance if not it does not exist already */
    static pp143_144_helper * instance();

    /*! destroy the instance */
    static void destroy();

    /*! Gaussian width */
    float width(const CASSEvent& event) { validate(event); return _values[event.id()].first; };

    /*! Gaussian height */
    float height(const CASSEvent& event) { validate(event); return _values[event.id()].second; };


protected:

    /*! Validate and, if necessary, process event

    @param event CASSEvent to process by us
    */
    void validate(const CASSEvent& event) {
        if(_processed.end() == find(_processed.begin(), _processed.end(), event.id())) {
            // reduce 2d histogram to row- and column-integrated 1d histograms,
            // perform Gauss-fits to 1d histograms,and store FWHM
            _values[event.id()] = std::make_pair(float(0.), float(0.));
            _processed.push_front(event.id());
            while(5 < _processed.size()) {
                _values.erase(_processed.back());
                _processed.pop_back();
            }
        }
    };

    /*! list of processed (and available) event ids

    This is our houskeeping list for deleting the oldest ids in order.
    Newest events are added in front, whereas the oldest (the last)
    events are delted when appropriate.
    */
    std::list<uint64_t> _processed;

    /*! map of values for the available ids */
    std::map<uint64_t, std::pair<float, float> > _values;


private:

    /*! Private constructor of singleton */
    pp143_144_helper() {};

    /*! Prevent copy-construction of singleton */
    pp143_144_helper(const pp143_144_helper&);

    /*! Prevent assignment (potentially resulting in a copy) of singleton */
    pp143_144_helper& operator=(const pp143_144_helper);

    /*! Prevent destruction unless going through destroy */
    ~pp143_144_helper() {};

    /*! pointer to the singleton instance */
    static pp143_144_helper *_instance;

    /*! Singleton operation locker */
    static QMutex _mutex;
};
pp143_144_helper *pp143_144_helper::_instance(0);
QMutex pp143_144_helper::_mutex;
pp143_144_helper * pp143_144_helper::instance() {
    QMutexLocker locker(&_mutex);
    if(0 == _instance)
        _instance = new pp143_144_helper();
    return _instance;
}
void pp143_144_helper::destroy() {
    QMutexLocker locker(&_mutex);
    delete _instance;
    _instance = 0;
}




pp143::pp143(cass::PostProcessors::histograms_t& hist, cass::PostProcessors::id_t id)
    : PostprocessorBackend(hist, id), _value(new Histogram0DFloat)
{
    // save storage in PostProcessors container
    assert(hist == _histograms);
    _histograms[_id] = _value;
}



pp143::~pp143()
{
    delete _value;
    _value = 0;
}



void pp143::operator()(const CASSEvent& event)
{
    *_value = pp143_144_helper::instance()->width(event);
}



pp144::pp144(cass::PostProcessors::histograms_t& hist, cass::PostProcessors::id_t id)
    : PostprocessorBackend(hist, id), _value(new Histogram0DFloat)
{
    // save storage in PostProcessors container
    assert(hist == _histograms);
    _histograms[_id] = _value;
}



pp144::~pp144()
{
    delete _value;
    _value = 0;
}



void pp144::operator()(const CASSEvent& event)
{
    *_value = pp143_144_helper::instance()->height(event);
}




} // end namespace cass



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-style: "Stroustrup"
// c-file-offsets: ((c . 0) (innamespace . 0))
// fill-column: 100
// End:
