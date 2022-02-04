/*!
  @file
  @author Kenta Suzuki
*/

#include "App.h"
#include <QApplication>
#include "MainWindow.h"

using namespace qtc;

namespace qtc {

class AppImpl
{
public:
    AppImpl(int argc, char** argv, App* self);
    App* self;

    QApplication* qapplication;
    MainWindow* w;
};

}


App::App(int argc, char** argv)
{
    impl = new AppImpl(argc, argv, this);
}


AppImpl::AppImpl(int argc, char** argv, App* self)
    : self(self)
{
    qapplication = new QApplication(argc, argv);
    qapplication->setWindowIcon(QIcon(":/qtc-app/icon/qtc-app.png"));
    w = new MainWindow;
    for(int i = 0; i < argc; ++i) {
        w->onImportFile(QString(argv[i]));
    }
    w->show();
}


App::~App()
{
    delete impl;
}


int App::exec()
{
    return impl->qapplication->exec();
}
