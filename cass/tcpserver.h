// CASS TCP server
//
// Copyright (C) 2010 Jochen Küppe

#include <QtNetwork/QTcpServer>

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
OB

CASS:GET:HISTOGRAM:<type>
=========================

Get current histogram of type <type>, which is a unsigned integer value.

The defined types are

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

signals:

    void quit();
    
    void readini(size_t what);

protected:

    
};




// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
