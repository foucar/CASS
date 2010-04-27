#include <QtGui>

#include "imageviewer.h"

#include "CASSsoap.nsmap"

using namespace std;


ImageViewer::ImageViewer(QWidget *parent, Qt::WFlags flags)
        : QMainWindow(parent, flags)
{
    _ui.setupUi(this);
    QSettings settings;

    _servername = new QLineEdit(settings.value("servername", "server?").toString());
    _servername->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    connect(_servername, SIGNAL(editingFinished()), this, SLOT(updateServer()));
    _ui.toolBar->addWidget(_servername);
    _serverport = new QSpinBox();
    _serverport->setKeyboardTracking(false);
    _serverport->setRange(1000, 50000);
    _serverport->setValue(settings.value("serverport", 10000).toInt());
    connect(_serverport, SIGNAL(valueChanged(int)), this, SLOT(updateServer()));
    _ui.toolBar->addWidget(_serverport);

    QWidget *spacer1 = new QWidget();
    spacer1->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    _ui.toolBar->addWidget(spacer1);

/*
    _picture = new QSpinBox();
    _picture->setRange(1, 50000);
    _picture->setValue(101);
    _ui.toolBar->addWidget(_picture);
*/
    _pictureformat = new QComboBox();
    QStringList formats;
#warning only write formats ?
    formats << "BMP" << "GIF" << "JPG" << "JPEG" << "PNG" << "PBM"
            << "PGM" << "PPM" << "TIFF" << "XBM" << "XPM";
    _ui.toolBar->addWidget(_pictureformat);
    _pictureformat->insertItems(0, formats);
    _pictureformat->setCurrentIndex(settings.value("pictureformat", 4).toInt());

    _zoom = new QDoubleSpinBox();
    _zoom->setKeyboardTracking(false);
    _zoom->setDecimals(1);
    _zoom->setRange(-50000., 50000.);
    _zoom->setValue(settings.value("zoom", 100.).toDouble());
    connect(_zoom, SIGNAL(valueChanged(double)), this, SLOT(zoomChanged(double)));
    _ui.toolBar->addWidget(_zoom);
    QLabel *zunit = new QLabel;
    zunit->setText("%");
    _ui.toolBar->addWidget(zunit);

    QWidget *spacer2 = new QWidget();
    spacer2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    _ui.toolBar->addWidget(spacer2);

    _running = new QCheckBox();
    connect(_running, SIGNAL(released()), this, SLOT(running()));
    _ui.toolBar->addWidget(_running);

    _period = new QDoubleSpinBox();
    _period->setRange(0.01, 100.);
    _period->setValue(settings.value("period", 10.).toDouble());
    _ui.toolBar->addWidget(_period);
    QLabel *punit = new QLabel;
    punit->setText("Hz");
    _ui.toolBar->addWidget(punit);

    imageLabel = new QLabel;
    imageLabel->setBackgroundRole(QPalette::Base);
    imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    imageLabel->setScaledContents(true);

    scrollArea = new QScrollArea;
    scrollArea->setBackgroundRole(QPalette::Dark);
    scrollArea->setWidget(imageLabel);
    setCentralWidget(scrollArea);

    _ui.fitToWindow->setChecked(settings.value("fittowindow", false).toBool());

    updateServer();
}


void ImageViewer::closeEvent(QCloseEvent *event)
{
    cout << "closeEvent" << endl;
    QSettings settings;
    settings.setValue("pictureformat", _pictureformat->currentIndex());
    settings.setValue("servername", _servername->text());
    settings.setValue("serverport", _serverport->value());
    settings.setValue("period", _period->value());
    settings.setValue("zoom", _zoom->value());
    settings.setValue("fittowindow", _ui.fitToWindow->isChecked());

    event->accept();
}


void ImageViewer::running()
{
    cout << "running" << endl;
    while(_running->isChecked()) {
        _time.start();
        on_open_triggered();
        statusBar()->showMessage(QString().setNum(1000./_time.elapsed(),'g',2)+" Hz");
        usleep(int(1000000./_period->value()));
        qApp->processEvents(QEventLoop::AllEvents);
    }
}

/*
void ImageViewer::on_open_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this,
                                    tr("Open File"), QDir::currentPath());
    if (!fileName.isEmpty()) {
        QImage image(fileName);
        if (image.isNull()) {
            QMessageBox::information(this, tr("Image Viewer"),
                                     tr("Cannot load %1.").arg(fileName));
            return;
        }
        imageLabel->setPixmap(QPixmap::fromImage(image));
        _scaleFactor = 1.0;

        _ui.print->setEnabled(true);
        _ui.fitToWindow->setEnabled(true);
        updateActions();

        if (!_ui.fitToWindow->isChecked())
            imageLabel->adjustSize();
    }
}
*/

void ImageViewer::updateServer()
{
    cout << "updateServer: ";
    _cass.soap_endpoint = (_servername->text() + ":" +
            _serverport->text()).toStdString().c_str();
    cout << _cass.soap_endpoint << endl;
}


void ImageViewer::on_open_triggered()
{
    bool ret;
    _cass.getImage(2, 101, &ret);
    if(ret)
        cout << "return value: 'true'" << endl;
    else {
        cout << "return value is 'false'" << endl;
        return;
    }
    soap_multipart::iterator attachment = _cass.dime.begin();

    cout << "DIME attachment:" << endl;
    cout << "Memory=" << (void*)(*attachment).ptr << endl;
    cout << "Size=" << (*attachment).size << endl;
    cout << "Type=" << ((*attachment).type?(*attachment).type:"null") << endl;
    cout << "ID=" << ((*attachment).id?(*attachment).id:"null") << endl;
    QImage image(QImage::fromData((uchar*)(*attachment).ptr, (*attachment).size, "PNG"));

    imageLabel->setPixmap(QPixmap::fromImage(image));
    _scaleFactor = 1.0;

    _ui.print->setEnabled(true);
    _ui.fitToWindow->setEnabled(true);
    updateActions();

    if(!_ui.fitToWindow->isChecked())
        imageLabel->adjustSize();
}


void ImageViewer::on_readIni_triggered()
{
    cout << "readIni" << endl;
}


void ImageViewer::on_quitServer_triggered()
{
    cout << "quitServer" << endl;
}


void ImageViewer::on_print_triggered()
{
    Q_ASSERT(imageLabel->pixmap());
#ifndef QT_NO_PRINTER
    QPrintDialog dialog(&printer, this);
    if (dialog.exec()) {
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
    cout << "zoomChanged: value=" << value << endl;
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
}


void ImageViewer::on_fitToWindow_triggered()
{
    bool fitToWindow = _ui.fitToWindow->isChecked();
    scrollArea->setWidgetResizable(fitToWindow);
    if (!fitToWindow) {
        on_normalSize_triggered();
    }
    updateActions();
}


void ImageViewer::on_about_triggered()
{
    QMessageBox::about(this, tr("About Image Viewer"),
            tr("<p>The <b>Image Viewer</b> example shows how to combine QLabel "
               "and QScrollArea to display an image. QLabel is typically used "
               "for displaying a text, but it can also display an image. "
               "QScrollArea provides a scrolling view around another widget. "
               "If the child widget exceeds the size of the frame, QScrollArea "
               "automatically provides scroll bars. </p><p>The example "
               "demonstrates how QLabel's ability to scale its contents "
               "(QLabel::scaledContents), and QScrollArea's ability to "
               "automatically resize its contents "
               "(QScrollArea::widgetResizable), can be used to implement "
               "zooming and scaling features. </p><p>In addition the example "
               "shows how to use QPainter to print an image.</p>"));
}



void ImageViewer::updateActions()
{
    _ui.zoomIn->setEnabled(!_ui.fitToWindow->isChecked());
    _ui.zoomOut->setEnabled(!_ui.fitToWindow->isChecked());
    _ui.normalSize->setEnabled(!_ui.fitToWindow->isChecked());
    _zoom->setEnabled(!_ui.fitToWindow->isChecked());
}


void ImageViewer::scaleImage(double factor)
{
    Q_ASSERT(imageLabel->pixmap());
    _scaleFactor *= factor;
    imageLabel->resize(_scaleFactor * imageLabel->pixmap()->size());

    adjustScrollBar(scrollArea->horizontalScrollBar(), factor);
    adjustScrollBar(scrollArea->verticalScrollBar(), factor);

    _ui.zoomIn->setEnabled(_scaleFactor < 3.0);
    _ui.zoomOut->setEnabled(_scaleFactor > 0.333);
}


void ImageViewer::adjustScrollBar(QScrollBar *scrollBar, double factor)
{
    scrollBar->setValue(int(factor * scrollBar->value()
                            + ((factor - 1) * scrollBar->pageStep()/2)));
}
