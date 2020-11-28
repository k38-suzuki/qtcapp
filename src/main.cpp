/*!
  @file
  @author Kenta Suzuki
*/

#include "MainWindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    a.setWindowIcon(QIcon(":/qtc-app/icon/qtc-app.png"));
    MainWindow w;
    w.show();

    return a.exec();
}
