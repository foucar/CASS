#include <QtGui>

#include "imageviewer.h"

#include "soapCASSsoapProxy.h"
#include "CASSsoap.nsmap"

using namespace std;


ImageViewer::ImageViewer(QWidget *parent, Qt::WFlags flags)
        : QMainWindow(parent, flags)
{
    _ui.setupUi(this);

    _servername = new QLineEdit("compute-0-0");
    _ui.toolBar->addWidget(_servername);

    _serverport = new QSpinBox();
    _serverport->setRange(1000, 50000);
    _serverport->setValue(12321);
    _ui.toolBar->addWidget(_serverport);

    imageLabel = new QLabel;
    imageLabel->setBackgroundRole(QPalette::Base);
    imageLabel->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    imageLabel->setScaledContents(true);

    scrollArea = new QScrollArea;
    scrollArea->setBackgroundRole(QPalette::Dark);
    scrollArea->setWidget(imageLabel);
    setCentralWidget(scrollArea);
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
        scaleFactor = 1.0;

        _ui.print->setEnabled(true);
        _ui.fitToWindow->setEnabled(true);
        updateActions();

        if (!_ui.fitToWindow->isChecked())
            imageLabel->adjustSize();
    }
}
*/


void ImageViewer::on_open_triggered()
{
    bool ret;
    CASSsoapProxy cass;
    cass.soap_endpoint = (_servername->text() + ":" + _serverport->text()).toStdString().c_str();
//    cass.soap_endpoint = "xfhix:12321";

    cass.getImage(2, 101, &ret);
    if(ret)
        cout << "return value: 'true'" << endl;
    else {
        cout << "return value is 'false'" << endl;
        return;
    }

    soap_multipart::iterator attachment = cass.dime.begin();

    cout << "DIME attachment:" << endl;
    cout << "Memory=" << (void*)(*attachment).ptr << endl;
    cout << "Size=" << (*attachment).size << endl;
    cout << "Type=" << ((*attachment).type?(*attachment).type:"null") << endl;
    cout << "ID=" << ((*attachment).id?(*attachment).id:"null") << endl;
    QImage image(QImage::fromData((uchar*)(*attachment).ptr, (*attachment).size, "PNG"));

    imageLabel->setPixmap(QPixmap::fromImage(image));
    scaleFactor = 1.0;

    _ui.print->setEnabled(true);
    _ui.fitToWindow->setEnabled(true);
    updateActions();

    if(!_ui.fitToWindow->isChecked())
        imageLabel->adjustSize();
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


void ImageViewer::on_zoomIn_triggered()
{
    scaleImage(1.25);
}


void ImageViewer::on_zoomOut_triggered()
{
    scaleImage(0.8);
}


void ImageViewer::on_normalSize_triggered()
{
    imageLabel->adjustSize();
    scaleFactor = 1.0;
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
}


void ImageViewer::scaleImage(double factor)
{
    Q_ASSERT(imageLabel->pixmap());
    scaleFactor *= factor;
    imageLabel->resize(scaleFactor * imageLabel->pixmap()->size());

    adjustScrollBar(scrollArea->horizontalScrollBar(), factor);
    adjustScrollBar(scrollArea->verticalScrollBar(), factor);

    _ui.zoomIn->setEnabled(scaleFactor < 3.0);
    _ui.zoomOut->setEnabled(scaleFactor > 0.333);
}


void ImageViewer::adjustScrollBar(QScrollBar *scrollBar, double factor)
{
    scrollBar->setValue(int(factor * scrollBar->value()
                            + ((factor - 1) * scrollBar->pageStep()/2)));
}
