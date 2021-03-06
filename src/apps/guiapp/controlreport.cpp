#include "controlreport.h"

#include "../fwapp/fwapp.h"

ControlReport::ControlReport(const QByteArray &payload)
{
    setPayload(payload);
}

ControlReport::ControlReport(quint8 identifier, const QByteArray &payload)
{
    setIdentifier(identifier);
    setPayload(payload);
}

bool ControlReport::isEmpty() const noexcept
{
    return m_payload.isEmpty();
}

void ControlReport::setIdentifier(quint8 identifier) noexcept
{
    m_identifier = identifier;
}

quint8 ControlReport::identifier() const noexcept
{
    return m_identifier;
}

void ControlReport::setPayload(const QByteArray &payload)
{
    m_payload = payload;
    const auto length = m_payload.size();
    if (length < UDB_HID_REPORT_PAYLOAD_SIZE)
        m_payload.append(QByteArray(UDB_HID_REPORT_PAYLOAD_SIZE - length, 0));
    else
        m_payload = m_payload.left(UDB_HID_REPORT_PAYLOAD_SIZE);
}

QByteArray ControlReport::payload() const
{
    return m_payload;
}

QByteArray ControlReport::data() const
{
    QByteArray result;
    result.append(m_identifier);
    result.append(m_payload);
    return result;
}

qint16 ControlReport::size() const noexcept
{
    return sizeof(m_identifier) + m_payload.size();
}
