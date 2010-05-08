// Copyright (C) 2010 Uwe Hoppe
// Copyright (C) 2010 Jochen Kuepper

//#define VERBOSE 1

#include <QtCore/QSettings>
#include <QtCore/QTime>
#include <QtGui/QLineEdit>
#include <QtGui/QCloseEvent>
#include <QtGui/QMessageBox>
#include <QtGui/QFileDialog>
#include <QtGui/QPrintDialog>
#include <QtGui/QPainter>

#include "imageviewer.h"
#include "CASSsoap.nsmap"


namespace jocassview
{
using namespace std;


ImageViewer::ImageViewer(QWidget *parent, Qt::WFlags flags)
    : QMainWindow(parent, flags), _updater(new QTimer(this)), _ready(true)
{
    QSettings settings;
    _ui.setupUi(this);
    qRegisterMetaType<QImage>("QImage");
    connect(&_gdthread, SIGNAL(newNone()), this, SLOT(updateNone()));
    connect(&_gdthread, SIGNAL(newImage(QImage)), this, SLOT(updatePixmap(QImage)));
    connect(&_gdthread, SIGNAL(newHistogram(cass::Histogram1DFloat*)), this, SLOT(updateHistogram1D(cass::Histogram1DFloat*)));
    connect(_ui.aboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    connect(_updater, SIGNAL(timeout()), this, SLOT(on_getData_triggered()));
    // Add servername and port to toolbar.
    _servername = new QLineEdit(settings.value("servername", "server?").toString());
    _servername->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    _servername->setToolTip("Name of the server to connect to.");
    _cass = new CASSsoapProxy;
    _gdthread.setSoap(_cass);
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
    _attachId->setEditable(true);
    //updateImageList(_attachId);    // todo: doesn't work yet. data is correctly serialized on server side, but doesn't arrive correctly...
    _ui.toolBar->addWidget(_attachId);
    // Add picture type respectively format to toolbar.
    _picturetype = new QComboBox();
    QStringList formats;
    formats << "PNG" << "TIFF" << "JPEG" << "GIF" << "BMP";
    _picturetype->setToolTip("Supported image, respectively, file formats.");
    _picturetype->insertItems(0, formats);
    _picturetype->setCurrentIndex(settings.value("picturetypeindex", 0).toInt());
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
    // Central label for image display.
    _imageLabel = new QLabel;
    _imageLabel->setBackgroundRole(QPalette::Base);
    _imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    _imageLabel->setScaledContents(true);
    _imageWidget = new QScrollArea;
    _imageWidget->setBackgroundRole(QPalette::Dark);
    _imageWidget->setWidget(_imageLabel);
    // widget for plots:
    _plotWidget = new plotWidget(_cass);
    // dock widget containing image or histograms:
    _dock = new QDockWidget(tr("Histogram"), this);
    //addDockWidget(Qt::RightDockWidgetArea, _dock);
    setCentralWidget(_dock);


    // Other preparations.
    _scaleFactor = settings.value("scaleFactor", 1.0).toDouble();
    _ui.fitToWindow->setChecked(settings.value("fittowindow", false).toBool());
    _imageWidget->setWidgetResizable(_ui.fitToWindow->isChecked());
    statusBar()->setToolTip("Actual frequency to get and display "
            "images averaged over (n) times.");
    updateActions();
}

void ImageViewer::updateImageList(QComboBox* box) {
    cass::PostProcessors::active_t stdlist = _gdthread.getIdList(_cass);
    for (cass::PostProcessors::active_t::iterator it = stdlist.begin(); it!=stdlist.end(); it++) {
	    std::cout << "list iteration..." << std::endl;
      QString itemstring(QString::number(*it));
      if (box->findText(itemstring)==-1)
         box->addItem( itemstring, QVariant(0) );
    }
}


void ImageViewer::showEvent(QShowEvent *event)
{
    _running->setFocus();
    event->accept();
}


void ImageViewer::resizeEvent(QResizeEvent *event)
{
    VERBOSEOUT(cout << "resizeEvent width=" << event->size().width()
            << " height=" << event->size().height() << endl);
    if(_ui.fitToWindow->isChecked()) {
#warning Resize window to keep the aspect ratio from _imagesize.
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
    settings.setValue("attachId", _attachId->currentText().toInt());
    settings.setValue("fittowindow", _ui.fitToWindow->isChecked());
    settings.setValue("scaleFactor", _scaleFactor);
    event->accept();
}



void ImageViewer::on_open_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this,
            tr("Open File"), QDir::currentPath());
    if(!fileName.isEmpty()) {
        QImage image(fileName);
        if(image.isNull()) {
            QMessageBox::information(this, tr("jocassviewer"),
                    tr("Cannot load %1.").arg(fileName));
            return;
        }
        _imageLabel->setPixmap(QPixmap::fromImage(image));
        _scaleFactor = 1.0;
        _ui.fitToWindow->setEnabled(true);
        updateActions();
        if(!_ui.fitToWindow->isChecked())
            _imageLabel->adjustSize();
    }
}


void ImageViewer::updateNone()
{
    _statusLED->setStatus(false);
    _ready = true;
    std::cout<< "updateNone" << std::endl;
}


void ImageViewer::updatePixmap(const QImage &image)
{
    VERBOSEOUT(cout << "updatePixmap: byteCount=" << image.byteCount()
            << " width=" << image.size().width()
            << " height=" << image.size().height() << endl);
    _imageLabel->setPixmap(QPixmap::fromImage(image));
    _imagesize = image.size();
    _dock->setWidget(_imageWidget);
std::cout<< "updatePixmap" <<std::endl;
    updateActions();
    VERBOSEOUT(cout << "updatePixmap: _scaleFactor=" << _scaleFactor << endl);
    _imageLabel->resize(_scaleFactor * _imageLabel->pixmap()->size());
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

void ImageViewer::updateHistogram1D(cass::Histogram1DFloat* hist)
{
    /*VERBOSEOUT(cout << "updatePixmap: byteCount=" << image.byteCount()
            << " width=" << image.size().width()
            << " height=" << image.size().height() << endl);*/
    _plotWidget->setData(hist);
    delete hist;
    _dock->setWidget(_plotWidget);
    
    updateActions();
    //VERBOSEOUT(cout << "updatePixmap: _scaleFactor=" << _scaleFactor << endl);
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

void getDataThread::getData(CASSsoapProxy *cass, int attachId)
{
    VERBOSEOUT(cout << "getDataThread::getImage" << endl);
    _dataType = dat_Any;
    _cass = cass;
    _attachId = attachId;
    start();
}

void getDataThread::setSoap(CASSsoapProxy* cassSoap) {
    _cass = cassSoap;
}

void getDataThread::getImage(CASSsoapProxy *cass, cass::ImageFormat format, int attachId)
{
    VERBOSEOUT(cout << "getDataThread::getImage" << endl);
    _dataType = dat_Image;
    _cass = cass;
    _format = format;
    _attachId = attachId;
    start();
}

std::string getDataThread::getMimeType(CASSsoapProxy *cass, int attachId)
{
    VERBOSEOUT(cout << "getDataThread::getMimeType" << endl);
    bool ret;
    cass->getMimeType(attachId, &ret);
    if(ret)
        std::cout << "return value: 'true'" << std::endl;
    else {
       std::cout << "return value is 'false'" << std::endl;
        return std::string("");
    }

    soap_multipart::iterator attachment = cass->dime.begin();

    std::cout << "DIME attachment:" << std::endl;
    std::cout << "Memory=" << (void*)(*attachment).ptr << std::endl;
    std::cout << "Size=" << (*attachment).size << std::endl;
    std::cout << "Type=" << ((*attachment).type?(*attachment).type:"null") << std::endl;
    std::cout << "ID=" << ((*attachment).id?(*attachment).id:"null") << std::endl;
    return std::string( (*attachment).ptr, (*attachment).size-1);
}

cass::PostProcessors::active_t getDataThread::getIdList(CASSsoapProxy *cass) {
    bool ret;
    cass->getPostprocessorIds(&ret);
    if(ret)
        std::cout << "return value: 'true'" << std::endl;
    else {
       std::cout << "return value is 'false'" << std::endl;
        return cass::PostProcessors::active_t();
    }

    soap_multipart::iterator attachment = cass->dime.begin();

    std::cout << "DIME attachment:" << std::endl;
    std::cout << "Memory=" << (void*)(*attachment).ptr << std::endl;
    std::cout << "Size=" << (*attachment).size << std::endl;
    std::cout << "Type=" << ((*attachment).type?(*attachment).type:"null") << std::endl;
    std::cout << "ID=" << ((*attachment).id?(*attachment).id:"null") << std::endl;
    cass::Serializer serializer( std::string((char *)(*attachment).ptr, (*attachment).size) );
    cass::IdList list(serializer);
    return list.getList();
};


void getDataThread::getHistogram1D(CASSsoapProxy *cass, int attachId)
{
    VERBOSEOUT(cout << "getDataThread::getImage" << endl);
    _dataType = dat_1DHistogram;
    _cass = cass;
    _attachId = attachId;
    start();
}

void getDataThread::getHistogram0D(CASSsoapProxy *cass, int attachId)
{
    VERBOSEOUT(cout << "getDataThread::getImage" << endl);
    _dataType = dat_0DHistogram;
    _cass = cass;
    _attachId = attachId;
    start();
}

void getDataThread::run()
{
    if (_dataType==dat_Any) {
	    std::string mime(getMimeType(_cass, _attachId));
            VERBOSEOUT(cout << "getDataThread::run mimetype: " << mime << endl);
	    if (!mime.compare(std::string("application/cass0Dhistogram"))) _dataType=dat_0DHistogram;
	    else if (!mime.compare(std::string("application/cass1Dhistogram"))) _dataType=dat_1DHistogram;
	    else if (!mime.compare(std::string("application/cass2Dhistogram"))) _dataType=dat_2DHistogram;
	    else if (!mime.compare(0,17,std::string("application/image"))) _dataType=dat_Image;
    }
    if (_dataType==dat_Any) {
        std::cout<< "getDataThread::run: cannot handle mime type dat_Any" << std::endl;
        emit newNone();
        return;
    }
    VERBOSEOUT(cout << "getDataThread::run " << _dataType << endl);
    bool ret;
    switch(_dataType) {
    case dat_Image:
    _cass->getImage(2, _attachId, &ret);
        break;
    case dat_1DHistogram:
	_cass->getHistogram(_attachId, &ret);
	break;
    }
    if(! ret) {
        cerr << "Did not get soap data" << endl;
        return;
    }
    VERBOSEOUT(cout << "getDataThread::run: Got soap data" << endl);
    soap_multipart::iterator attachment(_cass->dime.begin());
    if(_cass->dime.end() == attachment) {
        cerr << "Did not get attachment!" << endl;
        return;
    }
    VERBOSEOUT(cout << "getDataThread::run: DIME attachment:" << endl);
    VERBOSEOUT(cout << "  Memory=" << (void*)(*attachment).ptr << endl);
    VERBOSEOUT(cout << "  Size=" << (*attachment).size << endl);
    VERBOSEOUT(cout << "  Type=" << ((*attachment).type?(*attachment).type:"null")
            << endl);
    VERBOSEOUT(cout << "  ID=" << ((*attachment).id?(*attachment).id:"null") << endl);
    switch(_dataType) {
    case dat_Image: {
        QImage image(QImage::fromData((uchar*)(*attachment).ptr, (*attachment).size,
                imageformatName(cass::ImageFormat(_format)).c_str()));
        VERBOSEOUT(cout << "getDataThread::run: byteCount=" << image.byteCount() << endl);
        emit newImage(image);
        break; }
    case dat_1DHistogram:
	cass::Serializer serializer( std::string((char *)(*attachment).ptr, (*attachment).size) );
	cass::Histogram1DFloat* hist = new cass::Histogram1DFloat(serializer);
	emit newHistogram(hist);
	break;
    }
    _cass->destroy();
#warning Fix imageformat
}


void ImageViewer::on_getData_triggered()
{
    VERBOSEOUT(cout << "on_getData_triggered" << endl);
    if(_ready) {
        _statusLED->setStatus(true, Qt::green);
        _ready = false;
        _gdthread.setImageFormat(cass::ImageFormat(_picturetype->currentIndex() + 1));
        _gdthread.getData(_cass, _attachId->currentText().toInt());
    } else {
        _statusLED->setStatus(true, Qt::red);
    }
}

void ImageViewer::on_getHistogram_triggered()
{
    _gdthread.getHistogram1D(_cass, _attachId->currentText().toInt());
}

void ImageViewer::running()
{
    VERBOSEOUT(cout << "running" << endl);
    if(_running->isChecked()) {
        _updater->start(int(1000 / _rate->value()));
    } else {
        _updater->stop();
//        _statusLED->setStatus(false);
    }
}


void ImageViewer::on_writeIni_triggered()
{
    VERBOSEOUT(cout << "readIni" << endl);
    bool ret;
    _cass->writeini(0, &ret);
    if(!ret)
        QMessageBox::information(this, tr("jocassviewer"),
                tr("Error: Cannot communicate readini command."));
}

void ImageViewer::on_readIni_triggered()
{
    VERBOSEOUT(cout << "readIni" << endl);
    bool ret;
    _cass->readini(0, &ret);
    if(!ret)
        QMessageBox::information(this, tr("jocassviewer"),
                tr("Error: Cannot communicate readini command."));
}


void ImageViewer::on_quitServer_triggered()
{
    VERBOSEOUT(cout << "quitServer" << endl);
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
    VERBOSEOUT(cout << "on_fitToWindow_triggered" << endl);
    bool fitToWindow = _ui.fitToWindow->isChecked();
    _imageWidget->setWidgetResizable(fitToWindow);
    if(!fitToWindow) {
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
    VERBOSEOUT(cout << "updateActions" << endl);
    _ui.zoomIn->setEnabled(! _ui.fitToWindow->isChecked());
    _ui.zoomOut->setEnabled(! _ui.fitToWindow->isChecked());
    _ui.normalSize->setEnabled(! _ui.fitToWindow->isChecked());
    _zoom->setEnabled(! _ui.fitToWindow->isChecked());
}


void ImageViewer::updateServer()
{
    VERBOSEOUT(cout << "updateServer: ");
    _server = (_servername->text() + ":" +_serverport->text()).toStdString();
    _cass->soap_endpoint = _server.c_str();
    VERBOSEOUT(cout << _cass->soap_endpoint << endl);
}


void ImageViewer::scaleImage(double factor)
{
    VERBOSEOUT(cout << "scaleImage: factor=" << factor << endl);
    if(!_imageLabel->pixmap())
        return;
    _scaleFactor *= factor;
    _imageLabel->resize(_scaleFactor * _imageLabel->pixmap()->size());

    adjustScrollBar(_imageWidget->horizontalScrollBar(), factor);
    adjustScrollBar(_imageWidget->verticalScrollBar(), factor);

    _ui.zoomIn->setEnabled(_scaleFactor < 3.0);
    _ui.zoomOut->setEnabled(_scaleFactor > 0.333);
}


void ImageViewer::adjustScrollBar(QScrollBar *scrollBar, double factor)
{
    scrollBar->setValue(int(factor * scrollBar->value() +
                            ((factor - 1) * scrollBar->pageStep()/2)));
}


getDataThread::getDataThread()
{
    VERBOSEOUT(cout << "getDataThread::getDataThread" << endl);

}

}

// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
