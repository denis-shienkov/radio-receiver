#ifndef CONTROLDEVICE_P_H
#define CONTROLDEVICE_P_H

#include "controldevice.h"

#include <private/qobject_p.h>

#if defined(Q_OS_WIN32)
#  include <windows.h>
#else
#  error Unsupported OS
#endif

#include <memory>

class ControlDevicePrivate : public QObjectPrivate
{
    Q_DECLARE_PUBLIC(ControlDevice)
public:
    ControlDevicePrivate() {}

    bool open();
    void close();
    bool setup();

    QString systemPath;
    QString errorString;

    QVector<QByteArray> incomingReports;
    QVector<QByteArray> outgoingReports;

    ControlDevice::ControlDeviceState state = ControlDevice::ControlDeviceState::UnconnectedState;
    ControlDevice::ControlDeviceError error = ControlDevice::ControlDeviceError::NoError;

    QByteArray incomingReport;
    QByteArray outgoingReport;

#if defined(Q_OS_WIN32)
    bool startAsyncRead();
    bool completeAsyncRead(DWORD bytesTransferred, DWORD errorCode);
    bool cancelAsyncRead();
    bool startAsyncWrite();
    bool completeAsyncWrite(DWORD bytesTransferred, DWORD errorCode);
    bool cancelAsyncWrite();

    HANDLE deviceHandle = INVALID_HANDLE_VALUE;

    class Overlapped final : public OVERLAPPED
    {
        Q_DISABLE_COPY(Overlapped)
    public:
        explicit Overlapped(ControlDevicePrivate *d)
            : dptr(d) {}

        void clear()
        {
            ::ZeroMemory(this, sizeof(OVERLAPPED));
        }

        ControlDevicePrivate *dptr = nullptr;
    };

    std::unique_ptr<class Overlapped> incomingOverlapped;
    std::unique_ptr<class Overlapped> outgoingOverlapped;
#endif
};

#endif // CONTROLDEVICE_P_H
