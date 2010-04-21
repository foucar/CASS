// Copyright (C) 2009, 2010 Lutz Foucar
// Copyright (C) 2010 Jochen Küpper

#ifndef CASS_GLOBAL_H
#define CASS_GLOBAL_H

#include <cassert>
#include <iterator>
#include <list>
#include <vector>
#include <QtCore/qglobal.h>


#if defined(CASS_LIBRARY)
#  define CASSSHARED_EXPORT Q_DECL_EXPORT
#else
#  define CASSSHARED_EXPORT Q_DECL_IMPORT
#endif


namespace cass
{
    /** global variable to set the ring buffer size */
    const size_t RingBufferSize=200;
    /** global variable to set the number of worker threads*/
    const size_t NbrOfWorkers=16;
    /** the maximum size of one datagram should be 10 MB*/
    const size_t DatagramBufferSize=0x1000000;
    /** the type of a pixel of a ccd image*/
    typedef float pixel_t;



    /*! Helper function to delete duplicates from a std::list

    This keeps the earliest entry in the list and removes all later ones
    @param l List to remove duplicates from.
    */
    template<typename T>
    inline void unique(std::list<T>& l)
    {
        // shorten list by removing consecutive duplicates
        l.unique();
        // now remove remaining (harder) duplicates
        for(typename std::list<T>::iterator i1 = l.begin();
        i1 != l.end();
        ++i1) {
            typename std::list<T>::iterator i2(i1);
            ++i2;
            while(l.end() != (i2 = find(i2, l.end(), *i1)))
                l.erase(i2);
        }
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
