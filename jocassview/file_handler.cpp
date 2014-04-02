// Copyright (C) 2013 Lutz Foucar

/**
 * @file file_handler.cpp contains a file handler
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
#include "cbf_handle.hpp"


using namespace jocassview;
using namespace cass;
using namespace std;

FileHandler::FileHandler(const QString &filename)
  : _filename(filename)
{

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
  else if (fileInfo.suffix().toUpper() == QString("h5").toUpper() ||
           fileInfo.suffix().toUpper() == QString("hdf5").toUpper() )
  {
    instance.saveDataToH5(filename,data,"rw");
  }
  else if (fileInfo.suffix().toUpper() == QString("cbf").toUpper())
  {
    instance.saveDataToCBF(filename,data);
  }
  return;
}

void FileHandler::createContainer(const QString &filename)
{
  QFileInfo fileInfo(filename);
  if (fileInfo.exists())
  {
#ifdef HDF5
    hdf5::Handler(filename.toStdString(),"w");
#endif
  }
}

HistogramBackend* FileHandler::result(const QString &key, quint64)
{
  QFileInfo fileInfo(_filename);
  if (! fileInfo.exists())
    return 0;

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
    return loadDataFromHist();
  }
  if (fileInfo.suffix().toUpper() == QString("csv").toUpper())
  {
    return loadDataFromCSV();
  }
  if (fileInfo.suffix().toUpper() == QString("h5").toUpper() ||
      fileInfo.suffix().toUpper() == QString("hdf5").toUpper() )
  {
    return loadDataFromH5(key);
  }
  if (fileInfo.suffix().toUpper() == QString("cbf").toUpper())
  {
    return loadDataFromCBF();
  }
  return 0;
}

QStringList FileHandler::resultNames()
{
  QStringList items;
  if (isContainerFile(_filename))
  {
#ifdef HDF5
    try
    {
      hdf5::Handler h5handle(_filename.toStdString(),"r");

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
    items.append(getBaseName(_filename));
  }
  return items;
}

QString FileHandler::type()const
{
  return QString("File");
}

void FileHandler::setFilename(const QString &filename)
{
  _filename = filename;
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

cass::HistogramBackend* FileHandler::loadDataFromHist()
{
  /** use a file serializer to deserialize the data in the file to a hist object
   *  and get the dimension from the object.
   */
  cass::SerializerReadFile serializer( _filename.toStdString().c_str() );
  cass::HistogramBackend* hist = new cass::HistogramFloatBase(serializer);
  serializer.close();
  size_t dim(hist->dimension());
  delete hist;

  /** one needs to reopen the file using the correct derived class
   *  (serializing base class doesn't work) as it will give problems using
   *  when dynamic_casting the baseclass back to the derived class)
   */
  cass::SerializerReadFile serializer2(_filename.toStdString().c_str() );
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

cass::HistogramBackend* FileHandler::loadDataFromCSV()
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

cass::HistogramBackend* FileHandler::loadDataFromH5(const QString &keyname)
{
#ifdef HDF5
  try
  {
    hdf5::Handler h5handle(_filename.toStdString(),"r");

    /** get the list of all items in the h5 file */
    list<string> dataset(h5handle.datasets());
    QStringList items;
    for (list<string>::const_iterator it=dataset.begin(); it != dataset.end(); ++it)
      items.append(QString::fromStdString(*it));

    /** if no key is given or the given key is not on the list,
     *  request one from the user
     */
    QString key(keyname);
    qDebug()<<"loadDatafromH5"<<key<<key.isEmpty()<<(key =="")<< !items.contains(key);
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
                                                    key + "' in file '" + _filename + "'"));
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
                                                _filename + "'. Unknown error occured"));
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

cass::HistogramBackend* FileHandler::loadDataFromCBF()
{
  vector<float> matrix;
  pair<int,int> shape(0,0);
  string header;
  CBF::read(_filename.toStdString(), header, matrix, shape);

//  qDebug()<<QString::fromStdString(header);

  /** try to retrieve the center from the header */
  pair<double,double> center(0,0);
  QString head(QString::fromStdString(header));
  QTextStream headstream(&head);
  QString line;
  do
  {
    line = headstream.readLine();
    if (line.contains("# Beam_xy"))
    {
      QStringList tokens(line.split(' '));
      QString ccol = tokens[2];
      ccol.remove('(');
      ccol.remove(')');
      ccol.remove(',');
      QString crow = tokens[3];
      crow.remove('(');
      crow.remove(')');
      crow.remove(',');
      center.first = ccol.toDouble();
      center.second = crow.toDouble();
    }
  } while(!line.isNull());
  qDebug() << center.first<<center.second;

  double xmin = -center.first;
  double xmax = shape.first - center.first;
  double ymin = -center.second;
  double ymax = shape.second - center.second;
  cass::Histogram2DFloat *hist =
      new cass::Histogram2DFloat(shape.first, xmin, xmax,
                                 shape.second, ymin, ymax,
                                 "cols", "rows");
  copy(matrix.begin(), matrix.end(), hist->memory().begin());
  hist->key() = _filename.toStdString();

  return hist;
}

void FileHandler::saveDataToCBF(const QString &filename, HistogramBackend *data)
{
  cass::Histogram2DFloat *hist(dynamic_cast<cass::Histogram2DFloat*>(data));
  cass::CBF::write(filename.toStdString(), hist->memory(), hist->shape());
}
