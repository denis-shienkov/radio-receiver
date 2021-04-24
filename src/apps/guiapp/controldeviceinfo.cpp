#include "controldeviceinfo.h"
#include "controldeviceinfo_p.h"

ControlDeviceInfo::ControlDeviceInfo(const ControlDeviceInfo &) = default;

ControlDeviceInfo::ControlDeviceInfo(ControlDeviceInfoPrivate &dd)
    : d_ptr(&dd)
{
}

ControlDeviceInfo::~ControlDeviceInfo() = default;

ControlDeviceInfo &ControlDeviceInfo::operator=(const ControlDeviceInfo &) = default;

QString ControlDeviceInfo::systemPath() const
{
    return d_ptr->systemPath;
}

QString ControlDeviceInfo::product() const
{
    return d_ptr->product;
}

QString ControlDeviceInfo::manufacturer() const
{
    return d_ptr->manufacturer;
}

QString ControlDeviceInfo::serialNumber() const
{
    return d_ptr->serialNumber;
}
