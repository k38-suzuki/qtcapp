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
        IN_LMT_PKT, OUT_LMT_PKT,
        IN_DLY_JTR, OUT_DLY_JTR,
        IN_DLY_COR, OUT_DLY_COR,
        IN_LOS_COR, OUT_LOS_COR,
        IN_DPL_PCT, OUT_DPL_PCT,
        IN_DPL_COR, OUT_DPL_COR,
        IN_CRP_PCT, OUT_CRP_PCT,
        IN_CRP_COR, OUT_CRP_COR,
        IN_ROR_PCT, OUT_ROR_PCT,
        IN_ROR_COR, OUT_ROR_COR,
        IN_ROR_DST, OUT_ROR_DST,
        IN_RPK_OVH, OUT_RPK_OVH,
        IN_RCL_SIZ, OUT_RCL_SIZ,
        IN_RCL_OVH, OUT_RCL_OVH,
        IN_SLT_MND, OUT_SLT_MND,
        IN_SLT_MXD, OUT_SLT_MXD,
        NUM_DSPINS
    };

    enum ComboId {
        IN_DLY_DST, OUT_DLY_DST,
        IN_LOS_RDM, OUT_LOS_RDM,
        IN_SLT_DST, OUT_SLT_DST,
        NUM_COMBOS
    };

    void setValue(const int& index, const double& value);
    double spin(const int& index) const;
    void setText(const int& index, const QString text);
    QString combo(const int& index) const;

private:
    ConfigDialogImpl* impl;
    friend class ConfigDialogImpl;

public Q_SLOTS:
    void onResetButtonClicked();
};

#endif // QTCAPP_CONFIGDIALOG_H
