// Copyright (C) 2009 Jochen Küpper

#include <QtCore/QMutexLocker>
#include "FormatConverter.h"


namespace cass {

// define static members
FormatConverter *FormatConverter::_instance(0);
QMutex FormatConverter::_mutex;


FormatConverter::FormatConverter()
{
    // create all the necessary individual format converters
}



FormatConverter::~FormatConverter()
{
    // destruct all the individual format converters
}



void FormatConverter::destroy()
{
    QMutexLocker locker(&_mutex);
    delete _instance;
    _instance = 0;
}


FormatConverter *FormatConverter::instance()
{
    QMutexLocker locker(&_mutex);
    if(0 == _instance)
        _instance = new FormatConverter();
    return _instance;
}

}




// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
