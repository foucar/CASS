// Copyright (C) 2013 Lutz Foucar

/**
 * @file genertic_factory.hpp contains a factory that can be used for any class
 *
 * @author Lutz Foucar
 */

#ifndef _GENERIC_FACTORY_
#define _GENERIC_FACTORY_

#include <tr1/functional>
#include <tr1/memory>
#include <map>
#include <string>

namespace cass
{

/** function to create an instance of a class
 *
 * creates an instance of Dervied using new and returns a shared_ptr object of
 * the base class
 *
 * @tparam Base the Base class
 * @tparam Derived the derived class
 * @return shared pointer of Base class of Derived class
 */
template <class Base, class Derived>
std::tr1::shared_ptr<Base> instanciator()
{
  return std::tr1::shared_ptr<Base>(new Derived);
}

/** Factory for creating shared_pointers of classes derived from a base class
 *
 * One can use this class to facilitate the factory pattern. Instead of having
 * to add possible instanciatable derived classes at one point, one can use
 * this factory and let the derived class register itself to the factory.
 * To do this one can either use the helper struct cass::Registrar or call
 * the static member function Factory::addType.
 *
 * @note The base class whose derived class should be instanciated through
 *       this factory need to typedef what the shared pointer to them looks
 *       like.
 *
 * @tparam Base The type of the base class.
 *
 * @author Lutz Foucar
 */
template <class Base>
class Factory
{
  /** define a shared pointer of the base class */
  typedef typename Base::shared_pointer shared_pointer;

public:
  /** define a reference to this factory */
  typedef Factory<Base>& reference;

  /** define how the instanctiator function should look like */
  typedef std::tr1::function<shared_pointer()> instanciator_t;

  /** define the map of instanciator functions */
  typedef std::map<std::string,instanciator_t> instanciatorMap_t;

public:
  /** get an instance of the factory
   *
   * static function to make this a singleton. Creates a static instance of this
   * class and returns a reference to it.
   *
   * @note if one wants to use this factory in the multithreadded part, one has
   *       to think about mutexing it.
   *
   * @return reference to this instance
   */
  static reference instance()
  {
    static Factory<Base> instance;
    return instance;
  }

  /** create an instance of the requested derived type
   *
   * looks up whether the requested type has been registered, if not throws an
   * invalid_argument exception. If type is part of the map, then use the
   * instanciator to create the instance. Return the instance in a shared_ptr
   * object of the base class.
   *
   * @return shared_ptr object of the base class.
   * @param type The type of the derived class that should be instanciated
   */
  shared_pointer create(const typename instanciatorMap_t::key_type & type)
  {
    using namespace std;
    typename instanciatorMap_t::const_iterator it(_iMap.find(type));
    if (it == _iMap.end())
      throw invalid_argument("Factory::create(): Type '" + type +"' is not registered");
    return (it->second)();
  }

  /** register a derived type to the map
   *
   * @tparam Derived The type of the derived class
   * @param type the Key that the type should have in the instanciator map
   */
  template <class Derived>
  char addType(const typename instanciatorMap_t::key_type &type)
  {
    _iMap.insert(make_pair(type,&instanciator<Base,Derived>));
    return 0;
  }

private:
  /** map human readable names to instanciators for the object */
  instanciatorMap_t _iMap;
};

/** helper struct that will add Derived to the factory map
 *
 * @note put a static instance of this struct into the derived class
 *       implementation that should be registered. When the static variable is
 *       defined this constructor will automatically register the derived class
 *       to the instanciator map of the factory.
 *
 * @tparam Base the type of the base class
 * @tparam Derived the type of the derived class
 */
template<class Base, class Derived>
struct Registrar
{
  /** define the factory */
  typedef Factory<Base> factory_t;

  /** define a reference to the factory */
  typedef typename factory_t::reference factory_r;

  /** constuctor
   *
   * register the derived class in the factorys instanciator map.
   *
   * @param type the type as a human readable string
   */
  Registrar(const typename factory_t::instanciatorMap_t::key_type &type)
  {
    factory_r factory(factory_t::instance());
    factory.template addType<Derived>(type);
  }
};

}//end namespace cass
#endif
