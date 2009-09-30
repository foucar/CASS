#ifndef PARAMETER_BACKEND_H
#define PARAMETER_BACKEND_H

#include <QtCore/QSettings>
#include "cass.h"

namespace cass
{
    class CASSSHARED_EXPORT ParameterBackend : public QSettings
    {
    public:
        ParameterBackend():
                QSettings("./cass.ini",QSettings::IniFormat)
        {
            beginGroup("cass");
        }

        virtual ~ParameterBackend()
        {
            endGroup();
        }

        virtual void load()=0;
        virtual void save()=0;
    };
}//end namespace cass

#endif // PARAMETER_BACKEND_H
