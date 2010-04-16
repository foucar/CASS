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

/*** tool methods ***/

// tell server to reread ini file (or |what| part of it)
int ns__readini(size_t what, bool *success);

// quit server
int ns__quit(bool *success);

// get CASSEvent from server
int ns__getEvent(size_t type, unsigned t1, unsigned t2, bool *sucess);

// get histogram from server
int ns__getHistogram(size_t type, bool *sucess);

// get image from server in format |format| (possible formats: 1 == TIFF)  --how can I return QImage *result?
int ns__getImage(size_t format, size_t type, bool *success);
