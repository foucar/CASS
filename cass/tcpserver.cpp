// CASS TCP server
//
// Copyright (C) 2010 Jochen Küpper
// Copyright (C) 2010 Uwe Hoppe, FHI Berlin

#include <iostream>
#include <string>
#include <QtNetwork>

#include "tcpserver.h"


namespace cass
{
namespace TCP
{


void Server::incomingConnection(int id)
{
    Socket *socket = new Socket(this);
    connect(socket, SIGNAL(quit()), this, SLOT(emit_quit()));
    connect(socket, SIGNAL(readini()), this, SLOT(emit_readini()));
    socket->setSocketDescriptor(id);
#warning Do we need to delete socket?
}



Socket::Socket(QObject *parent)
    : QTcpSocket(parent)
{
    connect(this, SIGNAL(readyRead()), this, SLOT(readClient()));
    connect(this, SIGNAL(disconnected()), this, SLOT(deleteLater()));
    _nextblocksize = 0;
}



void Socket::readClient()
{
    // create datastream
    QDataStream in(this);
    in.setVersion(QDataStream::Qt_4_3);
    // read next cmd
    if(_nextblocksize == 0) {
        if(bytesAvailable() < qint64(sizeof(quint16)))
            return;
        in >> _nextblocksize;
    }
    if(bytesAvailable() < _nextblocksize)
        return;
    quint16 cmd(0);
    in >> cmd;
    quint32 what(0);
    // evaluate cmd
    switch(cmd) {
    case READINI:
        in >> what;
        emit readini(size_t(what));
        break;
    case EVENT: {
        quint32 t1, t2;
        in >> what >> t1 >> t2;
        const std::string event(dynamic_cast<Server *>(parent())->get_event(EventParameter(what, t1, t2)));
#warning send event back
        break;
    }
    case HISTOGRAM: {
        quint32 type;
        in >> type;
        std::string hist(dynamic_cast<Server *>(parent())->get_histogram(HistogramParameter(type)));
#warning send histogram back
        break;
    }
    case QUIT:
        emit quit();
        close();
        return;
    default:
        std::cerr << "Unknown command requested !" << std::endl;
        QDataStream out(this);
        out << quint16(0xFFFE);
        return;
    }
    QDataStream out(this);
    out << quint16(0xFFFF);
    _nextblocksize = 0;
}


} // namespace TCP
} // namespace cass



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
