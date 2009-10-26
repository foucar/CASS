//Author Lutz Foucar

#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>

class QLabel;
class QPushButton;

namespace cass
{
    class Dialog : public QDialog
    {
        Q_OBJECT

    public:
        Dialog(QWidget *parent = 0);

    signals:
        void load();
        void save();
        void quit();

    private:
        QLabel      *statusLabel;
        QPushButton *loadButton;
        QPushButton *saveButton;
        QPushButton *quitButton;
    };
}
#endif
