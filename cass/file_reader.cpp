// Copyright (C) 2011 Lutz Foucar

/**
 * @file file_reader.cpp contains the base class for all file readers
 *
 * @author Lutz Foucar
 */

#include "file_reader.h"

using namespace cass;
using namespace std;
using namespace std::tr1;

FileReader::shared_pointer FileReader::instance(const string &type)
{
  shared_pointer ptr;

  return ptr;
}

