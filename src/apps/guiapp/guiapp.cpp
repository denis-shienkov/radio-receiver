#include <QApplication>
#include <QDebug>

#include "controldeviceinfo.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    const auto devices = ControlDeviceInfo::availableDevices();
    qDebug() << "Found" << devices.count() << "devices";
    for (const auto &device : devices) {
        qDebug() << "device: ";
        qDebug() << "   - path          :" << device.systemPath();
        qDebug() << "   - product       :" << device.product();
        qDebug() << "   - manufacturer  :" << device.manufacturer();
        qDebug() << "   - serial number :" << device.serialNumber();
    }
    return app.exec();
}
