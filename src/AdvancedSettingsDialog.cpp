/*!
  @file
  @author Kenta Suzuki
*/

#include "AdvancedSettingsDialog.h"

class AdvancedSettingsDialogImpl
{
public:
    AdvancedSettingsDialogImpl(AdvancedSettingsDialog* self);

    AdvancedSettingsDialog* self;
};


AdvancedSettingsDialog::AdvancedSettingsDialog(QDialog *parent) : QDialog(parent)
{
    impl = new AdvancedSettingsDialogImpl(this);
}


AdvancedSettingsDialogImpl::AdvancedSettingsDialogImpl(AdvancedSettingsDialog* self)
    : self(self)
{
    self->setWindowTitle(QDialog::tr("Advanced Settings"));

}


AdvancedSettingsDialog::~AdvancedSettingsDialog()
{
    delete impl;
}
