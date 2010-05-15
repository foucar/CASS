/*
 * cass SOAP server definition
 *
 * This is a gSOAP header file with web service definitions
 *
 */

//gsoap ns service name: CASSsoap CFEL ASG software system
//gsoap ns service style: rpc
//gsoap ns service encoding: encoded
//gsoap ns service namespace: http://daq-amo-mon02.slac.stanford.edu/~kupper/cassserver.wsdl
//gsoap ns service location: http://daq-amo-mon02.slac.stanford.edu:12321

//gsoap ns schema namespace: urn:CASSsoap

/*** tool methods ***/

// quit server
int ns__quit(bool *success);


// tell server to reread ini file (or |what| part of it)
int ns__readini(size_t what, bool *success);

// tell server to reread ini file (or |what| part of it)
int ns__writeini(size_t what, bool *success);

// get list of active postprocessor-ids
int ns__getPostprocessorIds(bool *success);

// get expected mime type
int ns__getMimeType(size_t type, bool *success);


// get CASSEvent from server
int ns__getEvent(size_t type, unsigned t1, unsigned t2, bool *sucess);

// get histogram from server
int ns__getHistogram(size_t type, bool *sucess);

// get image from server in format |format| (as defined in cass.h)
int ns__getImage(int format, size_t type, bool *success);
