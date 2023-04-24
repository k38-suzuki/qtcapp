/**
   \file
   \author Kenta Suzuki
*/

#include "qtcapp/qtcapp.h"
#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QPalette>
#include <QProcess>
#include <QPushButton>
#include <QTextStream>
#include <QVBoxLayout>
#include <boost/format.hpp>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

using namespace qtcapp;
using namespace std;

namespace {

enum MenuID {
    FILE, EDIT,
    VIEW, OPTION,
    HELP, NUM_MENUS
};

vector<string> split(const string& s, char delim)
{
    vector<string> elements;
    stringstream ss(s);
    string item;
    while(getline(ss, item, delim)) {
    if(!item.empty()) {
            elements.push_back(item);
        }
    }
    return elements;
}

bool checkIP(const string& address)
{
    bool result = false;
    vector<string> ip_mask = split(address, '/');
    if(ip_mask.size() == 2) {
        int mask = atoi(ip_mask[1].c_str());
        if((mask >= 0) && (mask <= 32)) {
            string ipaddress = ip_mask[0];
            vector<string> ipelements = split(ipaddress, '.');
            if(ipelements.size() == 4) {
                for(int i = 0; i < 4; i++) {
                    int element = atoi(ipelements[i].c_str());
                    if((element >= 0) && (element <= 255)) {
                        result = true;
                    }
                }
            }
        }
    }
    return result;
}

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

struct ActionInfo {
    const char* label;
    bool checkable;
    bool checked;
    bool enabled;
    int menu;
};

ActionInfo actionInfo[] = {
    { "Import",            false, false,  true,   FILE },
    { "Export",            false, false,  true,   FILE },
    { "Quit",              false, false,  true,   FILE },
    { "Advanced settings", false, false, false, OPTION },
    { "Debug mode",         true, false,  true, OPTION }
};

struct ComboInfo {
    const char* label;
    int row;
    int cln;
};

ComboInfo comboInfo[] = {
    { "Interface", 0, 1 },
    { "IFB",       0, 3 }
};

ComboInfo comboInfo2[] = {
    { "",  4, 1 }, { "",  4, 2 },
    { "",  5, 1 }, { "",  5, 2 },
    { "", 19, 1 }, { "", 19, 2 }
};

struct LineInfo {
    const char* label;
    int row;
    int cln;
};

LineInfo lineInfo[] = {
    { "Source IP",      1, 1 },
    { "Destination IP", 1, 3 }
};

struct DoubleSpinInfo {
    int row;
    int cln;
    double lower;
    double upper;
    double value;
};

DoubleSpinInfo dspinInfo[] = {
    { 1, 1, 0.0,   100000.0, 0.0 }, { 1, 2, 0.0,   100000.0, 0.0 },
    { 2, 1, 0.0,      100.0, 0.0 }, { 2, 2, 0.0,      100.0, 0.0 },
    { 3, 1, 0.0, 11000000.0, 0.0 }, { 3, 2, 0.0, 11000000.0, 0.0 },
};

DoubleSpinInfo dspinInfo2[] = {
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

}

namespace qtcapp {

class ConfigDialog : public QDialog
{
public:
    ConfigDialog();

    enum SpinID {
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

    enum ComboID {
        IN_DLY_DST, OUT_DLY_DST,
        IN_LOS_RDM, OUT_LOS_RDM,
        IN_SLT_DST, OUT_SLT_DST,
        NUM_COMBOS
    };

    QDoubleSpinBox* optionSpins[NUM_DSPINS];
    QComboBox* optionCombos[NUM_COMBOS];

    void onResetButtonClicked();
};


class MainWindowImpl
{
public:
    MainWindowImpl(MainWindow* self);
    MainWindow* self;

    enum ActionID {
        IMPORT, EXPORT, QUIT,
        SETTING, DEBUG, NUM_ACTIONS
    };
    enum ComboID { IFC, IFB, NUM_COMBOS };
    enum LineID { SRC, DST, NUM_LINES };
    enum SpinID {
        IN_DLY_TIM, OUT_DLY_TIM,
        IN_LOS_PCT, OUT_LOS_PCT,
        IN_RAT_RAT, OUT_RAT_RAT,
        NUM_DSPINS
    };

    QMenu* menus[NUM_MENUS];
    QAction* actions[NUM_ACTIONS];
    QComboBox* combos[NUM_COMBOS];
    QLineEdit* lines[NUM_LINES];
    QDoubleSpinBox* dspins[NUM_DSPINS];
    QPushButton* applyButton;
    ConfigDialog* config;
    bool debugMode;
    bool isUpdated;
    bool isFinalized;

    bool load(const string& filename);

    void onClearButtonClicked();
    void onApplyButtonToggled(const bool& on);
    void onImportActionTriggered(const bool& on);
    void onExportActionTriggered(const bool& on);

    void initialize();
    void start(const string& program);
    void clear();
    void update();
    void finalize();
};

}


MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    impl = new MainWindowImpl(this);
}


MainWindowImpl::MainWindowImpl(MainWindow* self)
    : self(self)
{
    self->setWindowTitle("qtcapp");

    static const char* topMenus[] ={ "&File", "&Edit", "&View", "&Option", "&Help" };
    for(int i = 0; i < NUM_MENUS; ++i) {
        menus[i] = self->menuBar()->addMenu(topMenus[i]);
    }

    for(int i = 0; i < NUM_ACTIONS; ++i) {
        ActionInfo info = actionInfo[i];
        actions[i] = new QAction(info.label);
        QAction* action = actions[i];
        action->setCheckable(info.checkable);
        action->setChecked(info.checked);
        action->setEnabled(info.enabled);
        menus[info.menu]->addAction(action);
    }

    debugMode = false;
    isUpdated = false;
    isFinalized = true;

    config = new ConfigDialog;

    QGridLayout* gbox = new QGridLayout;
    for(int i = 0; i < NUM_COMBOS; ++i) {
        combos[i] = new QComboBox;
        QComboBox* combo = combos[i];
        ComboInfo info = comboInfo[i];
        gbox->addWidget(new QLabel(info.label), info.row, info.cln - 1);
        gbox->addWidget(combo, info.row, info.cln);
    }

    for(int i = 0; i < NUM_LINES; ++i) {
        lines[i] = new QLineEdit;
        QLineEdit* line = lines[i];
        line->setText("0.0.0.0/0");
        LineInfo info = lineInfo[i];
        gbox->addWidget(new QLabel(info.label), info.row, info.cln - 1);
        gbox->addWidget(line, info.row, info.cln);
    }

    QComboBox* ifcCombo = combos[IFC];
    static const int IFR_MAX = 10;
    struct ifreq ifr[IFR_MAX];
    struct ifconf ifc;
    int fd = socket(AF_INET, SOCK_DGRAM, 0);

    ifc.ifc_len = sizeof(ifr);
    ifc.ifc_ifcu.ifcu_buf = (char*)ifr;
    ioctl(fd, SIOCGIFCONF, &ifc);
    int nifs = ifc.ifc_len / sizeof(struct ifreq);
    for(int j = 0; j < nifs; j++) {
        string interfaceName = string(ifr[j].ifr_name);
        ifcCombo->addItem(interfaceName.c_str());
    }
    ::close(fd);

    QComboBox* ifbCombo = combos[IFB];
    const QStringList items = { "ifb0", "ifb1" };
    ifbCombo->addItems(items);

    QGridLayout* bsgbox = new QGridLayout;
    bsgbox->addWidget(new QLabel("Inbound"), 0, 1);
    bsgbox->addWidget(new QLabel("Outbound"), 0, 2);

    static const char* labels[] = { "Dealy Time [ms]", "Loss Percent [%]", "Rate Rate [kbit/s]" };
    for(int i = 0; i < 3; ++i) {
        bsgbox->addWidget(new QLabel(labels[i]), i + 1, 0);
    }

    for(int i = 0; i < NUM_DSPINS; ++i) {
        dspins[i] = new QDoubleSpinBox;
        QDoubleSpinBox* dspin = dspins[i];
        DoubleSpinInfo info = dspinInfo[i];
        dspin->setRange(info.lower, info.upper);
        dspin->setValue(info.value);
        bsgbox->addWidget(dspin, info.row, info.cln);
    }

    QPushButton* resetButton = new QPushButton("Reset");
    applyButton = new QPushButton("Apply");
    applyButton->setCheckable(true);
    bsgbox->addWidget(resetButton, 4, 1);
    bsgbox->addWidget(applyButton, 4, 2);

    QWidget* centralWidget = new QWidget;
    QVBoxLayout* vbox = new QVBoxLayout;
    vbox->addLayout(gbox);
    vbox->addSpacing(10);
    vbox->addLayout(bsgbox);
    vbox->addStretch();
    centralWidget->setLayout(vbox);
    self->setCentralWidget(centralWidget);

    self->connect(resetButton, &QPushButton::clicked, [&](){ onClearButtonClicked(); });
    self->connect(applyButton, QOverload<bool>::of(&QPushButton::toggled),
        [=](bool on){ onApplyButtonToggled(on); });
    self->connect(actions[QUIT], &QAction::triggered, [&, self](){ self->close(); });
    self->connect(actions[IMPORT], QOverload<bool>::of(&QAction::triggered),
        [=](bool on){ onImportActionTriggered(on); });
    self->connect(actions[EXPORT], QOverload<bool>::of(&QAction::triggered),
        [=](bool on){ onExportActionTriggered(on); });
    self->connect(actions[SETTING], &QAction::triggered, [&](){ config->show(); });
    self->connect(actions[DEBUG], QOverload<bool>::of(&QAction::triggered),
        [=](bool on){ debugMode = on; });
}


MainWindow::~MainWindow()
{
    delete impl;
}


bool MainWindow::load(const string& filename)
{
    return impl->load(filename);
}


bool MainWindowImpl::load(const string& filename)
{
    QFileInfo info(filename.c_str());
    QString ext = info.suffix();
    if(ext != "qtc") {
        return false;
    }

    if(!filename.empty()) {
        QFile file(filename.c_str());
        if(file.open(QIODevice::ReadOnly)) {
            QTextStream in(&file);
            for(int i = 0; i < NUM_COMBOS; ++i) {
                combos[i]->setCurrentText(in.readLine());
            }
            for(int i = 0; i < NUM_LINES; ++i) {
                lines[i]->setText(in.readLine());
            }
            for(int i = 0; i < NUM_DSPINS; ++i) {
                dspins[i]->setValue(in.readLine().toDouble());
            }
            for(int i = 0; i < ConfigDialog::NUM_DSPINS; ++i) {
                config->optionSpins[i]->setValue(in.readLine().toDouble());
            }
            for(int i = 0; i < ConfigDialog::NUM_COMBOS; ++i) {
                config->optionCombos[i]->setCurrentText(in.readLine());
            }
        }
    }
    return true;
}


void MainWindowImpl::onClearButtonClicked()
{
    for(int i = 0; i < NUM_COMBOS; ++i) {
        combos[i]->setCurrentIndex(0);
    }
    for(int i = 0; i < NUM_LINES; ++i) {
        lines[i]->setText("0.0.0.0/0");
    }
    for(int i = 0; i < NUM_DSPINS; ++i) {
        dspins[i]->setValue(0.0);
    }
}


void MainWindowImpl::onApplyButtonToggled(const bool& on)
{
    QPalette palette;
    if(on) {
        palette.setColor(QPalette::Button, QColor(Qt::red));
        initialize();
        update();
    } else {
        finalize();
    }
    combos[IFC]->setEnabled(!on);
    combos[IFB]->setEnabled(!on);
    applyButton->setPalette(palette);
}


void MainWindowImpl::onImportActionTriggered(const bool& on)
{
    QFileDialog dialog(self);
    dialog.setWindowTitle("Open a configuration file");
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setViewMode(QFileDialog::List);
    dialog.setLabelText(QFileDialog::Accept, "Open");
    dialog.setLabelText(QFileDialog::Reject, "Cancel");
    dialog.setOption(QFileDialog::DontUseNativeDialog);

    QStringList filters;
    filters << "QTC files (*.qtc)";
//    filters << "Any files (*)";
    dialog.setNameFilters(filters);

    QString filename;
    if(dialog.exec()) {
        filename = dialog.selectedFiles().front();
        self->load(filename.toStdString());
    }
}


void MainWindowImpl::onExportActionTriggered(const bool& on)
{
    QFileDialog dialog(self);
    dialog.setWindowTitle("Save a configuration file");
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.setViewMode(QFileDialog::List);
    dialog.setLabelText(QFileDialog::Accept, "Save");
    dialog.setLabelText(QFileDialog::Reject, "Cancel");
//    dialog.setOption(QFileDialog::DontConfirmOverwrite);
    dialog.setOption(QFileDialog::DontUseNativeDialog);

    QStringList filters;
    filters << "QTC files (*.qtc)";
//    filters << "Any files (*)";
    dialog.setNameFilters(filters);

    QString filename;
    if(dialog.exec() == QDialog::Accepted) {
        filename = dialog.selectedFiles().front();
        QFileInfo info(filename);
        QString suffix = info.suffix();
        if(suffix.isEmpty()) {
            filename += ".qtc";
        }
    }

    if(!filename.isEmpty()) {
        QFile file(filename);
        if(file.open(QIODevice::WriteOnly)) {
            QTextStream out(&file);
            for(int i = 0; i < NUM_COMBOS; ++i) {
                QComboBox* combo = combos[i];
                out << combo->currentText() << endl;
            }
            for(int i = 0; i < NUM_LINES; ++i) {
                QLineEdit* line = lines[i];
                out << line->text() << endl;
            }
            for(int i = 0; i < NUM_DSPINS; ++i) {
                QDoubleSpinBox* spin = dspins[i];
                out << spin->value() << endl;
            }
            for(int i = 0; i < ConfigDialog::NUM_DSPINS; ++i) {
                double value  = config->optionSpins[i]->value();
                out << value << endl;
            }
            for(int i = 0; i < ConfigDialog::NUM_COMBOS; ++i) {
                string text = config->optionCombos[i]->currentText().toStdString();
                out << text.c_str() << endl;
            }
            file.close();
        }
    }
}


void MainWindowImpl::initialize()
{
    string ifbName = combos[IFB]->currentText().toStdString();
    if(!isFinalized) {
        finalize();
    }
    start("sudo modprobe ifb;");
    start("sudo modprobe act_mirred;");
    start((boost::format("sudo ip link set dev %s up;") % ifbName.c_str()).str());
    isUpdated = false;
    isFinalized = false;
}


void MainWindowImpl::start(const string& program)
{
    if(!debugMode) {
        int ret = system(program.c_str());
    } else {
        cout << program << endl;
    }
}


void MainWindowImpl::clear()
{
    string ifcName = combos[IFC]->currentText().toStdString();
    string ifbName = combos[IFB]->currentText().toStdString();
    if(isUpdated) {
        start((boost::format("sudo tc qdisc del dev %s ingress;") % ifcName.c_str()).str());
        start((boost::format("sudo tc qdisc del dev %s root;") % ifbName.c_str()).str());
        start((boost::format("sudo tc qdisc del dev %s root;") % ifcName.c_str()).str());
        isUpdated = false;
    }
}


void MainWindowImpl::update()
{
    string ifcName = combos[IFC]->currentText().toStdString();
    string ifbName = combos[IFB]->currentText().toStdString();

    if(ifcName.empty()) {
        return;
    }

    double inboundDelayTime = dspins[IN_DLY_TIM]->value();
    double inboundLossPercent = dspins[IN_LOS_PCT]->value();
    double inboundRateRate = dspins[IN_RAT_RAT]->value();

    double inboundLimitPackets = config->optionSpins[ConfigDialog::IN_LMT_PKT]->value();
    double inboundDelayJitter = config->optionSpins[ConfigDialog::IN_DLY_JTR]->value();
    double inboundDelayCorrelation = config->optionSpins[ConfigDialog::IN_DLY_COR]->value();
    double inboundLossCorrelation = config->optionSpins[ConfigDialog::IN_LOS_COR]->value();
    double inboundDuplicationPercent = config->optionSpins[ConfigDialog::IN_DPL_PCT]->value();
    double inboundDuplicationCorrelation = config->optionSpins[ConfigDialog::IN_DPL_COR]->value();
    double inboundCorruptPercent = config->optionSpins[ConfigDialog::IN_CRP_PCT]->value();
    double inboundCorruptCorrelation = config->optionSpins[ConfigDialog::IN_CRP_COR]->value();
    double inboundReorderingPercent = config->optionSpins[ConfigDialog::IN_ROR_PCT]->value();
    double inboundReorderingCorrelation = config->optionSpins[ConfigDialog::IN_ROR_COR]->value();
    double inboundReorderingDistance = config->optionSpins[ConfigDialog::IN_ROR_DST]->value();
    double inboundRatePacketOverhead = config->optionSpins[ConfigDialog::IN_RPK_OVH]->value();
    double inboundRateCellSize = config->optionSpins[ConfigDialog::IN_RCL_SIZ]->value();
    double inboundRateCellOverhead = config->optionSpins[ConfigDialog::IN_RCL_OVH]->value();
    double inboundSlotMinDelay = config->optionSpins[ConfigDialog::IN_SLT_MND]->value();
    double inboundSlotMaxDelay = config->optionSpins[ConfigDialog::IN_SLT_MXD]->value();

    string item = config->optionCombos[ConfigDialog::IN_DLY_DST]->currentText().toStdString();
    bool isCheckedInboundDelayDistribution = true;
    string inboundDelayDistribution;
    if(item == "disabled") {
        isCheckedInboundDelayDistribution = false;
    } else {
        inboundDelayDistribution = item;
    }

    item = config->optionCombos[ConfigDialog::IN_LOS_RDM]->currentText().toStdString();
    bool isCheckedInboundLossRandom = true;
    if(item == "disabled") {
        isCheckedInboundLossRandom = false;
    }

    item = config->optionCombos[ConfigDialog::IN_SLT_DST]->currentText().toStdString();
    bool isCheckedInboundSlotDistribution = true;
    string inboundSlotDistribution;
    if(item == "disabled") {
        isCheckedInboundSlotDistribution = false;
    } else {
        inboundSlotDistribution = item;
    }

    double outboundDelayTime = dspins[OUT_DLY_TIM]->value();
    double outboundLossPercent = dspins[OUT_LOS_PCT]->value();
    double outboundRateRate = dspins[OUT_RAT_RAT]->value();

    double outboundLimitPackets = config->optionSpins[ConfigDialog::OUT_LMT_PKT]->value();
    double outboundDelayJitter = config->optionSpins[ConfigDialog::OUT_DLY_JTR]->value();
    double outboundDelayCorrelation = config->optionSpins[ConfigDialog::OUT_DLY_COR]->value();
    double outboundLossCorrelation = config->optionSpins[ConfigDialog::OUT_LOS_COR]->value();
    double outboundDuplicationPercent = config->optionSpins[ConfigDialog::OUT_DPL_PCT]->value();
    double outboundDuplicationCorrelation = config->optionSpins[ConfigDialog::OUT_DPL_COR]->value();
    double outboundCorruptPercent = config->optionSpins[ConfigDialog::OUT_CRP_PCT]->value();
    double outboundCorruptCorrelation = config->optionSpins[ConfigDialog::OUT_CRP_COR]->value();
    double outboundReorderingPercent = config->optionSpins[ConfigDialog::OUT_ROR_PCT]->value();
    double outboundReorderingCorrelation = config->optionSpins[ConfigDialog::OUT_ROR_COR]->value();
    double outboundReorderingDistance = config->optionSpins[ConfigDialog::OUT_ROR_DST]->value();
    double outboundRatePacketOverhead = config->optionSpins[ConfigDialog::OUT_RPK_OVH]->value();
    double outboundRateCellSize = config->optionSpins[ConfigDialog::OUT_RCL_SIZ]->value();
    double outboundRateCellOverhead = config->optionSpins[ConfigDialog::OUT_RCL_OVH]->value();
    double outboundSlotMinDelay = config->optionSpins[ConfigDialog::OUT_SLT_MND]->value();
    double outboundSlotMaxDelay = config->optionSpins[ConfigDialog::OUT_SLT_MXD]->value();

    item = config->optionCombos[ConfigDialog::OUT_DLY_DST]->currentText().toStdString();
    bool isCheckedOutboundDelayDistribution = true;
    string outboundDelayDistribution;
    if(item == "disabled") {
        isCheckedOutboundDelayDistribution = false;
    } else {
        outboundDelayDistribution = item;
    }

    item = config->optionCombos[ConfigDialog::OUT_LOS_RDM]->currentText().toStdString();
    bool isCheckedOutboundLossRandom = true;
    if(item == "disabled") {
        isCheckedOutboundLossRandom = false;
    }

    item = config->optionCombos[ConfigDialog::OUT_SLT_DST]->currentText().toStdString();
    bool isCheckedOutboundSlotDistribution = true;
    string outboundSlotDistribution;
    if(item == "disabled") {
        isCheckedOutboundSlotDistribution = false;
    } else {
        outboundSlotDistribution = item;
    }

    string inboundEffects;
    string outboundEffects;

    // Inbound Commads
    if(inboundLimitPackets > 0.0) {
        inboundEffects += "limit " + to_string((int)inboundLimitPackets);
    }

    if(inboundDelayTime > 0.0) {
        inboundEffects += " delay " + to_string((int)inboundDelayTime) + "ms";
        if(inboundDelayJitter > 0.0) {
            inboundEffects += " " + to_string((int)inboundDelayJitter) + "ms";
            if(inboundDelayCorrelation > 0.0) {
                inboundEffects += " " + to_string((int)inboundDelayCorrelation) + "%";
            }
        }
        if(isCheckedInboundDelayDistribution) {
            inboundEffects += " distribution " + inboundDelayDistribution;
        }
    }

    if(inboundLossPercent > 0.0) {
        inboundEffects += " loss";
        if(isCheckedInboundLossRandom) {
            inboundEffects += " random";
        }
        inboundEffects += " " + to_string((int)inboundLossPercent) + "%";
        if(inboundLossCorrelation > 0.0) {
            inboundEffects += " " + to_string((int)inboundLossCorrelation) +"%";
        }
    }

    if(inboundCorruptPercent > 0.0) {
        inboundEffects += " corrupt " + to_string((int)inboundCorruptPercent) + "%";
        if(inboundCorruptCorrelation > 0.0) {
            inboundEffects += " " + to_string((int)inboundCorruptCorrelation) +"%";
        }
    }

    if(inboundDuplicationPercent > 0.0) {
        inboundEffects += " duplicate " + to_string((int)inboundDuplicationPercent) + "%";
        if(inboundDuplicationCorrelation > 0.0) {
            inboundEffects += " " + to_string((int)inboundDuplicationCorrelation) +"%";
        }
    }

    if(inboundReorderingPercent > 0.0) {
        inboundEffects += " reorder " + to_string((int)inboundReorderingPercent) + "%";
        if(inboundReorderingCorrelation > 0.0) {
            inboundEffects += " " + to_string((int)inboundReorderingCorrelation) +"%";
        }
        if(inboundReorderingDistance > 0.0) {
            inboundEffects += " gap " + to_string((int)inboundReorderingDistance);
        }
    }

    if(inboundRateRate > 0.0) {
        inboundEffects += " rate " + to_string((int)inboundRateRate) + "kbps";
        if(inboundRatePacketOverhead > 0.0) {
            inboundEffects += " " + to_string((int)inboundRatePacketOverhead);
            if(inboundRateCellSize > 0.0) {
                inboundEffects += " " + to_string((int)inboundRateCellSize);
                if(inboundRateCellOverhead > 0.0) {
                    inboundEffects += " " + to_string((int)inboundRateCellOverhead);
                }
            }
        }
    }

    if(isCheckedInboundSlotDistribution) {
        inboundEffects += " slot " + inboundSlotDistribution;
    } else if(inboundSlotMinDelay > 0.0) {
        inboundEffects += " slot " + to_string((int)inboundSlotMinDelay) + "ms";
        if(inboundSlotMaxDelay > 0.0) {
            inboundEffects += " " + to_string((int)inboundSlotMaxDelay) + "ms";
        }
    }

    // Outbound Commands
    if(outboundLimitPackets > 0.0) {
        outboundEffects += "limit " + to_string((int)outboundLimitPackets);
    }

    if(outboundDelayTime > 0.0) {
        outboundEffects += " delay " + to_string((int)outboundDelayTime) + "ms";
        if(outboundDelayJitter > 0.0) {
            outboundEffects += " " + to_string((int)outboundDelayJitter) + "ms";
            if(outboundDelayCorrelation > 0.0) {
                outboundEffects += " " + to_string((int)outboundDelayCorrelation) + "%";
            }
        }
        if(isCheckedOutboundDelayDistribution) {
            outboundEffects += " distribution " + outboundDelayDistribution;
        }
    }

    if(outboundLossPercent > 0.0) {
        outboundEffects += " loss";
        if(isCheckedOutboundLossRandom) {
            outboundEffects += " random";
        }
        outboundEffects += " " + to_string((int)outboundLossPercent) + "%";
        if(outboundLossCorrelation > 0.0) {
            outboundEffects += " " + to_string((int)outboundLossCorrelation) +"%";
        }
    }

    if(outboundCorruptPercent > 0.0) {
        outboundEffects += " corrupt " + to_string((int)outboundCorruptPercent) + "%";
        if(outboundCorruptCorrelation > 0.0) {
            outboundEffects += " " + to_string((int)outboundCorruptCorrelation) +"%";
        }
    }

    if(outboundDuplicationPercent > 0.0) {
        outboundEffects += " duplicate " + to_string((int)outboundDuplicationPercent) + "%";
        if(outboundDuplicationCorrelation > 0.0) {
            outboundEffects += " " + to_string((int)outboundDuplicationCorrelation) +"%";
        }
    }

    if(outboundReorderingPercent > 0.0) {
        outboundEffects += " reorder " + to_string((int)outboundReorderingPercent) + "%";
        if(outboundReorderingCorrelation > 0.0) {
            outboundEffects += " " + to_string((int)outboundReorderingCorrelation) +"%";
        }
        if(outboundReorderingDistance > 0.0) {
            outboundEffects += " gap " + to_string((int)outboundReorderingDistance);
        }
    }

    if(outboundRateRate > 0.0) {
        outboundEffects += " rate " + to_string((int)outboundRateRate) + "kbps";
        if(outboundRatePacketOverhead > 0.0) {
            outboundEffects += " " + to_string((int)outboundRatePacketOverhead);
            if(outboundRateCellSize > 0.0) {
                outboundEffects += " " + to_string((int)outboundRateCellSize);
                if(outboundRateCellOverhead > 0.0) {
                    outboundEffects += " " + to_string((int)outboundRateCellOverhead);
                }
            }
        }
    }

    if(isCheckedOutboundSlotDistribution) {
        outboundEffects += " slot " + outboundSlotDistribution;
    } else if(outboundSlotMinDelay > 0.0) {
        outboundEffects += " slot " + to_string((int)outboundSlotMinDelay) + "ms";
        if(outboundSlotMaxDelay > 0.0) {
            outboundEffects += " " + to_string((int)outboundSlotMaxDelay) + "ms";
        }
    }

    string srcipName = lines[SRC]->text().toStdString();
    string dstipName = lines[DST]->text().toStdString();
    if(!checkIP(srcipName)) {
        srcipName = "0.0.0.0/0";
    }
    if(!checkIP(dstipName)) {
        dstipName = "0.0.0.0/0";
    }

    if(!isFinalized) {
        clear();
        start((boost::format("sudo tc qdisc add dev %s ingress handle ffff:;") % ifcName.c_str()).str());
        start((boost::format("sudo tc filter add dev %s parent ffff: protocol ip u32 match u32 0 0 action mirred egress redirect dev %s;") % ifcName.c_str() % ifbName.c_str()).str());
        start((boost::format("sudo tc qdisc add dev %s root handle 1: prio bands 16 priomap 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0;") % ifbName.c_str()).str());
        start((boost::format("sudo tc qdisc add dev %s parent 1:1 handle 10: netem limit %s;") % ifbName.c_str() % to_string((int)inboundLimitPackets)).str());
        if((!srcipName.empty()) && (!dstipName.empty())) {
            start((boost::format("sudo tc qdisc add dev %s parent 1:2 handle 20: netem %s;") % ifbName.c_str() % inboundEffects.c_str()).str());
            start((boost::format("sudo tc filter add dev %s protocol ip parent 1: prio 2 u32 match ip src %s match ip dst %s flowid 1:2;") % ifbName.c_str() % dstipName.c_str() % srcipName.c_str()).str());
        }
        start((boost::format("sudo tc qdisc add dev %s root handle 1: prio bands 16 priomap 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0;") % ifcName.c_str()).str());
        start((boost::format("sudo tc qdisc add dev %s parent 1:1 handle 10: netem limit %s;") % ifcName.c_str() % to_string((int)outboundLimitPackets)).str());
        if((!srcipName.empty()) && (!dstipName.empty())) {
            start((boost::format("sudo tc qdisc add dev %s parent 1:2 handle 20: netem %s;") % ifcName.c_str() % outboundEffects.c_str()).str());
            start((boost::format("sudo tc filter add dev %s protocol ip parent 1: prio 2 u32 match ip src %s match ip dst %s flowid 1:2;") % ifcName.c_str() % srcipName.c_str() % dstipName.c_str()).str());
        }
        isUpdated = true;
    }
}


void MainWindowImpl::finalize()
{
    string ifbName = combos[IFB]->currentText().toStdString();
    if(!isFinalized) {
        clear();
        start((boost::format("sudo ip link set dev %s down;") % ifbName.c_str()).str());
        start("sudo rmmod ifb;");
        isFinalized = true;
    }
}



ConfigDialog::ConfigDialog()
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
        optionSpins[i] = new QDoubleSpinBox;
        QDoubleSpinBox* dspin = optionSpins[i];
        DoubleSpinInfo info = dspinInfo2[i];
        dspin->setRange(info.lower, info.upper);
        dspin->setValue(info.value);
        gbox->addWidget(dspin, info.row, info.cln);
    }

    QStringList distributions = { "disabled", "uniform", "normal", "pareto", "paretonormal" };
    QStringList states = { "disabled", "enabled" };
    for(int i = 0; i < NUM_COMBOS; ++i) {
        optionCombos[i] = new QComboBox;
        QComboBox* combo = optionCombos[i];
        if(i == 2 || i == 3) {
            combo->addItems(states);
        } else {
            combo->addItems(distributions);
        }
        combo->setCurrentIndex(0);
        ComboInfo info = comboInfo2[i];
        gbox->addWidget(combo, info.row, info.cln);
    }

    QPushButton* resetButton = new QPushButton("&Reset");
    QPushButton* okButton = new QPushButton("&Ok");
    okButton->setDefault(true);
    QDialogButtonBox* buttonBox = new QDialogButtonBox(this);
    buttonBox->addButton(resetButton, QDialogButtonBox::ResetRole);
    buttonBox->addButton(okButton, QDialogButtonBox::AcceptRole);
    connect(resetButton, &QPushButton::clicked, [&](){ onResetButtonClicked(); });
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));

    vbox->addLayout(gbox);
    vbox->addWidget(buttonBox);
    setLayout(vbox);
}


void ConfigDialog::onResetButtonClicked()
{
    for(int i = 0; i < NUM_DSPINS; ++i) {
        QDoubleSpinBox* dspin = optionSpins[i];
        DoubleSpinInfo info = dspinInfo2[i];
        dspin->setRange(info.lower, info.upper);
        dspin->setValue(info.value);
    }

    for(int i = 0; i < NUM_COMBOS; ++i) {
        optionCombos[i]->setCurrentIndex(0);
    }
}
