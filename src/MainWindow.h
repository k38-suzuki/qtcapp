/*!
  @file
  @author Kenta Suzuki
*/

#ifndef QTCAPP_MAINWINDOW_H
#define QTCAPP_MAINWINDOW_H

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
    void onApplyButtonToggled(const bool& on);
    void onShowActionTriggered(const bool& on);
    void onSettingActionTriggered(const bool& on);
    void onDebugActionTriggered(const bool& on);
    void onCurrentIFBChanged(QString ifbName);
};

#endif // QTCAPP_MAINWINDOW_H
