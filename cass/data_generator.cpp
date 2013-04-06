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

DataGenerator::instanciatorMap_t *DataGenerator::_instanciatorMap(0);

DataGenerator::instanciatorMap_t* DataGenerator::getInstanciatorMap()
{
  if(!_instanciatorMap)
    _instanciatorMap = new instanciatorMap_t;
  return _instanciatorMap;
}

DataGenerator::~DataGenerator()
{

}

DataGenerator::shared_pointer
DataGenerator::instance(const DataGenerator::instanciatorMap_t::key_type &type)
{
  const instanciatorMap_t& iMap(*getInstanciatorMap());
  instanciatorMap_t::const_iterator it(iMap.find(type));
  instanciatorMap_t::const_iterator iMapEnd(iMap.end());
  if(it == iMapEnd)
    throw invalid_argument("DataGenerator::instance(): Data generator type '" +
                           type + "' hasn't been registered to the factory.");
  return it->second();
}

void DataGenerator::fill(CASSEvent &)
{

}

void DataGenerator::load()
{

}
