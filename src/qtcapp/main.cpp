/**
   \file
   \author Kenta Suzuki
*/

#include <QApplication>
#include <qtc/MainWindow>

using namespace qtc;

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
