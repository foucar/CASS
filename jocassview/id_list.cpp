// Copyright (C) 2010 -2015 Lutz Foucar

/**
 * @file jocassview/id_list.cpp contains the classes that can serialize
 *                              the key list
 *
 * @author Lutz Foucar
 */

#include "id_list.h"

#if QT_VERSION >= 0x050000
#include <QtWidgets/QMessageBox>
#else
#include <QtGui/QMessageBox>
#endif


using namespace jocassview;
using namespace std;

IdList::IdList()
    : Serializable(1),
      _size(0)
{

}

IdList::IdList(const QStringList &list)
    : Serializable(1),
      _list(list),
      _size(0)
{

}

IdList::IdList(cass::SerializerBackend &in)
  : Serializable(1)
{
  deserialize(in);
}

const QStringList& IdList::getList()const
{
  return _list;
}

void IdList::serialize(cass::SerializerBackend &out) const
{
  out.startChecksumGroupForWrite();
  writeVersion(out);
  out.add(_size);
  out.endChecksumGroupForWrite();
  for (int i(0); i < _list.size() ; ++i)
    out.add(_list.at(i).toStdString());
}

bool IdList::deserialize(cass::SerializerBackend &in)
{
  _list.clear();
  in.startChecksumGroupForRead();
  uint16_t ver(in.retrieve<uint16_t>());
  if(ver != _version)
  {
    QMessageBox::information(0, QObject::tr("IDList"),
                             QObject::tr("Error: Deserialization version doesn't match"));
    return false;
  }
  _size = in.retrieve<size_t>();
  if (!in.endChecksumGroupForRead())
  {
    QMessageBox::information(0, QObject::tr("IDList"),
                             QObject::tr("Error: Wrong Checksum"));
    return false;
  }
  for(size_t ii=0; ii<_size; ++ii)
    _list.append(QString::fromStdString(in.retrieve<string>()));
  return true;
}
