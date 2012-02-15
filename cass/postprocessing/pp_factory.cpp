//Copyright (C) 2012 Lutz Foucar

/**
 * @file pp_factory.cpp contains factory to create postprocessors.
 *
 * @author Lutz Foucar
 */
#include <stdexcept>

#include "pp_factory.h"

using namespace cass;

PostProcessorFactory::string2instanciatorpointer_t PostProcessorFactory::_string2instanciator;

std::tr1::shared_ptr<PostProcessor> PostProcessorFactory::createInstance(const std::string& type)
{
  string2instanciator_t::iterator it (getMap()->find(type));
  if(it == getMap()->end())
    throw invalid_argument("blah");
  return it->second();
}

PostProcessorFactory::string2instanciatorpointer_t PostProcessorFactory::getMap()
{
  if(!_string2instanciator)
    _string2instanciator = string2instanciatorpointer_t(new string2instanciator_t);
  return _string2instanciator;
}
