// Copyright (C) 2014 Lutz Foucar

/**
 * @file data.cpp contains the base class for add viewer data
 *
 * @author Lutz Foucar
 */

#include "data.h"

using namespace jocassview;

Data::Data()
  : _wasUpdated(false)
{

}

Data::~Data()
{

}

void Data::setSourceName(const QString &name)
{
  _sourceName = name;
}

QString Data::sourceName()const
{
  return _sourceName;
}

bool Data::wasUpdated()const
{
  return _wasUpdated;
}
