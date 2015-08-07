// Copyright (C) 2011 Lutz Foucar

/**
 * @file file_reader.cpp contains the base class for all file readers
 *
 * @author Lutz Foucar
 */

#include <sstream>
#include <stdexcept>

#include <QtCore/QFileInfo>

#include "file_reader.h"

#include "lma_reader.h"
#include "txt_reader.h"
#include "raw_sss_reader.h"
#include "frms6_reader.h"

#ifdef LCLSLIBRARY
#include "xtc_reader.h"
#endif

using namespace cass;
using namespace std;
using namespace std::tr1;

FileReader::shared_pointer FileReader::instance(const string &filename)
{
  QFileInfo info(QString::fromStdString(filename));
  string type = info.suffix().toStdString();
  shared_pointer ptr;
  if (type == "lma")
    ptr = shared_pointer(new ACQIRIS::LmaReader());
  else if (type == "txt")
    ptr = shared_pointer(new TxtReader());
  else if (type == "sss")
    ptr = shared_pointer(new pixeldetector::RAWSSSReader());
  else if (type == "frms6")
    ptr = shared_pointer(new pixeldetector::Frms6Reader());
#ifdef LCLSLIBRARY
  else if (type == "xtc")
    ptr = shared_pointer(new XtcReader());
#endif
  else
    throw invalid_argument("FileReader::instance: file reader type '" + type +
                           "' is unknown.");
  /** set the filename */
  ptr->filename(filename);
  return ptr;
}

