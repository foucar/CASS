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
    bool ret;

    if(argc < 3) {
        cerr << "Must specify 'server:port' and image-'type' on commandline!" << endl;
        return 1;
    }
    cout << "Connecting to server at " << argv[1] << endl;
    // define proxy instance
    CASSsoapProxy cass;
    cass.soap_endpoint = argv[1];
    // quit on type == -1
    if(-1 == QString(argv[2]).toInt()) {
        // quit server
        cass.quit(&ret);
        if(ret)
            cout << "quit server return value: 'true'" << endl;
        else
            cout << "quit server return value is 'false'" << endl;
    } else {
        // get an image
        cass.getImage(2, QString(argv[2]).toInt(), &ret);
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
            QFile imgfile(QString("image-") + QString(argv[2]) + ".png");
            imgfile.open(QIODevice::WriteOnly);
            imgfile.write((*attachment).ptr, (*attachment).size);
            imgfile.close();
        }
        // get a histogram
        cass.getHistogram(QString(argv[2]).toInt(), &ret);
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
            QFile histfile(QString("hist-") + QString(argv[2]) + ".dat");
            histfile.open(QIODevice::WriteOnly);
            histfile.write((*attachment).ptr, (*attachment).size);
            histfile.close();
        }
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
