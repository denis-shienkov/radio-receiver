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

    QVector<ControlReport> recvReports;
    QVector<ControlReport> sendReports;

    ControlDevice::ControlDeviceState state = ControlDevice::ControlDeviceState::UnconnectedState;
    ControlDevice::ControlDeviceError error = ControlDevice::ControlDeviceError::NoError;

    QByteArray recvData;
    QByteArray sendData;

    struct Capabilities {
        quint16 recvReportSize = 0;
        quint16 sendReportSize = 0;
    };

    Capabilities capabilities = {};

#if defined(Q_OS_WIN32)
    bool startAsyncRecv();
    bool completeAsyncRecv(DWORD bytesTransferred, DWORD errorCode);
    bool cancelAsyncRecv();
    bool startAsyncSend();
    bool completeAsyncSend(DWORD bytesTransferred, DWORD errorCode);
    bool cancelAsyncSend();

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

    std::unique_ptr<class Overlapped> recvOverlapped;
    std::unique_ptr<class Overlapped> sendOverlapped;
#endif
};

#endif // CONTROLDEVICE_P_H
