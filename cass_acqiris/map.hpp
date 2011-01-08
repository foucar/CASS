//Copyright (C) 2010 Lutz Foucar

/**
 * @file map.hpp file contains a custom made map template
 *
 * @author Lutz Foucar
 */

#ifndef MAP_HPP
#define MAP_HPP

#include <map>

namespace cass
{
  /** std::map with more functions
   *
   * @todo make operator [] also a const version that will throw error when
   *       key is not present
   * @todo add a map <key, functionsbaseclass*> with functions that should be
   *       called when the requested key is not present. The function might
   *       have : value_type operator()(data source,this) as to be callable
   *       function operator
   * @author Lutz Foucar
   */
  template <typename Key, typename T>
  class Map
  {
  public:
    typedef Key key_type;
    typedef T value_type;

  public:
    /** access members of the map container as done in the std::map */
    value_type& operator[](const key_type& key){return _map[key];}

  private:
    /** the container */
    std::map<key_type,value_type> _map;
  };
}

#endif // MAP_HPP
