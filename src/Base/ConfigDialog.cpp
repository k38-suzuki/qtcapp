/*!
  @file
  @author Kenta Suzuki
*/

#include "ConfigDialog.h"
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

using namespace qtc;

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

DoubleSpinInfo configdspinInfo[] = {
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

ComboInfo configcomboInfo[] = {
    {  4, 1 }, {  4, 2 },
    {  5, 1 }, {  5, 2 },
    { 19, 1 }, { 19, 2 }
};

}


ConfigDialog::ConfigDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Advanced Settings");
    QVBoxLayout* vbox = new QVBoxLayout;
    QGridLayout* gbox = new QGridLayout;

    gbox->addWidget(new QLabel("Inbound"), 0, 1);
    gbox->addWidget(new QLabel("Outbound"), 0, 2);

    for(int i = 0; i < 19; ++i) {
        QLabel* label = new QLabel(names[i]);
        gbox->addWidget(label, i + 1, 0);
    }

    for(int i = 0; i < NUM_DSPINS; ++i) {
        dspins_[i] = new QDoubleSpinBox;
        QDoubleSpinBox* dspin = dspins_[i];
        DoubleSpinInfo info = configdspinInfo[i];
        dspin->setRange(info.lower, info.upper);
        dspin->setValue(info.value);
        gbox->addWidget(dspin, info.row, info.cln);
    }

    QStringList distributions = { "disabled", "uniform", "normal", "pareto", "paretonormal" };
    QStringList states = { "disabled", "enabled" };
    for(int i = 0; i < NUM_COMBOS; ++i) {
        combos_[i] = new QComboBox;
        QComboBox* combo = combos_[i];
        if(i == 2 || i == 3) {
            combo->addItems(states);
        } else {
            combo->addItems(distributions);
        }
        combo->setCurrentIndex(0);
        ComboInfo info = configcomboInfo[i];
        gbox->addWidget(combo, info.row, info.cln);
    }

    QPushButton* resetButton = new QPushButton("&Reset");
    QPushButton* okButton = new QPushButton("&Ok");
    okButton->setDefault(true);
    QDialogButtonBox* buttonBox = new QDialogButtonBox(this);
    buttonBox->addButton(resetButton, QDialogButtonBox::ResetRole);
    buttonBox->addButton(okButton, QDialogButtonBox::AcceptRole);
    connect(resetButton,SIGNAL(clicked(bool)), this, SLOT(onResetButtonClicked()));
    connect(buttonBox,SIGNAL(accepted()), this, SLOT(accept()));

    vbox->addLayout(gbox);
    vbox->addWidget(buttonBox);
    setLayout(vbox);
}


void ConfigDialog::setValue(const int& index, const double& value)
{
    dspins_[index]->setValue(value);
}


double ConfigDialog::spin(const int& index) const
{
    return dspins_[index]->value();
}


void ConfigDialog::setText(const int &index, const QString text)
{
    combos_[index]->setCurrentText(text);
}


QString ConfigDialog::combo(const int& index) const
{
    return combos_[index]->currentText();
}


void ConfigDialog::onResetButtonClicked()
{
    for(int i = 0; i < NUM_DSPINS; ++i) {
        QDoubleSpinBox* spin = dspins_[i];
        DoubleSpinInfo info = configdspinInfo[i];
        spin->setRange(info.lower, info.upper);
        spin->setValue(info.value);
    }

    for(int i = 0; i < NUM_COMBOS; ++i) {
        QComboBox* combo = combos_[i];
        combo->setCurrentIndex(0);
    }
}
