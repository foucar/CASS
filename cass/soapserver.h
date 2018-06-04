/**
 * @file soapserver.h cass SOAP server definition
 *
 * This is a gSOAP header file with web service definitions
 *
 * @author Jochen Kuepper
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

// tell server to clear a given histogram
int ns__clearHistogram(const std::string &type, bool *success);

// tell server to process a command in given processor
int ns__receiveCommand(const std::string &type, const std::string &command, bool *success);

// get list of active processor-ids
int ns__getPostprocessorIds(bool *success);

// get CASSEvent from server
int ns__getEvent(size_t type, unsigned t1, unsigned t2, bool *success);

// get result from server
int ns__getHistogram(const std::string &type, ULONG64 eventId, bool *success);

// get list of results from server
int ns__getResults(bool sameEventId, bool *success);

// control darkcal calibration
int ns__controlDarkcal(const std::string &controlCommand, bool *success);

