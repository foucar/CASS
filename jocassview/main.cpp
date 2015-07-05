// Copyright (C) 2010 Uwe Hoppe
// Copyright (C) 2010 Jochen Küpper
// Copyright (C) 2010 Nicola Coppola
// Copyright (C) 2013,2014 Lutz Foucar

/**
 * @file jocassview/main.cpp the main starter for jocassview
 *
 * @author Lutz Foucar
 */

#include <iostream>

#include <QtCore/QSettings>
#include <QtCore/QFileInfo>

#if QT_VERSION >= 0x050000
#include <QtWidgets/QApplication>
#else
#include <QtGui/QApplication>
#endif

#include "jocassviewer.h"
#include "cl_parser.hpp"
#include "jocassview_version.h"

using namespace jocassview;

int main(int argc, char *argv[])
{
  QApplication app(argc, argv);
  QCoreApplication::setOrganizationName("CFEL-ASG");
  QCoreApplication::setOrganizationDomain("endstation.asg.cfel.de");
  QCoreApplication::setApplicationName("jocassview");
  QSettings::setDefaultFormat(QSettings::IniFormat);
  QApplication::setWindowIcon(QIcon(":images/mpg.svg"));

  /** open an instance of the jocassviwer */
  JoCASSViewer jocassviewer;

  /** parse command line parameters and if first parameter is given open file */
  cass::CommandlineArgumentParser parser;
  std::string filename("nofile");
  parser.add("-f","filename of file that one wants to open",filename);
  std::string key("empty");
  parser.add("--h5key","key of the datafield in the hdf5 file",key);
  bool showUsage(false);
  parser.add("-h","show this help",showUsage);
  bool showVersion(false);
  parser.add("--version","display the version of jocassview",showVersion);
  bool restore(false);
  parser.add("--restore","restore the previous session",restore);

  parser(QCoreApplication::arguments());

  /** show help and exit if requested */
  if (showUsage)
  {
    parser.usage();
    exit(0);
  }

  /** show version and exit if requested */
  if (showVersion)
  {
    std::cout <<VERSION <<std::endl;
    exit(0);
  }

  if (restore)
  {
    jocassviewer.refreshDisplayableItemsList();
    QSettings settings;
    QStringList viewers(settings.value("OpenedViewers").toStringList());
    for (QStringList::ConstIterator it = viewers.begin(); it != viewers.end(); ++it)
    {
      jocassviewer.setDisplayedItem(*it,true,true);
    }
  }

  if ( filename == "nofile")
    jocassviewer.startViewer();
  else
  {
    QString fname(QString::fromStdString(filename));
    QString keyname(QString::fromStdString(key));
    if (keyname == "empty")
      keyname = QFileInfo(fname).baseName();
    /** @todo remove all non alphanumerical characters from the keystring before
     *        calling function
     */
    jocassviewer.openFile(fname,keyname);
  }

  return app.exec();
}
