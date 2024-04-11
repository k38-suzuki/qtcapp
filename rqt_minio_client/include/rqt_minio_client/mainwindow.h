/**
   @author Kenta Suzuki
*/

#ifndef rqt_minio_client__mainwindow_H
#define rqt_minio_client__mainwindow_H

#include <QMainWindow>

namespace rqt_minio_client {

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

protected:
    virtual void closeEvent(QCloseEvent* event) override;

private:
    class Impl;
    Impl* impl;
};

}

#endif // rqt_minio_client__mainwindow_H
