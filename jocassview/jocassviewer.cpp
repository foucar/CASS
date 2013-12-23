// Copyright (C) 2013 Lutz Foucar

/**
 * @file jocassview.cpp contains the jocassviewer
 *
 * @author Lutz Foucar
 */

#include <utility>
#include <vector>
#include <iostream>

#include <QtCore/QFileInfo>
#include <QtCore/QDebug>

#include <QtGui/QMessageBox>

#include "jocassviewer.h"

#include "main_window.h"
#include "hdf5_handle.hpp"
#include "histogram.h"

using namespace jocassview;
using namespace std;


JoCASSViewer::JoCASSViewer(QObject *parent)
  : QObject(parent)
{
  _mw = new MainWindow();

  connect(_mw,SIGNAL(load_file_triggered(QString)),this,SLOT(loadData(QString)));
  connect(_mw,SIGNAL(item_to_display_changed(QString)),this,SLOT(setCurrentRetrieveItem(QString)));

  _mw->show();
}

JoCASSViewer::~JoCASSViewer()
{
  delete _mw;
}

void JoCASSViewer::loadData(QString filename, QString key)
{
  QFileInfo fileInfo(filename);
  if (! fileInfo.exists())
    return;

  if (fileInfo.suffix().toUpper() == QString("png").toUpper() ||
      fileInfo.suffix().toUpper() == QString("tiff").toUpper() ||
      fileInfo.suffix().toUpper() == QString("jpg").toUpper() ||
      fileInfo.suffix().toUpper() == QString("jpeg").toUpper() ||
      fileInfo.suffix().toUpper() == QString("giv").toUpper() ||
      fileInfo.suffix().toUpper() == QString("bmp").toUpper() )
  {
    loadDataFromImage(filename);
  }
  if (fileInfo.suffix().toUpper() == QString("hst").toUpper())
  {
    loadDataFromHist(filename);
  }
  if (fileInfo.suffix().toUpper() == QString("csv").toUpper())
  {
    loadDataFromCSV(filename);
  }
  if (fileInfo.suffix().toUpper() == QString("h5").toUpper() ||
      fileInfo.suffix().toUpper() == QString("hdf5").toUpper() )
  {
    loadDataFromH5(filename,key);
  }
}

void JoCASSViewer::loadDataFromImage(const QString &filename)
{
  QMessageBox::information(_mw,tr("Info"),tr("Image loading not yet implemented"));
}


void JoCASSViewer::loadDataFromHist(const QString &filename)
{
  QMessageBox::information(_mw,tr("Info"),tr("Histogram loading not yet implemented"));
}

void JoCASSViewer::loadDataFromCSV(const QString &filename)
{
  QMessageBox::information(_mw,tr("Info"),tr("CSV loading not yet implemented"));
}

void JoCASSViewer::loadDataFromH5(const QString &filename, const QString &key)
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
    _mw->setDisplayableItems(items);

    /** cache the filename */
    _filename = filename;

    /** if a key is given try to load it from the file and set the right item
     *  the displayable items list
     */
    if (key != "")
    {
      _mw->setDisplayedItem(key);
      loadH5KeyFromFile(key);
    }

    /** connect the item changed to displaying the new key */
    disconnect(_mw,SIGNAL(item_to_display_changed(QString)),this,SLOT(setCurrentRetrieveItem(QString)));
    connect(_mw,SIGNAL(item_to_display_changed(QString)),this,SLOT(loadH5KeyFromFile(QString)));
  }
  catch(const invalid_argument & err)
  {
    QMessageBox::critical(_mw,tr("Error"),QString::fromStdString(err.what()));
  }
  catch(const logic_error& err)
  {
    QMessageBox::critical(_mw,tr("Error"),QString::fromStdString(err.what()));
  }
  catch(const runtime_error & err)
  {
    QMessageBox::critical(_mw,tr("Error"),QString::fromStdString(err.what()));
  }
  catch(...)
  {
    QMessageBox::critical(_mw,tr("Error"),QString("JoCASSViewer::loadDataFromH5(): can't open '" +
                                                  filename + "'. Unknown error occured"));
  }
#endif
}

void JoCASSViewer::loadH5KeyFromFile(QString key)
{
#ifdef HDF5
  try
  {
    hdf5::Handler h5handle(_filename.toStdString(),"r");
    switch (h5handle.dimension(key.toStdString()))
    {
    case (0):
    {
      float value(h5handle.readScalar<float>(key.toStdString()));
      _mw->displayItem(value);
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
        _mw->displayItem(hist);
      }
      else
      {
        _mw->displayItem(array[0]);
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
      _mw->displayItem(hist);
      break;
    }
    default:
      QMessageBox::critical(_mw,tr("Error"),QString("JoCASSViewer::loadDataFromH5(): Unknown dimension of dataset '" +
                                                    key + "' in file '" + _filename + "'"));
    }
  }
  catch(const invalid_argument & err)
  {
    QMessageBox::critical(_mw,tr("Error"),QString::fromStdString(err.what()));
  }
  catch(const logic_error& err)
  {
    QMessageBox::critical(_mw,tr("Error"),QString::fromStdString(err.what()));
  }
  catch(const runtime_error & err)
  {
    QMessageBox::critical(_mw,tr("Error"),QString::fromStdString(err.what()));
  }
  catch(...)
  {
    QMessageBox::critical(_mw,tr("Error"),QString("JoCASSViewer::loadDataFromH5(): can't open '" +
                                                  _filename + "'. Unknown error occured"));
  }
#endif
}

void JoCASSViewer::setCurrentRetrieveItem(QString key)
{

}
