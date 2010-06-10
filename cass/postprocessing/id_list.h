// Copyright (C) 2010 Lutz Foucar

#ifndef __IDLIST_H__
#define __IDLIST_H__

#include "postprocessor.h"

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

    /** default constructor */
    IdList()
      : Serializable(1), _size(0)
    {}

    /** constructor creating list from incomming list */
    IdList(const PostProcessors::postprocessorKeys_t& list);

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
    void setList(const PostProcessors::postprocessors_t &list);

    /** getter for the internal list */
    const PostProcessors::postprocessorKeys_t& getList() { return _list; }

    /** deserialize the list from the serializer */
    bool deserialize(SerializerBackend *in);

    /** @overload */
    bool deserialize(SerializerBackend& in) { return deserialize(&in); }

    /** serialize the list to the serializer */
    void serialize(SerializerBackend *out);

    /** @overload */
    void serialize(SerializerBackend &out) { serialize(&out); }

  private:
    /** a list of all postprocessor keys */
    PostProcessors::postprocessorKeys_t _list;

    /** the size of the postprocessor keys list */
    size_t _size;
  };

} //end namespace
#endif
