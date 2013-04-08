// Copyright (C) 2012 Lutz Foucar

/**
 * @file data_generator.h file contains base class for all data generators.
 *
 * @author Lutz Foucar
 */

#ifndef _DATAGENERATOR_H_
#define _DATAGENERATOR_H_

#include <tr1/memory>

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

  /** virtual destructor */
  virtual ~DataGenerator();

  /** fill data into the cassevent
   *
   * @param evt the cassevent that should be filled.
   */
  virtual void fill(CASSEvent& evt);

  /** load the settings of this from the ini file */
  virtual void load();
};
} //end namespace cass
#endif
