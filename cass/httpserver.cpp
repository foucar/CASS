#include "httpserver.h"
#include "postprocessing/postprocessor.h"
#include "histogram.h"
#include "postprocessing/id_list.h"
#include "cass_exceptions.h"



void req_histogram2DImage::createResponseBuffer()
{
  _responseBuffer = "";
  // todo: parse address (histogram name) and get requested histogram
  // todo: fill buffer with image of histogram
  int dotpos = _address.find_last_of(".");
  std::string type( _address.substr(0, dotpos));
  std::cout << "DING:\n";
  std::cout << _address << std::endl;
  std::cout << type << std::endl;
  try {
    std::vector<JOCTET>* buffer(_server.histogram_getter().jpegImage(cass::HistogramParameter(type)));
    std::cout << "req_histogram2DImage::createResponseBuffer: buffer size: " << buffer->size() << std::endl;
    for (int ii=0;ii<buffer->size();++ii)
      _responseBuffer += (*buffer)[ii];
    // todo: can this be done without copying?
  }
  catch (cass::InvalidPostProcessorError& e)
  {
    std::cout << "requested invalid histogram: " <<  e.what() << std::endl;
  }
}

void req_histogram2DPage::createResponseBuffer()
{
  _responseBuffer = "";
  _responseBuffer += "<html><head> <meta http-equiv=\"refresh\" content=\"1\"></head><body>\n";
  //_responseBuffer += "<html><head><title>2D Histogram</title></head><body>\n";

  std::string imageFileName( ADDR_HIST2DImage );
  int dotpos = _address.find_last_of(".");
  imageFileName += _address.substr( 0, dotpos );
  imageFileName += ".jpg";
  _responseBuffer += "<html><body>\n";
  _responseBuffer += "<img src=\"";
  _responseBuffer += imageFileName;
  _responseBuffer += "\" alt=\"";
  _responseBuffer += imageFileName;
  _responseBuffer += "\"></p>";
  _responseBuffer += imageFileName;
  _responseBuffer += "\n";
  _responseBuffer += "</body></html>\n";
}

void req_histogram1DImage::createResponseBuffer()
{
  _responseBuffer = "";
  // todo: parse address (histogram name) and get requested histogram
  // todo: fill buffer with image of histogram
}

void req_histogram1DPage::createResponseBuffer()
{
  _responseBuffer = "";
  //_responseBuffer += "<html><head> <meta http-equiv=\"refresh\" content=\"5\"></head><body>\n";
  //_responseBuffer += "<html><head><title>2D Histogram</title></head><body>\n";
  _responseBuffer += "<html><body>\n";
  _responseBuffer += "plot to be inserted here...\n";
  _responseBuffer += "</body></html>\n";
}

void req_overviewPage::createResponseBuffer()
{
  // get list of published (=visible/unhidden) histograms
  cass::PostProcessors *pp(cass::PostProcessors::instance(""));
  cass::IdList* idlist(pp->getIdList());
  cass::PostProcessors::keyList_t list = idlist->getList();
  _responseBuffer = "";
  _responseBuffer += "<html><body>\n";
  _responseBuffer += "List of active and published postprocessors:\n";
  for (cass::PostProcessors::keyList_t::const_iterator it=list.begin(); it!=list.end(); ++it)
    _responseBuffer += "<li><a href=\"" + std::string(ADDR_HIST2DPage) + *it + ".html\">" + *it + "</a></li>\n";
  _responseBuffer += "end of list.\n";
  _responseBuffer += "</body></html>\n";
}
// serve functions:
// handle_request is called from server thread. It passes a MHD_Connection
// and address to the serve functions, lets parseAddress create
// a request object and let the request object serve itself.
int httpServer::handle_request(void *cls, struct MHD_Connection *connection, const char *address,
                   const char *method, const char *version, const char *upload_data,
                   size_t *upload_data_size, void **con_cls)
{
  requestType* reqOb = parseAddress(address, connection);
  reqOb->sendResponse();
  delete reqOb;
  std::cout << "handle_request connection: " << connection << std::endl;
  return 1;
}
int httpServer::handle_request_callback(void *cls, struct MHD_Connection *connection, const char *address,
                   const char *method, const char *version, const char *upload_data,
                   size_t *upload_data_size, void **con_cls)
{
  httpServer* _this = (httpServer*) cls;
  std::cout << "handle_request_callback connection: " << connection << std::endl;
  return _this->handle_request(cls, connection, address, method, version, upload_data, upload_data_size, con_cls);
}
requestType* httpServer::parseAddress(const char* address, MHD_Connection* connection)
{
  int begin = strlen(address); // offset of address substring
  // look for lowest level in address:
  while (begin>0) {
    if (address[begin-1]=='/') break;
    --begin;
  }
  std::cout << "parseAddress: " << address << std::endl;
  std::cout << "parseAddress2: " << address+begin << std::endl;
  if (!strncmp(address+begin, ADDR_HIST2DImage, strlen(ADDR_HIST2DImage))) return new req_histogram2DImage(connection, address+begin+strlen(ADDR_HIST2DImage), *this);
  if (!strncmp(address+begin, ADDR_HIST2DPage, strlen(ADDR_HIST2DPage))) return new req_histogram2DPage(connection, address+begin+strlen(ADDR_HIST2DPage), *this);
  if (!strncmp(address+begin, ADDR_HIST1DImage, strlen(ADDR_HIST1DImage))) return new req_histogram1DImage(connection, address+begin+strlen(ADDR_HIST1DImage), *this);
  if (!strncmp(address+begin, ADDR_HIST1DPage, strlen(ADDR_HIST1DPage))) return new req_histogram1DPage(connection, address+begin+strlen(ADDR_HIST1DPage), *this);
  return new req_overviewPage(connection, address+begin, *this);
}

