/**
   \file
   \author Kenta Suzuki
*/

#include <QApplication>
#include "qtcapp/NetEmWidget.h"

using namespace netem;

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    NetEmWidget w;
    w.show();
    return app.exec();
}