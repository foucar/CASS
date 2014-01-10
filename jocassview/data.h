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

protected:
  /** the name of the source */
  QString _sourceName;
};
}//end namespace jocassview

#endif
