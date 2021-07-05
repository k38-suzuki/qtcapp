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

    enum MenuId {
        FILE, EDIT,
        VIEW, OPTION,
        HELP, NUM_MENUS
    };

    enum ActionId {
        IMPORT, EXPORT, QUIT,
        SETTING, DEBUG, NUM_ACTIONS
    };

    enum ComboId { IFC, IFB, NUM_COMBOS };

    enum LineId { SRC, DST, NUM_LINES };

    enum SpinId {
        IN_DLY_TIM, OUT_DLY_TIM,
        IN_LOS_PCT, OUT_LOS_PCT,
        IN_RAT_RAT, OUT_RAT_RAT,
        NUM_SPINS
    };

    void onImportFile(QString filename);

private:
    MainWindowImpl* impl;
    friend class MainWindowImpl;

public Q_SLOTS:
    void onClearButtonClicked();
    void onApplyButtonToggled(const bool& on);
    void onImportActionTriggered(const bool& on);
    void onExportActionTriggered(const bool& on);
    void onDebugActionTriggered(const bool& on);
    void onCurrentIFBChanged(QString ifbName);
};

#endif // QTCAPP_MAINWINDOW_H
