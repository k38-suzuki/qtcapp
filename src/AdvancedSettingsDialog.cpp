/*!
  @file
  @author Kenta Suzuki
*/

#include "AdvancedSettingsDialog.h"

#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QSpinBox>
#include <QPushButton>

class AdvancedSettingsDialogImpl
{
public:
    AdvancedSettingsDialogImpl(AdvancedSettingsDialog* self);

    AdvancedSettingsDialog* self;

    QDoubleSpinBox* iduplication;
    QDoubleSpinBox* icorruption;
    QSpinBox* igap;
    QDoubleSpinBox* ireordering0;
    QDoubleSpinBox* ireordering1;
    QDoubleSpinBox* oduplication;
    QDoubleSpinBox* ocorruption;
    QSpinBox* ogap;
    QDoubleSpinBox* oreordering0;
    QDoubleSpinBox* oreordering1;
};


AdvancedSettingsDialog::AdvancedSettingsDialog(QDialog *parent) : QDialog(parent)
{
    impl = new AdvancedSettingsDialogImpl(this);
}


AdvancedSettingsDialogImpl::AdvancedSettingsDialogImpl(AdvancedSettingsDialog* self)
    : self(self)
{
    self->setWindowTitle(QDialog::tr("Advanced Settings"));
    QVBoxLayout* vbox = new QVBoxLayout();

    iduplication = new QDoubleSpinBox();
    iduplication->setRange(0, 100.00);
    icorruption = new QDoubleSpinBox();
    icorruption->setRange(0, 100.0);
    igap = new QSpinBox();
    igap->setRange(0, 100);
    ireordering0 = new QDoubleSpinBox();
    ireordering0->setRange(0, 100.0);
    ireordering1 = new QDoubleSpinBox();
    ireordering1->setRange(0, 100.0);

    oduplication = new QDoubleSpinBox();
    oduplication->setRange(0, 100.0);
    ocorruption = new QDoubleSpinBox();
    ocorruption->setRange(0, 100.0);
    ogap = new QSpinBox();
    ogap->setRange(0, 100);
    oreordering0 = new QDoubleSpinBox();
    oreordering0->setRange(0, 100.0);
    oreordering1 = new QDoubleSpinBox();
    oreordering1->setRange(0, 100.0);

    QGridLayout* pbox = new QGridLayout();
    pbox->addWidget(new QLabel(QLabel::tr("Inbound Duplication [%]")), 0, 0);
    pbox->addWidget(iduplication, 0, 1);
    pbox->addWidget(new QLabel(QLabel::tr("Inbound Corruption [%]")), 1, 0);
    pbox->addWidget(icorruption, 1, 1);
    pbox->addWidget(new QLabel(QLabel::tr("Inbound Re-ordering [%] / Correlation [%] / Gap [distance]")), 2, 0);
    pbox->addWidget(ireordering0, 2, 1);
    pbox->addWidget(new QLabel(QLabel::tr("/")), 2, 2);
    pbox->addWidget(ireordering1, 2, 3);
    pbox->addWidget(new QLabel(QLabel::tr("/")), 2, 4);
    pbox->addWidget(igap, 2, 5);

    pbox->addWidget(new QLabel(QLabel::tr("Outbound Duplication [%]")), 4, 0);
    pbox->addWidget(oduplication, 4, 1);
    pbox->addWidget(new QLabel(QLabel::tr("Outbound Corruption [%]")), 5, 0);
    pbox->addWidget(ocorruption, 5, 1);
    pbox->addWidget(new QLabel(QLabel::tr("Outbound Re-ordering [%] / Correlation [%] / Gap [distance]")), 6, 0);
    pbox->addWidget(oreordering0, 6, 1);
    pbox->addWidget(new QLabel(QLabel::tr("/")), 6, 2);
    pbox->addWidget(oreordering1, 6, 3);
    pbox->addWidget(new QLabel(QLabel::tr("/")), 6, 4);
    pbox->addWidget(ogap, 6, 5);
    vbox->addLayout(pbox);

    QPushButton* okButton = new QPushButton(QPushButton::tr("&Ok"));
    okButton->setDefault(true);
    QDialogButtonBox* buttonBox = new QDialogButtonBox(self);
    buttonBox->addButton(okButton, QDialogButtonBox::AcceptRole);
    QObject::connect(buttonBox,SIGNAL(accepted()), self, SLOT(accept()));
    vbox->addWidget(buttonBox);

    self->setLayout(vbox);
}


AdvancedSettingsDialog::~AdvancedSettingsDialog()
{
    delete impl;
}


double AdvancedSettingsDialog::inboundDuplication()
{
    return impl->iduplication->value();
}


double AdvancedSettingsDialog::inboundCorruption()
{
    return impl->icorruption->value();
}


double AdvancedSettingsDialog::inboundReordering(int index)
{
    if(index == 0) {
        return impl->ireordering0->value();
    } else if(index == 1) {
        return impl->ireordering1->value();
    } else if(index == 2) {
        return impl->igap->value();
    }
}


double AdvancedSettingsDialog::outboundDuplication()
{
    return impl->oduplication->value();
}


double AdvancedSettingsDialog::outboundCorruption()
{
    return impl->ocorruption->value();
}


double AdvancedSettingsDialog::outboundReordering(int index)
{
    if(index == 0) {
        return impl->oreordering0->value();
    } else if(index == 1) {
        return impl->oreordering1->value();
    } else if(index == 2) {
        return impl->ogap->value();
    }
}


void AdvancedSettingsDialog::clear()
{
    impl->iduplication->setValue(0.0);
    impl->icorruption->setValue(0.0);
    impl->ireordering0->setValue(0.0);
    impl->ireordering1->setValue(0.0);
    impl->igap->setValue(0);
    impl->oduplication->setValue(0.0);
    impl->ocorruption->setValue(0.0);
    impl->oreordering0->setValue(0.0);
    impl->oreordering1->setValue(0.0);
    impl->ogap->setValue(0);
}
