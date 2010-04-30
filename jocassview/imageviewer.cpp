// Copyright (C) 2010 Uwe Hoppe

#include <QtGui>
#include <deque>
#include "imageviewer.h"
#include "CASSsoap.nsmap"


namespace jocassview
{
    using namespace std;


    ImageViewer::ImageViewer(QWidget *parent, Qt::WFlags flags)
            : QMainWindow(parent, flags)
    {
        _ui.setupUi(this);
        connect(_ui.aboutQt, SIGNAL(triggered()), qApp, SLOT(aboutQt()));
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

        _attachId = new QSpinBox();
        _attachId->setRange(1, 50000);
        _attachId->setValue(settings.value("attachId", 101).toInt());
        _ui.toolBar->addWidget(_attachId);

        _picturetype = new QComboBox();
        QStringList formats;
#warning only write formats ?
        formats << "BMP" << "GIF" << "JPG" << "JPEG" << "PNG" << "PBM"
                << "PGM" << "PPM" << "TIFF" << "XBM" << "XPM";
        _ui.toolBar->addWidget(_picturetype);
        _picturetype->insertItems(0, formats);
        _picturetype->setCurrentIndex(settings.value("pictureformat", 4).toInt());

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

        _ristatus = new QRadioButton();
        readIniStatusLED(0, false);
        _ui.toolBar->addWidget(_ristatus);

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
        _scaleFactor = settings.value("scaleFactor", 1.0).toDouble();

        _ui.fitToWindow->setChecked(settings.value("fittowindow", false).toBool());
        updateServer();
    }


    void ImageViewer::showEvent(QShowEvent *event)
    {
        _running->setFocus();
        event->accept();
    }


    void ImageViewer::closeEvent(QCloseEvent *event)
    {
        QSettings settings;
        settings.setValue("pictureformat", _picturetype->currentIndex());
        settings.setValue("servername", _servername->text());
        settings.setValue("serverport", _serverport->value());
        settings.setValue("period", _period->value());
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

#warning Once only
            _ui.print->setEnabled(true);
            _ui.fitToWindow->setEnabled(true);
            updateActions();

            if(!_ui.fitToWindow->isChecked())
                imageLabel->adjustSize();
        }
    }


    void ImageViewer::on_getImage_triggered()
    {
        _cass.getImage(2, _attachId->value(), &_ret);
#warning fix image format
//    _cass.getImage(_picturetype->currentIndex(), _attachId->value(), &_ret);
        if(_ret)
            readIniStatusLED(0, true);
        else {
            readIniStatusLED(1, true);
            return;
        }
        soap_multipart::iterator attachment(_cass.dime.begin());
        cout << "DIME attachment:" << endl;
        cout << "Memory=" << (void*)(*attachment).ptr << endl;
        cout << "Size=" << (*attachment).size << endl;
        cout << "Type=" << ((*attachment).type?(*attachment).type:"null") << endl;
        cout << "ID=" << ((*attachment).id?(*attachment).id:"null") << endl;
        QImage image(QImage::fromData((uchar*)(*attachment).ptr, (*attachment).size,
                _picturetype->currentText().toStdString().c_str()));
        imageLabel->setPixmap(QPixmap::fromImage(image));

#warning Once only
        _ui.print->setEnabled(true);
        _ui.fitToWindow->setEnabled(true);
        updateActions();

        if(_ui.fitToWindow->isChecked())
            return;
        cout << "getImage: _scaleFactor=" << _scaleFactor << endl;
        imageLabel->resize(_scaleFactor * imageLabel->pixmap()->size());
    }


    void ImageViewer::running()
    {
        cout << "running" << endl;
        deque<double> ct;
        deque<double>::iterator it;
        size_t an(10);
        double times(0.);
        while(_running->isChecked()) {
            _time.start();
            on_getImage_triggered();
            ct.push_back(_time.elapsed());
            times = 0.;
            if(ct.size() > an)
                ct.pop_front();
            for(it=ct.begin(); it!=ct.end(); ++it)
                times += *it;
            times /= ct.size();
            statusBar()->showMessage(QString().setNum(1000./times, 'g', 2) + " Hz (" +
                    QString().setNum(ct.size()) + ")");
            usleep(int(1000000./_period->value()));
            qApp->processEvents(QEventLoop::AllEvents);
        }
        readIniStatusLED(0, false);
    }


    void ImageViewer::on_readIni_triggered()
    {
        cout << "readIni" << endl;
        _cass.readini(0, &_ret);
        if(!_ret)
            QMessageBox::information(this, tr("jocassviewer"),
                    tr("Error: Cann't read ini."));
    }


    void ImageViewer::on_quitServer_triggered()
    {
        cout << "quitServer" << endl;
        _cass.quit(&_ret);
        if(!_ret)
            QMessageBox::information(this, tr("jocassviewer"),
                    tr("Error: Cann't quit server."));
    }


    void ImageViewer::on_print_triggered()
    {
        if(!imageLabel->pixmap())
            return;
#ifndef QT_NO_PRINTER
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


    void ImageViewer::readIniStatusLED(int color, bool on)
    {
        QPalette palette;
        QBrush brush(Qt::SolidPattern);
        switch(color)
        {
        case 0: // green
            brush.setColor(QColor(0, 255, 0, 255));
            break;
        case 1: // red
            brush.setColor(QColor(255, 0, 0, 255));
            break;
        default:
            return;
        }
        palette.setBrush(QPalette::Active, QPalette::Text, brush);
        _ristatus->setPalette(palette);
        _ristatus->setChecked(on);
    }


    void ImageViewer::updateActions()
    {
        _ui.zoomIn->setEnabled(!_ui.fitToWindow->isChecked());
        _ui.zoomOut->setEnabled(!_ui.fitToWindow->isChecked());
        _ui.normalSize->setEnabled(!_ui.fitToWindow->isChecked());
        _zoom->setEnabled(!_ui.fitToWindow->isChecked());
    }


    void ImageViewer::updateServer()
    {
        cout << "updateServer: ";
        _server = (_servername->text() + ":" +_serverport->text()).toStdString();
        _cass.soap_endpoint = _server.c_str();
        cout << _cass.soap_endpoint << endl;
    }


    void ImageViewer::scaleImage(double factor)
    {
        cout << "scaleImage: factor=" << factor << endl;
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

}
