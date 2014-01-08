// Copyright (C) 2014 Lutz Foucar

/**
 * @file data_source.h contains the base class data sources
 *
 * @author Lutz Foucar
 */

#ifndef _JOCASSVIEWDATA_
#define _JOCASSVIEWDATA_

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
  cass::HistogramBackend* result(const QString &key, quint64 id) = 0;
};
}//end namespace jocassview
