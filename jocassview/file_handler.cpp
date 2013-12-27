// Copyright (C) 2013 Lutz Foucar

/**
 * @file file_handler.h contains a file handler
 *
 * @author Lutz Foucar
 */

#include <stdexcept>
#include <vector>

#include <QtCore/QString>
#include <QtCore/QFileInfo>
#include <QtCore/QObject>

#include <QtGui/QImage>
#include <QtGui/QMessageBox>
#include <QtGui/QInputDialog>

#include "file_handler.h"

#ifdef HDF5
#include "hdf5_handle.hpp"
#endif
#include "histogram.h"


using namespace jocassview;
using namespace std;

cass::HistogramBackend* FileHandler::getData(const QString &filename, const QString &key)
{
  QFileInfo fileInfo(filename);
  if (! fileInfo.exists())
    return 0;

  FileHandler instance;

  if (fileInfo.suffix().toUpper() == QString("png").toUpper() ||
      fileInfo.suffix().toUpper() == QString("tiff").toUpper() ||
      fileInfo.suffix().toUpper() == QString("jpg").toUpper() ||
      fileInfo.suffix().toUpper() == QString("jpeg").toUpper() ||
      fileInfo.suffix().toUpper() == QString("giv").toUpper() ||
      fileInfo.suffix().toUpper() == QString("bmp").toUpper() )
  {
    QMessageBox::critical(0,QObject::tr("Error"),QObject::tr("Can't convert image to Data."));
  }
  if (fileInfo.suffix().toUpper() == QString("hst").toUpper())
  {
    return instance.loadDataFromHist(filename);
  }
  if (fileInfo.suffix().toUpper() == QString("csv").toUpper())
  {
    return instance.loadDataFromCSV(filename);
  }
  if (fileInfo.suffix().toUpper() == QString("h5").toUpper() ||
      fileInfo.suffix().toUpper() == QString("hdf5").toUpper() )
  {
    return instance.loadDataFromH5(filename,key);
  }
  return 0;
}

QImage FileHandler::loadImage(const QString &filename)
{
  QMessageBox::information(0,QObject::tr("Info"),QObject::tr("Image loading not yet implemented"));
  return QImage();
}


cass::HistogramBackend * FileHandler::loadDataFromHist(const QString &filename)
{
  QMessageBox::information(0,QObject::tr("Info"),QObject::tr("Histogram loading not yet implemented"));
  return 0;
}

cass::HistogramBackend * FileHandler::loadDataFromCSV(const QString &filename)
{
  QMessageBox::information(0,QObject::tr("Info"),QObject::tr("CSV loading not yet implemented"));
  return 0;
}

cass::HistogramBackend * FileHandler::loadDataFromH5(const QString &filename, const QString &keyname)
{
#ifdef HDF5
  try
  {
    hdf5::Handler h5handle(filename.toStdString(),"r");

    /** fill the combobox with all displayable items in the file */
    list<string> dataset(h5handle.datasets());
    QStringList items;
    for (list<string>::const_iterator it=dataset.begin(); it != dataset.end(); ++it)
      items.append(QString::fromStdString(*it));

    /** if no key is given request one from the user */
    QString key(keyname);
    if (key == "" || key.isEmpty())
    {
      bool ok(false);
      QString item(QInputDialog::getItem(0, QObject::tr("Select Key"),
                                         QObject::tr("Key:"), items, 0, false, &ok));
      if (ok && !item.isEmpty())
        key = item;
      else
        return 0;
    }

    switch (h5handle.dimension(key.toStdString()))
    {
    case (0):
    {
      float value(h5handle.readScalar<float>(key.toStdString()));
      cass::Histogram0DFloat * hist(new cass::Histogram0DFloat());
      hist->fill(value);
      hist->key() = key.toStdString();
      return hist;
      break;
    }
    case (1):
    {
      vector<float> array;
      size_t length(2);
      float xlow,xup;
      h5handle.readArray(array,length,key.toStdString());
      if (length > 1)
      {
        try { xlow = h5handle.readScalarAttribute<float>("xLow",key.toStdString()); }
        catch(const invalid_argument & what) { xlow = 0; }
        try { xup = h5handle.readScalarAttribute<float>("xUp",key.toStdString()); }
        catch(const invalid_argument & what) { xup = length; }
        cass::Histogram1DFloat * hist(new cass::Histogram1DFloat(length,xlow,xup));
        copy(array.begin(),array.end(),hist->memory().begin());
        hist->key() = key.toStdString();
        return hist;
      }
      else
      {
        cass::Histogram0DFloat * hist(new cass::Histogram0DFloat());
        hist->fill(array[0]);
        hist->key() = key.toStdString();
        return hist;
      }
      break;
    }
    case (2):
    {
      vector<float> matrix;
      pair<size_t,size_t> shape;
      h5handle.readMatrix(matrix,shape,key.toStdString());
      float xlow,xup,ylow,yup;
      try { xlow = h5handle.readScalarAttribute<float>("xLow",key.toStdString()); }
      catch(const invalid_argument & what) { xlow = 0; }
      try { xup = h5handle.readScalarAttribute<float>("xUp",key.toStdString()); }
      catch(const invalid_argument & what) { xup = shape.first; }
      try { ylow = h5handle.readScalarAttribute<float>("yLow",key.toStdString()); }
      catch(const invalid_argument & what) { ylow = 0; }
      try { yup = h5handle.readScalarAttribute<float>("yUp",key.toStdString()); }
      catch(const invalid_argument & what) { yup = shape.second; }
      cass::Histogram2DFloat * hist(new cass::Histogram2DFloat(shape.first,xlow,xup,
                                                               shape.second,ylow,yup));
      copy(matrix.begin(),matrix.end(),hist->memory().begin());
      hist->key() = key.toStdString();
      return hist;
      break;
    }
    default:
      QMessageBox::critical(0,QObject::tr("Error"),QString("FileHandler::loadDataFromH5(): Unknown dimension of dataset '" +
                                                    key + "' in file '" + filename + "'"));
      return 0;
    }
  }
  catch(const invalid_argument & err)
  {
    QMessageBox::critical(0,QObject::tr("Error"),QString::fromStdString(err.what()));
  }
  catch(const logic_error& err)
  {
    QMessageBox::critical(0,QObject::tr("Error"),QString::fromStdString(err.what()));
  }
  catch(const runtime_error & err)
  {
    QMessageBox::critical(0,QObject::tr("Error"),QString::fromStdString(err.what()));
  }
  catch(...)
  {
    QMessageBox::critical(0,QObject::tr("Error"),QString("FileHandler::loadDataFromH5(): can't open '" +
                                                filename + "'. Unknown error occured"));
  }
  return 0;
#endif
}
