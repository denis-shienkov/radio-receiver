#ifndef CONTROLREPORT_H
#define CONTROLREPORT_H

#include <QByteArray>
#include <array>

class ControlReport
{
public:
    explicit ControlReport(const QByteArray &payload);
    explicit ControlReport(quint8 identifier, const QByteArray &payload);

    void setIdentifier(quint8 identifier) noexcept;
    quint8 identifier() const noexcept;

    void setPayload(const QByteArray &payload);
    QByteArray payload() const;

    QByteArray data() const;

private:
    quint8 m_identifier = 0;
    QByteArray m_payload;
};

#endif // CONTROLREPORT_H
