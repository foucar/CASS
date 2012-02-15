//Copyright (C) 2012 Lutz Foucar

/**
 * @file pp_factory.h contains factory to create postprocessors.
 *
 * @author Lutz Foucar
 */

#ifndef __POSTPROCESSOR_FACTORY_H__
#define __POSTPROCESSOR_FACTORY_H__

#include <map>
#include <string>
#include <tr1/functional>
#include <tr1/memory>


namespace cass
{

class PostProcessor;

/** A factory to create PostProcessors
 *
 * @author Lutz Foucar
 */
class PostProcessorFactory
{
public:
  /** create a PostProcessor instance of the requested type
   *
   * @return the requested PostProcessor instance
   * @param type the type of PostProcessor that should be returned
   */
  static std::tr1::shared_ptr<PostProcessor> createInstance(const std::string & type);

protected:
  /** the instanciator that creates the PostProcessor instance */
  typedef std::tr1::function<std::tr1::shared_ptr<PostProcessor>()> instanciator_t;

  /** map to map the strint to the right instanciator */
  typedef std::map<std::string, instanciator_t> string2instanciator_t;

  /** pointer to the map */
  typedef std::tr1::shared_ptr<string2instanciator_t> string2instanciatorpointer_t;

  /** retrive the map
   *
   * if the map has not been created create it and then return the shared pointer
   * to it.
   *
   * @return the shared pointer to the map
   */
  static string2instanciatorpointer_t getMap();

private:
  /** map that will map the string to instanciators
   *
   * @note never delete'ed. (exist until program termination)
   *       because we can't guarantee correct destruction order
   */
  static string2instanciatorpointer_t _string2instanciator;
};

/** function to create the requested PostProcessor
 *
 * @tparam T the Type of PostProcessor to be created
 * @return shared pointer to the requested PostProcessor type
 */
template<typename T>
std::tr1::shared_ptr<PostProcessor> createPP()
{
  return std::tr1::shared_ptr<PostProcessor>(new T);
}

/** struct that will register the PostProcessor in the map
 *
 * @note put a static instance of this struct into the class that should be
 *       registered. When the static variable is defined this constructor will
 *       automatically register the derived class to the factory.
 *
 * @tparam the type of the derived PostProcessor
 */
template<typename T>
struct PostProcessorRegister : PostProcessorFactory
{
  /** constuctor
   *
   * register the PostProcessors instanciator  together with the type name to
   * the map.
   *
   * @param type the type as a human readable string
   */
  PostProcessorRegister(const std::string & type)
  {
    getMap()->insert(std::make_pair(type, &createPP<T>));
  }
};

}//end namespace cass

#endif // POSTPROCESSOR_FACTORY_H
