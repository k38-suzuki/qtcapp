/*!
  @file
  @author Kenta Suzuki
*/

#include "MainWindow.h"
#include <QCheckBox>
#include <QComboBox>
#include <QDebug>
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
#include <sstream>
#include <stdlib.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include "ConfigDialog.h"

#define IFR_MAX 10

using namespace qtc;
using namespace std;

namespace {

vector<string> split(const string& s, char delim)
{
    vector<string> elements;
    stringstream ss(s);
    string item;
    while (getline(ss, item, delim)) {
    if (!item.empty()) {
            elements.push_back(item);
        }
    }
    return elements;
}

bool addressCheck(const std::string& address)
{
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

                    } else {
                        return false;
                    }
                }
            } else {
                return false;
            }
        } else {
            return false;
        }
    } else {
        return false;
    }
    return true;
}

struct ActionInfo {
    QString label;
    bool checkable;
    bool checked;
    int menu;
};

ActionInfo actionInfo[] = {
    { "Import",            false, false,   MainWindow::FILE },
    { "Export",            false, false,   MainWindow::FILE },
    { "Quit",              false, false,   MainWindow::FILE },
    { "Advanced settings", false, false, MainWindow::OPTION },
    { "Debug mode",         true, false, MainWindow::OPTION }
};

struct ComboInfo {
    QString label;
    int row;
    int cln;
};

ComboInfo comboInfo[] = {
    { "Interface", 0, 1 },
    { "IFB",       0, 3 }
};

struct LineInfo {
    QString label;
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

DoubleSpinInfo doublespinInfo[] = {
    { 1, 1, 0.0,   100000.0, 0.0 }, { 1, 2, 0.0,   100000.0, 0.0 },
    { 2, 1, 0.0,      100.0, 0.0 }, { 2, 2, 0.0,      100.0, 0.0 },
    { 3, 1, 0.0, 11000000.0, 0.0 }, { 3, 2, 0.0, 11000000.0, 0.0 },
};

}


namespace qtc {

class MainWindowImpl
{
public:
    MainWindowImpl(MainWindow* self);
    MainWindow* self;

    QMenu* menus[MainWindow::NUM_MENUS];
    QAction* actions[MainWindow::NUM_ACTIONS];
    QComboBox* combos[MainWindow::NUM_COMBOS];
    QLineEdit* lines[MainWindow::NUM_LINES];
    QDoubleSpinBox* dspins[MainWindow::NUM_DSPINS];

    bool debugMode;

    QPushButton* resetButton;
    QPushButton* applyButton;

    ConfigDialog* config;

    void onTCInitialize();
    void onTCClear();
    void onTCFinalize();
    void onTCExecute();
    void onCommandExecute(const string& message);
};

}


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    impl = new MainWindowImpl(this);
    impl->onTCInitialize();
}


MainWindowImpl::MainWindowImpl(MainWindow* self)
    : self(self)
{
    self->setWindowTitle("Qtcapp");

    QStringList topMenus ={ "&File", "&Edit", "&View", "&Option", "&Help" };
    for(int i = 0; i < MainWindow::NUM_MENUS; ++i) {
        menus[i] = self->menuBar()->addMenu(topMenus[i]);
    }

    for(int i = 0; i < MainWindow::NUM_ACTIONS; ++i) {
        ActionInfo info = actionInfo[i];
        actions[i] = new QAction(info.label);
        QAction* action = actions[i];
        action->setCheckable(info.checkable);
        action->setChecked(info.checked);
        menus[info.menu]->addAction(action);
    }

    debugMode = false;

    config = new ConfigDialog();

    QGridLayout* gbox = new QGridLayout();
    for(int i = 0; i < MainWindow::NUM_COMBOS; ++i) {
        combos[i] = new QComboBox();
        QComboBox* combo = combos[i];
        ComboInfo info = comboInfo[i];
        gbox->addWidget(new QLabel(info.label), info.row, info.cln - 1);
        gbox->addWidget(combo, info.row, info.cln);
    }

    for(int i = 0; i < MainWindow::NUM_LINES; ++i) {
        lines[i] = new QLineEdit();
        QLineEdit* line = lines[i];
        line->setText("0.0.0.0/0");
        LineInfo info = lineInfo[i];
        gbox->addWidget(new QLabel(info.label), info.row, info.cln - 1);
        gbox->addWidget(line, info.row, info.cln);
    }

    QComboBox* ifcCombo = combos[MainWindow::IFC];
    struct ifreq ifr[IFR_MAX];
    struct ifconf ifc;
    int fd = socket(AF_INET, SOCK_DGRAM, 0);

    ifc.ifc_len = sizeof(ifr);
    ifc.ifc_ifcu.ifcu_buf = (char*)ifr;
    ioctl(fd, SIOCGIFCONF, &ifc);
    int nifs = ifc.ifc_len / sizeof(struct ifreq);
    for(int j = 0; j < nifs; j++) {
        string interfaceName = string(ifr[j].ifr_name);
        if(interfaceName != "lo") {
            ifcCombo->addItem(QString::fromStdString(interfaceName));
        }
    }
    ifcCombo->addItem("lo");
    ::close(fd);

    QComboBox* ifbCombo = combos[MainWindow::IFB];
    const QStringList items = { "ifb0", "ifb1" };
    ifbCombo->addItems(items);

    QGridLayout* bsgbox = new QGridLayout();
    bsgbox->addWidget(new QLabel("Inbound"), 0, 1);
    bsgbox->addWidget(new QLabel("Outbound"), 0, 2);

    QStringList labels = { "Dealy Time [ms]", "Loss Percent [%]", "Rate Rate [kbit/s]" };
    for(int i = 0; i < 3; ++i) {
        bsgbox->addWidget(new QLabel(labels[i]), i + 1, 0);
    }

    for(int i = 0; i < MainWindow::NUM_DSPINS; ++i) {
        dspins[i] = new QDoubleSpinBox();
        QDoubleSpinBox* dspin = dspins[i];
        DoubleSpinInfo info = doublespinInfo[i];
        dspin->setRange(info.lower, info.upper);
        dspin->setValue(info.value);
        bsgbox->addWidget(dspin, info.row, info.cln);
    }

    resetButton = new QPushButton("Reset");
    applyButton = new QPushButton("Apply");
    applyButton->setCheckable(true);
    bsgbox->addWidget(resetButton, 4, 1);
    bsgbox->addWidget(applyButton, 4, 2);

    QWidget* centralWidget = new QWidget();
    QVBoxLayout* vbox = new QVBoxLayout();
    vbox->addLayout(gbox);
    vbox->addSpacing(10);
    vbox->addLayout(bsgbox);
    centralWidget->setLayout(vbox);
    self->setCentralWidget(centralWidget);

    self->connect(resetButton, SIGNAL(clicked()), self, SLOT(onClearButtonClicked()));
    self->connect(applyButton, SIGNAL(toggled(bool)), self, SLOT(onApplyButtonToggled(bool)));
    self->connect(actions[MainWindow::QUIT], SIGNAL(triggered()), self, SLOT(close()));
    self->connect(actions[MainWindow::IMPORT], SIGNAL(triggered(bool)), self, SLOT(onImportActionTriggered(bool)));
    self->connect(actions[MainWindow::EXPORT], SIGNAL(triggered(bool)), self, SLOT(onExportActionTriggered(bool)));
    self->connect(actions[MainWindow::SETTING], SIGNAL(triggered(bool)), config, SLOT(show()));
    self->connect(actions[MainWindow::DEBUG], SIGNAL(triggered(bool)), self, SLOT(onDebugActionTriggered(bool)));
    self->connect(ifbCombo, SIGNAL(currentTextChanged(QString)), self, SLOT(onCurrentIFBChanged(QString)));
}


MainWindow::~MainWindow()
{
    impl->onTCFinalize();
    delete impl;
}


void MainWindow::onImportFile(QString filename)
{
    QFileInfo info(filename);
    QString suffix = info.suffix();
    if(suffix.isEmpty()) {
        filename.clear();
    }

    if(!filename.isEmpty()) {
        QFile file(filename);
        if(file.open(QIODevice::ReadOnly)) {
            QTextStream in(&file);
            for(int i = 0; i < NUM_COMBOS; ++i) {
                impl->combos[i]->setCurrentText(in.readLine());
            }

            for(int i = 0; i < NUM_LINES; ++i) {
                impl->lines[i]->setText(in.readLine());
            }

            for(int i = 0; i < NUM_DSPINS; ++i) {
                impl->dspins[i]->setValue(in.readLine().toDouble());
            }

            for(int i = 0; i < ConfigDialog::NUM_DSPINS; ++i) {
                impl->config->setValue(i, in.readLine().toDouble());
            }

            for(int i = 0; i < ConfigDialog::NUM_COMBOS; ++i) {
                impl->config->setText(i, in.readLine());
            }
        }
    }
}


void MainWindow::onClearButtonClicked()
{
    for(int i = 0; i < MainWindow::NUM_COMBOS; ++i) {
        impl->combos[i]->setCurrentIndex(0);
    }

    for(int i = 0; i < MainWindow::NUM_LINES; ++i) {
        impl->lines[i]->setText("0.0.0.0/0");
    }

    for(int i = 0; i < MainWindow::NUM_DSPINS; ++i) {
        impl->dspins[i]->setValue(0.0);
    }
}


void MainWindow::onApplyButtonToggled(const bool& on)
{
    QPalette palette;

    if(on) {
        palette.setColor(QPalette::Button, QColor(Qt::red));
        impl->onTCExecute();
    } else {
        impl->onTCClear();
    }
    impl->combos[MainWindow::IFC]->setEnabled(!on);
    impl->combos[MainWindow::IFB]->setEnabled(!on);
    impl->applyButton->setPalette(palette);
}


void MainWindow::onImportActionTriggered(const bool& on)
{
    QFileDialog dialog(this);
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
        onImportFile(filename);
    }
}


void MainWindow::onExportActionTriggered(const bool& on)
{
    QFileDialog dialog(this);
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
                QComboBox* combo = impl->combos[i];
                out << combo->currentText() << endl;
            }

            for(int i = 0; i < NUM_LINES; ++i) {
                QLineEdit* line = impl->lines[i];
                out << line->text() << endl;
            }

            for(int i = 0; i < NUM_DSPINS; ++i) {
                QDoubleSpinBox* spin = impl->dspins[i];
                out << spin->value() << endl;
            }

            for(int i = 0; i < ConfigDialog::NUM_DSPINS; ++i) {
                double value  = impl->config->spin(i);
                out << value << endl;
            }

            for(int i = 0; i < ConfigDialog::NUM_COMBOS; ++i) {
                QString text = impl->config->combo(i);
                out << text << endl;
            }
            file.close();
        }
    }
}


void MainWindow::onDebugActionTriggered(const bool& on)
{
    impl->debugMode = on;
}


void MainWindow::onCurrentIFBChanged(QString ifbName)
{
    impl->onTCInitialize();
}


void MainWindowImpl::onTCInitialize()
{
    string ifbName = combos[MainWindow::IFB]->currentText().toStdString();
    string message = (boost::format("sudo modprobe ifb;"
                                    "sudo modprobe act_mirred;"
                                    "sudo ip link set dev %s up;")
                % ifbName.c_str()).str();
    onCommandExecute(message);
}


void MainWindowImpl::onTCClear()
{
    string ifcName = combos[MainWindow::IFC]->currentText().toStdString();
    string ifbName = combos[MainWindow::IFB]->currentText().toStdString();
    string message = (boost::format("sudo tc qdisc del dev %s ingress;"
                                    "sudo tc qdisc del dev %s root;"
                                    "sudo tc qdisc del dev %s root;")
                % ifcName.c_str()
                % ifbName.c_str()
                % ifcName.c_str()
                ).str();
    onCommandExecute(message);
}


void MainWindowImpl::onTCFinalize()
{
    string ifbName = combos[MainWindow::IFB]->currentText().toStdString();
    string message = (boost::format("sudo ip link set dev %s down;"
                                    "sudo rmmod ifb;")
                % ifbName.c_str()
                ).str();
    onCommandExecute(message);
}


void MainWindowImpl::onTCExecute()
{
    string ifcName = combos[MainWindow::IFC]->currentText().toStdString();
    string ifbName = combos[MainWindow::IFB]->currentText().toStdString();

    if(ifcName.empty()) {
        return;
    }

    double inboundDelayTime = dspins[MainWindow::IN_DLY_TIM]->value();
    double inboundLossPercent = dspins[MainWindow::IN_LOS_PCT]->value();
    double inboundRateRate = dspins[MainWindow::IN_RAT_RAT]->value();

    double inboundLimitPackets = config->spin(ConfigDialog::IN_LMT_PKT);
    double inboundDelayJitter = config->spin(ConfigDialog::IN_DLY_JTR);
    double inboundDelayCorrelation = config->spin(ConfigDialog::IN_DLY_COR);
    double inboundLossCorrelation = config->spin(ConfigDialog::IN_LOS_COR);
    double inboundDuplicationPercent = config->spin(ConfigDialog::IN_DPL_PCT);
    double inboundDuplicationCorrelation = config->spin(ConfigDialog::IN_DPL_COR);
    double inboundCorruptPercent = config->spin(ConfigDialog::IN_CRP_PCT);
    double inboundCorruptCorrelation = config->spin(ConfigDialog::IN_CRP_COR);
    double inboundReorderingPercent = config->spin(ConfigDialog::IN_ROR_PCT);
    double inboundReorderingCorrelation = config->spin(ConfigDialog::IN_ROR_COR);
    double inboundReorderingDistance = config->spin(ConfigDialog::IN_ROR_DST);
    double inboundRatePacketOverhead = config->spin(ConfigDialog::IN_RPK_OVH);
    double inboundRateCellSize = config->spin(ConfigDialog::IN_RCL_SIZ);
    double inboundRateCellOverhead = config->spin(ConfigDialog::IN_RCL_OVH);
    double inboundSlotMinDelay = config->spin(ConfigDialog::IN_SLT_MND);
    double inboundSlotMaxDelay = config->spin(ConfigDialog::IN_SLT_MXD);

    QString item = config->combo(ConfigDialog::IN_DLY_DST);
    bool isCheckedInboundDelayDistribution = true;
    string inboundDelayDistribution;
    if(item == "disabled") {
        isCheckedInboundDelayDistribution = false;
    } else {
        inboundDelayDistribution = item.toStdString();
    }

    item = config->combo(ConfigDialog::IN_LOS_RDM);
    bool isCheckedInboundLossRandom = true;
    if(item == "disabled") {
        isCheckedInboundLossRandom = false;
    }

    item = config->combo(ConfigDialog::IN_SLT_DST);
    bool isCheckedInboundSlotDistribution = true;
    string inboundSlotDistribution;
    if(item == "disabled") {
        isCheckedInboundSlotDistribution = false;
    } else {
        inboundSlotDistribution = item.toStdString();
    }

    double outboundDelayTime = dspins[MainWindow::OUT_DLY_TIM]->value();
    double outboundLossPercent = dspins[MainWindow::OUT_LOS_PCT]->value();
    double outboundRateRate = dspins[MainWindow::OUT_RAT_RAT]->value();

    double outboundLimitPackets = config->spin(ConfigDialog::OUT_LMT_PKT);
    double outboundDelayJitter = config->spin(ConfigDialog::OUT_DLY_JTR);
    double outboundDelayCorrelation = config->spin(ConfigDialog::OUT_DLY_COR);
    double outboundLossCorrelation = config->spin(ConfigDialog::OUT_LOS_COR);
    double outboundDuplicationPercent = config->spin(ConfigDialog::OUT_DPL_PCT);
    double outboundDuplicationCorrelation = config->spin(ConfigDialog::OUT_DPL_COR);
    double outboundCorruptPercent = config->spin(ConfigDialog::OUT_CRP_PCT);
    double outboundCorruptCorrelation = config->spin(ConfigDialog::OUT_CRP_COR);
    double outboundReorderingPercent = config->spin(ConfigDialog::OUT_ROR_PCT);
    double outboundReorderingCorrelation = config->spin(ConfigDialog::OUT_ROR_COR);
    double outboundReorderingDistance = config->spin(ConfigDialog::OUT_ROR_DST);
    double outboundRatePacketOverhead = config->spin(ConfigDialog::OUT_RPK_OVH);
    double outboundRateCellSize = config->spin(ConfigDialog::OUT_RCL_SIZ);
    double outboundRateCellOverhead = config->spin(ConfigDialog::OUT_RCL_OVH);
    double outboundSlotMinDelay = config->spin(ConfigDialog::OUT_SLT_MND);
    double outboundSlotMaxDelay = config->spin(ConfigDialog::OUT_SLT_MXD);

    item = config->combo(ConfigDialog::OUT_DLY_DST);
    bool isCheckedOutboundDelayDistribution = true;
    string outboundDelayDistribution;
    if(item == "disabled") {
        isCheckedOutboundDelayDistribution = false;
    } else {
        outboundDelayDistribution = item.toStdString();
    }

    item = config->combo(ConfigDialog::OUT_LOS_RDM);
    bool isCheckedOutboundLossRandom = true;
    if(item == "disabled") {
        isCheckedOutboundLossRandom = false;
    }

    item = config->combo(ConfigDialog::OUT_SLT_DST);
    bool isCheckedOutboundSlotDistribution = true;
    string outboundSlotDistribution;
    if(item == "disabled") {
        isCheckedOutboundSlotDistribution = false;
    } else {
        outboundSlotDistribution = item.toStdString();
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

    string srcipName = lines[MainWindow::SRC]->text().toStdString();
    string dstipName = lines[MainWindow::DST]->text().toStdString();
    if(!addressCheck(srcipName)) {
        srcipName = "0.0.0.0/0";
    }
    if(!addressCheck(dstipName)) {
        dstipName = "0.0.0.0/0";
    }

    string srcMessage;
    string dstMessage;

    if((!srcipName.empty()) && (!dstipName.empty())) {
        dstMessage = (
                    boost::format("sudo tc qdisc add dev %s parent 1:2 handle 20: netem %s;"
                                  "sudo tc filter add dev %s protocol ip parent 1: prio 2 u32 match ip src %s match ip dst %s flowid 1:2;")
                    % ifbName.c_str() % inboundEffects.c_str()
                    % ifbName.c_str() % dstipName.c_str() % srcipName.c_str()
                    ).str();
        srcMessage = (
                    boost::format("sudo tc qdisc add dev %s parent 1:2 handle 20: netem %s;"
                                  "sudo tc filter add dev %s protocol ip parent 1: prio 2 u32 match ip src %s match ip dst %s flowid 1:2;")
                    % ifcName.c_str() % outboundEffects.c_str()
                    % ifcName.c_str() % srcipName.c_str() % dstipName.c_str()
                    ).str();
    }

    string message = (
                boost::format("sudo tc qdisc add dev %s ingress handle ffff:;"
                              "sudo tc filter add dev %s parent ffff: protocol ip u32 match u32 0 0 action mirred egress redirect dev %s;"
                              "sudo tc qdisc add dev %s root handle 1: prio bands 16 priomap 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0;"
                              "sudo tc qdisc add dev %s parent 1:1 handle 10: netem limit %s;"
                              "%s"
                              "sudo tc qdisc add dev %s root handle 1: prio bands 16 priomap 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0;"
                              "sudo tc qdisc add dev %s parent 1:1 handle 10: netem limit %s;"
                              "%s")
                % ifcName.c_str()
                % ifcName.c_str() % ifbName.c_str()
                % ifbName.c_str()
                % ifbName.c_str()
                % to_string((int)inboundLimitPackets)
                % dstMessage.c_str()
                % ifcName.c_str()
                % ifcName.c_str()
                % to_string((int)outboundLimitPackets)
                % srcMessage.c_str()
                ).str();

    onCommandExecute(message);
}


void MainWindowImpl::onCommandExecute(const string& message)
{
    vector<string> commands = split(message, ';');
    for(size_t i = 0; i < commands.size(); ++i) {
        QString command = QString::fromStdString(commands[i]);
        if(!debugMode) {
            QProcess::execute(command);
        } else {
            qDebug() << command;
        }
    }
}
