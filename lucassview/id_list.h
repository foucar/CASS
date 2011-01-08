// Copyright (C) 2010 Lutz Foucar

/**
 * @file id_list.h file contains the classes that can serialize the key list
 *
 * @author Lutz Foucar
 */

#ifndef __IDLIST_H__
#define __IDLIST_H__

#include <string>
#include <list>

#include "serializable.h"

namespace cass
{
  /** id-list
   *
   * used for SOAP communication of id-lists (copy of the IdList of cass, but
   * without the qt components).
   *
   * @author Stephan Kassemeyer
   * @author Lutz Foucar
   */
  class IdList : public Serializable
  {
  public:

    /** default constructor */
    IdList()
      : Serializable(1), _size(0)
    {}

    /** constructor creating list from incomming list */
    IdList(const std::list<std::string>& list);

    /** construct from serializer */
    IdList(SerializerBackend* in)
      : Serializable(1)
    {
      deserialize(in);
    }

    /** @overload */
    IdList( SerializerBackend &in)
      : Serializable(1)
    {
      deserialize(in);
    }

    /** clear the list */
    void clear();

    /** copy the list to us */
    void setList(const std::list<std::string> &list);

    /** getter for the internal list */
    const std::list<std::string> &getList() { return _list; }

    /** deserialize the list from the serializer */
    bool deserialize(SerializerBackend *in);

    /** @overload */
    bool deserialize(SerializerBackend& in) { return deserialize(&in); }

    /** serialize the list to the serializer */
    void serialize(SerializerBackend *out)const;

    /** @overload */
    void serialize(SerializerBackend &out)const { serialize(&out); }

  private:
    /** a list of all postprocessor keys */
    std::list<std::string> _list;

    /** the size of the postprocessor keys list */
    size_t _size;
  };

} //end namespace
#endif
