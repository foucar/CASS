//Copyright (C) 2011 Lutz Foucar

/**
 * @file lucassview/tcpclient.cpp file contains the classes connect to cass
 *
 * @author Lutz Foucar
 */
#include <stdexcept>
#include <sstream>
#include <string>

#include "tcpclient.h"
#include "soapCASSsoapProxy.h"
#include "CASSsoap.nsmap"
#include "id_list.h"

using namespace lucassview;

TCPClient::TCPClient(const std::string &server)
  :_server(server),
   _transferredBytes(0)
{}

void TCPClient::reloadIni() const
{
  using namespace std;
  bool ret(false);
  CASSsoapProxy client;
  client.soap_endpoint = _server.c_str();
  client.readini(0, &ret);
  if(!ret)
    throw runtime_error("TCPClient::reloadIni(): Could not communicate writeini command");
}

void TCPClient::controlCalibration(const std::string &command) const
{
  using namespace std;
  bool ret(false);
  CASSsoapProxy client;
  client.soap_endpoint = _server.c_str();
  client.controlDarkcal(command, &ret);
  if(!ret)
    throw runtime_error("TCPClient::controlCalibration(): Could not communicate command '"+ command +"'");
}

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
    ss << "TCPClient(): Could not retrieve the List of Histograms on '"<<_server<<"'";
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
  _transferredBytes += (*attachment).size;
  cass::Serializer serializer( std::string((char *)(*attachment).ptr, (*attachment).size) );
  cass::IdList list(serializer);
  std::list<std::string> returnlist (list.getList());
  return returnlist;
}

cass::Result<float>::shared_pointer TCPClient::operator() (const std::string &histogramkey)const
{
  using namespace std;
  using namespace std::tr1;
  using namespace cass;
  bool ret(false);
  CASSsoapProxy client;
  client.soap_endpoint = _server.c_str();
  client.getHistogram(histogramkey,0,&ret);
  if(!ret)
  {
    stringstream ss;
    ss << "TCPClient(): Did not get Histogram with key '"<<histogramkey<<"'";
    throw runtime_error(ss.str());
  }
  soap_multipart::iterator attachment(client.dime.begin());
  if(client.dime.end() == attachment)
  {
    stringstream ss;
    ss << "TCPClient(key): There is no attachmend in the received soap data";
    throw runtime_error(ss.str());
  }
//  cout << "TCPClient: DIME attachment:" << endl
//      << " TCPClient: Memory=" << (void*)(*attachment).ptr << endl
//      << " TCPClient: Size=" << (*attachment).size << endl
//      << " TCPClient: Type=" << ((*attachment).type?(*attachment).type:"null") << endl
//      << " TCPClient: ID=" << ((*attachment).id?(*attachment).id:"null") << endl;
  _transferredBytes += (*attachment).size;
  cass::Result<float>::shared_pointer result(new cass::Result<float>());
  cass::Serializer serializer( std::string((char *)(*attachment).ptr, (*attachment).size) );
  serializer >> *result;
  return result;
}
