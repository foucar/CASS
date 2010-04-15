/*
 * cass SOAP server definition
 *
 * This is a gSOAP header file with web service definitions
 *
 */

//gsoap ns service name: CASSsoap CFEL ASG software system
//gsoap ns service style: rpc
//gsoap ns service encoding: encoded
//gsoap ns service namespace: http://atca104.slac.stanford.edu/~kupper/cassserver.wsdl
//gsoap ns service location: http://atca104.slac.stanford.edu:12321

//gsoap ns schema namespace: urn:CASSsoap

#include <string>
typedef std::string xsd__string;

/*** tool methods ***/

// tell server to reread ini file -- why do I need an argument?
int ns__readini(bool *success);

// quit server -- why do I need an argument?
int ns__quit(bool *success);

// get CASSEvent from server
int ns__getEvent(int type, bool *sucess);

// get histogram from server
int ns__getHistogram(int type, bool *sucess);

// get image from server in format |format| (possible formats: 1 == TIFF)  --how can I return QImage *result?
int ns__getImage(int format, int type, bool *success);
