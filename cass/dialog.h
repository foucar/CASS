//Author Lutz Foucar

#ifndef DIALOG_H
#define DIALOG_H

#include <QWidget>

class QLabel;
class QPushButton;

namespace cass
{
    class Window : public QWidget
    {
        Q_OBJECT

    public:
        Window();

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
