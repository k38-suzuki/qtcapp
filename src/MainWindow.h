/*!
  @file
  @author Kenta Suzuki
*/

#ifndef QTC_APP_MAIN_WINDOW_H
#define QTC_APP_MAIN_WINDOW_H

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
    void onApplyButtonToggled(bool on);
    void onShowActionTriggered(bool on);
    void onCurrentIFBChanged(QString ifbName);
};

#endif // QTC_APP_MAIN_WINDOW_H
