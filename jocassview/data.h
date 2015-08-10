// Copyright (C) 2014 Lutz Foucar

/**
 * @file data.h contains the base class for add viewer data
 *
 * @author Lutz Foucar
 */

#ifndef _JOCASSVIEWDATA_
#define _JOCASSVIEWDATA_

#include <QtCore/QString>

#include "result.hpp"

namespace jocassview
{
/** base class for all data wrappers
 *
 * @author Lutz Foucar
 */
class Data
{
public:
  /** define the result type */
  typedef cass::Result<float> result_t;

  /** constructor
   *
   * will set the _wasUpdated flag to false
   */
  Data();

  /** virtual destrutor */
  virtual ~Data();

  /** fill the data with the result
   *
   * @param result the result to fill into this data container
   */
  virtual void setResult(result_t::shared_pointer result) = 0;

  /** retrieve the result
   *
   * @return pointer to the result
   */
  virtual result_t::shared_pointer result() = 0;

  /** set the source name
   *
   * @param name the sources type
   */
  virtual void setSourceName(const QString &name);

  /** retrieve the source name
   *
   * @return the source name for this data
   */
  virtual QString sourceName()const;

  /** retrieve was upDated flag
   *
   * @return true when the data was updated since the last call to result(),
   *         false otherwise
   */
  virtual bool wasUpdated() const;

protected:
  /** the name of the source */
  QString _sourceName;

  /** flag to tell whether the data was updated */
  bool _wasUpdated;
};
}//end namespace jocassview

#endif
