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
    for(int i = 0; i < argc; ++i) {
        w.onImportFile(QString(argv[i]));
    }
    w.show();

    return a.exec();
}
