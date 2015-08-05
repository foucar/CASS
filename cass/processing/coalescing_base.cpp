// Copyright (C) 2011 Lutz Foucar

/**
 * @file coalescing_base.cpp file contains base class for all coalescing functors.
 *
 * @author Lutz Foucar
 */

#include <stdexcept>

#include "coalescing_base.h"

#include "coalesce_simple.h"

using namespace cass;
using namespace pixeldetector;
using namespace std;
using namespace std::tr1;

CoalescingBase::shared_pointer CoalescingBase::instance(const string &type)
{
  shared_pointer ptr;
  if (type == "simple")
    ptr = shared_pointer(new SimpleCoalesce());
//  else if (type == "lma")
//    ptr = shared_pointer(new LmaReader());
  else
    throw invalid_argument("CoalescingBase::instance: Coalescing function type '" +
                           type  +"' is unknown.");
  return ptr;
}
