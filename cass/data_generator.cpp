// Copyright (C) 2012 Lutz Foucar

/**
 * @file data_generator.cpp file contains base class for all data generators.
 *
 * @author Lutz Foucar
 */

#include <stdexcept>

#include "data_generator.h"


using namespace cass;
using namespace std;

DataGenerator::instanciatorMap_p DataGenerator::_instanciatorMap;
QMutex DataGenerator::_instanciatorMapLock;

DataGenerator::instanciatorMap_p DataGenerator::getInstanciatorMap()
{
  QMutexLocker locker(&_instanciatorMapLock);
  if(!_instanciatorMap)
    _instanciatorMap = instanciatorMap_p(new instanciatorMap_t);
  return _instanciatorMap;
}

DataGenerator::shared_pointer DataGenerator::instance(const string &type)
{
  instanciatorMap_t::iterator it(getInstanciatorMap()->find(type));
  if(it == getInstanciatorMap()->end())
    throw invalid_argument("DataGenerator::instance(): Data generator type '" + type +
                           "' hasn't been registered to the factory.");
  return it->second();
}
