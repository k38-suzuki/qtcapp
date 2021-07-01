/*!
  @file
  @author Kenta Suzuki
*/

#ifndef QTCAPP_CONFIGDIALOG_H
#define QTCAPP_CONFIGDIALOG_H

#include <QDialog>

class ConfigDialogImpl;

class ConfigDialog : public QDialog
{
    Q_OBJECT

public:
    ConfigDialog(QWidget* parent = 0);
    virtual ~ConfigDialog();

    enum SpinId {
        IN_LMT_PKTSP, OUT_LMT_PKTSP,
        IN_DLY_JTRSP, OUT_DLY_JTRSP,
        IN_DLY_CORSP, OUT_DLY_CORSP,
        IN_LOS_CORSP, OUT_LOS_CORSP,
        IN_DPL_PCTSP, OUT_DPL_PCTSP,
        IN_DPL_CORSP, OUT_DPL_CORSP,
        IN_CRP_PCTSP, OUT_CRP_PCTSP,
        IN_CRP_CORSP, OUT_CRP_CORSP,
        IN_ROR_PCTSP, OUT_ROR_PCTSP,
        IN_ROR_CORSP, OUT_ROR_CORSP,
        IN_ROR_DSTSP, OUT_ROR_DSTSP,
        IN_RPK_OVHSP, OUT_RPK_OVHSP,
        IN_RCL_SIZSP, OUT_RCL_SIZSP,
        IN_RCL_OVHSP, OUT_RCL_OVHSP,
        IN_SLT_MNDSP, OUT_SLT_MNDSP,
        IN_SLT_MXDSP, OUT_SLT_MXDSP,
        NUM_SPINS
    };

    enum CheckId {
        IN_DLY_DSTCK, OUT_DLY_DSTCK,
        IN_LOS_RDMCK, OUT_LOS_RDMCK,
        IN_SLT_DSTCK, OUT_SLT_DSTCK,
        NUM_CHECKS
    };

    enum ComboId {
        IN_DLY_DSTCB, OUT_DLY_DSTCB,
        IN_SLT_DSTCB, OUT_SLT_DSTCB,
        NUM_COMBOS
    };

private:
    ConfigDialogImpl* impl;
    friend class ConfigDialogImpl;

public Q_SLOTS:
    void onResetButtonClicked();
};

#endif // QTCAPP_CONFIGDIALOG_H
