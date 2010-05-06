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
    connect(&_githread, SIGNAL(newImage(QImage)), this, SLOT(updatePixmap(QImage)));
    connect(_ui.aboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
    connect(_updater, SIGNAL(timeout()), this, SLOT(on_getImage_triggered()));
    // Add servername and port to toolbar.
    _servername = new QLineEdit(settings.value("servername", "server?").toString());
    _servername->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    _servername->setToolTip("Name of the server to connect to.");
    connect(_servername, SIGNAL(editingFinished()), this, SLOT(updateServer()));
    _ui.toolBar->addWidget(_servername);
    _serverport = new QSpinBox();
    _serverport->setKeyboardTracking(false);
    _serverport->setRange(1000, 50000);
    _serverport->setValue(settings.value("serverport", 12321).toInt());
    _serverport->setToolTip("Port of the server to connect to.");
    connect(_serverport, SIGNAL(valueChanged(int)), this, SLOT(updateServer()));
    _ui.toolBar->addWidget(_serverport);
    // Add spacer to toolbar.
    QWidget *spacer1(new QWidget());
    spacer1->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    _ui.toolBar->addWidget(spacer1);
    // Add Attachment identifier to toolbar.
    _attachId = new QSpinBox();
    _attachId->setRange(1, 50000);
    _attachId->setToolTip("Attachment identifier.");
    _attachId->setValue(settings.value("attachId", 101).toInt());
    _ui.toolBar->addWidget(_attachId);
    // Add picture type respectively format to toolbar.
    _picturetype = new QComboBox();
    QStringList formats;
#warning Fix imageformat
    formats << "PNG" << "TIFF" << "JPEG";
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
    imageLabel = new QLabel;
    imageLabel->setBackgroundRole(QPalette::Base);
    imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    imageLabel->setScaledContents(true);
    scrollArea = new QScrollArea;
    scrollArea->setBackgroundRole(QPalette::Dark);
    scrollArea->setWidget(imageLabel);
    setCentralWidget(scrollArea);
    // Other preparations.
    _scaleFactor = settings.value("scaleFactor", 1.0).toDouble();
    _ui.fitToWindow->setChecked(settings.value("fittowindow", false).toBool());
    scrollArea->setWidgetResizable(_ui.fitToWindow->isChecked());
    statusBar()->setToolTip("Actual frequency to get and display "
            "images averaged over (n) times.");
    _cass = new CASSsoapProxy;
    updateServer();
    updateActions();
}


void ImageViewer::showEvent(QShowEvent *event)
{
    _running->setFocus();
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
    settings.setValue("attachId", _attachId->value());
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
        imageLabel->setPixmap(QPixmap::fromImage(image));
        _scaleFactor = 1.0;
        _ui.fitToWindow->setEnabled(true);
        updateActions();
        if(!_ui.fitToWindow->isChecked())
            imageLabel->adjustSize();
    }
}


void ImageViewer::updatePixmap(const QImage &image)
{
    VERBOSEOUT(cout << "ImageViewer::updatePixmap: byteCount="
            << image.byteCount() << endl);
    imageLabel->setPixmap(QPixmap::fromImage(image));
    updateActions();
    VERBOSEOUT(cout << "updatePixmap: _scaleFactor=" << _scaleFactor << endl);
    imageLabel->resize(_scaleFactor * imageLabel->pixmap()->size());
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


void getImageThread::getImage(CASSsoapProxy *cass, cass::ImageFormat format, int attachId)
{
    VERBOSEOUT(cout << "getImageThread::getImage" << endl);
    _cass = cass;
    _format = format;
    _attachId = attachId;
    start();
}


void getImageThread::run()
{
    VERBOSEOUT(cout << "getImageThread::run" << endl);
    bool ret;
    _cass->getImage(_format, _attachId, &ret);
    if(! ret) {
        cerr << "Did not get image" << endl;
        return;
    }
    VERBOSEOUT(cout << "getImageThread::run: Got image" << endl);
    soap_multipart::iterator attachment(_cass->dime.begin());
    if(_cass->dime.end() == attachment) {
        cerr << "Did not get attachment!" << endl;
        return;
    }
    VERBOSEOUT(cout << "getImageThread::run: DIME attachment:" << endl);
    VERBOSEOUT(cout << "  Memory=" << (void*)(*attachment).ptr << endl);
    VERBOSEOUT(cout << "  Size=" << (*attachment).size << endl);
    VERBOSEOUT(cout << "  Type=" << ((*attachment).type?(*attachment).type:"null")
            << endl);
    VERBOSEOUT(cout << "  ID=" << ((*attachment).id?(*attachment).id:"null") << endl);
#warning Fix imageformat
    QImage image(QImage::fromData((uchar*)(*attachment).ptr, (*attachment).size,
            imageformatName(cass::ImageFormat(_format - 1)).c_str()));
    VERBOSEOUT(cout << "getImageThread::run: byteCount=" << image.byteCount() << endl);
    emit newImage(image);
}


void ImageViewer::on_getImage_triggered()
{
    VERBOSEOUT(cout << "on_getImage_triggered" << endl);
    if(_ready) {
        _statusLED->setStatus(true, Qt::green);
        _ready = false;
#warning Fix imageformat
        _githread.getImage(_cass, cass::TIFF, _attachId->value());
    } else {
        _statusLED->setStatus(true, Qt::red);
    }
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
    if(! imageLabel->pixmap())
        return;
    QPrintDialog dialog(&printer, this);
    if(dialog.exec()) {
        QPainter painter(&printer);
        QRect rect = painter.viewport();
        QSize size = imageLabel->pixmap()->size();
        size.scale(rect.size(), Qt::KeepAspectRatio);
        painter.setViewport(rect.x(), rect.y(), size.width(), size.height());
        painter.setWindow(imageLabel->pixmap()->rect());
        painter.drawPixmap(0, 0, *imageLabel->pixmap());
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
    imageLabel->adjustSize();
    _scaleFactor = 1.0;
    _zoom->blockSignals(true);
    _zoom->setValue(100.);
    _zoom->blockSignals(false);
}


void ImageViewer::on_fitToWindow_triggered()
{
    VERBOSEOUT(cout << "on_fitToWindow_triggered" << endl);
    bool fitToWindow = _ui.fitToWindow->isChecked();
    scrollArea->setWidgetResizable(fitToWindow);
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
    if(!imageLabel->pixmap())
        return;
    _scaleFactor *= factor;
    imageLabel->resize(_scaleFactor * imageLabel->pixmap()->size());

    adjustScrollBar(scrollArea->horizontalScrollBar(), factor);
    adjustScrollBar(scrollArea->verticalScrollBar(), factor);

    _ui.zoomIn->setEnabled(_scaleFactor < 3.0);
    _ui.zoomOut->setEnabled(_scaleFactor > 0.333);
}


void ImageViewer::adjustScrollBar(QScrollBar *scrollBar, double factor)
{
    scrollBar->setValue(int(factor * scrollBar->value() +
                            ((factor - 1) * scrollBar->pageStep()/2)));
}


getImageThread::getImageThread()
{
    VERBOSEOUT(cout << "getImageThread::getImageThread" << endl);

}




}



// Local Variables:
// coding: utf-8
// mode: C++
// c-file-offsets: ((c . 0) (innamespace . 0))
// c-file-style: "Stroustrup"
// fill-column: 100
// End:
