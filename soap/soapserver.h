/*
 * cass SOAP server definition
 *
 * This is a gSOAP header file with web service definitions
 *
 */

//gsoap ns service name: CASS CFEL ASG software system
//gsoap ns service style: rpc
//gsoap ns service encoding: encoded
//gsoap ns service namespace: http://atca104.slac.stanford.edu/~kupper/cassserver.wsdl
//gsoap ns service location: http://atca104.slac.stanford.edu:12321

//gsoap ns schema namespace: urn:cassserver

#include <string>
#include <QtGui/QImage>
typedef std::string xsd__string;


/*** tool methods ***/

// tell server to reread ini file
int ns__readini();

// quit server
int ns__quit();

// get CASSEvent from server
int ns__getEvent(xsd__string *result);

// get histogram from server
int ns__getHistogram(xsd__string *result);

// get image from server
int ns__getImage(QImage *result);
