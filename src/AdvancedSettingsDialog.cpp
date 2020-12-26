/*!
  @file
  @author Kenta Suzuki
*/

#include "AdvancedSettingsDialog.h"

#include <QButtonGroup>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QRadioButton>
#include <QSpinBox>
#include <QPushButton>

class AdvancedSettingsDialogImpl
{
public:
    AdvancedSettingsDialogImpl(AdvancedSettingsDialog* self);

    AdvancedSettingsDialog* self;

    QSpinBox* iduplication;
    QSpinBox* icorruption;
    QSpinBox* igap;
    QSpinBox* ireordering0;
    QSpinBox* ireordering1;
    QSpinBox* irdelay;
    QRadioButton ireorderButton;
    QRadioButton igapButton;
    QSpinBox* oduplication;
    QSpinBox* ocorruption;
    QSpinBox* ogap;
    QSpinBox* oreordering0;
    QSpinBox* oreordering1;
    QSpinBox* ordelay;
    QRadioButton oreorderButton;
    QRadioButton ogapButton;
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

    iduplication = new QSpinBox();
    iduplication->setRange(0, 100);
    icorruption = new QSpinBox();
    icorruption->setRange(0, 100);
    igap = new QSpinBox();
    igap->setRange(0, 100);
    ireordering0 = new QSpinBox();
    ireordering0->setRange(0, 100);
    ireordering1 = new QSpinBox();
    ireordering1->setRange(0, 100);
    irdelay = new QSpinBox();
    irdelay->setRange(0, 100000);
    QButtonGroup* igroup = new QButtonGroup();
    igroup->addButton(&ireorderButton);
    igroup->addButton(&igapButton);
    ireorderButton.setChecked(true);
    igap->setEnabled(false);
    irdelay->setEnabled(false);

    oduplication = new QSpinBox();
    oduplication->setRange(0, 100);
    ocorruption = new QSpinBox();
    ocorruption->setRange(0, 100);
    ogap = new QSpinBox();
    ogap->setRange(0, 100);
    oreordering0 = new QSpinBox();
    oreordering0->setRange(0, 100);
    oreordering1 = new QSpinBox();
    oreordering1->setRange(0, 100);
    ordelay = new QSpinBox();
    ordelay->setRange(0, 100000);
    QButtonGroup* ogroup = new QButtonGroup();
    ogroup->addButton(&oreorderButton);
    ogroup->addButton(&ogapButton);
    oreorderButton.setChecked(true);
    ogap->setEnabled(false);
    ordelay->setEnabled(false);

    QGridLayout* pbox = new QGridLayout();
    pbox->addWidget(new QLabel(QLabel::tr("Inbound Duplication [%]")), 0, 0);
    pbox->addWidget(iduplication, 0, 3);
    pbox->addWidget(new QLabel(QLabel::tr("Inbound Corruption [%]")), 1, 0);
    pbox->addWidget(icorruption, 1, 3);
//    pbox->addWidget(new QLabel(QLabel::tr("Inbound Reordering")), 2, 0);
//    pbox->addWidget(&ireorderButton, 2, 1);
//    pbox->addWidget(new QLabel(QLabel::tr("[%] / [%]")), 2, 2);
//    pbox->addWidget(ireordering0, 2, 3);
//    pbox->addWidget(new QLabel(QLabel::tr("/")), 2, 4);
//    pbox->addWidget(ireordering1, 2, 5);
//    pbox->addWidget(&igapButton, 3, 1);
//    pbox->addWidget(new QLabel(QLabel::tr("gap [packets] / delay [ms]")), 3, 2);
//    pbox->addWidget(igap, 3, 3);
//    pbox->addWidget(new QLabel(QLabel::tr("/")), 3, 4);
//    pbox->addWidget(irdelay, 3, 5);

    pbox->addWidget(new QLabel(QLabel::tr("Outbound Duplication [%]")), 4, 0);
    pbox->addWidget(oduplication, 4, 3);
    pbox->addWidget(new QLabel(QLabel::tr("Outbound Corruption [%]")), 5, 0);
    pbox->addWidget(ocorruption, 5, 3);
//    pbox->addWidget(new QLabel(QLabel::tr("Outbound Reordering")), 6, 0);
//    pbox->addWidget(&oreorderButton, 6, 1);
//    pbox->addWidget(new QLabel(QLabel::tr("[%] / [%]")), 6, 2);
//    pbox->addWidget(oreordering0, 6, 3);
//    pbox->addWidget(new QLabel(QLabel::tr("/")), 6, 4);
//    pbox->addWidget(oreordering1, 6, 5);
//    pbox->addWidget(&ogapButton, 7, 1);
//    pbox->addWidget(new QLabel(QLabel::tr("gap [packets] / delay [ms]")), 7, 2);
//    pbox->addWidget(ogap, 7, 3);
//    pbox->addWidget(new QLabel(QLabel::tr("/")), 7, 4);
//    pbox->addWidget(ordelay, 7, 5);

    vbox->addLayout(pbox);

    QPushButton* okButton = new QPushButton(QPushButton::tr("&Ok"));
    okButton->setDefault(true);
    QDialogButtonBox* buttonBox = new QDialogButtonBox(self);
    buttonBox->addButton(okButton, QDialogButtonBox::AcceptRole);
    QObject::connect(buttonBox,SIGNAL(accepted()), self, SLOT(accept()));
    vbox->addWidget(buttonBox);

    QObject::connect(&ireorderButton, SIGNAL(toggled(bool)), self, SLOT(onInboundReordering(bool)));
    QObject::connect(&oreorderButton, SIGNAL(toggled(bool)), self, SLOT(onOutboundReordering(bool)));

    self->setLayout(vbox);
}


AdvancedSettingsDialog::~AdvancedSettingsDialog()
{
    delete impl;
}


int AdvancedSettingsDialog::inboundDuplication()
{
    return impl->iduplication->value();
}


int AdvancedSettingsDialog::inboundCorruption()
{
    return impl->icorruption->value();
}


int AdvancedSettingsDialog::outboundDuplication()
{
    return impl->oduplication->value();
}


int AdvancedSettingsDialog::outboundCorruption()
{
    return impl->ocorruption->value();
}


void AdvancedSettingsDialog::onInboundReordering(bool on)
{
    impl->ireordering0->setEnabled(on);
    impl->ireordering1->setEnabled(on);
    impl->igap->setEnabled(!on);
    impl->irdelay->setEnabled(!on);
}


void AdvancedSettingsDialog::onOutboundReordering(bool on)
{
    impl->oreordering0->setEnabled(on);
    impl->oreordering1->setEnabled(on);
    impl->ogap->setEnabled(!on);
    impl->ordelay->setEnabled(!on);
}


void AdvancedSettingsDialog::clear()
{
    impl->iduplication->setValue(0);
    impl->icorruption->setValue(0);
    impl->ireordering0->setValue(0);
    impl->ireordering1->setValue(0);
    impl->igap->setValue(0);
    impl->irdelay->setValue(0);
    impl->oduplication->setValue(0);
    impl->ocorruption->setValue(0);
    impl->oreordering0->setValue(0);
    impl->oreordering1->setValue(0);
    impl->ogap->setValue(0);
    impl->ordelay->setValue(0);
}
