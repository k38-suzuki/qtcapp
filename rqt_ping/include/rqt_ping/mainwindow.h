/**
   @author Kenta Suzuki
*/

#ifndef rqt_ping__mainwindow_H
#define rqt_ping__mainwindow_H

#include <QMainWindow>

namespace rqt_ping {

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

#endif // rqt_ping__mainwindow_H
