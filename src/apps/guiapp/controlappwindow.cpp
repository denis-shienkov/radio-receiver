#include "controlappwindow.h"
#include "controldeviceinfo.h"
#include "ui_controlappwindow.h"

ControlAppWindow::ControlAppWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_ui(std::make_unique<Ui::ControlAppWindow>())
{
    m_ui->setupUi(this);
    enumerateDevices();
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
