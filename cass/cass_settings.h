// Copyright (C) 2009, 2010 Lutz Foucar

#ifndef _CASS_SETTINGS_H_
#define _CASS_SETTINGS_H_

#include <string>
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
      : QSettings(_filename.c_str(),QSettings::defaultFormat())
    {}

    /** virtual destructor */
    virtual ~CASSSettings() {}

    /** function to set the filename */
    static void setFilename(const std::string &in) { _filename = in;}

  protected:
    /** cass.ini filname
     * name with complete path to the cass.ini to load, needed to circumvent
     * QSettings limitations.
     */
    static std::string _filename;
  };
}//end namespace cass

#endif
