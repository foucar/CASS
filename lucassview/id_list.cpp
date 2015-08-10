// Copyright (C) 2010, 2015 Lutz Foucar

/**
 * @file cass/processing/id_list.cpp file contains the classes that can
 *              serialize the key list
 *
 * @author Stephan Kassemeyer
 */


#include "id_list.h"

using namespace cass;

void IdList::serialize(SerializerBackend &out)const
{
  out.startChecksumGroupForWrite();
  writeVersion(out);
  out.add(_size);
  out.endChecksumGroupForWrite();
  for (names_t::const_iterator it=_list.begin(); it!=_list.end(); it++)
    out.add(*it);
}

bool IdList::deserialize(SerializerBackend &in)
{
  _list.clear();
  //check whether the version fits//
  in.startChecksumGroupForRead();
  checkVersion(in);
  _size = in.retrieve<size_t>();
  if (!in.endChecksumGroupForRead())
    return false;
  for(size_t ii=0; ii<_size; ++ii)
    _list.push_back(in.retrieve<std::string>());
  return true;
}

