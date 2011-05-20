// Copyright (C) 2011 Lutz Foucar

/**
 * @file rootfile_helper.h contains singleton definition for creating root files
 *
 * @author Lutz Foucar
 */

#ifndef _ROOTFILE_HELPER_H_
#define _ROOTFILE_HELPER_H_

#include <string>
#include <map>

#include <QtCore/QMutex>

class TFile;

namespace cass
{
  /** root file creation
   *
   * create an instance of a rootfile and return it. In case ROOT file aready
   * exists return the instance of that root file.
   *
   * @author Lutz Foucar
   */
  class ROOTFileHelper
  {
  private:
    /** make default constructor private */
    ROOTFileHelper() {}

  public:
    /** create and return an instance of the rootfile
     *
     * create an instance of the root file with the given parameters. If the
     * rootfile already exists don't create a new one but return the one already
     * existing
     *
     * @return an instance of the TFile
     * @param rootfilename filename of the rootfile
     * @param options string containing the option with which the root file
     *                should be opened.
     *
     */
    static TFile* create(const std::string& rootfilename,
                         const std::string& options = "RECREATE");

  private:
    /** container for all the root files */
    static std::map<std::string,TFile *> _rootfiles;

    /** Singleton Mutex to lock write operations*/
    static QMutex _mutex;
  };
}

#endif
