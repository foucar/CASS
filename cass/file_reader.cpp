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
#include "txt_reader.h"
#include "../cass_ccd/raw_sss_reader.h"
#include "../cass_pnccd/frms6_reader.h"
#include "../cass_pixeldetector/raw_sss_reader.h"
#include "../cass_pixeldetector/frms6_reader.h"


using namespace cass;
using namespace std;
using namespace std::tr1;

FileReader::shared_pointer FileReader::instance(const string &type)
{
  /** @todo use "Or" here */
  shared_pointer ptr;
  if (type == "xtc")
    ptr = shared_pointer(new XtcReader());
  else if (type == "xtc_new")
    ptr = shared_pointer(new XtcReader());
  else if (type == "lma")
    ptr = shared_pointer(new ACQIRIS::LmaReader());
  else if (type == "lma_new")
    ptr = shared_pointer(new ACQIRIS::LmaReader());
  else if (type == "sss")
    ptr = shared_pointer(new CCD::RAWSSSReader());
  else if (type == "sss_new")
    ptr = shared_pointer(new pixeldetector::RAWSSSReader());
  else if (type == "frms6")
    ptr = shared_pointer(new pnCCD::Frms6Reader());
  else if (type == "frms6_new")
    ptr = shared_pointer(new pixeldetector::Frms6Reader());
  else if (type == "txt")
    ptr = shared_pointer(new TxtReader());
  else
  {
    stringstream ss;
    ss << "FileReader::instance: file reader type '"<< type<<"' is unknown."<< (type == "xtc");
    throw invalid_argument(ss.str());
  }
  return ptr;
}

