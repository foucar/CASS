// Copyright (C) 2010 Lutz Foucar

#include "id_list.h"


cass::IdList(const PostProcessors::postprocessorkeysList_t& list)
  : Serializable(1), _list(list), _size(list.size())
{
  VERBOSEOUT(std::cerr << "Initial list size = " << _size << std::endl);
}

bool cass::IdList::deserialize(SerializerBackend *in)
{
  _list.clear();
  //check whether the version fits//
  in->startChecksumGroupForRead();
  uint16_t ver = in->retrieveUint16();
  if(ver != _version)
  {
    std::cerr<<"version conflict in IdList: "<<ver<<" "<<_version<<std::endl;
    return false;
  }
  _size = in->retrieveSizet();
  VERBOSEOUT(std::cerr << "list size " << _size << std::endl);
  if (!in->endChecksumGroupForRead())
  {
    VERBOSEOUT(std::cerr<<"wrong checksum IdList"<<std::endl);
    return false;
  }
  for(size_t ii=0; ii<_size; ++ii)
    _list.push_back(in->retrieveString());
  VERBOSEOUT(std::cerr << "list is done " << std::endl);
  return true;
}

void cass::IdList::serialize(SerializerBackend *out)
{
  out->startChecksumGroupForWrite();
  out->addUint16(_version);
  out->addSizet(_size);
  out->endChecksumGroupForWrite();
  for (PostProcessors::postprocessorkeysList_t::iterator it=_list.begin();
       it!=_list.end();
       it++)
    out->addString(*it);
}

void cass::IdList::clear()
{
  _list.clear();
  _size=0;
}

void setList(const PostProcessors::postprocessors_t &list)
{
  clear();
  _list = list;
  _size = list.size();
}
