#include <QApplication>
#include <QScreen>
#include "mainwindow.h"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    app.setApplicationName("Al-Noor Hospital System");
    app.setOrganizationName("Al-Noor");

    MainWindow window;

    QScreen* screen = QGuiApplication::primaryScreen();
    QRect sg = screen->availableGeometry();

    window.resize(1280, 800);
    window.move((sg.width()  - window.width())  / 2,
                (sg.height() - window.height()) / 2);

    window.show();

    return app.exec();
}
