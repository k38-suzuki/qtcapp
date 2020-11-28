/*!
  @file
  @author Kenta Suzuki
*/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class MainWindowImpl;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    MainWindowImpl* impl;
    friend class MainWindowImpl;

public Q_SLOTS:
    void onClearButtonClicked();
    void onApplyButtonClicked(bool on);
};

#endif // MAINWINDOW_H
