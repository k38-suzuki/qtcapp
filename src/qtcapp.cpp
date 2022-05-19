/*!
  @file
  @author Kenta Suzuki
*/

#include <QApplication>
#include "MainWindow.h"

using namespace qtc;

int main(int argc, char** argv)
{

    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/qtc-app/icon/qtc-app.png"));
    MainWindow w;
    for(int i = 0; i < argc; ++i) {
        w.load(QString(argv[i]).toStdString());
    }
    w.show();
    return app.exec();
}
