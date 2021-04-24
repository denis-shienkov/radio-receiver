#include "controlappwindow.h"
#include "ui_controlappwindow.h"

ControlAppWindow::ControlAppWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_ui(std::make_unique<Ui::ControlAppWindow>())
{
    m_ui->setupUi(this);
}

ControlAppWindow::~ControlAppWindow() = default;
