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
#include <QtCore/QDebug>

#include <QtGui/QImage>
#include <QtGui/QImageReader>
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

QStringList FileHandler::getKeyList(const QString &filename)
{
  QStringList items;
  if (isContainerFile(filename))
  {
#ifdef HDF5
    try
    {
      hdf5::Handler h5handle(filename.toStdString(),"r");

      list<string> dataset(h5handle.datasets());
      for (list<string>::const_iterator it=dataset.begin(); it != dataset.end(); ++it)
        items.append(QString::fromStdString(*it));
    }
    catch(...)
    {
      QMessageBox::critical(0,QObject::tr("Error"),QObject::tr("FileHandler::getKeyList(): some error occured"));
    }
#endif
  }
  else
  {
    items.append(getBaseName(filename));
  }
  return items;
}

QString FileHandler::getBaseName(const QString &filename)
{
  QFileInfo fileInfo(filename);
  if (! fileInfo.exists())
    return QString();

  return fileInfo.baseName();
}

bool FileHandler::isContainerFile(const QString &filename)
{
 QFileInfo fileInfo(filename);
  if (! fileInfo.exists())
    return false;

  bool retval(false);
  if (fileInfo.suffix().toUpper() == QString("h5").toUpper() ||
      fileInfo.suffix().toUpper() == QString("hdf5").toUpper() )
    retval = true;

  return retval;
}

void FileHandler::saveData(const QString &filename, cass::HistogramBackend *data)
{
  QFileInfo fileInfo(filename);
  if (fileInfo.exists())
    return;

  FileHandler instance;

  if (fileInfo.suffix().toUpper() == QString("png").toUpper() ||
      fileInfo.suffix().toUpper() == QString("tiff").toUpper() ||
      fileInfo.suffix().toUpper() == QString("jpg").toUpper() ||
      fileInfo.suffix().toUpper() == QString("bmp").toUpper() )
  {
    instance.saveImage(filename,dynamic_cast<cass::Histogram2DFloat*>(data)->qimage());
  }
  else if (fileInfo.suffix().toUpper() == QString("hst").toUpper())
  {
    instance.saveDataToHist(filename,data);
  }
  else if (fileInfo.suffix().toUpper() == QString("csv").toUpper())
  {
    instance.saveDataToCSV(filename,data);
  }
  return;
}

void FileHandler::saveDataToContainer(const QString &filename, cass::HistogramBackend *data)
{
  QFileInfo fileInfo(filename);
  FileHandler instance;
  if (fileInfo.exists())
    instance.saveDataToH5(filename,data,"rw");
  else
    instance.saveDataToH5(filename,data,"w");
}

QImage FileHandler::loadImage(const QString &filename)
{
  QImageReader imageReader(filename);
  if (imageReader.canRead())
    return QImage(filename);
  return QImage();
}

void FileHandler::saveImage(const QString &filename, const QImage &image)
{
  if(! image.save(filename, "PNG"))
    QMessageBox::critical(0, QObject::tr("Error"), QObject::tr("Image'") + filename +
                          QObject::tr("' could not be saved!"));
}

cass::HistogramBackend * FileHandler::loadDataFromHist(const QString &filename)
{
  /** use a file serializer to deserialize the data in the file to a hist object
   *  and get the dimension from the object.
   */
  cass::SerializerReadFile serializer( filename.toStdString().c_str() );
  cass::HistogramBackend* hist = new cass::HistogramFloatBase(serializer);
  serializer.close();
  size_t dim(hist->dimension());
  delete hist;

  /** one needs to reopen the file using the correct derived class
   *  (serializing base class doesn't work) as it will give problems using
   *  when dynamic_casting the baseclass back to the derived class)
   */
  cass::SerializerReadFile serializer2( filename.toStdString().c_str() );
  switch(dim)
  {
  case 0:
    hist = new cass::Histogram0DFloat( serializer2 );
    break;
  case 1:
    hist = new cass::Histogram1DFloat( serializer2 );
    break;
  case 2:
    hist = new cass::Histogram2DFloat( serializer2 );
    break;
  }
  serializer2.close();

  return hist;
}

void FileHandler::saveDataToHist(const QString &filename, cass::HistogramBackend *data)
{
  cass::SerializerWriteFile serializer( filename.toStdString().c_str() );
  data->serialize( serializer );
  serializer.close();
}

cass::HistogramBackend * FileHandler::loadDataFromCSV(const QString &filename)
{
  QMessageBox::information(0,QObject::tr("Info"),QObject::tr("CSV loading not yet implemented"));
  return 0;
}

void FileHandler::saveDataToCSV(const QString &filename, cass::HistogramBackend *data)
{
  QFile file(filename);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    return;
  QTextStream out(&file);
  switch (data->dimension())
  {
  case 0:
  {
    cass::Histogram0DFloat *hist(dynamic_cast<cass::Histogram0DFloat*>(data));
    for (size_t i(0); i < hist->memory().size(); ++i)
      out << hist->memory()[i] << endl;
    break;
  }
  case 1:
  {
    cass::Histogram1DFloat *hist(dynamic_cast<cass::Histogram1DFloat*>(data));
    const cass::AxisProperty &xaxis(hist->axis()[cass::Histogram1DFloat::xAxis]);
    out<<"x-axis value, y-axis value"<<endl;
    for (size_t i(0); i < xaxis.nbrBins(); ++i)
      out <<xaxis.position(i)<<";"<< hist->bin(i) << endl;
    break;
  }
  case 2:
  {
    cass::Histogram2DFloat *hist(dynamic_cast<cass::Histogram2DFloat*>(data));
    const cass::AxisProperty &xaxis(hist->axis()[cass::Histogram2DFloat::xAxis]);
    const cass::AxisProperty &yaxis(hist->axis()[cass::Histogram2DFloat::yAxis]);
    out<<"x-axis value, y-axis value, z-axis value"<<endl;
    for (size_t i(0); i < xaxis.nbrBins(); ++i)
      for (size_t j(0); j < yaxis.nbrBins(); ++j)
        out << xaxis.position(i) << ";" << yaxis.position(j) <<";"<<hist->bin(i,j) << endl;
    break;
  }
  }
}

cass::HistogramBackend* FileHandler::loadDataFromH5(const QString &filename, const QString &keyname)
{
#ifdef HDF5
  try
  {
    hdf5::Handler h5handle(filename.toStdString(),"r");

    /** get the list of all items in the h5 file */
    list<string> dataset(h5handle.datasets());
    QStringList items;
    for (list<string>::const_iterator it=dataset.begin(); it != dataset.end(); ++it)
      items.append(QString::fromStdString(*it));

    /** if no key is given or the given key is not on the list,
     *  request one from the user
     */
    QString key(keyname);
    qDebug()<<key<<key.isEmpty()<<(key =="")<< !items.contains(key);
    if (key == "" || key.isEmpty() || !items.contains(key))
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

void FileHandler::saveDataToH5(const QString &filename, cass::HistogramBackend *data, const QString &mode)
{
#ifdef HDF5
  try
  {
    hdf5::Handler h5handle(filename.toStdString(),mode.toStdString());
    switch (data->dimension())
    {
    case 0:
    {
      cass::Histogram0DFloat *hist(dynamic_cast<cass::Histogram0DFloat*>(data));
      h5handle.writeScalar(hist->memory().front(),data->key());
      break;
    }
    case 1:
    {
      cass::Histogram1DFloat *hist(dynamic_cast<cass::Histogram1DFloat*>(data));
      h5handle.writeArray(hist->memory(),data->axis()[cass::Histogram1DFloat::xAxis].size(),
          data->key());
      h5handle.writeScalarAttribute(data->axis()[cass::Histogram1DFloat::xAxis].lowerLimit(),
          "xLow", data->key());
      h5handle.writeScalarAttribute(data->axis()[cass::Histogram1DFloat::xAxis].upperLimit(),
          "xUp", data->key());
      break;
    }
    case 2:
    {
      cass::Histogram2DFloat *hist(dynamic_cast<cass::Histogram2DFloat*>(data));
      h5handle.writeMatrix(hist->memory(),hist->shape(),data->key(),9);
      h5handle.writeScalarAttribute(data->axis()[cass::Histogram2DFloat::xAxis].lowerLimit(),
          "xLow", data->key());
      h5handle.writeScalarAttribute(data->axis()[cass::Histogram2DFloat::xAxis].upperLimit(),
          "xUp", data->key());
      h5handle.writeScalarAttribute(data->axis()[cass::Histogram2DFloat::yAxis].lowerLimit(),
          "yLow", data->key());
      h5handle.writeScalarAttribute(data->axis()[cass::Histogram2DFloat::yAxis].upperLimit(),
          "yUp", data->key());
      break;
    }
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
#endif
}
