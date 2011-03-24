#include "httpserver.h"
#include "postprocessing/postprocessor.h"
#include "postprocessing/id_list.h"



void req_histogram2DImage::createResponseBuffer()
{
  _responseBuffer = "";
  // todo: parse address (histogram name) and get requested histogram
  // todo: fill buffer with image of histogram
}

void req_histogram2DPage::createResponseBuffer()
{
  _responseBuffer = "";
  _responseBuffer += "<html><body>\n";
  _responseBuffer += "image to be inserted here...\n";
  _responseBuffer += "<html><body>\n";
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
  _responseBuffer += "<html><body>\n";
}
// serve functions:
// handle_request is called from server thread. It passes a MHD_Connection
// and address to the serve functions, lets parseAddress create
// a request object and let the request object serve itself.
int handle_request(void *cls, struct MHD_Connection *connection, const char *address,
                   const char *method, const char *version, const char *upload_data,
                   size_t *upload_data_size, void **con_cls)
{
  requestType* reqOb = parseAddress(address, connection);
  reqOb->sendResponse();
  delete reqOb;
}
requestType* parseAddress(const char* address, MHD_Connection* connection)
{
  int begin = strlen(address); // offset of address substring
  // look for lowest level in address:
  while (begin>0) {
    if (address[begin-1]=='/') break;
    --begin;
  }
  if (!strncmp(address+begin, ADDR_HIST2DImage, strlen(ADDR_HIST2DImage))) return new req_histogram2DImage(connection, address+begin+strlen(ADDR_HIST2DImage));
  if (!strncmp(address+begin, ADDR_HIST2DPage, strlen(ADDR_HIST2DPage))) return new req_histogram2DPage(connection, address+begin+strlen(ADDR_HIST2DPage));
  return new req_overviewPage(connection, address+begin);
}

