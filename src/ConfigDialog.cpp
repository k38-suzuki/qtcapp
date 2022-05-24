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

using namespace qtc;
using namespace std;

namespace {

const QStringList names = {
          "Limit Packets [packets]",           "Delay Jitter [ms]",
            "Delay Correlation [%]",      "Delay Distribution [-]",
                  "Loss Random [-]",        "Loss Correlation [%]",
          "Duplication Percent [%]", "Duplication Correlation [%]",
           "Corruption Percent [%]",  "Corruption Correlation [%]",
           "Reordering Percent [%]",  "Reordering Correlation [%]",
    "Reordering Distance [packets]", "Rate Packet Overhead [byte]",
            "Rate Cell Size [byte]",   "Rate Cell Overhead [byte]",
              "Slot Min Delay [ms]",         "Slot Max Delay [ms]",
            "Slot Distribution [-]"
};

struct DoubleSpinInfo {
    int row;
    int cln;
    double lower;
    double upper;
    double value;
};

DoubleSpinInfo dspinInfo[] = {
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
};

ComboInfo comboInfo[] = {
    {  4, 1 }, {  4, 2 },
    {  5, 1 }, {  5, 2 },
    { 19, 1 }, { 19, 2 }
};

}

namespace qtc {

class ConfigDialogImpl
{
public:
    ConfigDialogImpl(ConfigDialog* self);
    ConfigDialog* self;

    QDoubleSpinBox* dspins[ConfigDialog::NUM_DSPINS];
    QComboBox* combos[ConfigDialog::NUM_COMBOS];

    void onResetButtonClicked();
};

}


ConfigDialog::ConfigDialog(QWidget* parent)
    : QDialog(parent)
{
    impl = new ConfigDialogImpl(this);
}


ConfigDialogImpl::ConfigDialogImpl(ConfigDialog* self)
    : self(self)
{
    self->setWindowTitle("Advanced Settings");
    QVBoxLayout* vbox = new QVBoxLayout;
    QGridLayout* gbox = new QGridLayout;

    gbox->addWidget(new QLabel("Inbound"), 0, 1);
    gbox->addWidget(new QLabel("Outbound"), 0, 2);

    for(int i = 0; i < 19; ++i) {
        QLabel* label = new QLabel(names[i]);
        gbox->addWidget(label, i + 1, 0);
    }

    for(int i = 0; i < ConfigDialog::NUM_DSPINS; ++i) {
        dspins[i] = new QDoubleSpinBox;
        QDoubleSpinBox* dspin = dspins[i];
        DoubleSpinInfo info = dspinInfo[i];
        dspin->setRange(info.lower, info.upper);
        dspin->setValue(info.value);
        gbox->addWidget(dspin, info.row, info.cln);
    }

    QStringList distributions = { "disabled", "uniform", "normal", "pareto", "paretonormal" };
    QStringList states = { "disabled", "enabled" };
    for(int i = 0; i < ConfigDialog::NUM_COMBOS; ++i) {
        combos[i] = new QComboBox;
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
    self->connect(resetButton, &QPushButton::clicked, [&](){ onResetButtonClicked(); });
    self->connect(buttonBox, SIGNAL(accepted()), self, SLOT(accept()));

    vbox->addLayout(gbox);
    vbox->addWidget(buttonBox);
    self->setLayout(vbox);
}


ConfigDialog::~ConfigDialog()
{
    delete impl;
}


void ConfigDialog::setValue(const int& index, const double& value)
{
    impl->dspins[index]->setValue(value);
}


double ConfigDialog::spin(const int& index) const
{
    return impl->dspins[index]->value();
}


void ConfigDialog::setText(const int &index, const string& text)
{
    impl->combos[index]->setCurrentText(text.c_str());
}


string ConfigDialog::combo(const int& index) const
{
    return impl->combos[index]->currentText().toStdString();
}


void ConfigDialogImpl::onResetButtonClicked()
{
    for(int i = 0; i < ConfigDialog::NUM_DSPINS; ++i) {
        QDoubleSpinBox* dspin = dspins[i];
        DoubleSpinInfo info = dspinInfo[i];
        dspin->setRange(info.lower, info.upper);
        dspin->setValue(info.value);
    }

    for(int i = 0; i < ConfigDialog::NUM_COMBOS; ++i) {
        combos[i]->setCurrentIndex(0);
    }
}
