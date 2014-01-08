// Copyright (C) 2014 Lutz Foucar

/**
 * @file data.cpp contains the base class for add viewer data
 *
 * @author Lutz Foucar
 */

#include "data.h"

using namespace jocassview;

Data::~Data()
{

}

void Data::setSourceType(const QString &type)
{
  _sourceType = type;
}

QString Data::sourceType()const
{
  return _sourceType;
}
