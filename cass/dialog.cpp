//Author Lutz Foucar

#include <QtGui>
#include "dialog.h"

cass::Window::Window()
{
    statusLabel = new QLabel;
    inputRateLabel = new QLabel;
    processRateLabel = new QLabel;

    loadButton = new QPushButton(tr("Load Settings"));
    saveButton = new QPushButton(tr("Save Settings"));
    startButton = new QPushButton(tr("Start Queue"));

    quitButton = new QPushButton(tr("Quit"));
    quitButton->setAutoDefault(false);

    statusLabel->setText(tr("load / save CASS Settings"));
    updateInputRate(0);
    updateProcessRate(0);

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
    mainLayout->addWidget(inputRateLabel);
    mainLayout->addWidget(processRateLabel);
    mainLayout->addLayout(buttonLayout);
    setLayout(mainLayout);

    setWindowTitle(tr("CASS"));
}

void cass::Window::updateInputRate(double rate)
{
  inputRateLabel->setText(tr("Input: %1 Hz").arg(rate));
}

void cass::Window::updateProcessRate(double rate)
{
  processRateLabel->setText(tr("Process: %1 Hz").arg(rate));
}
