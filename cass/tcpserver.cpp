// CASS TCP server
//
// Copyright (C) 2010 Jochen Küpper
// Copyright (C) 2010 Uwe Hoppe, FHI Berlin

#include <iostream>
#include <map>
#include <string>
#include <QtNetwork>

#include "tcpserver.h"


namespace cass
{
namespace TCP
{

using namespace std;


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
    QDataStream in(this);
    in.setVersion(QDataStream::Qt_4_3);
    if(_nextblocksize == 0) {
        if(bytesAvailable() < qint64(sizeof(quint16)))
            return;
        in >> _nextblocksize;
    }
    if(bytesAvailable() < _nextblocksize)
        return;
    map<QString, cmd_t> commands;
    map<QString, cmd_t>::const_iterator cit;
    commands.insert(make_pair("CASS:READINI", READINI));
    commands.insert(make_pair("CASS:EVENT", EVENT));
    commands.insert(make_pair("CASS:HISTOGRAM", HISTOGRAM));
    commands.insert(make_pair("CASS:QUIT", QUIT));
    QString str;
    in >> str;
    cit = commands.find(str.section(':', 0, 1));
    switch(cit->second) {
    case READINI: {
        quint32 what;
        in >> what;
        emit readini(what);
        QByteArray block;
        QDataStream out(&block, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_4_3);
        out << quint64(0)
            << quint16(READINI)
            << quint16(0xFFFF);
        out.device()->seek(0);
        out << quint64(block.size() - sizeof(quint64));
        write(block);
        break;
        }
    case EVENT: {
        quint32 what;
        quint32 t1, t2;
        in >> what >> t1 >> t2;
        const std::string event(dynamic_cast<Server *>(parent())->get_event(EventParameter(what, t1, t2)));
        QByteArray block(event.c_str());
        quint64 size(block.size());
        block.push_front(size);
        write(block);
        break;
        }
    case HISTOGRAM: {
        quint32 type;
        in >> type;
        std::string hist(dynamic_cast<Server *>(parent())->get_histogram(HistogramParameter(type)));
        QByteArray block(hist.c_str());
        quint64 size(block.size());
        block.push_front(size);
        write(block);
        break;
        }
    case QUIT: {
        emit quit();
        close();
        return;
        }
    default:
        cerr << "Unknown command requested !" << endl;
        QByteArray block;
        QDataStream out(&block, QIODevice::WriteOnly);
        out.setVersion(QDataStream::Qt_4_3);
        out << quint64(0)
            << "CASS:ERROR"
            << quint16(0xFFFF);
        out.device()->seek(0);
        out << quint64(block.size() - sizeof(quint64));
        write(block);
    }
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
