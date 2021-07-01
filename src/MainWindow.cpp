/*!
  @file
  @author Kenta Suzuki
*/

#include "MainWindow.h"
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QFrame>
#include <QGridLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QPalette>
#include <QProcess>
#include <QPushButton>
#include <QVBoxLayout>
#include <boost/format.hpp>
#include <sstream>
#include <stdlib.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <iostream>
#include "ConfigDialog.h"

#define IFR_MAX 10

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


struct SpinInfo {
    int row;
    int cln;
    double lower;
    double upper;
    double value;
};


SpinInfo spinInfo[] = {
    { 1, 1, 0.0,   100000.0, 0.0 }, { 1, 2, 0.0,   100000.0, 0.0 },
    { 2, 1, 0.0,      100.0, 0.0 }, { 2, 2, 0.0,      100.0, 0.0 },
    { 3, 1, 0.0, 11000000.0, 0.0 }, { 3, 2, 0.0, 11000000.0, 0.0 },
};

}


class MainWindowImpl
{
public:
    MainWindowImpl(MainWindow* self);
    MainWindow* self;

    enum ComboId { IFC, IFB, NUM_COMBOS };
    enum LineId { SRC, DST, NUM_LINES };
    enum SpinId {
        IN_DLY_TIM, OUT_DLY_TIM,
        IN_LOS_PCT, OUT_LOS_PCT,
        IN_RAT_RAT, OUT_RAT_RAT,
        NUM_SPINS
    };

    QComboBox* combos[NUM_COMBOS];
    QLineEdit* lines[NUM_LINES];
    QDoubleSpinBox* spins[NUM_SPINS];

    QAction* showAct;
    bool showCommands;
    QAction* settingAct;
    QAction* debugAct;
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


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    impl = new MainWindowImpl(this);
    impl->onTCInitialize();
}


MainWindowImpl::MainWindowImpl(MainWindow* self)
    : self(self)
{
    self->setWindowTitle("QtcApp");

    QMenu* fileMenu = self->menuBar()->addMenu("&File");
    QMenu* editMenu = self->menuBar()->addMenu("&Edit");
    QMenu* viewMenu = self->menuBar()->addMenu("&View");
    QMenu* optionMenu = self->menuBar()->addMenu("&Option");
    QMenu* helpMenu = self->menuBar()->addMenu("&Help");

    QAction* quitAct = new QAction("Quit");
    fileMenu->addAction(quitAct);

    showAct = new QAction("Show commands");
    showAct->setCheckable(true);
    showAct->setChecked(false);
    editMenu->addAction(showAct);
    showCommands = false;

    settingAct = new QAction("Advanced settings");
    optionMenu->addAction(settingAct);
    debugAct = new QAction("Debug mode");
    debugAct->setCheckable(true);
    debugAct->setChecked(false);
    optionMenu->addAction(debugAct);
    debugMode = false;

    config = new ConfigDialog();

    QGridLayout* gbox = new QGridLayout();
    for(int i = 0; i < NUM_COMBOS; ++i) {
        combos[i] = new QComboBox();
        QComboBox* combo = combos[i];
        ComboInfo info = comboInfo[i];
        gbox->addWidget(new QLabel(info.label), info.row, info.cln - 1);
        gbox->addWidget(combo, info.row, info.cln);
    }

    for(int i = 0; i < NUM_LINES; ++i) {
        lines[i] = new QLineEdit();
        QLineEdit* line = lines[i];
        line->setText("0.0.0.0/0");
        LineInfo info = lineInfo[i];
        gbox->addWidget(new QLabel(info.label), info.row, info.cln - 1);
        gbox->addWidget(line, info.row, info.cln);
    }

    QComboBox* ifcCombo = combos[IFC];
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

    QComboBox* ifbCombo = combos[IFB];
    const QStringList items = { "ifb0", "ifb1" };
    ifbCombo->addItems(items);

    QGridLayout* bsgbox = new QGridLayout();
    bsgbox->addWidget(new QLabel("Inbound"), 0, 1);
    bsgbox->addWidget(new QLabel("Outbound"), 0, 2);

    QStringList labels = { "Dealy Time [ms]", "Loss Percent [%]", "Rate Rate [kbit/s]" };
    for(int i = 0; i < 3; ++i) {
        bsgbox->addWidget(new QLabel(labels[i]), i + 1, 0);
    }

    for(int i = 0; i < NUM_SPINS; ++i) {
        spins[i] = new QDoubleSpinBox();
        QDoubleSpinBox* spin = spins[i];
        SpinInfo info = spinInfo[i];
        spin->setRange(info.lower, info.upper);
        spin->setValue(info.value);
        bsgbox->addWidget(spin, info.row, info.cln);
    }

    resetButton = new QPushButton("Reset");
    applyButton = new QPushButton("Apply");
    applyButton->setCheckable(true);
    bsgbox->addWidget(resetButton, 4, 1);
    bsgbox->addWidget(applyButton, 4, 2);

    QWidget* centralWidget = new QWidget();
    QVBoxLayout* vbox = new QVBoxLayout();
    vbox->addLayout(gbox);
    QFrame* frame = new QFrame();
    frame->setFrameShape(QFrame::HLine);
    vbox->addWidget(frame);
    vbox->addLayout(bsgbox);
    centralWidget->setLayout(vbox);
    self->setCentralWidget(centralWidget);

    self->connect(resetButton, SIGNAL(clicked()), self, SLOT(onClearButtonClicked()));
    self->connect(applyButton, SIGNAL(toggled(bool)), self, SLOT(onApplyButtonToggled(bool)));
    self->connect(showAct, SIGNAL(triggered(bool)), self, SLOT(onShowActionTriggered(bool)));
    self->connect(settingAct, SIGNAL(triggered(bool)), config, SLOT(show()));
    self->connect(debugAct, SIGNAL(triggered(bool)), self, SLOT(onDebugActionTriggered(bool)));
    self->connect(debugAct, SIGNAL(triggered(bool)), showAct, SLOT(setChecked(bool)));
    self->connect(ifbCombo, SIGNAL(currentTextChanged(QString)), self, SLOT(onCurrentIFBChanged(QString)));
    self->connect(quitAct, SIGNAL(triggered()), self, SLOT(close()));
}


MainWindow::~MainWindow()
{
    impl->onTCFinalize();
    delete impl;
}


void MainWindow::onClearButtonClicked()
{
    for(int i = 0; i < MainWindowImpl::NUM_COMBOS; ++i) {
        impl->combos[i]->setCurrentIndex(0);
    }

    for(int i = 0; i < MainWindowImpl::NUM_LINES; ++i) {
        impl->lines[i]->setText("0.0.0.0/0");
    }

    for(int i = 0; i < MainWindowImpl::NUM_SPINS; ++i) {
        impl->spins[i]->setValue(0.0);
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
    impl->combos[MainWindowImpl::IFC]->setEnabled(!on);
    impl->combos[MainWindowImpl::IFB]->setEnabled(!on);
    impl->applyButton->setPalette(palette);
}


void MainWindow::onShowActionTriggered(const bool& on)
{
    impl->showCommands = on;
}


void MainWindow::onDebugActionTriggered(const bool& on)
{
    impl->debugMode = on;
    impl->showCommands = on;
}


void MainWindow::onCurrentIFBChanged(QString ifbName)
{
    impl->onTCInitialize();
}


void MainWindowImpl::onTCInitialize()
{
    string ifbName = combos[IFB]->currentText().toStdString();
    string message = (boost::format("sudo modprobe ifb;"
                                    "sudo modprobe act_mirred;"
                                    "sudo ip link set dev %s up;")
                % ifbName.c_str()).str();
    onCommandExecute(message);
}


void MainWindowImpl::onTCClear()
{
    string ifcName = combos[IFC]->currentText().toStdString();
    string ifbName = combos[IFB]->currentText().toStdString();
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
    string ifbName = combos[IFB]->currentText().toStdString();
    string message = (boost::format("sudo ip link set dev %s down;"
                                    "sudo rmmod ifb;")
                % ifbName.c_str()
                ).str();
    onCommandExecute(message);
}


void MainWindowImpl::onTCExecute()
{
    string ifcName = combos[IFC]->currentText().toStdString();
    string ifbName = combos[IFB]->currentText().toStdString();

    if(ifcName.empty()) {
        return;
    }

    double inboundDelayTime = spins[IN_DLY_TIM]->value();
    double inboundLossPercent = spins[IN_LOS_PCT]->value();
    double inboundRateRate = spins[IN_RAT_RAT]->value();

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

    double outboundDelayTime = spins[OUT_DLY_TIM]->value();
    double outboundLossPercent = spins[OUT_LOS_PCT]->value();
    double outboundRateRate = spins[OUT_RAT_RAT]->value();

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

    string srcipName = lines[SRC]->text().toStdString();
    string dstipName = lines[DST]->text().toStdString();
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
        }
        if(showCommands) {
            cout << command.toStdString() << endl;
        }
    }
}
