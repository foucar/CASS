#include <microhttpd.h>
#include <iostream>
#include <string>

#include "histogram_getter.h"

#define HTTP_PORT 8000
#define ADDR_HIST2DImage "hst2im"
#define ADDR_HIST2DPage "hst2pg"
#define ADDR_HIST1DImage "hst1im"
#define ADDR_HIST1DPage "hst1pg"

class httpServer;

class requestType
{
  public:
    requestType(MHD_Connection* connection, const char* address, httpServer& server) : _connection(connection), _address(address), _responseBuffer(""), _server(server) {}
    virtual ~requestType() {
      MHD_destroy_response(_response);
    }
    void sendResponse() {  // public interface function: wrap response in http and send it.
      createResponseBuffer();
      _response = MHD_create_response_from_buffer(_responseBuffer.size(), (void*) _responseBuffer.c_str(), MHD_RESPMEM_MUST_COPY);  // TODO: need something more clever than just copying to keep data around... maybe a queue like in soap server.
      /*int ret = */MHD_queue_response(_connection, MHD_HTTP_OK, _response);
      //MHD_destroy_response(_response);   // TODO: segfault!?
    }
  protected:
    virtual void createResponseBuffer() = 0;   // virtual function, has to provide (i.e. fill into _responseBuffer) the data that is about to be send.
    std::string _address;
    std::string _responseBuffer;
    MHD_Response * _response;
    MHD_Connection* _connection;
    httpServer& _server;
};


class req_histogram2DImage : public requestType
{
  public:
    req_histogram2DImage(MHD_Connection* connection, const char* address, httpServer& server) :requestType(connection, address, server){}
  private:
    virtual void createResponseBuffer();
};
class req_histogram2DPage: public requestType
{
  public:
    req_histogram2DPage(MHD_Connection* connection, const char* address, httpServer& server) :requestType(connection, address, server){}
  private:
    virtual void createResponseBuffer();
};
class req_histogram1DImage : public requestType
{
  public:
    req_histogram1DImage(MHD_Connection* connection, const char* address, httpServer& server) :requestType(connection, address, server){}
  private:
    virtual void createResponseBuffer();
};
class req_histogram1DPage: public requestType
{
  public:
    req_histogram1DPage(MHD_Connection* connection, const char* address, httpServer& server) :requestType(connection, address, server){}
  private:
    virtual void createResponseBuffer();
};
class req_overviewPage: public requestType
{
  public:
    req_overviewPage(MHD_Connection* connection, const char* address, httpServer& server) :requestType(connection, address, server){}
  private:
    virtual void createResponseBuffer();
};


class httpServer
{
  public:
    httpServer(const cass::HistogramGetter& histogram_getter):_histogram_getter(histogram_getter) {}
    void start() { _daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY, HTTP_PORT, NULL, NULL,
                                            &httpServer::handle_request_callback, (void*)this, MHD_OPTION_END); }
    void stop() { MHD_stop_daemon(_daemon); }
    const cass::HistogramGetter& histogram_getter() {return _histogram_getter;}
    // serve functions:
    // handle_request is called from server thread. It passes a MHD_Connection
    // and address to the serve functions, lets parseAddress create
    // a request object and let the request object serve itself.
    int handle_request(void *cls, struct MHD_Connection *connection, const char *address,
                       const char *method, const char *version, const char *upload_data,
                       size_t *upload_data_size, void **con_cls);
    static int handle_request_callback(void *cls, struct MHD_Connection *connection, const char *address,
                       const char *method, const char *version, const char *upload_data,
                       size_t *upload_data_size, void **con_cls);
    requestType* parseAddress(const char* address, MHD_Connection* connection);

  private:
    struct MHD_Daemon *_daemon;
    const cass::HistogramGetter& _histogram_getter;
};

