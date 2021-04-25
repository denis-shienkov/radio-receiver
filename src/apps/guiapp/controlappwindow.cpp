#include "controlappwindow.h"
#include "controldevice.h"
#include "controldeviceinfo.h"
#include "ui_controlappwindow.h"

ControlAppWindow::ControlAppWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_ui(std::make_unique<Ui::ControlAppWindow>())
    , m_device(new ControlDevice(this))
{
    m_ui->setupUi(this);
    m_ui->deviceSendLineEdit->setInputMask("hhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhhh");
    enumerateDevices();

    connect(m_ui->deviceOpenButton, &QPushButton::clicked, this, [this]{
        const auto deviceState = m_device->state();
        if (deviceState == ControlDevice::ControlDeviceState::UnconnectedState) {
            const auto systemPath = m_ui->deviceComboBox->currentData().toString();
            m_device->setSystemPath(systemPath);
            m_device->connectDevice();
        } else if (deviceState == ControlDevice::ControlDeviceState::ConnectedState) {
            m_device->disconnectDevice();
        }
    });

    connect(m_ui->deviceSendButton, &QPushButton::clicked, this, [this] {
        const QByteArray data = QByteArray::fromHex(m_ui->deviceSendLineEdit->text().toLocal8Bit());
        ControlReport report(data);
        m_device->sendReport(report);
    });

    auto handleDeviceState = [this](ControlDevice::ControlDeviceState state) {
        if (state == ControlDevice::ControlDeviceState::UnconnectedState) {
            m_ui->deviceOpenButton->setEnabled(true);
            m_ui->deviceSendButton->setEnabled(false);
            m_ui->deviceOpenButton->setText(tr("Open"));
        } else if (state == ControlDevice::ControlDeviceState::ConnectedState) {
            m_ui->deviceOpenButton->setEnabled(true);
            m_ui->deviceSendButton->setEnabled(true);
            m_ui->deviceOpenButton->setText(tr("Close"));
        } else {
            m_ui->deviceOpenButton->setEnabled(false);
            m_ui->deviceSendButton->setEnabled(false);
        }
    };
    connect(m_device, &ControlDevice::stateChanged, this, handleDeviceState);

    handleDeviceState(m_device->state());
}

ControlAppWindow::~ControlAppWindow() = default;

void ControlAppWindow::enumerateDevices()
{
    m_ui->deviceComboBox->clear();
    const auto devices = ControlDeviceInfo::availableDevices();
    for (const auto &device : devices) {
        m_ui->deviceComboBox->addItem(device.product(), device.systemPath());
    }
}
