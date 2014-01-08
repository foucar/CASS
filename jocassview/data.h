// Copyright (C) 2014 Lutz Foucar

/**
 * @file data.h contains the base class for add viewer data
 *
 * @author Lutz Foucar
 */

#ifndef _JOCASSVIEWDATA_
#define _JOCASSVIEWDATA_

#include <QtCore/QString>

namespace cass
{
class HistogramBackend;
}//end namespace cass

namespace jocassview
{
/** base class for all data wrappers
 *
 * @author Lutz Foucar
 */
class Data
{
public:
  /** virtual destrutor */
  virtual ~Data();

  /** fill the data with the result
   *
   * @param result the result to fill into this data container
   */
  virtual void setResult(cass::HistogramBackend* result) = 0;

  /** retrieve the result
   *
   * @return pointer to the result
   */
  virtual cass::HistogramBackend* result() = 0;

  /** set the source type
   *
   * @param type the sources type
   */
  virtual void setSourceType(const QString &type);

  /** retrieve the source type
   *
   * @return the source type for this data
   */
  virtual QString sourceType()const;

protected:
  /** the source type */
  QString _sourceType;
};
}//end namespace jocassview

#endif
