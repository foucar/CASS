// Copyright (C) 2013 Lutz Foucar

/**
 * @file jocassview/id_list.h file contains the classes that can serialize the
 *                    key list
 *
 * @author Lutz Foucar
 */

#ifndef __JOCASSVIEWIDLIST_H__
#define __JOCASSVIEWIDLIST_H__

#include <QtCore/QStringList>

#include "serializable.hpp"

namespace jocassview
{
/** id-list
 *
 * used for SOAP communication of id-lists (copy of the IdList of cass, but
 * without the qt components).
 *
 * @author Stephan Kassemeyer
 * @author Lutz Foucar
 */
class IdList : public cass::Serializable
{
public:

  /** default constructor */
  IdList();

  /** construct from serializer
   *
   * @param in the serializer
   */
  IdList(cass::SerializerBackend& in);

  /** clear the list */
  void clear();

  /** getter for the internal list
   *
   * @return the list that is managed by this
   */
  const QStringList &getList() const;

  /** implmented but doesn't do anything
   *
   * @param out unused parameter
   */
  void serialize(cass::SerializerBackend &out) const;

  /** deserialize the list from the serializer
   *
   * @return true if one coud deserialize this object fine
   * @param in the deserializer
   */
  bool deserialize(cass::SerializerBackend &in);

private:
  /** a list of all processor keys */
  QStringList _list;

  /** the size of the processor keys list */
  size_t _size;
};

} //end namespace
#endif
