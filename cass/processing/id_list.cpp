// Copyright (C) 2010, 2015 Lutz Foucar

/**
 * @file cass/processing/id_list.cpp file contains the classes that can
 *              serialize the key list
 *
 * @author Stephan Kassemeyer
 */


#include "id_list.h"

#include "log.h"

using namespace cass;

IdList::IdList(const Processor::names_t &list)
  : Serializable(1),
    _list(list),
    _size(list.size())
{
  Log::add(Log::VERBOSEINFO,"IdList::IdList(): Initial list size = "  +
           toString(_size));
}

void IdList::serialize(SerializerBackend &out)const
{
  out.startChecksumGroupForWrite();
  writeVersion(out);
  out.add(_size);
  out.endChecksumGroupForWrite();
  for (Processor::names_t::const_iterator it=_list.begin(); it!=_list.end(); it++)
    out.add(*it);
}

bool IdList::deserialize(SerializerBackend &in)
{
  _list.clear();
  //check whether the version fits//
  in.startChecksumGroupForRead();
  checkVersion(in);
  _size = in.retrieve<size_t>();
  Log::add(Log::VERBOSEINFO,"IdList::deserialize(): list size " +
           toString(_size));
  if (!in.endChecksumGroupForRead())
  {
    Log::add(Log::VERBOSEINFO,"IdList::deserialize(): wrong checksum IdList");
    return false;
  }
  for(size_t ii=0; ii<_size; ++ii)
    _list.push_back(in.retrieve<std::string>());
  Log::add(Log::VERBOSEINFO,"IdList::deserialize(): list is done ");
  return true;
}

void IdList::clear()
{
  _list.clear();
  _size=0;
}

void IdList::setList(const Processor::names_t &list)
{
  clear();
  _list = list;
  _size = list.size();
}
