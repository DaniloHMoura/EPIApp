#include <QApplication>
#include "EPIApp.h"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    EPIApp window;
    window.show();
    return app.exec();
}
