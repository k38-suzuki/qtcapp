/**
   \file
   \author Kenta Suzuki
*/

#ifndef QTC_MAIN_WINDOW_H
#define QTC_MAIN_WINDOW_H

#include <QMainWindow>

namespace qtc {

class MainWindowImpl;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget* parent = 0);
    virtual ~MainWindow();

    bool load(const std::string& filename);

private:
    MainWindowImpl* impl;
    friend class MainWindowImpl;
};

}

#endif // QTC_MAIN_WINDOW_H
