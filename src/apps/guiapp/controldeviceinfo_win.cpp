#include "controldeviceinfo.h"
#include "controldeviceinfo_p.h"
#include <QStringList>

#include "../fwapp/fwapp.h"

#include <windows.h>
#include <setupapi.h>
#include <hidsdi.h>

#include <memory>

static GUID fetchInterfaceGuid()
{
    GUID guid;
    ::HidD_GetHidGuid(&guid);
    return guid;
}

static HIDD_ATTRIBUTES fetchAttributes(HANDLE h)
{
    HIDD_ATTRIBUTES attributes = {};
    attributes.Size = sizeof(attributes);
    ::HidD_GetAttributes(h, &attributes);
    return attributes;
}

static QString fetchSerialNumber(HANDLE h)
{
    QByteArray result(256, 0);
    if (::HidD_GetSerialNumberString(h, result.data(), result.size()))
        return QString::fromWCharArray(reinterpret_cast<const wchar_t *>(result.constData())).trimmed();
    return {};
}

static QString fetchProduct(HANDLE h)
{
    QByteArray result(256, 0);
    if (::HidD_GetProductString(h, result.data(), result.size()))
        return QString::fromWCharArray(reinterpret_cast<const wchar_t *>(result.constData())).trimmed();
    return {};
}

static QString fetchManufacturer(HANDLE h)
{
    QByteArray result(256, 0);
    if (::HidD_GetManufacturerString(h, result.data(), result.size()))
        return QString::fromWCharArray(reinterpret_cast<const wchar_t *>(result.constData())).trimmed();
    return {};
}

static std::unique_ptr<ControlDeviceInfoPrivate> queryUniqueInformation(const PSP_INTERFACE_DEVICE_DETAIL_DATA deviceDetailData)
{
    const QString systemPath = QString::fromWCharArray(deviceDetailData->DevicePath);
    const DWORD desiredAccess = 0; // we do not want to read/write, we want only query a capabilities
    const DWORD shareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
    const HANDLE h = ::CreateFile(reinterpret_cast<const wchar_t*>(systemPath.utf16()),
                                  desiredAccess, shareMode, Q_NULLPTR, OPEN_EXISTING, 0, Q_NULLPTR);
    if (!h || (h == INVALID_HANDLE_VALUE))
        return {};

    const HIDD_ATTRIBUTES attributes = fetchAttributes(h);
    if (attributes.VendorID != USB_VENDOR_ID
        || attributes.ProductID != USB_PRODUCT_ID) {
        return {};
    }

    auto device = std::make_unique<ControlDeviceInfoPrivate>();
    device->systemPath = systemPath;
    device->serialNumber = fetchSerialNumber(h);
    device->product = fetchProduct(h);
    device->manufacturer = fetchManufacturer(h);

    ::CloseHandle(h);
    return device;
}

QList<ControlDeviceInfo> ControlDeviceInfo::availableDevices()
{
    const GUID hidguid = fetchInterfaceGuid();
    const DWORD options = DIGCF_PRESENT | DIGCF_DEVICEINTERFACE;
    const HDEVINFO deviceInfoHandle = ::SetupDiGetClassDevs(&hidguid, Q_NULLPTR, Q_NULLPTR, options);
    if (deviceInfoHandle == INVALID_HANDLE_VALUE)
        return {};

    QList<ControlDeviceInfo> devices;
    DWORD deviceInterfaceIndex = 0;
    for (;;) {
        SP_DEVICE_INTERFACE_DATA interfaceData;
        interfaceData.cbSize = sizeof(interfaceData);
        if (!::SetupDiEnumDeviceInterfaces(deviceInfoHandle, Q_NULLPTR, &hidguid,
                                           deviceInterfaceIndex++, &interfaceData)) {
            break;
        }

        DWORD requiredSize = 0;
        if (!::SetupDiGetDeviceInterfaceDetail(deviceInfoHandle, &interfaceData, Q_NULLPTR,
                                               0, &requiredSize, Q_NULLPTR)) {
            const DWORD error = ::GetLastError();
            if (error != ERROR_INSUFFICIENT_BUFFER)
                break;
        }

        QByteArray buffer(requiredSize, 0);
        PSP_INTERFACE_DEVICE_DETAIL_DATA deviceDetailData =
            reinterpret_cast<PSP_INTERFACE_DEVICE_DETAIL_DATA>(buffer.data());
        deviceDetailData->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);

        if (!::SetupDiGetDeviceInterfaceDetail(deviceInfoHandle, &interfaceData, deviceDetailData,
                                               buffer.size(), Q_NULLPTR, Q_NULLPTR)) {
            break;
        }

        auto device = queryUniqueInformation(deviceDetailData);
        if (device && !device->systemPath.isEmpty())
            devices.append(ControlDeviceInfo(*device.release()));
    }

    ::SetupDiDestroyDeviceInfoList(deviceInfoHandle);
    return devices;
}
