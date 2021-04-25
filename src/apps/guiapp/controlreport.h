#ifndef CONTROLREPORT_H
#define CONTROLREPORT_H

#include <QByteArray>
#include <array>

class ControlReport
{
public:
    explicit ControlReport(const QByteArray &payload);
    explicit ControlReport(quint8 identifier, const QByteArray &payload);

    bool isEmpty() const noexcept;

    void setIdentifier(quint8 identifier) noexcept;
    quint8 identifier() const noexcept;

    void setPayload(const QByteArray &payload);
    QByteArray payload() const;

    QByteArray data() const;
    qint16 size() const noexcept;

private:
    quint8 m_identifier = 0;
    QByteArray m_payload;
};

Q_DECLARE_TYPEINFO(ControlReport, Q_RELOCATABLE_TYPE);

#endif // CONTROLREPORT_H
