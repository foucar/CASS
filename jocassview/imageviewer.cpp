// Copyright (C) 2010 Uwe Hoppe
// Copyright (C) 2010 Jochen Kuepper


#include <QtCore/QSettings>
#include <QtCore/QTime>
#include <QtGui/QLineEdit>
#include <QtGui/QCloseEvent>
#include <QtGui/QMessageBox>
#include <QtGui/QFileDialog>
#include <QtGui/QPrintDialog>
#include <QtGui/QPainter>
#include <QImageReader>
#include <fstream>
#include <algorithm>

#include "imageviewer.h"
#include "CASSsoap.nsmap"
#include "cass/postprocessing/id_list.h"

using namespace jocassview;
using namespace std;


void ImageLabel::mouseMoveEvent(QMouseEvent *event)
{
  QCursor cursor(this->cursor());
  QPoint position(cursor.pos().x(), cursor.pos().y());
  int x(mapFromGlobal(position).x());
  int y(mapFromGlobal(position).y());
  VERBOSEOUT(cout << "mouseMoveEvent: x=" << x << " y=" << y << endl);
  emit newCursorPosition(x, y);
  event->accept();
}


void ImageViewer::updateStatusBar(int x, int y)
{
  statusBar()->showMessage(QString("rate = ") + QString().setNum(_updaterate, 'g', 2) + " Hz; position = ("
                           + QString().setNum(x) + ',' + QString().setNum(y) + ")");
}


ImageViewer::ImageViewer(QWidget *parent, Qt::WFlags flags)
  : QMainWindow(parent, flags), _updater(new QTimer(this)), _ready(true),
  _updaterate(0)
{
  QSettings settings;
  _lastImage = NULL;
  _lastHist = NULL;
  _ui.setupUi(this);
  qRegisterMetaType<QImage>("QImage");
  connect(&_gdthread, SIGNAL(newNone()), this, SLOT(updateNone()));
  connect(&_gdthread, SIGNAL(newImage(const QImage*)), this, SLOT(updatePixmap(const QImage*)));
  connect(&_gdthread, SIGNAL(newHistogram(cass::Histogram2DFloat*)), this, SLOT(updateHistogram(cass::Histogram2DFloat*)));
  connect(&_gdthread, SIGNAL(newHistogram(cass::Histogram1DFloat*)), this, SLOT(updateHistogram(cass::Histogram1DFloat*)));
  connect(&_gdthread, SIGNAL(newHistogram(cass::Histogram0DFloat*)), this, SLOT(updateHistogram(cass::Histogram0DFloat*)));
  connect(_ui.aboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
  connect(_updater, SIGNAL(timeout()), this, SLOT(on_getData_triggered()));
  // Add servername and port to toolbar.
  _servername = new QLineEdit(settings.value("servername", "server?").toString());
  _servername->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
  _servername->setToolTip("Name of the server to connect to.");
  _cass = new CASSsoapProxy;
  connect(_servername, SIGNAL(editingFinished()), this, SLOT(updateServer()));
  _ui.toolBar->addWidget(_servername);
  _serverport = new QSpinBox();
  _serverport->setKeyboardTracking(false);
  _serverport->setRange(1000, 50000);
  _serverport->setValue(settings.value("serverport", 12321).toInt());
  _serverport->setToolTip("Port of the server to connect to.");
  connect(_serverport, SIGNAL(valueChanged(int)), this, SLOT(updateServer()));
  updateServer();
  _ui.toolBar->addWidget(_serverport);
  // Add spacer to toolbar.
  QWidget *spacer1(new QWidget());
  spacer1->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  _ui.toolBar->addWidget(spacer1);
  // Add Attachment identifier to toolbar.
  _attachId = new QComboBox();
  _attachId->setToolTip("Attachment identifier.");
  _attachId->setDuplicatesEnabled(false);
  _attachId->setInsertPolicy(QComboBox::InsertAlphabetically);
  _attachId->setEditable(false);
  _attachId->installEventFilter(this);
  _attachId->setSizeAdjustPolicy(QComboBox::AdjustToContents);
  updateImageList(_attachId);
  _ui.toolBar->addWidget(_attachId);
  // Add picture type respectively format to toolbar.
  _picturetype = new QComboBox();
  QStringList formats;
  formats << cass::imageformatName(cass::PNG).c_str()
      << cass::imageformatName(cass::TIFF).c_str()
      << cass::imageformatName(cass::JPEG).c_str()
      << cass::imageformatName(cass::GIF).c_str()
      << cass::imageformatName(cass::BMP).c_str()
      << "HIST";
  _picturetype->setToolTip("Supported image file formats.");
  _picturetype->insertItems(0, formats);
  _ui.toolBar->addWidget(_picturetype);
  // Add zoom setting to toolbar.
  _zoom = new QDoubleSpinBox();
  _zoom->setKeyboardTracking(false);
  _zoom->setDecimals(1);
  _zoom->setRange(-50000., 50000.);
  _zoom->setValue(settings.value("zoom", 100.).toDouble());
  _zoom->setToolTip("Set scalefactor for image.");
  connect(_zoom, SIGNAL(valueChanged(double)), this, SLOT(zoomChanged(double)));
  _ui.toolBar->addWidget(_zoom);
  QLabel *zunit(new QLabel);
  zunit->setText("%");
  _ui.toolBar->addWidget(zunit);
  // Add spacer to toolbar.
  QWidget *spacer2(new QWidget());
  spacer2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  _ui.toolBar->addWidget(spacer2);
  // Add running to toolbar.
  _running = new QCheckBox();
  connect(_running, SIGNAL(released()), this, SLOT(running()));
  _running->setToolTip("If checked, continuously retrieve and display images.");
  _ui.toolBar->addWidget(_running);
  // Add status LED to toolbar.
  _statusLED = new StatusLED();
  _statusLED->setToolTip("Status indicator (green= , red= ).");
  _statusLED->setStatus(false);
  _ui.toolBar->addWidget(_statusLED);
  // Add rate to toolbar.
  _rate = new QDoubleSpinBox();
  _rate->setRange(0.01, 100.);
  _rate->setValue(settings.value("rate", 10.).toDouble());
  _rate->setToolTip("Image update frequency.");
  _ui.toolBar->addWidget(_rate);
  QLabel *punit = new QLabel;
  punit->setText("Hz");
  _ui.toolBar->addWidget(punit);

  // todo: encapsulate this in class imageWidget public QWidget...
  // Central label for image display.
  _imageLabel = new ImageLabel;
  _imageLabel->setBackgroundRole(QPalette::Base);
  _imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
  _imageLabel->setScaledContents(true);
  _imageLabel->setMouseTracking(true);
  connect(_imageLabel, SIGNAL(newCursorPosition(int, int)),
          this, SLOT(updateStatusBar(int, int)));
  _imageScroller = new QScrollArea;
  _imageLayout = new QVBoxLayout;
  _imageValuesLayout = new QHBoxLayout;
  _imageWidget = new QWidget;
  _imageMapping = new QComboBox;
  _imageMinValue = new QLineEdit;
  _imageMaxValue = new QLineEdit;
  _imageMinLabel = new QLabel;
  _imageMaxLabel = new QLabel;
  _imageScroller->setBackgroundRole(QPalette::Dark);
  _imageScroller->setWidget(_imageLabel);
  _imageValuesLayout->addWidget(_imageMapping);
  _imageValuesLayout->addWidget(_imageMinLabel);
  _imageValuesLayout->addWidget(_imageMinValue);
  _imageMapping->setEditable(false);
  _imageMapping->addItem(tr("Lin"));
  _imageMapping->addItem(tr("Log"));
  _imageMapping->addItem(tr("Sqrt"));
  _imageMinLabel->setText(tr("Min:"));
  _imageMaxLabel->setText(tr("Max:"));
  _imageValuesLayout->addWidget(_imageMaxLabel);
  _imageValuesLayout->addWidget(_imageMaxValue);
  _imageLayout->addWidget(_imageScroller);
  _imageLayout->addLayout(_imageValuesLayout);
  _imageWidget->setLayout(_imageLayout);
  // widget for plots:
  _plotWidget1D = new plotWidget1D;
  _plotWidget0D = new plotWidget0D(1000);
  connect(_attachId, SIGNAL(editTextChanged(const QString &)), _plotWidget0D, SLOT(setHistogramKey(const QString&)));
  connect(_attachId, SIGNAL(currentIndexChanged(const QString &)), _plotWidget0D, SLOT(setHistogramKey(const QString&)));
  // dock widget containing image or histograms:
  _dock = new QDockWidget(tr("Histogram"), this);
  _spectrogramWidget = new spectrogramWidget;
  //addDockWidget(Qt::RightDockWidgetArea, _dock);
  setCentralWidget(_dock);
  connect(_picturetype, SIGNAL(currentIndexChanged(int)), this, SLOT(pictureTypeChanged(int)));
  _picturetype->setCurrentIndex(settings.value("picturetypeindex", 0).toInt());
  // Other preparations.
  _scaleFactor = settings.value("scaleFactor", 1.0).toDouble();
  _ui.fitToWindow->setChecked(settings.value("fittowindow", false).toBool());
  _imageScroller->setWidgetResizable(_ui.fitToWindow->isChecked());
  statusBar()->setToolTip("Actual frequency to get and display "
                          "images averaged over (n) times.");
  updateActions();
}

ImageViewer::~ImageViewer() {
  delete _lastImage;
  delete _lastHist;
  delete _cass;
  delete _imageLabel;
  delete _plotWidget1D;
  delete _plotWidget0D;
  delete _spectrogramWidget;
  delete _imageScroller;

}


bool ImageViewer::eventFilter(QObject *obj, QEvent *event)
{
  if (obj == _attachId)
  {
    if (event->type() == QEvent::MouseButtonPress)
    {
      updateImageList(_attachId);
      return false;
    }
    else
    {
      return false;
    }
  }
  else
  {
    // pass the event on to the parent class
    return QMainWindow::eventFilter(obj, event);
  }
  return false;
}


cass::PostProcessors::keyList_t ImageViewer::getIdList()
{
  bool ret;
  int retcode=_cass->getPostprocessorIds(&ret);
  if( (retcode==SOAP_OK) && ret)
  {
    VERBOSEOUT(std::cout << "ImageViewer::getIdList(): return value: 'true'" << std::endl);
  }
  else
  {
    VERBOSEOUT(std::cout << "ImageViewer::getIdList(): return value is 'false'" << std::endl);
    return cass::PostProcessors::keyList_t();
  }
  soap_multipart::iterator attachment = _cass->dime.begin();
  VERBOSEOUT(std::cout << "ImageViewer::getIdList(): DIME attachment:" << std::endl
             << "ImageViewer::getIdList(): Memory=" << (void*)(*attachment).ptr << std::endl
             << "ImageViewer::getIdList(): Size=" << (*attachment).size << std::endl
             << "ImageViewer::getIdList(): Type=" << ((*attachment).type?(*attachment).type:"null") << std::endl
             << "ImageViewer::getIdList(): ID=" << ((*attachment).id?(*attachment).id:"null") << std::endl);
  cass::Serializer serializer( std::string((char *)(*attachment).ptr, (*attachment).size) );
  cass::IdList list(serializer);
  return list.getList();
}

void ImageViewer::updateImageList(QComboBox* box)
{
  /** @todo make list always add items at right alphabetical postion */
  cass::PostProcessors::keyList_t stdlist = getIdList();
  for (cass::PostProcessors::keyList_t::iterator it = stdlist.begin(); it!=stdlist.end(); it++)
  {
    VERBOSEOUT(std::cout << "ImageViewer::updateImageList(): list iteration..." << std::endl);
    QString itemstring(QString::fromStdString(*it));
    if (box->findText(itemstring)==-1)
      box->addItem( itemstring, QVariant(0) );
  }
  cass::PostProcessors::keyList_t removelist;
  for (int i=0;i<box->count();++i)
  {
    VERBOSEOUT(std::cout << "ImageViewer::updateImageList(): total counts"<< box->count()
               <<" index:"<<i
               <<" text:"<< box->itemText(i).toStdString()
               << std::endl);
    if (stdlist.end() == std::find(stdlist.begin(),stdlist.end(),box->itemText(i).toStdString()))
    {
      VERBOSEOUT(std::cout << "ImageViewer::updateImageList(): "<< box->itemText(i).toStdString()
                 <<" not on list => remove"
                 << std::endl);
      removelist.push_back(box->itemText(i).toStdString());
    }
#ifdef VERBOSE
    else
    {
      VERBOSEOUT(std::cout << "ImageViewer::updateImageList(): "<< box->itemText(i).toStdString()
                 <<" is on list"
                 << std::endl);
    }
#endif
  }
  for (cass::PostProcessors::keyList_t::iterator it(removelist.begin()); it!=removelist.end(); it++)
    box->removeItem(box->findText(it->c_str()));
}


void ImageViewer::showEvent(QShowEvent *event)
{
  _running->setFocus();
  event->accept();
}


void ImageViewer::resizeEvent(QResizeEvent *event)
{
  VERBOSEOUT(cout << "ImageViewer::resizeEvent: width=" << event->size().width()
             << " height=" << event->size().height() << endl);
  if(_ui.fitToWindow->isChecked())
  {
    /** @todo Resize window to keep the aspect ratio from _imagesize.*/
  }
  event->accept();
}


void ImageViewer::closeEvent(QCloseEvent *event)
{
  QSettings settings;
  settings.setValue("picturetypeindex", _picturetype->currentIndex());
  settings.setValue("servername", _servername->text());
  settings.setValue("serverport", _serverport->value());
  settings.setValue("rate", _rate->value());
  settings.setValue("zoom", _zoom->value());
  settings.setValue("attachId", _attachId->currentText());
  settings.setValue("fittowindow", _ui.fitToWindow->isChecked());
  settings.setValue("scaleFactor", _scaleFactor);
  event->accept();
}



void ImageViewer::on_open_triggered()
{
  // todo: use cass::imageExtension(cass::PNG)...   unfortunately cannot iterate over enums
  QString filter("Images (*.png *.tiff *.jpg *.jpeg *.gif *.bmp);;Csv plot files (*.csv);;Histogram binary files (*.hst)");
  QString fileName = QFileDialog::getOpenFileName(this,
                                                  tr("Open File"), QDir::currentPath(), filter);
  if(!fileName.isEmpty()) loadData(fileName, false);
}

void ImageViewer::on_overlay_data_triggered()
{
  // todo: use cass::imageExtension(cass::PNG)...   unfortunately cannot iterate over enums
  QString filter("Images (*.png *.tiff *.jpg *.jpeg *.gif *.bmp);;Csv plot files (*.csv);;Histogram binary files (*.hst)");
  QString fileName = QFileDialog::getOpenFileName(this,
                                                  tr("Open File"), QDir::currentPath(), filter);
  if(!fileName.isEmpty()) loadData(fileName, true);
}


void ImageViewer::loadData( QString fileName, bool overlay )
{
  QFileInfo fileInfo(fileName);
  if (! fileInfo.exists()) return;
  QImageReader imreader(fileName);
  if ( imreader.canRead() )
  {
    // read image
    QImage* image = new QImage(fileName);
    if(image->isNull())
    {
      QMessageBox::information(this, tr("jocassviewer"),
                               tr("Cannot load %1.").arg(fileName));
      return;
    }
    updatePixmap(image);
    if(!_ui.fitToWindow->isChecked())
      _imageLabel->adjustSize();
  }
  if ( fileInfo.suffix().toUpper() == QString("csv").toUpper() )
  {
    // read csv file into 1d histogram.
  }
  if ( fileInfo.suffix().toUpper() == QString("hst").toUpper() )
  {
    // deserialize binary stream into histogram.
    cass::SerializerReadFile serializer( fileName.toStdString().c_str() );
    cass::HistogramFloatBase* hist = new cass::HistogramFloatBase(serializer);
    serializer.close();
    size_t dim = hist->dimension();

    //hack reopen histogram with correct deserializer (serializing base class doesn't work, see below):
    delete hist;
    cass::SerializerReadFile serializer2( fileName.toStdString().c_str() );
    switch(dim)
    {
    case 0:
      hist = new cass::Histogram0DFloat( serializer2 );
      if (overlay && _dock->widget()==_plotWidget0D)
      {
        _plotWidget0D->addOverlay( reinterpret_cast<cass::Histogram1DFloat*>(hist), fileInfo.baseName() );  // todo: in future, check if 0d histogram has 1d accumulation enabled.
        delete hist;
      }
      else
        updateHistogram( reinterpret_cast<cass::Histogram0DFloat*>(hist) ); break;
            case 1:
      hist = new cass::Histogram1DFloat( serializer2 );
      if (overlay && _dock->widget()==_plotWidget1D)
      {
        _plotWidget1D->addOverlay( reinterpret_cast<cass::Histogram1DFloat*>(hist), fileInfo.baseName() );
        delete hist;
      }
      else
        updateHistogram( reinterpret_cast<cass::Histogram1DFloat*>(hist) ); break;
            case 2:
      hist = new cass::Histogram2DFloat( serializer2 );
      updateHistogram( reinterpret_cast<cass::Histogram2DFloat*>(hist) ); break;
    }
    serializer2.close();

    /* doesn't work: returns hist->min and hist->max from baseclass...
        switch(dim) {
            case 0: updateHistogram( reinterpret_cast<cass::Histogram0DFloat*>(hist) ); break;
            case 1: updateHistogram( reinterpret_cast<cass::Histogram1DFloat*>(hist) ); break;
            case 2: updateHistogram( reinterpret_cast<cass::Histogram2DFloat*>(hist) ); break;
        }*/

  }
}



void ImageViewer::on_save_data_triggered()
{
  if (_dock->widget() == _imageWidget)
  {
    QString filter("Portable Network Graphics (*.png)");
    QString fileName(QFileDialog::getSaveFileName(this, tr("Save Image File"), QDir::currentPath(), filter));
    saveImage(fileName);
  }
  else if (_dock->widget() == _spectrogramWidget)
  {
    QString filter("Portable Networks Graphics (*.png);;Comma Separated Values (*.dat *.csv);;Histogram binary files (*.hst)");
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), QDir::currentPath(), filter);
    QFileInfo fileInfo(fileName);
    if((fileInfo.suffix().toUpper() == QString("CSV")) || (fileInfo.suffix().toUpper() == QString("DAT")))
    {
      save2dAscii(fileName);
    }
    else if(fileInfo.suffix().toUpper() == QString("PNG"))
    {
      saveImage(fileName);
    }
    else if(fileInfo.suffix().toUpper() == QString("HST"))
    {
      saveHistogram(fileName);
    }
  }
  else if(_dock->widget() == _plotWidget1D)
  {
    QString filter("Comma Separated Values (*.dat *.csv);;Histogram binary files (*.hst)");
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save File"), QDir::currentPath(), filter);
    QFileInfo fileInfo(fileName);
    if((fileInfo.suffix().toUpper() == QString("CSV")) || (fileInfo.suffix().toUpper() == QString("DAT")))
    {
      save1dAscii(fileName);
    }
    else if(fileInfo.suffix().toUpper() == QString("HST"))
    {
      saveHistogram(fileName);
    }
  }
}



void ImageViewer::on_auto_save_data_triggered()
{
  QString fillZeros;
  for(int ii=_attachId->currentText().length(); ii<3; ii++)
    fillZeros+=QString("0");
  QString fileName = QDir::currentPath() + "/" + fillZeros + _attachId->currentText() + "_" + QDateTime::currentDateTime().toString();
  if(_dock->widget() == _imageWidget) {
    save2dAscii(fileName + QString(".dat"));
    saveImage(fileName + QString(".png"));
    saveHistogram(fileName + QString(".hst"));
  }
  else if(_dock->widget() == _spectrogramWidget) {
    save2dAscii(fileName + QString(".dat"));
    saveImage(fileName + QString(".png"));
    saveHistogram(fileName + QString(".hst"));
  }
  else if(_dock->widget() == _plotWidget1D) {
    save1dAscii(fileName + QString(".dat"));
    saveHistogram(fileName + QString(".hst"));
  }
}



void ImageViewer::save1dAscii(QString fileName)
{
  ofstream outfile;
  outfile.open(fileName.toStdString().c_str());
  _histogramlock.lockForRead();
  cass::Histogram1DFloat* hist(dynamic_cast<cass::Histogram1DFloat*>(_lastHist));
  const cass::AxisProperty &axis(_lastHist->axis()[0]);
  for(size_t ii=0; ii< hist->size(); ++ii)
  {
    outfile << axis.position(ii) << ";" << hist->bin(ii) << std::endl;
  }
  _histogramlock.unlock();
  outfile.close();
}



void ImageViewer::save2dAscii(QString fileName)
{
  ofstream outfile;
  outfile.open(fileName.toStdString().c_str());
  _histogramlock.lockForRead();
  cass::Histogram2DFloat* hist(dynamic_cast<cass::Histogram2DFloat*>(_lastHist));
  const cass::AxisProperty &xaxis(_lastHist->axis()[0]);
  const cass::AxisProperty &yaxis(_lastHist->axis()[1]);
  outfile << "# 2d-histogram data: " << endl;
  // write x-axis positions
  outfile << "# x-positions: ";
  for(size_t ix=0; ix< xaxis.size(); ++ix)
    outfile<< xaxis.position(ix) << ", ";
  outfile << endl;
  // write y-axis positions
  outfile << "# y-positions: ";
  for(size_t iy=0; iy< yaxis.size(); ++iy)
    outfile<< yaxis.position(iy) << ", ";
  outfile << endl;
  // write data - one row per row ;-)
  outfile << "# data: " << endl;
  for(size_t iy=0; iy< yaxis.size(); ++iy)
  {
    for(size_t ix=0; ix< xaxis.size(); ++ix)
    {
      outfile << hist->bin(ix, iy)<< ", " ;
    }
    outfile << endl;
  }
  _histogramlock.unlock();
  outfile.close();
}



void ImageViewer::saveHistogram(QString filename)
{
  _histogramlock.lockForRead();
  cass::SerializerWriteFile serializer( filename.toStdString().c_str() );
  _lastHist->serialize( serializer );
  _histogramlock.unlock();
  serializer.close();
}



void ImageViewer::saveImage(QString fileName)
{
  VERBOSEOUT(std::cout << fileName.toStdString() << std::endl);
  if(fileName.isEmpty())
  {
    QMessageBox::warning(this, tr("jocassviewer"), tr("Do not have an filename for saving image!"));
    return;
  }
  if(_dock->widget() == _imageWidget)
  {
    QImage image(_imageLabel->pixmap()->toImage());
    if(image.isNull())
    {
      QMessageBox::warning(this, tr("jocassviewer"), tr("Cannot retrieve image from display!"));
      return;
    }
    if(! image.save(fileName, "PNG"))
      QMessageBox::warning(this, tr("jocassviewer"), tr("Image could not be saved!"));
    updateActions();
  }
  else if(_dock->widget() == _spectrogramWidget)
  {
    QImage image(_spectrogramWidget->qimage());
    if(image.isNull())
    {
      QMessageBox::warning(this, tr("jocassviewer"), tr("Cannot retrieve image from display!"));
      return;
    }
    if(! image.save(fileName, "PNG"))
    {
      QMessageBox::warning(this, tr("jocassviewer"), tr("Image could not be saved!"));
      return;
    }
    updateActions();
  }
  else
  {
    QMessageBox::warning(this, tr("jocassviewer"), tr("Do not have an image for saving!"));
  }
}


void ImageViewer::updateNone()
{
  _statusLED->setStatus(false);
  _ready = true;
  VERBOSEOUT(std::cout<< "updateNone" << std::endl);
}


void ImageViewer::updatePixmap(const QImage *image)
{
  VERBOSEOUT(cout << "ImageViewer::updatePixmap(): byteCount=" << image->byteCount()
             << " width=" << image->size().width()
             << " height=" << image->size().height() << endl);
  _imageLabel->setPixmap(QPixmap::fromImage(*image));
  _imagesize = image->size();
  if (image!=_lastImage) delete _lastImage;
  _lastImage = image;
  if (_dock->widget()!=_imageWidget) _dock->setWidget(_imageWidget);
  updateActions();
  VERBOSEOUT(cout << "updatePixmap: _scaleFactor=" << _scaleFactor << endl);
  _imageLabel->resize(_scaleFactor * _imageLabel->pixmap()->size());
  // set rate info
  static QTime time;
  int elapsed(time.restart());
  if(_updaterate < 0.01)
    _updaterate = 1000./elapsed;
  else
    _updaterate = 0.95 * _updaterate + 0.05 * 1000./elapsed;
  _statusLED->setStatus(false);
  _ready = true;
  VERBOSEOUT(std::cout << "min: " << _imageMinValue->text().toFloat() << std::endl
             << "max: " << _imageMaxValue->text().toFloat() << std::endl
             << "value mapping: " << _imageMapping->currentIndex() << " : " <<  _imageMapping->currentText().toStdString() << std::endl);
}


void ImageViewer::updateHistogram(cass::Histogram2DFloat* hist)
{
  _spectrogramWidget->setData(hist);
  if (hist!=_lastHist)
  {
    _histogramlock.lockForWrite();
    delete _lastHist;
    _histogramlock.unlock();
  }
  _lastHist = hist;
  if (_dock->widget() != _spectrogramWidget)
    _dock->setWidget(_spectrogramWidget);
  updateActions();
  VERBOSEOUT(cout << "ImageViewer::updateHistogram(2d): _scaleFactor=" << _scaleFactor << endl);
  // set rate info
  static QTime time;
  static float rate(0.);
  int elapsed(time.restart());
  if(rate < 0.01)
    rate = 1000./elapsed;
  else
    rate = 0.95 * rate + 0.05 * 1000./elapsed;
  statusBar()->showMessage(QString().setNum(rate, 'g', 2) + " Hz");
  _statusLED->setStatus(false);
  _ready = true;
}


void ImageViewer::updateHistogram(cass::Histogram1DFloat* hist)
{
  _plotWidget1D->setData(hist);
  if (hist!=_lastHist)
  {
    _histogramlock.lockForWrite();
    delete _lastHist;
    _histogramlock.unlock();
  }
  _lastHist = hist;
  if (_dock->widget()!=_plotWidget1D) _dock->setWidget(_plotWidget1D);

  updateActions();
  VERBOSEOUT(cout << "ImageViewer::updateHistogram(1d): _scaleFactor=" << _scaleFactor << endl);
  // set rate info
  static QTime time;
  static float rate(0.);
  int elapsed(time.restart());
  if(rate < 0.01)
    rate = 1000./elapsed;
  else
    rate = 0.95 * rate + 0.05 * 1000./elapsed;
  statusBar()->showMessage(QString().setNum(rate, 'g', 2) + " Hz");
  _statusLED->setStatus(false);
  _ready = true;
}

/*
void ImageViewer::updateHistogram(cass::HistogramFloatBase* hist)
{
    size_t dim = hist->dimension();
    switch(dim)
    {
        case 0: updateHistogram( reinterpret_cast<Histogram0DFloat*>(hist) ); break;
        case 1: updateHistogram( reinterpret_cast<Histogram1DFloat*>(hist) ); break;
        case 2: updateHistogram( reinterpret_cast<Histogram2DFloat*>(hist) ); break;
    }
}*/

void ImageViewer::updateHistogram(cass::Histogram0DFloat* hist)
{
  _plotWidget0D->setData(hist);
  if (hist!=_lastHist)
  {
    _histogramlock.lockForWrite();
    delete _lastHist;
    _histogramlock.unlock();
  }
  _lastHist = hist;
  if (_dock->widget()!=_plotWidget0D) _dock->setWidget(_plotWidget0D);

  updateActions();
  VERBOSEOUT(cout << "ImageViewer::updateHistogram(0d): _scaleFactor=" << _scaleFactor << endl);
  // set rate info
  static QTime time;
  static float rate(0.);
  int elapsed(time.restart());
  if(rate < 0.01)
    rate = 1000./elapsed;
  else
    rate = 0.95 * rate + 0.05 * 1000./elapsed;
  statusBar()->showMessage(QString().setNum(rate, 'g', 2) + " Hz");
  _statusLED->setStatus(false);
  _ready = true;
}

getDataThread::~getDataThread()
{
  delete _cass;
  delete _cassCmd;
}

void getDataThread::updateServer(std::string server)
{
  _cass->soap_endpoint = server.c_str();
  _cassCmd->soap_endpoint = server.c_str();
}

void getDataThread::getData(const std::string& attachId, int useSpectrogram)
{
  VERBOSEOUT(cout << "getDataThread::getData()" << endl);
  _dataType = dat_Any;
  delete _attachId;
  _attachId = new std::string(attachId);
  _useSpectrogram = useSpectrogram;
  start();
}


void getDataThread::getImage(cass::ImageFormat format, const std::string& attachId)
{
  VERBOSEOUT(cout << "getDataThread::getImage()" << endl);
  _dataType = dat_Image;
  _format = format;
  delete _attachId;
  _attachId = new std::string(attachId);
  start();
}

void getDataThread::getHistogram1D(const std::string& attachId)
{
  VERBOSEOUT(cout << "getDataThread::getHistogram1D()" << endl);
  _dataType = dat_1DHistogram;
  delete _attachId;
  _attachId = new std::string(attachId);
  start();
}

void getDataThread::getHistogram0D(const std::string& attachId)
{
  VERBOSEOUT(cout << "getDataThread::getHistogram0D()" << endl);
  _dataType = dat_0DHistogram;
  delete _attachId;
  _attachId = new std::string(attachId);
  start();
}

void getDataThread::run()
{

  VERBOSEOUT(cout << "getDataThread::run(): " << _dataType << endl);
  bool ret = FALSE;
  // todo: get rid of _useSpectrogram

  _cass->getHistogram(*_attachId, 0, &ret);
  if(! ret)
  {
    std::cerr << "getDataThread::run(): Did not get soap data" << std::endl;
    emit newNone();
    return;
  }
  VERBOSEOUT(cout << "getDataThread::run: Got soap data" << endl);
  soap_multipart::iterator attachment(_cass->dime.begin());
  if(_cass->dime.end() == attachment)
  {
    cerr << "getDataThread::run() :Did not get attachment!" << endl;
    emit newNone();
    return;
  }
  VERBOSEOUT(std::cout << "getDataThread::run(): DIME attachment:" << std::endl
             << " getDataThread::run(): Memory=" << (void*)(*attachment).ptr << endl
             << " getDataThread::run(): Size=" << (*attachment).size << endl
             << " getDataThread::run(): Type=" << ((*attachment).type?(*attachment).type:"null") << endl
             << " getDataThread::run(): ID=" << ((*attachment).id?(*attachment).id:"null") << endl);

  // find out Type of Data (Histogramtype, dimension):
  std::string mime((*attachment).type);
  VERBOSEOUT(cout << "getDataThread::run(): mimetype: " << mime << endl);
  if (!mime.compare(std::string("application/cass0Dhistogram")))      _dataType=dat_0DHistogram;
  else if (!mime.compare(std::string("application/cass1Dhistogram"))) _dataType=dat_1DHistogram;
  else if (!mime.compare(std::string("application/cass2Dhistogram"))) _dataType=dat_2DHistogram;
  else if (!mime.compare(0,6,std::string("image/"))) _dataType=dat_Image;  // todo: use cass::imageformatName(cass::PNG)...

  switch(_dataType)
  {
  case dat_2DHistogram:
    {
      cass::Serializer serializer( std::string((char *)(*attachment).ptr, (*attachment).size) );
      cass::Histogram2DFloat* hist = new cass::Histogram2DFloat(serializer);
      emit newHistogram(hist);  // slot deletes hist when done.
      break;
    }
  case dat_1DHistogram:
    {
      cass::Serializer serializer( std::string((char *)(*attachment).ptr, (*attachment).size) );
      cass::Histogram1DFloat* hist = new cass::Histogram1DFloat(serializer);
      emit newHistogram(hist);  // slot deletes hist when done.
      break;
    }
  case dat_0DHistogram:
    {
      cass::Serializer serializer( std::string((char *)(*attachment).ptr, (*attachment).size) );
      cass::Histogram0DFloat* hist = new cass::Histogram0DFloat(serializer);
      emit newHistogram(hist);  // slot deletes hist when done.
      break;
    }
  default:
    emit newNone();
    break;
  }
  _cass->destroy();
  /** @todo Fix imageformat*/
}

void ImageViewer::pictureTypeChanged(int /*newIndex*/)
{
  if (_picturetype->currentText()==QString("HIST")) _useSpectrogram = true;
  else _useSpectrogram = false;
  if (_useSpectrogram && _dock->widget() == _imageWidget && _lastHist && !_running->isChecked() )
    updateHistogram( dynamic_cast<cass::Histogram2DFloat*>(_lastHist) );
  else if ( !_useSpectrogram && _dock->widget() == _spectrogramWidget && _lastImage && !_running->isChecked() )
    updatePixmap( _lastImage );
}

void ImageViewer::on_getData_triggered()
{
  VERBOSEOUT(cout << "ImageViewer::on_getData_triggered()" << endl);
  if(_ready)
  {
    _statusLED->setStatus(true, Qt::green);
    _ready = false;
    _gdthread.getData(_attachId->currentText().toStdString(), _useSpectrogram );
  }
  else
  {
    _statusLED->setStatus(true, Qt::red);
  }
}

void ImageViewer::on_getHistogram_triggered()
{
  _gdthread.getHistogram1D(_attachId->currentText().toStdString());
}

void ImageViewer::on_clearHistogram_triggered()
{
  bool ret;
  _cass->clearHistogram( _attachId->currentText().toStdString(), &ret);
  if(!ret)
    QMessageBox::information(this, tr("jocassviewer"),
                             tr("Error: Cannot communicate readini command."));

}

void ImageViewer::on_sendCommand_triggered()
{
  bool ret;
  QString command(QInputDialog::getText(this, tr("send Command to ") + _attachId->currentText(), tr("command to send to ") + _attachId->currentText() + ":"));
  _cass->receiveCommand( _attachId->currentText().toStdString(), command.toStdString(), &ret);
  if(!ret)
    QMessageBox::information(this, tr("jocassviewer"),
                             tr("Error: Cannot communicate receiveCommand command."));

}

void ImageViewer::running()
{
  VERBOSEOUT(cout << "ImageViewer::running()" << endl);
  if(_running->isChecked())
  {
    _updater->start(int(1000 / _rate->value()));
  }
  else
  {
    _updater->stop();
    //        _statusLED->setStatus(false);
  }
}


void ImageViewer::on_writeIni_triggered()
{
  VERBOSEOUT(cout << "ImageViewer::on_writeIni_triggered()" << endl);
  bool ret;
  _cass->writeini(0, &ret);
  if(!ret)
    QMessageBox::information(this, tr("jocassviewer"),
                             tr("Error: Cannot communicate writeini command."));
}

void ImageViewer::on_readIni_triggered()
{
  VERBOSEOUT(cout << "ImageViewer::on_readIni_triggered()" << endl);
  bool ret;
  _cass->readini(0, &ret);
  if(!ret)
    QMessageBox::information(this, tr("jocassviewer"),
                             tr("Error: Cannot communicate readini command."));
}


void ImageViewer::on_quitServer_triggered()
{
  VERBOSEOUT(cout << "ImageViewer::on_quitServer_triggered()" << endl);
  bool ret;
  _cass->quit(&ret);
  if(!ret)
    QMessageBox::information(this, tr("jocassviewer"),
                             tr("Error: Cannot communicate quit server command."));
}


void ImageViewer::on_print_triggered()
{
#ifndef QT_NO_PRINTER
  if(! _imageLabel->pixmap())
    return;
  QPrintDialog dialog(&printer, this);
  if(dialog.exec()) {
    QPainter painter(&printer);
    QRect rect = painter.viewport();
    QSize size = _imageLabel->pixmap()->size();
    size.scale(rect.size(), Qt::KeepAspectRatio);
    painter.setViewport(rect.x(), rect.y(), size.width(), size.height());
    painter.setWindow(_imageLabel->pixmap()->rect());
    painter.drawPixmap(0, 0, *_imageLabel->pixmap());
  }
#endif
}


void ImageViewer::zoomChanged(double value)
{
  scaleImage(value / _scaleFactor / 100);
}


void ImageViewer::on_zoomIn_triggered()
{
  _zoom->blockSignals(true);
  _zoom->setValue(1.25 * _scaleFactor * 100.);
  _zoom->blockSignals(false);
  scaleImage(1.25);
}


void ImageViewer::on_zoomOut_triggered()
{
  _zoom->blockSignals(true);
  _zoom->setValue(0.8 * _scaleFactor * 100.);
  _zoom->blockSignals(false);
  scaleImage(0.8);
}


void ImageViewer::on_normalSize_triggered()
{
  _imageLabel->adjustSize();
  _scaleFactor = 1.0;
  _zoom->blockSignals(true);
  _zoom->setValue(100.);
  _zoom->blockSignals(false);
}


void ImageViewer::on_fitToWindow_triggered()
{
  VERBOSEOUT(cout << "ImageViewer::on_fitToWindow_triggered()" << endl);
  bool fitToWindow = _ui.fitToWindow->isChecked();
  _imageScroller->setWidgetResizable(fitToWindow);
  if(!fitToWindow)
  {
    on_normalSize_triggered();
  }
  updateActions();
}


void ImageViewer::on_about_triggered()
{
  QMessageBox::about(this, tr("About jocassview"), tr(
      "<p>The <b>joCASSview</b> is a display client for the CASS software.</p>"));
}


void ImageViewer::updateActions()
{
  VERBOSEOUT(cout << "ImageViewer::updateActions()" << endl);
  _ui.zoomIn->setEnabled(! _ui.fitToWindow->isChecked());
  _ui.zoomOut->setEnabled(! _ui.fitToWindow->isChecked());
  _ui.normalSize->setEnabled(! _ui.fitToWindow->isChecked());
  _zoom->setEnabled(! _ui.fitToWindow->isChecked());
}


void ImageViewer::updateServer()
{
  VERBOSEOUT(cout << "ImageViewer::updateServer(): ");
  _server = (_servername->text() + ":" +_serverport->text()).toStdString();
  _gdthread.updateServer(_server);
  _cass->soap_endpoint = _server.c_str();
  VERBOSEOUT(cout << _cass->soap_endpoint << endl);

}


void ImageViewer::scaleImage(double factor)
{
  VERBOSEOUT(cout << "ImageViewer::scaleImage(): factor=" << factor << endl);
  if(! _imageLabel->pixmap())
    return;
  _scaleFactor *= factor;
  _imageLabel->resize(_scaleFactor * _imageLabel->pixmap()->size());
  adjustScrollBar(_imageScroller->horizontalScrollBar(), factor);
  adjustScrollBar(_imageScroller->verticalScrollBar(), factor);
  _ui.zoomIn->setEnabled(_scaleFactor < 3.0);
  _ui.zoomOut->setEnabled(_scaleFactor > 0.333);
}


void ImageViewer::adjustScrollBar(QScrollBar *scrollBar, double factor)
{
  scrollBar->setValue(int(factor * scrollBar->value() +
                          ((factor - 1) * scrollBar->pageStep()/2)));
}


getDataThread::getDataThread()
  : _format(cass::PNG)
{
  VERBOSEOUT(cout << "getDataThread::getDataThread()" << endl);
  _cass = new CASSsoapProxy;
  _cassCmd = new CASSsoapProxy;
  _dataType = dat_Any;
  _attachId = NULL;

}

void jocassview::ImageViewer::on_actionControl_Darkcal_triggered()
{
  bool ret;
  _cass->controlDarkcal("start",&ret);
  if (!ret)
    QMessageBox::information(this, tr("jocassviewer"),
                             tr("Error: Cannot communicate control darkcal command."));
}


// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:

