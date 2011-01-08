// Copyright (C) 2010 Lutz Foucar

/**
 * @file id_list.cpp file contains the classes that can serialize the key list
 *
 * @author Lutz Foucar
 */

#include <iostream>

#include "id_list.h"


cass::IdList::IdList(const std::list<std::string>& list)
  : Serializable(1), _list(list), _size(list.size())
{
  (std::cerr << "IdList::IdList(): Initial list size = " << _size << std::endl);
}

bool cass::IdList::deserialize(SerializerBackend *in)
{
  _list.clear();
  //check whether the version fits//
  in->startChecksumGroupForRead();
  uint16_t ver = in->retrieveUint16();
  if(ver != _version)
  {
    std::cerr<<"IdList::deserialize(): version conflict in IdList: "<<ver<<" "<<_version<<std::endl;
    return false;
  }
  _size = in->retrieveSizet();
  (std::cerr << "IdList::deserialize(): list size " << _size << std::endl);
  if (!in->endChecksumGroupForRead())
  {
    (std::cerr<<"IdList::deserialize(): wrong checksum IdList"<<std::endl);
    return false;
  }
  for(size_t ii=0; ii<_size; ++ii)
    _list.push_back(in->retrieveString());
  (std::cerr << "IdList::deserialize(): list is done " << std::endl);
  return true;
}

void cass::IdList::serialize(SerializerBackend *out)const
{
  out->startChecksumGroupForWrite();
  out->addUint16(_version);
  out->addSizet(_size);
  out->endChecksumGroupForWrite();
  for (std::list<std::string>::const_iterator it=_list.begin();
       it!=_list.end();
       it++)
    out->addString(*it);
}

void cass::IdList::clear()
{
  _list.clear();
  _size=0;
}

void cass::IdList::setList(const std::list<std::string> &list)
{
  clear();
  _list = list;
  _size = list.size();
}
