#ifndef CONTROLDEVICEINFO_H
#define CONTROLDEVICEINFO_H

#include <QSharedDataPointer>

class ControlDeviceInfoPrivate;

class ControlDeviceInfo
{
public:
    ControlDeviceInfo() = delete;
    ControlDeviceInfo(const ControlDeviceInfo &other);
    ~ControlDeviceInfo();

    void swap(ControlDeviceInfo &other) noexcept
    {
        qSwap(d_ptr, other.d_ptr);
    }

    ControlDeviceInfo &operator=(const ControlDeviceInfo &other);
    ControlDeviceInfo &operator=(ControlDeviceInfo &&other) noexcept
    {
        swap(other);
        return *this;
    }

    QString systemPath() const;
    QString product() const;
    QString manufacturer() const;
    QString serialNumber() const;

    static QList<ControlDeviceInfo> availableDevices();

private:
    explicit ControlDeviceInfo(ControlDeviceInfoPrivate &dd);
    QSharedDataPointer<ControlDeviceInfoPrivate> d_ptr;
};

#endif // CONTROLDEVICEINFO_H
