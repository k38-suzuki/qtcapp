/**
   \file
   \author Kenta Suzuki
*/

#ifndef QTCAPP_MAIN_WINDOW_H
#define QTCAPP_MAIN_WINDOW_H

#include <QMainWindow>
#include "exportdecl.h"

namespace netem {

class MainWindowImpl;

class DECLSPEC MainWindow : public QMainWindow
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

#endif // QTCAPP_MAIN_WINDOW_H
