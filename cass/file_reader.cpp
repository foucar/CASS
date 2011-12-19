// Copyright (C) 2011 Lutz Foucar

/**
 * @file file_reader.cpp contains the base class for all file readers
 *
 * @author Lutz Foucar
 */

#include <sstream>
#include <stdexcept>

#include "file_reader.h"

#include "xtc_reader.h"
#include "lma_reader.h"
#include "raw_sss_reader.h"
#include "frms6_reader.h"
#include "txt_reader.h"

using namespace cass;
using namespace std;
using namespace std::tr1;

FileReader::shared_pointer FileReader::instance(const string &type)
{
  shared_pointer ptr;
  if (type == "xtc")
    ptr = shared_pointer(new XtcReader());
  else if (type == "lma")
    ptr = shared_pointer(new ACQIRIS::LmaReader());
  else if (type == "sss")
    ptr = shared_pointer(new CCD::RAWSSSReader());
  else if (type == "frms6")
    ptr = shared_pointer(new pnCCD::Frms6Reader());
  else if (type == "txt")
    ptr = shared_pointer(new TxtReader());
  else
  {
    stringstream ss;
    ss << "FileReader::instance: file reader type '"<< type<<"' is unknown.";
    throw invalid_argument(ss.str());
  }
  return ptr;
}

