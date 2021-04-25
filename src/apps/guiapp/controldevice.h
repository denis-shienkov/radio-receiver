#ifndef CONTROLDEVICE_H
#define CONTROLDEVICE_H

#include "controlreport.h"

#include <QObject>

class ControlDevicePrivate;

class ControlDevice : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(ControlDevice)
    Q_DISABLE_COPY(ControlDevice)
    Q_ENUMS(ControlDeviceState ControlDeviceError)

public:
    enum class ControlDeviceState {
        UnconnectedState,
        ConnectingState,
        ConnectedState,
        ClosingState
    };

    enum class ControlDeviceError {
        NoError,
        OpenError,
        CloseError,
        WriteError,
        ReadError,
        UnsupportedOperationError,
        ResourceError,
        NotOpenError
    };

    explicit ControlDevice(QObject *parent = nullptr);
    explicit ControlDevice(const QString &systemPath, QObject *parent = nullptr);

    void setSystemPath(const QString &systemPath);
    QString systemPath() const;

    bool connectDevice();
    void disconnectDevice();
    ControlDeviceState state() const;

    ControlDeviceError error() const;
    QString errorString() const;
    void clearError();

    qint64 reportsAvailable() const;
    qint64 reportsToWrite() const;

    void sendReport(const ControlReport &report);
    ControlReport receiveReport();

signals:
    void systemPathChanged(const QString &systemPath);
    void errorOccurred(ControlDevice::ControlDeviceError error);
    void stateChanged(ControlDevice::ControlDeviceState state);
    void reportsReceived();
    void reportsWritten(qint64 reportsCount);

private:
    void setError(ControlDeviceError error, const QString &errorString = QString());
    void setState(ControlDeviceState state);


//#if defined (Q_OS_WIN32)
//    Q_PRIVATE_SLOT(d_func(), void _q_notified(quint32, quint32, OVERLAPPED*))
//#endif

};

Q_DECLARE_TYPEINFO(ControlDevice::ControlDeviceError, Q_PRIMITIVE_TYPE);
Q_DECLARE_TYPEINFO(ControlDevice::ControlDeviceState, Q_PRIMITIVE_TYPE);

#endif // CONTROLDEVICE_H
