/*!
  @file
  @author Kenta Suzuki
*/

#ifndef QTC_APP_ADVANCED_SETTINGS_Dialog_H
#define QTC_APP_ADVANCED_SETTINGS_Dialog_H

#include <QDialog>

class AdvancedSettingsDialogImpl;

class AdvancedSettingsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit AdvancedSettingsDialog(QDialog *parent = nullptr);
    virtual ~AdvancedSettingsDialog();

    int inboundDuplication();
    int inboundCorruption();
    int outboundDuplication();
    int outboundCorruption();
    void clear();

private:
    AdvancedSettingsDialogImpl* impl;
    friend class AdvancedSettingsDialogImpl;

signals:

public Q_SLOTS:
    void onInboundReordering(bool on);
    void onOutboundReordering(bool on);
};

#endif // QTC_APP_ADVANCED_SETTINGS_Dialog_H
