//Copyright (C) 2013 Lutz Foucar

/**
 * @file cached_list.hpp contains a list for caching results
 *
 * @author Lutz Foucar
 */

#ifndef _CACHED_LIST_H_
#define _CACHED_LIST_H_

#include <vector>
#include <algorithm>
#include <stdexcept>
#include <functional>
#include <tr1/memory>
#include <tr1/functional>

#include <QtCore/QMutex>
#include <QtCore/QMutexLocker>

#include "result.hpp"
#include "cass_event.h"

namespace cass
{
/** a list of results for caching
 *
 * @todo make item_type and id_type template parameters
 *
 * @author Lutz Foucar
 */
class CachedList
{
public:
  /** define the type of which the list is for */
  typedef Result<float> item_type;

  /** define a shared pointer of the item */
  typedef std::tr1::shared_ptr<item_type> item_sp;

  /** define the type of the id used */
  typedef CASSEvent::id_t id_type;

  /** define an entry in the list */
  typedef std::pair<id_type,item_sp> entry_type;

  /** define the container of items with their ids */
  typedef std::vector<entry_type> list_type;

  /** define an iterator for the list */
  typedef list_type::iterator iter_type;

  /** retrieve an item with the right id
   *
   * @param id
   */
  const item_type &item(const id_type &id)
  {
    QMutexLocker lock(&_mutex);
    iter_type it(findId(id));
    if (_list.end() == it)
      throw std::logic_error("CachedList::item(): Item with id '" + toString(id) +
                             "' is not in the list.");
    return *(it->second);
  }

  /** retrieve the latest item
   *
   * @return the latest item
   */
  const item_type& latest() const
  {
    return *(_latest->second);
  }

  /** retrieve the latest item
   *
   * @return the latest item
   */
  item_type& latest()
  {
    return *(_latest->second);
  }

  /** set which one is the latest item
   *
   * change the lock of the item from write lock to read lock
   *
   * @param it pointer to the item to be set as latest
   */
  void latest(const iter_type it)
  {
    QMutexLocker lock(&_mutex);
    _latest = it;
  }

  /** unlock the item with id
   *
   * make the entry available again by setting the id to 0
   *
   * @param id the id of the item to be released
   */
  void release(const id_type &id)
  {
    QMutexLocker lock(&_mutex);
    findId(id)->first = 0;
  }

  /** get an item for processing
   *
   * find the next item in the list that has the eventid not set (e.g.: id is 0)
   * Set the id to the event id and return the iterator pointing to the entry.
   *
   * Clear the item before return it
   *
   * @return iterator to the entry that will be allocated for the id
   * @param id the id that the item will have in the list
   */
  iter_type newItem(const id_type &id)
  {
    QMutexLocker lock(&_mutex);
    while(_current->first || _current == _latest)
    {
      ++_current;
      if (_current == _list.end())
        _current = _list.begin();
    }
    _current->first = id;
    QWriteLocker wlock(&(_current->second->lock));
    _current->second->clear();
    return _current;
  }

  /** create the list of items
   *
   * @param item the template of which copies will be placed in the container
   * @param size the number of items that should be in the container
   */
  void setup(item_sp item, size_t size)
  {
    QMutexLocker lock(&_mutex);
    _list.clear();
    for (size_t i=0; i<size; ++i)
      _list.push_back(std::make_pair(0,item->clone()));
    _latest = _current = _list.begin();
  }

  /** clear the items in the list
   *
   * lock and go through all items, lock them and clear them. After they have
   * been cleard unlock them again.
   */
  void clearItems()
  {
    QMutexLocker lock(&_mutex);
    iter_type it(_list.begin());
    iter_type End(_list.end());
    while (it != End)
    {
      QWriteLocker lock(&(it->second->lock));
      it->second->clear();
    }
  }

private:
  /** get an iterator to the item for id
   *
   * @param id the id for which the entry should be returned
   */
  iter_type findId(const id_type &id)
  {
    using std::find_if;
    using std::equal_to;
    using std::tr1::bind;
    using std::tr1::placeholders::_1;

    return find_if(_list.begin(), _list.end(),
                   bind(equal_to<id_type>(),id,
                        bind<id_type>(&entry_type::first,_1)));
  }

private:
  /** the list */
  list_type _list;

  /** mutex for locking the internal list */
  QMutex _mutex;

  /** iterator to the latest entry */
  iter_type _latest;

  /** iterator the currently used entry */
  iter_type _current;
};

}//end namespace cass
#endif
