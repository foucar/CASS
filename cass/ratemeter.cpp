#include <iostream>

#include "ratemeter.h"

cass::Ratemeter::Ratemeter():
        QObject(),
        counter(0)
{
    time = new QTime();
    time->start();
}

cass::Ratemeter::~Ratemeter()
{
    delete time;
}

void cass::Ratemeter::nextEvent()
{
    ++counter;
    if (counter == 32)
    {
       std::cout<< "\rRate is: "<< static_cast<double>(counter)*1000./static_cast<double>(time->elapsed()) <<" Hz"<<std::endl;
       counter = 0;
       time->restart();
    }
}
