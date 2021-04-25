#undef _WIN32_WINNT
#define _WIN32_WINNT    0x0600 // for CancelIoEx

#include "controldevice_p.h"

#include <windows.h>
#include <hidsdi.h>

#include <QDebug>

static HIDP_CAPS fetchCapabilities(HANDLE h)
{
    PHIDP_PREPARSED_DATA pd = nullptr;
    if (!::HidD_GetPreparsedData(h, &pd))
        return {};
    HIDP_CAPS caps = {};
    const NTSTATUS st = ::HidP_GetCaps(pd, &caps);
    ::HidD_FreePreparsedData(pd);
    if (st != HIDP_STATUS_SUCCESS)
        return {};
    return caps;
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

//    cancelAsyncRead();
//    cancelAsyncWrite();

    if (!::CloseHandle(deviceHandle))
        q->setError(ControlDevice::ControlDeviceError::CloseError, qt_error_string(::GetLastError()));

    deviceHandle = INVALID_HANDLE_VALUE;

    incomingReports.clear();
    outgoingReports.clear();

    incomingReport.clear();
    outgoingReport.clear();
}

bool ControlDevicePrivate::setup()
{
    Q_Q(ControlDevice);

    const HIDP_CAPS caps = fetchCapabilities(deviceHandle);
    incomingReport.resize(caps.InputReportByteLength);
    outgoingReport.resize(caps.OutputReportByteLength);

    incomingOverlapped = std::make_unique<Overlapped>(this);

    setBuffersCnt(deviceHandle, 512);
    if (!startAsyncRead())
        return false;
    return true;
}

bool ControlDevicePrivate::startAsyncRead()
{
    Q_Q(ControlDevice);

    incomingOverlapped->clear();
    if (!::ReadFileEx(deviceHandle, incomingReport.data(), incomingReport.size(), incomingOverlapped.get(),
                      [](DWORD errorCode, DWORD bytesTransfered, OVERLAPPED *overlappedBase) {
                          const auto overlapped = static_cast<Overlapped *>(overlappedBase);
                          if (overlapped->dptr) {
                              const bool success = overlapped->dptr->completeAsyncRead(bytesTransfered, errorCode);
                              if (success)
                                  overlapped->dptr->startAsyncRead();
                          }})) {
        const DWORD errorCode = ::GetLastError();
        if (errorCode != ERROR_IO_PENDING) {
            q->setError(ControlDevice::ControlDeviceError::ReadError, qt_error_string(errorCode));
            return false;
        }
    }
    return true;
}

bool ControlDevicePrivate::completeAsyncRead(DWORD bytesTransferred, DWORD errorCode)
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

    if (bytesTransferred > 0) { // TODO: need to check for the input chunk size?
        incomingReports.append(incomingReport);
        emit q->reportsReceived();
    }
    return true;
}

bool ControlDevicePrivate::cancelAsyncRead()
{
    Q_Q(ControlDevice);

    if (::CancelIoEx(deviceHandle, incomingOverlapped.get()) == 0) {
        const DWORD errorCode = ::GetLastError();
        if (errorCode != ERROR_NOT_FOUND) {
            q->setError(ControlDevice::ControlDeviceError::ReadError, qt_error_string(errorCode));
            return false;
        }
    }
    return true;
}

bool ControlDevicePrivate::startAsyncWrite()
{
    Q_Q(ControlDevice);

    if (outgoingReports.isEmpty())
        return true; // Nothing to write.
    if (!outgoingReport.isEmpty())
        return true; // Already is pending.

    outgoingReport = outgoingReports.first();

    incomingOverlapped->clear();
    if (!::WriteFileEx(deviceHandle, outgoingReport.data(), outgoingReport.size(), outgoingOverlapped.get(),
                      [](DWORD errorCode, DWORD bytesTransfered, OVERLAPPED *overlappedBase) {
                          const auto overlapped = static_cast<Overlapped *>(overlappedBase);
                          if (overlapped->dptr) {
                              const bool success = overlapped->dptr->completeAsyncWrite(bytesTransfered, errorCode);
                              if (success)
                                  overlapped->dptr->startAsyncWrite();
                          }})) {
        const DWORD errorCode = ::GetLastError();
        if (errorCode != ERROR_IO_PENDING) {
            q->setError(ControlDevice::ControlDeviceError::WriteError, qt_error_string(errorCode));
            return false;
        }
    }
    return true;
}

bool ControlDevicePrivate::completeAsyncWrite(DWORD bytesTransferred, DWORD errorCode)
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

    if (bytesTransferred > 0) { // TODO: need to check for the output chunk size?
        outgoingReport.clear();
        outgoingReports.takeFirst(); // Now we can remove sent report.
        emit q->reportsWritten(1);
    }

    return true;
}

bool ControlDevicePrivate::cancelAsyncWrite()
{
    Q_Q(ControlDevice);

    if (::CancelIoEx(deviceHandle, outgoingOverlapped.get()) == 0) {
        const DWORD errorCode = ::GetLastError();
        if (errorCode != ERROR_NOT_FOUND) {
            q->setError(ControlDevice::ControlDeviceError::WriteError, qt_error_string(errorCode));
            return false;
        }
    }
    return true;
}
