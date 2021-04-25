#include "controldevice.h"
#include "controldevice_p.h"

ControlDevice::ControlDevice(QObject *parent)
    : QObject(*new ControlDevicePrivate, parent)
{
}

ControlDevice::ControlDevice(const QString &systemPath, QObject *parent)
    : QObject(*new ControlDevicePrivate, parent)
{
    setSystemPath(systemPath);
}

void ControlDevice::setSystemPath(const QString &systemPath)
{
    Q_D(ControlDevice);
    if (d->systemPath == systemPath)
        return;
    d->systemPath = systemPath;
    emit systemPathChanged(d->systemPath);
}

QString ControlDevice::systemPath() const
{
    Q_D(const ControlDevice);
    return d->systemPath;
}

void ControlDevice::setState(ControlDeviceState state)
{
    Q_D(ControlDevice);
    if (d->state == state)
        return;
    d->state = state;
    emit stateChanged(d->state);
}

ControlDevice::ControlDeviceState ControlDevice::state() const
{
    return d_func()->state;
}

void ControlDevice::clearError()
{
    setError(ControlDevice::ControlDeviceError::NoError);
}

void ControlDevice::setError(ControlDeviceError error, const QString &errorString)
{
    Q_D(ControlDevice);
    d->error = error;
    d->errorString = errorString;
    emit errorOccurred(d->error);
}

ControlDevice::ControlDeviceError ControlDevice::error() const
{
    Q_D(const ControlDevice);
    return d->error;
}

QString ControlDevice::errorString() const
{
    Q_D(const ControlDevice);
    return d->errorString;
}

bool ControlDevice::connectDevice()
{
    Q_D(ControlDevice);

    setState(ControlDeviceState::ConnectingState);
    clearError();
    if (d->open()) {
        setState(ControlDeviceState::ConnectedState);
        return true;
    }
    setState(ControlDeviceState::UnconnectedState);
    return false;
}

void ControlDevice::disconnectDevice()
{
    Q_D(ControlDevice);

    setState(ControlDeviceState::ClosingState);
    d->close();
    setState(ControlDeviceState::UnconnectedState);
}

qint64 ControlDevice::reportsAvailable() const
{
    return d_func()->recvReports.count();
}

qint64 ControlDevice::reportsToWrite() const
{
    return d_func()->sendReports.count();
}

void ControlDevice::sendReport(const ControlReport &report)
{
    Q_D(ControlDevice);

    if (report.size() != d->capabilities.sendReportSize)
        return;
    d->sendReports.append(report);

#if defined (Q_OS_WIN32)
    d->startAsyncSend();
#endif
}

ControlReport ControlDevice::receiveReport()
{
    Q_D(ControlDevice);

    if (d->recvReports.isEmpty())
        return ControlReport{{}}; // empty report
    return d->recvReports.takeFirst();
}
