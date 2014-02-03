//Copyright (C) 2011 Lutz Foucar

/**
 * @file tcpclient.cpp file contains the classes connect to cass
 *
 * @author Lutz Foucar
 */
#include <stdexcept>
#include <sstream>
#include <string>

#include "tcpclient.h"
#include "soapCASSsoapProxy.h"
#include "CASSsoap.nsmap"
//#include "id_list.h"
#include "histogram.h"

using namespace jocassview;
using namespace std;

TCPClient::TCPClient()
  : _transferredBytes(0)
{

}

TCPClient::TCPClient(const QString &server)
  :_server(server),
   _transferredBytes(0)
{}

QStringList TCPClient::getIdList()const
{
//  bool ret(false);
//  CASSsoapProxy client;
//  client.soap_endpoint = _server.c_str();
//  int retcode (client.getPostprocessorIds(&ret));
//  if( (retcode != SOAP_OK) || !ret)
//  {
//    stringstream ss;
//    ss << "TCPClient(): Could not retrieve the List of Histograms on '"<<_server<<"'";
//    throw runtime_error(ss.str());
//  }
//  soap_multipart::iterator attachment (client.dime.begin());
//  if(client.dime.end() == attachment)
//  {
//    stringstream ss;
//    ss << "There is no attachmend in the received soap data";
//    throw runtime_error(ss.str());
//  }
////  cout << "TCPClient: DIME attachment:" << endl
////      << " TCPClient: Memory=" << (void*)(*attachment).ptr << endl
////      << " TCPClient: Size=" << (*attachment).size << endl
////      << " TCPClient: Type=" << ((*attachment).type?(*attachment).type:"null") << endl
////      << " TCPClient: ID=" << ((*attachment).id?(*attachment).id:"null") << endl;
//  _transferredBytes += (*attachment).size;
//  cass::Serializer serializer( std::string((char *)(*attachment).ptr, (*attachment).size) );
//  cass::IdList list(serializer);
//  std::list<std::string> returnlist (list.getList());
//  return returnlist;
}

std::tr1::shared_ptr<cass::HistogramFloatBase> TCPClient::getData(const QString &histogramkey)const
{
//  using namespace std;
//  using namespace std::tr1;
//  using namespace cass;
//  bool ret(false);
//  CASSsoapProxy client;
//  client.soap_endpoint = _server.c_str();
//  client.getHistogram(histogramkey,0,&ret);
//  if(!ret)
//  {
//    stringstream ss;
//    ss << "TCPClient(): Did not get Histogram with key '"<<histogramkey<<"'";
//    throw runtime_error(ss.str());
//  }
//  soap_multipart::iterator attachment(client.dime.begin());
//  if(client.dime.end() == attachment)
//  {
//    stringstream ss;
//    ss << "TCPClient(key): There is no attachmend in the received soap data";
//    throw runtime_error(ss.str());
//  }
////  cout << "TCPClient: DIME attachment:" << endl
////      << " TCPClient: Memory=" << (void*)(*attachment).ptr << endl
////      << " TCPClient: Size=" << (*attachment).size << endl
////      << " TCPClient: Type=" << ((*attachment).type?(*attachment).type:"null") << endl
////      << " TCPClient: ID=" << ((*attachment).id?(*attachment).id:"null") << endl;
//  _transferredBytes += (*attachment).size;
//  string mimeType((*attachment).type);
//  shared_ptr<HistogramFloatBase> hist;
//  if(mimeType == "application/cass2Dhistogram")
//  {
//      cass::Serializer serializer( std::string((char *)(*attachment).ptr, (*attachment).size) );
//      hist = shared_ptr<HistogramFloatBase>(new Histogram2DFloat(serializer));
//  }
//  else if(mimeType == "application/cass1Dhistogram")
//  {
//      cass::Serializer serializer( std::string((char *)(*attachment).ptr, (*attachment).size) );
//      hist = shared_ptr<HistogramFloatBase>(new Histogram1DFloat(serializer));
//  }
//  else if(mimeType == "application/cass0Dhistogram")
//  {
//      cass::Serializer serializer( std::string((char *)(*attachment).ptr, (*attachment).size) );
//      hist = shared_ptr<HistogramFloatBase>(new Histogram0DFloat(serializer));
//  }
//  else
//  {
//    stringstream ss;
//    ss << "The mime type '"<<mimeType<<"' is unknown";
//    throw runtime_error(ss.str());
//  }
//  return hist;
}

size_t TCPClient::receivedBytes()const
{
  return _transferredBytes;
}

void TCPClient::reloadIni() const
{
//  bool ret(false);
//  CASSsoapProxy client;
//  client.soap_endpoint = _server.c_str();
//  client.readini(0, &ret);
//  if(!ret)
//    throw runtime_error("TCPClient::reloadIni(): Could not communicate writeini command");
}

void TCPClient::broadcastCommand(const QString &command) const
{
//  bool ret(false);
//  CASSsoapProxy client;
//  client.soap_endpoint = _server.c_str();
//  client.controlDarkcal(command, &ret);
//  if(!ret)
//    throw runtime_error("TCPClient::controlCalibration(): Could not communicate command '"+ command +"'");
}

void TCPClient::sendCommandTo(const QString &key, const QString &command) const
{

}

void TCPClient::setServer(const QString &serverstring)
{
  _server = serverstring;
}

void TCPClient::quitServer() const
{

}

void TCPClient::clearHistograms(QString key) const
{

}
