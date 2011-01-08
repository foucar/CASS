//Copyright (C) 2011 Lutz Foucar

/**
 * @file tcpclient.cpp file contains the classes connect to cass
 *
 * @author Lutz Foucar
 */
#include <stdexcept>
#include <sstream>

#include "tcpclient.h"
#include "soapCASSsoapProxy.h"
#include "CASSsoap.nsmap"
#include "id_list.h"
#include "histogram.h"

using namespace lucassview;

TCPClient::TCPClient(const std::string &server)
  :_server(server)
{}

std::list<std::string> TCPClient::operator() ()const
{
  using namespace std;
  bool ret(false);
  CASSsoapProxy client;
  client.soap_endpoint = _server.c_str();
  int retcode (client.getPostprocessorIds(&ret));
  if( (retcode != SOAP_OK) || !ret)
  {
    stringstream ss;
    ss << "Could not retrieve the List of Histograms on '"<<_server<<"'";
    throw runtime_error(ss.str());
  }
  soap_multipart::iterator attachment (client.dime.begin());
  if(client.dime.end() == attachment)
  {
    stringstream ss;
    ss << "There is no attachmend in the received soap data";
    throw runtime_error(ss.str());
  }
//  cout << "TCPClient: DIME attachment:" << endl
//      << " TCPClient: Memory=" << (void*)(*attachment).ptr << endl
//      << " TCPClient: Size=" << (*attachment).size << endl
//      << " TCPClient: Type=" << ((*attachment).type?(*attachment).type:"null") << endl
//      << " TCPClient: ID=" << ((*attachment).id?(*attachment).id:"null") << endl;
  cass::Serializer serializer( std::string((char *)(*attachment).ptr, (*attachment).size) );
  cass::IdList list(serializer);
  std::list<std::string> returnlist (list.getList());
  return returnlist;
}

cass::HistogramBackend *TCPClient::operator() (const std::string &histogramkey)const
{
  using namespace std;
  bool ret(false);
  CASSsoapProxy client;
  client.soap_endpoint = _server.c_str();
  client.getHistogram(histogramkey,0,&ret);
  if(!ret)
  {
    stringstream ss;
    ss << "Did not get Histogram with key '"<<histogramkey<<"'";
    throw runtime_error(ss.str());
  }
  soap_multipart::iterator attachment(client.dime.begin());
  if(client.dime.end() == attachment)
  {
    stringstream ss;
    ss << "There is no attachmend in the received soap data";
    throw runtime_error(ss.str());
  }
//  cout << "TCPClient: DIME attachment:" << endl
//      << " TCPClient: Memory=" << (void*)(*attachment).ptr << endl
//      << " TCPClient: Size=" << (*attachment).size << endl
//      << " TCPClient: Type=" << ((*attachment).type?(*attachment).type:"null") << endl
//      << " TCPClient: ID=" << ((*attachment).id?(*attachment).id:"null") << endl;
  std::string mimeType((*attachment).type);
  cass::HistogramBackend* hist(0);
  if(mimeType == "application/cass2Dhistogram")
  {
      cass::Serializer serializer( std::string((char *)(*attachment).ptr, (*attachment).size) );
      hist = new cass::Histogram2DFloat(serializer);
  }
  else if(mimeType == "application/cass1Dhistogram")
  {
      cass::Serializer serializer( std::string((char *)(*attachment).ptr, (*attachment).size) );
      hist = new cass::Histogram1DFloat(serializer);
  }
  else if(mimeType == "application/cass0Dhistogram")
  {
      cass::Serializer serializer( std::string((char *)(*attachment).ptr, (*attachment).size) );
      hist = new cass::Histogram0DFloat(serializer);
  }
  else
  {
    stringstream ss;
    ss << "The mime type '"<<mimeType<<"' is unknown";
    throw runtime_error(ss.str());
  }
  return hist;
}
