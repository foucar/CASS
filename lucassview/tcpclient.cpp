//Copyright (C) 2011 Lutz Foucar

/**
 * @file tcpclient.cpp file contains the classes connect to cass
 *
 * @author Lutz Foucar
 */

#include "tcpclient.h"
#include "soapCASSsoapProxy.h"
#include "CASSsoap.nsmap"

using namespace lucassview;

TCPClient::TCPClient(const std::string &server)
{

}

std::list<std::string> TCPClient::operator() ()const
{
  bool ret;
  CASSsoapProxy cass;
  //	cass.soap_endpoint = (_servername->text() + ":" + _serverport->text()).toStdString().c_str();
  cass.soap_endpoint = "xfhix:12321";

  std::string string;
  cass.getHistogram(string,0,&ret);
  //	if(ret)
  //			cout << "return value: 'true'" << endl;
  //	else {
  //			cout << "return value is 'false'" << endl;
  //			return;
  //	}

  soap_multipart::iterator attachment = cass.dime.begin();
}

cass::HistogramBackend *TCPClient::operator() (const std::string &histogramkey)const
{

}
