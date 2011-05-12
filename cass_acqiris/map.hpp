//Copyright (C) 2010 Lutz Foucar

/**
 * @file map.hpp file contains a custom made map template
 *
 * @author Lutz Foucar
 */

#ifndef MAP_HPP
#define MAP_HPP

#include <map>
#include <stdexcept>
#include <sstream>

namespace cass
{
  /** std::map with optional more functions
   *
   * @note add a map <key, functionsbaseclass*> with functions that should be
   *       called when the requested key is not present. The function might
   *       have : value_type operator()(data source,this) as to be callable
   *       function operator
   *
   * @author Lutz Foucar
   */
  template <typename Key, typename T>
  class Map
  {
  public:
    typedef Key key_type;
    typedef T value_type;
    typedef typename std::map<key_type,value_type>::iterator iterator;
    typedef typename std::map<key_type,value_type>::const_iterator const_iterator;

  public:
    /** access members of the map container as done in the std::map */
    value_type& operator[](const key_type& key){return _map[key];}

    /** access members of the map container as done in the std::map */
    value_type operator[](const key_type& key)const
    {
//      const_iterator valueIt(_map.find(key));
//      if (_map.end() == valueIt)
//      {
//        stringstream ss;
//        ss << "Map::operator[] const: There is no key called '"<<key<<"' in container";
//        throw runtime_error(ss.str());
//      }
//      return valueIt->second;
      return (_map.find(key)->second);
    }

    /** retrieve iterator to beginning of map */
    iterator begin()   {return _map.begin();}

    /** retrieve const iterator to beginning of map */
    const_iterator begin()const {return _map.begin();}

    /** retrieve iterator to end of map */
    iterator end() {return _map.end();}

    /** retrieve const iterator to end of map */
    const_iterator end()const {return _map.end();}


  private:
    /** the container */
    std::map<key_type,value_type> _map;
  };
}

#endif // MAP_HPP
