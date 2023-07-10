/**
   \file
   \author Kenta Suzuki
*/

#include <QApplication>
#include "qtcapp/netem_widget.h"

using namespace rqt_netem;

int main(int argc, char** argv)
{
    QApplication app(argc, argv);

    NetEmWidget w;
    w.show();
    return app.exec();
}
