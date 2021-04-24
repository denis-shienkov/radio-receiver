#ifndef CONTROLDEVICEINFO_P_H
#define CONTROLDEVICEINFO_P_H

#include <QSharedData>

class ControlDeviceInfoPrivate : public QSharedData
{
public:
    QString systemPath;
    QString product;
    QString manufacturer;
    QString serialNumber;
};

#endif // CONTROLDEVICEINFO_P_H
