#include <QApplication>
#include <QDebug>

#include "controlappwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    ControlAppWindow window;
    window.show();
    return app.exec();
}
