//Author Lutz Foucar

#include <QtGui>
#include "dialog.h"

cass::Window::Window()
{
    statusLabel = new QLabel;

    loadButton = new QPushButton(tr("Load Settings"));
    saveButton = new QPushButton(tr("Save Settings"));
    startButton = new QPushButton(tr("Start Queue"));

    quitButton = new QPushButton(tr("Quit"));
    quitButton->setAutoDefault(false);

    statusLabel->setText(tr("load / save CASS Settings"));

    connect(quitButton, SIGNAL(clicked()), this, SIGNAL(quit()));
    connect(loadButton, SIGNAL(clicked()), this, SIGNAL(load()));
    connect(saveButton, SIGNAL(clicked()), this, SIGNAL(save()));
    connect(startButton, SIGNAL(clicked()), this, SIGNAL(start()));

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch(1);
//    buttonLayout->addWidget(startButton);
    buttonLayout->addWidget(loadButton);
    buttonLayout->addWidget(saveButton);
    buttonLayout->addWidget(quitButton);
    buttonLayout->addStretch(1);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(statusLabel);
    mainLayout->addLayout(buttonLayout);
    setLayout(mainLayout);

    setWindowTitle(tr("CASS"));
}
