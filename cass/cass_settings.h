// Copyright (C) 2009, 2010 Lutz Foucar

#ifndef _CASS_SETTINGS_H_
#define _CASS_SETTINGS_H_

#include <QtCore/QSettings>
#include "cass.h"

namespace cass
{
  /** Settings for CASS
   *
   * This class is needed since one cannot set the filename of the cass.ini
   * globally.
   *
   * @author Lutz Foucar
   */
  class CASSSHARED_EXPORT CASSSettings : public QSettings
  {
  public:
    /** constructor
     *
     * Will take the filename as it is stored in CassIniFilename. Default is
     * (userScope)/organizationName/applicationName.ini
     */
    CASSSettings()
//      : QSettings(CassIniFilename.c_str(),QSettings::defaultFormat())
    {}

    /** virtual destructor */
    virtual ~CASSSettings() {}
  };
}//end namespace cass

#endif
