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

#if QT_VERSION >= 0x050000
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QInputDialog>
#else
#include <QtGui/QMessageBox>
#include <QtGui/QInputDialog>
#endif
#include <QtGui/QImage>
#include <QtGui/QImageReader>

#include "file_handler.h"

#ifdef HDF5
#include "hdf5_handle.hpp"
#endif
#include "result.hpp"
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

  bool retval(false);
  if (fileInfo.suffix().toUpper() == QString("h5").toUpper() ||
      fileInfo.suffix().toUpper() == QString("hdf5").toUpper() )
    retval = true;

  return retval;
}

void FileHandler::saveData(const QString &filename, result_t::shared_pointer data)
{
  QFileInfo fileInfo(filename);
  if (fileInfo.exists())
    return;

  FileHandler instance;

  if (fileInfo.suffix().toUpper() == QString("hst").toUpper())
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
    instance.saveDataToH5(filename,data,"w");
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

DataSource::result_t::shared_pointer FileHandler::result(const QString &key, quint64)
{
  QFileInfo fileInfo(_filename);
  if (! fileInfo.exists())
    return result_t::shared_pointer();

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
  return result_t::shared_pointer();
}

QStringList FileHandler::resultNames()
{
  QStringList items;
  if (isContainerFile(_filename) && QFileInfo::exists(_filename))
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

DataSource::result_t::shared_pointer FileHandler::loadDataFromHist()
{
  /** use a file serializer to deserialize the data in the file to a hist object
   *  and get the dimension from the object.
   */
  cass::SerializerReadFile serializer( _filename.toStdString().c_str() );
  result_t::shared_pointer result(new result_t());
  serializer >> (*result);
  serializer.close();

  return result;
}

void FileHandler::saveDataToHist(const QString &filename, result_t::shared_pointer data)
{
  cass::SerializerWriteFile serializer( filename.toStdString().c_str() );
  serializer << (*data);
  serializer.close();
}

DataSource::result_t::shared_pointer FileHandler::loadDataFromCSV()
{
  QMessageBox::information(0,QObject::tr("Info"),QObject::tr("CSV loading not yet implemented"));
  return result_t::shared_pointer();
}

void FileHandler::saveDataToCSV(const QString &filename, result_t::shared_pointer data)
{
  QFile file(filename);
  if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    return;
  QTextStream out(&file);
  switch (data->dim())
  {
  case 0:
  {
    for (size_t i(0); i < data->size(); ++i)
      out << (*data)[i] << endl;
    break;
  }
  case 1:
  {
    const result_t::axe_t &xaxis(data->axis(result_t::xAxis));
    out<<"x-axis value, y-axis value"<<endl;
    for (size_t i(0); i < xaxis.nBins; ++i)
      out <<xaxis.pos(i)<<";"<< (*data)[i] << endl;
    break;
  }
  case 2:
  {
    const result_t::axe_t &xaxis(data->axis(result_t::xAxis));
    const result_t::axe_t &yaxis(data->axis(result_t::yAxis));
    out<<"x-axis value, y-axis value, z-axis value"<<endl;
    for (size_t yy(0); yy < yaxis.nBins; ++yy)
      for (size_t xx(0); xx < xaxis.nBins; ++xx)
        out << xaxis.pos(xx) << ";" << yaxis.pos(yy) <<";"<<(*data)[yy*xaxis.nBins + xx] << endl;
    break;
  }
  }
}

#ifdef HDF5
DataSource::result_t::shared_pointer FileHandler::loadDataFromH5(const QString &keyname)
{
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
    //qDebug()<<"loadDatafromH5"<<key<<key.isEmpty()<<(key =="")<< !items.contains(key);
    if (key == "" || key.isEmpty() || !items.contains(key))
    {
      bool ok(false);
      QString item(QInputDialog::getItem(0, QObject::tr("Select Key"),
                                         QObject::tr("Key:"), items, 0, false, &ok));
      if (ok && !item.isEmpty())
        key = item;
      else
        return result_t::shared_pointer();
    }

    switch (h5handle.dimension(key.toStdString()))
    {
    case (0):
    {
      float value(h5handle.readScalar<float>(key.toStdString()));
      result_t::shared_pointer result(new result_t());
      result->setValue(value);
      result->name(key.toStdString());
      return result;
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
        result_t::shared_pointer result(new result_t(result_t::axe_t(length,xlow,xup)));
        copy(array.begin(),array.end(),result->begin());
        result->name(key.toStdString());
        return result;
      }
      else
      {
        result_t::shared_pointer result(new result_t());
        result->setValue(array[0]);
        result->name(key.toStdString());
        return result;
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
      result_t::shared_pointer result(new result_t
                                      (result_t::axe_t(shape.first,xlow,xup),
                                       result_t::axe_t(shape.second,ylow,yup)));
      copy(matrix.begin(),matrix.end(),result->begin());
      result->name(key.toStdString());
      return result;
      break;
    }
    case (3):
    {
      QString text(QString::fromStdString(h5handle.readString(key.toStdString())));
      //qDebug()<<text;
      QMessageBox::information(0,key,text);
      break;
    }
    default:
      QMessageBox::critical(0,QObject::tr("Error"),QString("FileHandler::loadDataFromH5(): Unknown dimension of dataset '" +
                                                    key + "' in file '" + _filename + "'"));
      return result_t::shared_pointer();
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
  return result_t::shared_pointer();
}

void FileHandler::saveDataToH5(const QString &filename, result_t::shared_pointer data, const QString &mode)
{
  try
  {
    hdf5::Handler h5handle(filename.toStdString(),mode.toStdString());
    switch (data->dim())
    {
    case 0:
    {
      h5handle.writeScalar(data->front(),data->name());
      break;
    }
    case 1:
    {
      const result_t::axe_t &xaxis(data->axis(result_t::xAxis));
      h5handle.writeArray(data->storage(),data->shape().first, data->name());
      h5handle.writeScalarAttribute(xaxis.low, "xLow", data->name());
      h5handle.writeScalarAttribute(xaxis.up, "xUp", data->name());
      break;
    }
    case 2:
    {
      const result_t::axe_t &xaxis(data->axis(result_t::xAxis));
      const result_t::axe_t &yaxis(data->axis(result_t::yAxis));
      h5handle.writeMatrix(data->storage(),data->shape(),data->name(),9);
      h5handle.writeScalarAttribute(xaxis.low, "xLow", data->name());
      h5handle.writeScalarAttribute(xaxis.up, "xUp", data->name());
      h5handle.writeScalarAttribute(yaxis.low, "yLow", data->name());
      h5handle.writeScalarAttribute(yaxis.up, "yUp", data->name());
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
}
#else
cass::HistogramBackend* FileHandler::loadDataFromH5(const QString &)
{
  return 0;
}

void FileHandler::saveDataToH5(const QString &, result_t::shared_pointer, const QString &)
{

}

#endif

DataSource::result_t::shared_pointer FileHandler::loadDataFromCBF()
{
  vector<float> matrix;
  pair<int,int> shape(0,0);
  string header;
  CBF::read(_filename.toStdString(), header, matrix, shape);

//  qDebug()<<QString::fromStdString(header)<<shape.first<<"x"<<shape.second;

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
  //qDebug() << center.first<<center.second;

  double xmin = -center.first;
  double xmax = shape.first - center.first;
  double ymin = -center.second;
  double ymax = shape.second - center.second;
  result_t::shared_pointer result
      (new result_t
       (result_t::axe_t(shape.first, xmin, xmax, "cols"),
        result_t::axe_t(shape.second, ymin, ymax, "rows")));
  copy(matrix.begin(), matrix.end(), result->begin());
  result->name(_filename.toStdString());

  return result;
}

void FileHandler::saveDataToCBF(const QString &filename, result_t::shared_pointer data)
{
  cass::CBF::write(filename.toStdString(), data->begin(), data->shape());
}
