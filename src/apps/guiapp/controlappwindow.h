#ifndef CONTROLAPPWINDOW_H
#define CONTROLAPPWINDOW_H

#include <QMainWindow>

#include <memory>

namespace Ui {
class ControlAppWindow;
}

class ControlAppWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ControlAppWindow(QWidget *parent = nullptr);
    ~ControlAppWindow();

private:
    void enumerateDevices();
    std::unique_ptr<Ui::ControlAppWindow> m_ui;
};

#endif // CONTROLAPPWINDOW_H
