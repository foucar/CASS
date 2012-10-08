// Copyright (C) 2012 Lutz Foucar

/**
 * @file data_generator.h file contains base class for all data generators.
 *
 * @author Lutz Foucar
 */

#ifndef _DATAGENERATOR_H_
#define _DATAGENERATOR_H_

#include <tr1/memory>
#include <tr1/functional>
#include <map>
#include <string>

#include <QtCore/QMutex>

namespace cass
{
class CASSEvent;

/** base class for all data generators
 *
 * data generators will fill a cassevent with data
 *
 * @author Lutz Foucar
 */
class DataGenerator
{
public:

  /** typedef the shared pointer of this */
  typedef std::tr1::shared_ptr<DataGenerator> shared_pointer;

  /** instanciator that creates the PostProcessor instance */
  typedef std::tr1::function<shared_pointer()> instanciator_t;

  /** map to map the strint to the right instanciator */
  typedef std::map<std::string, instanciator_t> instanciatorMap_t;

  /** pointer to the map */
  typedef instanciatorMap_t* instanciatorMap_p;

  /** create and return an instance of the requested type
   *
   * @return a shared pointer of the requested class
   * @param type the requested type
   */
  static shared_pointer instance(const std::string &type);

  /** virtual destructor */
  virtual ~DataGenerator() {}

  /** fill data into the cassevent
   *
   * @param evt the cassevent that should be filled.
   */
  virtual void fill(CASSEvent& /*evt*/) {}

  /** load the settings of this from the ini file */
  virtual void load() {}

protected:
  /** retrieve the map of instanciators
   *
   * Will be created the first time this is called. Consecutive calls will just
   * return the created pointer. The map pointer is never deleted. Only when the
   * program quits the shared_ptr will take care of relesing the resources.
   *
   * @return pointer to the instanciator map
   */
  static instanciatorMap_p getInstanciatorMap();

private:
  /** map the types to their instanciators */
  static instanciatorMap_p _instanciatorMap;

  /** a lock for locking access to the _instanciatorMap */
  static QMutex _instanciatorMapLock;
};


/** function to create the requested Data generator
 *
 * @tparam T the Type of data generator to be created
 * @return shared pointer to the requested data generator type
 */
template<typename T>
DataGenerator::shared_pointer DataGeneratorInstance()
{
  return DataGenerator::shared_pointer(new T());
}

/** struct that will register the Data generator in the map
 *
 * @note put a static instance of this struct into the data generator
 *       implementation that should be registered. When the static variable is
 *       defined this constructor will automatically register the derived class
 *       to the instanciator map of the data generator base class.
 *
 * @tparam the type of the derived data generator
 */
template<typename T>
struct DataGeneratorRegister : DataGenerator
{
  /** constuctor
   *
   * register the PostProcessors instanciator  together with the type name to
   * the map.
   *
   * @param type the type as a human readable string
   */
  DataGeneratorRegister(const std::string &type)
  {
    getInstanciatorMap()->insert(std::make_pair(type, &DataGeneratorInstance<T>));
  }
};

} //end namespace cass
#endif
