/*!
  @file
  @author Kenta Suzuki
*/

#include "ConfigDialog.h"
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace {

const QStringList names = {
    "Limit Packets [packets]",       "Delay Jitter [ms]",
    "Delay Correlation [%]",         "Delay Distribution [-]",
    "Loss Random [-]",               "Loss Correlation [%]",
    "Duplication Percent [%]",       "Duplication Correlation [%]",
    "Corruption Percent [%]",        "Corruption Correlation [%]",
    "Reordering Percent [%]",        "Reordering Correlation [%]",
    "Reordering Distance [packets]", "Rate Packet Overhead [byte]",
    "Rate Cell Size [byte]",         "Rate Cell Overhead [byte]",
    "Slot Min Delay [ms]",           "Slot Max Delay [ms]",
    "Slot Distribution [-]"
};


struct SpinInfo {
    int row;
    int cln;
    double lower;
    double upper;
    double value;
};


SpinInfo spinInfo[] = {
    {  1, 1, 0.0,  10000.0, 2000.0 }, {  1, 2, 0.0,  10000.0, 2000.0 },
    {  2, 1, 0.0, 100000.0,    0.0 }, {  2, 2, 0.0, 100000.0,    0.0 },
    {  3, 1, 0.0,    100.0,    0.0 }, {  3, 2, 0.0,    100.0,    0.0 },
    {  6, 1, 0.0,    100.0,    0.0 }, {  6, 2, 0.0,    100.0,    0.0 },
    {  7, 1, 0.0,    100.0,    0.0 }, {  7, 2, 0.0,    100.0,    0.0 },
    {  8, 1, 0.0,    100.0,    0.0 }, {  8, 2, 0.0,    100.0,    0.0 },
    {  9, 1, 0.0,    100.0,    0.0 }, {  9, 2, 0.0,    100.0,    0.0 },
    { 10, 1, 0.0,    100.0,    0.0 }, { 10, 2, 0.0,    100.0,    0.0 },
    { 11, 1, 0.0,    100.0,    0.0 }, { 11, 2, 0.0,    100.0,    0.0 },
    { 12, 1, 0.0,    100.0,    0.0 }, { 12, 2, 0.0,    100.0,    0.0 },
    { 13, 1, 0.0,    100.0,    0.0 }, { 13, 2, 0.0,    100.0,    0.0 },
    { 14, 1, 0.0,    100.0,    0.0 }, { 14, 2, 0.0,    100.0,    0.0 },
    { 15, 1, 0.0,    100.0,    0.0 }, { 15, 2, 0.0,    100.0,    0.0 },
    { 16, 1, 0.0,    100.0,    0.0 }, { 16, 2, 0.0,    100.0,    0.0 },
    { 17, 1, 0.0, 100000.0,    0.0 }, { 17, 2, 0.0, 100000.0,    0.0 },
    { 18, 1, 0.0, 100000.0,    0.0 }, { 18, 2, 0.0, 100000.0,    0.0 },
};


struct ComboInfo {
    int row;
    int cln;
    bool enabled;
};


ComboInfo comboInfo[] = {
    {  4, 1, false }, {  4, 2, false },
    {  5, 1, false }, {  5, 2, false },
    { 19, 1, false }, { 19, 2, false }
};

}

class ConfigDialogImpl
{
public:
    ConfigDialogImpl(ConfigDialog* self);
    ConfigDialog* self;

    QDoubleSpinBox* spins[ConfigDialog::NUM_SPINS];
    QComboBox* combos[ConfigDialog::NUM_COMBOS];
};


ConfigDialog::ConfigDialog(QWidget* parent)
{
    impl = new ConfigDialogImpl(this);
}


ConfigDialogImpl::ConfigDialogImpl(ConfigDialog *self)
    : self(self)
{
    self->setWindowTitle("Advanced Settings");
    QVBoxLayout* vbox = new QVBoxLayout();
    QGridLayout* gbox = new QGridLayout();

    gbox->addWidget(new QLabel("Inbound"), 0, 1);
    gbox->addWidget(new QLabel("Outbound"), 0, 2);

    for(int i = 0; i < 19; ++i) {
        QLabel* label = new QLabel(names[i]);
        gbox->addWidget(label, i + 1, 0);
    }

    for(int i = 0; i < ConfigDialog::NUM_SPINS; ++i) {
        spins[i] = new QDoubleSpinBox();
        QDoubleSpinBox* spin = spins[i];
        SpinInfo info = spinInfo[i];
        spin->setRange(info.lower, info.upper);
        spin->setValue(info.value);
        gbox->addWidget(spin, info.row, info.cln);
    }

    QStringList distributions = { "disabled", "uniform", "normal", "pareto", "paretonormal" };
    QStringList states = { "disabled", "enabled" };
    for(int i = 0; i < ConfigDialog::NUM_COMBOS; ++i) {
        combos[i] = new QComboBox();
        QComboBox* combo = combos[i];
        if(i == 2 || i == 3) {
            combo->addItems(states);
        } else {
            combo->addItems(distributions);
        }
        combo->setCurrentIndex(0);
        ComboInfo info = comboInfo[i];
        gbox->addWidget(combo, info.row, info.cln);
    }

    QPushButton* resetButton = new QPushButton("&Reset");
    QPushButton* okButton = new QPushButton("&Ok");
    okButton->setDefault(true);
    QDialogButtonBox* buttonBox = new QDialogButtonBox(self);
    buttonBox->addButton(resetButton, QDialogButtonBox::ResetRole);
    buttonBox->addButton(okButton, QDialogButtonBox::AcceptRole);
    self->connect(resetButton,SIGNAL(clicked(bool)), self, SLOT(onResetButtonClicked()));
    self->connect(buttonBox,SIGNAL(accepted()), self, SLOT(accept()));

    vbox->addLayout(gbox);
    vbox->addWidget(buttonBox);
    self->setLayout(vbox);
}


ConfigDialog::~ConfigDialog()
{
    delete impl;
}


double ConfigDialog::spin(const int& index) const
{
    return impl->spins[index]->value();
}


QString ConfigDialog::combo(const int& index) const
{
    return impl->combos[index]->currentText();
}


void ConfigDialog::onResetButtonClicked()
{
    for(int i = 0; i < ConfigDialog::NUM_SPINS; ++i) {
        QDoubleSpinBox* spin = impl->spins[i];
        SpinInfo info = spinInfo[i];
        spin->setRange(info.lower, info.upper);
        spin->setValue(info.value);
    }

    for(int i = 0; i < ConfigDialog::NUM_COMBOS; ++i) {
        QComboBox* combo = impl->combos[i];
        combo->setCurrentIndex(0);
    }
}
