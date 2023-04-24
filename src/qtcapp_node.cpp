/**
   \file
   \author Kenta Suzuki
*/

#include <QApplication>
#include "qtcapp/qtcapp.h"

using namespace qtcapp;

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    MainWindow w;
    for(int i = 0; i < argc; ++i) {
        w.load(QString(argv[i]).toStdString());
    }
    w.show();
    return app.exec();
}
