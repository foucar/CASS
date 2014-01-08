// Copyright (C) 2014 Lutz Foucar

/**
 * @file data_source.h contains the base class data sources
 *
 * @author Lutz Foucar
 */

#ifndef _DATASOURCE_
#define _DATASOURCE_

#include <QtGlobal>
#include <QtCore/QString>
#include <QtCore/QStringList>

namespace cass
{
class HistogramBackend;
}//end namespace cass

class QString;

namespace jocassview
{
/** base class for data sources
 *
 * @author Lutz Foucar
 */
class DataSource
{
public:
  /** retrieve a result from the source
   *
   * @param key the key of the result
   * @param id The event id of the result
   */
  virtual cass::HistogramBackend* result(const QString &key, quint64 id=0) = 0;

  /** retrieve the list items that can be displayed
   *
   * @return the list of strings that name the items that can be displayed
   */
  virtual QStringList resultNames() = 0;

  /** return the type of this source
   *
   * @return the type of the source
   */
  virtual QString type()const=0;
};
}//end namespace jocassview

#endif
