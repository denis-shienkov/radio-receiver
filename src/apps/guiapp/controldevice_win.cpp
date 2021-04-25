#undef _WIN32_WINNT
#define _WIN32_WINNT    0x0600 // for CancelIoEx

#include "controldevice_p.h"

#include "../fwapp/fwapp.h"

#include <windows.h>
#include <hidsdi.h>

#include <QDebug>

static ControlDevicePrivate::Capabilities fetchCapabilities(HANDLE h)
{
    PHIDP_PREPARSED_DATA pd = nullptr;
    if (!::HidD_GetPreparsedData(h, &pd))
        return {};
    HIDP_CAPS caps = {};
    const NTSTATUS st = ::HidP_GetCaps(pd, &caps);
    ::HidD_FreePreparsedData(pd);
    if (st != HIDP_STATUS_SUCCESS)
        return {};
    return {caps.InputReportByteLength, caps.OutputReportByteLength};
}

static bool setBuffersCnt(HANDLE h, int len)
{
    return  HidD_SetNumInputBuffers(h, len);
}

bool ControlDevicePrivate::open()
{
    Q_Q(ControlDevice);

    const DWORD desiredAccess = GENERIC_WRITE | GENERIC_READ;
    const DWORD shareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;
    deviceHandle = ::CreateFile(reinterpret_cast<const wchar_t*>(systemPath.utf16()),
                                desiredAccess, shareMode, Q_NULLPTR, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, Q_NULLPTR);
    if (deviceHandle == INVALID_HANDLE_VALUE) {
        q->setError(ControlDevice::ControlDeviceError::OpenError, qt_error_string(::GetLastError()));
        return false;
    } else if (!setup()) {
        ::CloseHandle(deviceHandle);
        deviceHandle = INVALID_HANDLE_VALUE;
        return false;
    }
    return true;
}

void ControlDevicePrivate::close()
{
    Q_Q(ControlDevice);

//    cancelAsyncRecv();
//    cancelAsyncSend();

    if (!::CloseHandle(deviceHandle))
        q->setError(ControlDevice::ControlDeviceError::CloseError, qt_error_string(::GetLastError()));

    deviceHandle = INVALID_HANDLE_VALUE;

    recvReports.clear();
    sendReports.clear();

    recvData.clear();
    sendData.clear();
}

bool ControlDevicePrivate::setup()
{
    Q_Q(ControlDevice);

    capabilities = fetchCapabilities(deviceHandle);

    enum { EXPECTED_REPORT_SIZE = UDB_HID_REPORT_PAYLOAD_SIZE + 1};

    if (capabilities.recvReportSize != EXPECTED_REPORT_SIZE) {
        q->setError(ControlDevice::ControlDeviceError::OpenError,
                    ControlDevice::tr("Input report size mismatch"));
        return false;
    } else if (capabilities.sendReportSize != EXPECTED_REPORT_SIZE) {
        q->setError(ControlDevice::ControlDeviceError::OpenError,
                    ControlDevice::tr("Output report size mismatch"));
        return false;
    }

    recvOverlapped = std::make_unique<Overlapped>(this);
    sendOverlapped = std::make_unique<Overlapped>(this);

    setBuffersCnt(deviceHandle, 512);
    if (!startAsyncRecv())
        return false;
    return true;
}

bool ControlDevicePrivate::startAsyncRecv()
{
    Q_Q(ControlDevice);

    recvData.resize(capabilities.recvReportSize);
    recvOverlapped->clear();
    if (!::ReadFileEx(deviceHandle, recvData.data(), recvData.size(), recvOverlapped.get(),
                      [](DWORD errorCode, DWORD bytesTransfered, OVERLAPPED *overlappedBase) {
                          const auto overlapped = static_cast<Overlapped *>(overlappedBase);
                          if (overlapped->dptr) {
                              const bool success = overlapped->dptr->completeAsyncRecv(bytesTransfered, errorCode);
                              if (success)
                                  overlapped->dptr->startAsyncRecv();
                          }})) {
        const DWORD errorCode = ::GetLastError();
        if (errorCode != ERROR_IO_PENDING) {
            q->setError(ControlDevice::ControlDeviceError::ReadError, qt_error_string(errorCode));
            return false;
        }
    }
    return true;
}

bool ControlDevicePrivate::completeAsyncRecv(DWORD bytesTransferred, DWORD errorCode)
{
    Q_Q(ControlDevice);

    if (errorCode != ERROR_SUCCESS) {
        if (errorCode == ERROR_OPERATION_ABORTED)
            return false;

        const auto error = (errorCode == ERROR_DEVICE_NOT_CONNECTED)
                               ? ControlDevice::ControlDeviceError::ResourceError
                               : ControlDevice::ControlDeviceError::ReadError;
        q->setError(error, qt_error_string(errorCode));
        return false; // stop
    }

    if (bytesTransferred == capabilities.recvReportSize) {
        recvReports.append(ControlReport{recvData});
        emit q->reportsReceived();
    } else {
        q->setError(ControlDevice::ControlDeviceError::ReadError,
                    ControlDevice::tr("Input report size mismatch"));
    }
    return true;
}

bool ControlDevicePrivate::cancelAsyncRecv()
{
    Q_Q(ControlDevice);

    if (::CancelIoEx(deviceHandle, recvOverlapped.get()) == 0) {
        const DWORD errorCode = ::GetLastError();
        if (errorCode != ERROR_NOT_FOUND) {
            q->setError(ControlDevice::ControlDeviceError::ReadError, qt_error_string(errorCode));
            return false;
        }
    }
    return true;
}

bool ControlDevicePrivate::startAsyncSend()
{
    Q_Q(ControlDevice);

    if (sendReports.isEmpty())
        return true; // Nothing to write.
    if (!sendData.isEmpty())
        return true; // Already is pending.

    sendData = sendReports.first().data();
    sendOverlapped->clear();
    if (!::WriteFileEx(deviceHandle, sendData.data(), sendData.size(), sendOverlapped.get(),
                       [](DWORD errorCode, DWORD bytesTransfered, OVERLAPPED *overlappedBase) {
                           const auto overlapped = static_cast<Overlapped *>(overlappedBase);
                           if (overlapped->dptr) {
                               const bool success = overlapped->dptr->completeAsyncSend(bytesTransfered, errorCode);
                               if (success)
                                   overlapped->dptr->startAsyncSend();
                           }})) {
        const DWORD errorCode = ::GetLastError();
        if (errorCode != ERROR_IO_PENDING) {
            q->setError(ControlDevice::ControlDeviceError::WriteError, qt_error_string(errorCode));
            return false;
        }
    }
    return true;
}

bool ControlDevicePrivate::completeAsyncSend(DWORD bytesTransferred, DWORD errorCode)
{
    Q_Q(ControlDevice);

    if (errorCode != ERROR_SUCCESS) {
        if (errorCode == ERROR_OPERATION_ABORTED)
            return false;

        const auto error = (errorCode == ERROR_DEVICE_NOT_CONNECTED)
                               ? ControlDevice::ControlDeviceError::ResourceError
                               : ControlDevice::ControlDeviceError::WriteError;
        q->setError(error, qt_error_string(errorCode));
        return false; // stop
    }

    if (bytesTransferred == capabilities.sendReportSize) {
        sendData.clear();
        sendReports.takeFirst(); // Now we can remove sent report.
        emit q->reportsWritten(1);
    } else {
        q->setError(ControlDevice::ControlDeviceError::WriteError,
                    ControlDevice::tr("Output report size mismatch"));
    }

    return true;
}

bool ControlDevicePrivate::cancelAsyncSend()
{
    Q_Q(ControlDevice);

    if (::CancelIoEx(deviceHandle, sendOverlapped.get()) == 0) {
        const DWORD errorCode = ::GetLastError();
        if (errorCode != ERROR_NOT_FOUND) {
            q->setError(ControlDevice::ControlDeviceError::WriteError, qt_error_string(errorCode));
            return false;
        }
    }
    return true;
}
