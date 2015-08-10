// Copyright (C) 2010, 2015 Lutz Foucar

/**
 * @file cass/processing/id_list.h file contains the classes that can
 *               serialize the key list
 *
 * @author Stephan Kassemeyer
 */

#ifndef __IDLIST_H__
#define __IDLIST_H__

#include <list>

#include "serializable.hpp"

namespace cass
{
/** id-list
 *
 * used for SOAP communication of id-lists
 *
 * @author Stephan Kassemeyer
 * @author Lutz Foucar
 */
class IdList : public Serializable
{
public:
  /** define the list of names */
  typedef std::list<std::string> names_t;

  /** default constructor */
  IdList()
    : Serializable(1), _size(0)
  {}

  /** construct from serializer */
  IdList( SerializerBackend &in)
    : Serializable(1)
  {
    deserialize(in);
  }

  /** clear the list */
  void clear();

  /** getter for the internal list */
  const names_t& getList() { return _list; }

  /** deserialize the list from the serializer */
  bool deserialize(SerializerBackend &in);

  /** serialize the list to the serializer */
  void serialize(SerializerBackend &out)const;

private:
  /** a list of all processor keys */
  names_t _list;

  /** the size of the processor keys list */
  size_t _size;
};

} //end namespace
#endif
