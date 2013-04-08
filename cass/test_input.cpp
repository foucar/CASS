// Copyright (C) 2012 Lutz Foucar

/**
 * @file test_input.h file contains declaration of a input for testing purposes
 *
 * @author Lutz Foucar
 */

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <sstream>
#include <stdexcept>

#include <QtCore/QStringList>


#include "test_input.h"

#include "cass_event.h"
#include "cass_settings.h"
#include "data_generator.h"
#include "generic_factory.hpp"
#include "log.h"
#include "cass_exceptions.h"

using namespace std;
using namespace cass;

void TestInput::instance(RingBuffer<CASSEvent,RingBufferSize> &ringbuffer,
                         Ratemeter &ratemeter,
                         QObject *parent)
{
  if(_instance)
    throw logic_error("TestInput::instance(): The instance of the base class is already initialized");
  _instance = shared_pointer(new TestInput(ringbuffer,ratemeter,parent));
}

TestInput::TestInput(RingBuffer<CASSEvent,RingBufferSize> &ringbuffer,
                     Ratemeter &ratemeter,
                     QObject *parent)
  :InputBase(ringbuffer,ratemeter,parent)
{
  Log::add(Log::VERBOSEINFO, "TestInput::TestInput(): constructed");
  load();
}

void TestInput::load()
{
  CASSSettings s;
  s.beginGroup("TestInput");
  QStringList usedFillers(s.value("Generators").toStringList());

  _generators.clear();

  for (QStringList::const_iterator it(usedFillers.begin()); it != usedFillers.end(); ++it)
  {
    Factory<DataGenerator> &generatorFactory(Factory<DataGenerator>::instance());
    Factory<DataGenerator>::instanciatorMap_t::key_type type(it->toStdString());
    DataGenerator::shared_pointer generator(generatorFactory.create(type));
    generator->load();
    _generators.push_back(generator);
  }
}

void TestInput::run()
{
  _status = lmf::PausableThread::running;

  while (true)
  {
    if (_control == _quit)
      break;

    /** retrieve a new element from the ringbuffer */
    CASSEvent *cassevent(0);
    _ringbuffer.nextToFill(cassevent);

    /** fill the cassevent object with help of the fillers */
    try
    {
      for (generators_t::iterator gen(_generators.begin()); gen != _generators.end(); ++gen)
        (*gen)->fill(*cassevent);
      cassevent->id() = ++_counter;
      _ringbuffer.doneFilling(cassevent, true);
    }
    catch(const DataGenerationError &error)
    {
      Log::add(Log::ERROR,"TestInput::run(): Error generating a data packet");
      _ringbuffer.doneFilling(cassevent, false);
    }
    newEventAdded();
  }
  Log::add(Log::INFO,"TestInput::run():Quitting.");
}
