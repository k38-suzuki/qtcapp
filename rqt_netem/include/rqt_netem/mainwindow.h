/**
   @author Kenta Suzuki
*/

#ifndef rqt_netem__mainwindow_H
#define rqt_netem__mainwindow_H

#include <QMainWindow>

namespace rqt_netem {

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private:
    class Impl;
    Impl* impl;
};

}

#endif // rqt_netem__mainwindow_H
