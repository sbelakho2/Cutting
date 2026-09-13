#include <QApplication>
#include "mainwindow.hpp"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("Starz S1 Reference Prototype — NOT FOR MACHINE OPERATION");
    app.setOrganizationName("Starz Electronics");
    app.setOrganizationDomain("starz.tn");

    starz::hmi::MainWindow window;
    window.show();

    return app.exec();
}
