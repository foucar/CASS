// CASS TCP server
//
// Copyright (C) 2010 Jochen Küpper

#include <QtNetwork/QTcpServer>

typedef int Event;
typedef int Histogram;

/* @brief CASS TCP server

@author Jochen Küpper

CASS provides a TCP server on port ? that allows to program the server and to retrieve experimental
data using the following commands:

CASS:READINI:<what>
===================

Tell the CASS server to reload its ini file, where <what> is an unsigned integer that specifies which
parts of cass.ini shall be read

This command emits the signal readini(size_t).


CASS:QUIT
=========

Shut down the server.

This command emits the signal quit().



CASS:GET:EVENT:<what>:<t1>:<t2>
===============================

Get latest available event.
Currently, the unisgned integers <what>, <t1>, and <t2> are unused. They might be used to read
partial events (<what>) or all events from a given time-range ([<t1>:<t2>]) at some later stage.

This command calls the supplied get_event functor.



CASS:GET:HISTOGRAM:<type>
=========================

Get current histogram of type <type>, which is a unsigned integer value.

This command calls the supplied get_histogram functor.


The defined histogram types are

    1: Last plain image from pnCCD-1
    2: Last plain image from pnCCD 2
  101: Running average of pnCCD-1 images with
       - an average length of cass.ini:cass/histogram/101/average
       - geometric binning (x and y) of cass.ini:cass/histogram/101/binning
       - background subtraction of the images specified in cass.ini:cass/histogram/101/background
*/
class TCPserver : public QTcpServer
{
    Q_OBJECT;

public:

    struct event_parameter {
        size_t what;
        unsigned int t1, t2;
    };

    struct histogram_parameter {
        size_t type;
    };

    /** Constructor

    @param event Specify get_event functor
    @param hist Specify get_histogram functor
    */
    TCPserver(std::unary_function<event_parameter&, Event> event,
              std::unary_function<histogram_parameter&, Histogram> hist);


signals:

    void quit();

    void readini(size_t what);


protected:

    /** get_event functor */
    std::unary_function<event_parameter&, Event> get_event;

    /** get_histogram functor */
    std::unary_function<histogram_parameter&, Histogram> get_histogram;
};




// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
