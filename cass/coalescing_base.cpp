// Copyright (C) 2011 Lutz Foucar

/**
 * @file coalescing_base.cpp file contains base class for all coalescing functors.
 *
 * @author Lutz Foucar
 */

#include <sstream>
#include <stdexcept>

#include "coalescing_base.h"

using namespace cass;
using namespace std;
using namespace std::tr1;

CoalescingBase::shared_pointer CoalescingBase::instance(const string &type)
{
  shared_pointer ptr;
//  if (type == "xtc")
//    ptr = shared_pointer(new XtcReader());
//  else if (type == "lma")
//    ptr = shared_pointer(new LmaReader());
//  else
  {
    stringstream ss;
    ss << "CoalescingBase::instance: Coalescing function type '"<< type
        <<"' is unknown.";
    throw invalid_argument(ss.str());
  }
  return ptr;
}
