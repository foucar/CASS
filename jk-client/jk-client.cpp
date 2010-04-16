// Copyright (C) 2010 Jochen Küpper

#include <iostream>
#include <QtCore/QByteArray>
#include <QtCore/QFile>
#include <QtGui/QImage>

#include "soapCASSsoapProxy.h"
#include "CASSsoap.nsmap"

using namespace std;


int main(int argc, char *argv[])
{
    if(argc < 2) {
        cerr << "Must specify 'server:port' on commandline!" << endl;
        return 1;
    }
    cout << "Connecting to server at " << argv[1] << endl;
    // define proxy instance
    CASSsoapProxy cass;
    cass.soap_endpoint = argv[1];

    bool ret;
    cass.getImage(1, 101, &ret);
    if(ret)
        cout << "return value: 'true'" << endl;
    else
        cout << "return value is 'false'" << endl;

    for(soap_multipart::iterator attachment = cass.dime.begin(); attachment != cass.dime.end(); ++attachment) {
        cout << "DIME attachment:" << endl;
        cout << "Memory=" << (void*)(*attachment).ptr << endl;
        cout << "Size=" << (*attachment).size << endl;
        cout << "Type=" << ((*attachment).type?(*attachment).type:"null") << endl;
        cout << "ID=" << ((*attachment).id?(*attachment).id:"null") << endl;
        QByteArray data((*attachment).ptr, (*attachment).size);
        QFile imgfile("image.tiff");
        imgfile.open(QIODevice::WriteOnly);
        imgfile.write(data);
        imgfile.close();
    }
    return 0;
}






// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
