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
#include <QHBoxLayout>
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
#define DELAY_MAX 100000
#define RATE_MAX 11000000
#define LOSS_MAX 100.0

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

}


class MainWindowImpl
{
public:
    MainWindowImpl(MainWindow* self);
    MainWindow* self;

    QComboBox* ifcCombo;
    QComboBox* ifbCombo;
    QLineEdit* srcLine;
    QLineEdit* dstLine;

    QDoubleSpinBox* inboundDelayTimeSpin;
    QDoubleSpinBox* inboundLossPercentSpin;
    QDoubleSpinBox* inboundRateRateSpin;

    QDoubleSpinBox* outboundDelayTimeSpin;
    QDoubleSpinBox* outboundLossPercentSpin;
    QDoubleSpinBox* outboundRateRateSpin;

    QAction* showAct;
    bool showCommands;
    QAction* settingAct;
    QAction* debugAct;
    bool debugMode;

    QPushButton* clrButton;
    QPushButton* aplButton;

    ConfigDialog* config;

    QHBoxLayout* makeSeparator(QString text);

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

    QMenu* fileMenu = self->menuBar()->addMenu(QObject::tr("&File"));
    QMenu* editMenu = self->menuBar()->addMenu(QObject::tr("&Edit"));
    QMenu* viewMenu = self->menuBar()->addMenu(QObject::tr("&View"));
    QMenu* optionMenu = self->menuBar()->addMenu(QObject::tr("&Option"));
    QMenu* helpMenu = self->menuBar()->addMenu(QObject::tr("&Help"));

    showAct = new QAction(QObject::tr("Show commands"));
    showAct->setCheckable(true);
    showAct->setChecked(false);
    editMenu->addAction(showAct);
    showCommands = false;

    settingAct = new QAction(QObject::tr("Advanced settings"));
    optionMenu->addAction(settingAct);
    debugAct = new QAction(QObject::tr("Debug mode"));
    debugAct->setCheckable(true);
    debugAct->setChecked(false);
    optionMenu->addAction(debugAct);
    debugMode = false;

    QAction* quitAct = new QAction(QObject::tr("Quit"));
    fileMenu->addAction(quitAct);

    config = new ConfigDialog();

    // Basic Tab
    QWidget* bswidget = new QWidget();
    QVBoxLayout* bsvbox = new QVBoxLayout();
    ifcCombo = new QComboBox();

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

    ifbCombo = new QComboBox();
    const QStringList items = { "ifb0", "ifb1" };
    ifbCombo->addItems(items);

    srcLine = new QLineEdit();
    srcLine->setText("0.0.0.0/0");

    dstLine = new QLineEdit();
    dstLine->setText("0.0.0.0/0");

    QGridLayout* sbox = new QGridLayout();
    int stindex = 0;
    sbox->addWidget(new QLabel(QObject::tr("Interface")), stindex, 0);
    sbox->addWidget(ifcCombo, stindex, 1);
    sbox->addWidget(new QLabel(QObject::tr("IFB")), stindex, 2);
    sbox->addWidget(ifbCombo, stindex++, 3);
    sbox->addWidget(new QLabel(QObject::tr("Source IP")), stindex, 0);
    sbox->addWidget(srcLine, stindex, 1);
    sbox->addWidget(new QLabel(QObject::tr("Destination IP")), stindex, 2);
    sbox->addWidget(dstLine, stindex++, 3);

    inboundDelayTimeSpin = new QDoubleSpinBox();
    inboundDelayTimeSpin->setRange(0.0, DELAY_MAX);
    inboundLossPercentSpin = new QDoubleSpinBox();
    inboundLossPercentSpin->setRange(0.0, LOSS_MAX);
    inboundRateRateSpin = new QDoubleSpinBox();
    inboundRateRateSpin->setRange(0.0, RATE_MAX);

    outboundDelayTimeSpin = new QDoubleSpinBox();
    outboundDelayTimeSpin->setRange(0.0, DELAY_MAX);
    outboundLossPercentSpin = new QDoubleSpinBox();
    outboundLossPercentSpin->setRange(0.0, LOSS_MAX);
    outboundRateRateSpin = new QDoubleSpinBox();
    outboundRateRateSpin->setRange(0.0, RATE_MAX);

    QGridLayout* bsbox = new QGridLayout();
    int bsindex = 0;
    bsbox->addWidget(new QLabel(QObject::tr("Dealy Time [ms]")), bsindex, 0);
    bsbox->addWidget(inboundDelayTimeSpin, bsindex, 1);
    bsbox->addWidget(new QLabel(QObject::tr("Dealy Time [ms]")), bsindex, 2);
    bsbox->addWidget(outboundDelayTimeSpin, bsindex++, 3);
    bsbox->addWidget(new QLabel(QObject::tr("Loss Percent [%]")), bsindex, 0);
    bsbox->addWidget(inboundLossPercentSpin, bsindex, 1);
    bsbox->addWidget(new QLabel(QObject::tr("Loss Percent [%]")), bsindex, 2);
    bsbox->addWidget(outboundLossPercentSpin, bsindex++, 3);
    bsbox->addWidget(new QLabel(QObject::tr("Rate Rate [kbit/s]")), bsindex, 0);
    bsbox->addWidget(inboundRateRateSpin, bsindex, 1);
    bsbox->addWidget(new QLabel(QObject::tr("Rate Rate [kbit/s]")), bsindex, 2);
    bsbox->addWidget(outboundRateRateSpin, bsindex++, 3);

    QHBoxLayout* bshbox = new QHBoxLayout();
    bshbox->addLayout(makeSeparator(QObject::tr("Inbound")));
    bshbox->addLayout(makeSeparator(QObject::tr("Outbound")));

    bsvbox->addLayout(sbox);
    bsvbox->addLayout(bshbox);
    bsvbox->addLayout(bsbox);
    bsvbox->addStretch();
    bswidget->setLayout(bsvbox);

    QHBoxLayout* tbox = new QHBoxLayout();
    clrButton = new QPushButton(QObject::tr("Clear"));
    aplButton = new QPushButton(QObject::tr("Apply"));
    aplButton->setCheckable(true);
    tbox->addWidget(clrButton);
    tbox->addWidget(aplButton);

    QWidget* central = new QWidget();
    QVBoxLayout* vbox = new QVBoxLayout();
    vbox->addWidget(bswidget);
    vbox->addLayout(tbox);
    central->setLayout(vbox);

    self->setCentralWidget(central);

    self->connect(clrButton, SIGNAL(clicked()), self, SLOT(onClearButtonClicked()));
    self->connect(aplButton, SIGNAL(toggled(bool)), self, SLOT(onApplyButtonToggled(bool)));
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


QHBoxLayout* MainWindowImpl::makeSeparator(QString text)
{
    QHBoxLayout* hbox = new QHBoxLayout();
    QFrame* line0 = new QFrame();
    line0->setFrameShape(QFrame::HLine);
    QFrame* line1 = new QFrame();
    line1->setFrameShape(QFrame::HLine);
    QLabel* label = new QLabel(text);
    label->setAlignment(Qt::AlignCenter);

    if(!text.isEmpty()) {
        hbox->addWidget(line0);
        hbox->addWidget(label);
        hbox->addWidget(line1);
    } else {
        hbox->addWidget(line0);
    }

    return hbox;
}


void MainWindow::onClearButtonClicked()
{
    impl->ifcCombo->setCurrentIndex(0);
    impl->ifbCombo->setCurrentIndex(0);
    impl->srcLine->setText("0.0.0.0/0");
    impl->dstLine->setText("0.0.0.0/0");

    impl->inboundDelayTimeSpin->setValue(0.0);
    impl->inboundLossPercentSpin->setValue(0.0);
    impl->inboundRateRateSpin->setValue(0.0);
    impl->outboundDelayTimeSpin->setValue(0.0);
    impl->outboundLossPercentSpin->setValue(0.0);
    impl->outboundRateRateSpin->setValue(0.0);
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
    impl->ifcCombo->setEnabled(!on);
    impl->ifbCombo->setEnabled(!on);
    impl->aplButton->setPalette(palette);
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
    string ifbName = ifbCombo->currentText().toStdString();
    string message = (boost::format("sudo modprobe ifb;"
                                    "sudo modprobe act_mirred;"
                                    "sudo ip link set dev %s up;")
                % ifbName.c_str()).str();
    onCommandExecute(message);
}


void MainWindowImpl::onTCClear()
{
    string ifcName = ifcCombo->currentText().toStdString();
    string ifbName = ifbCombo->currentText().toStdString();
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
    string ifbName = ifbCombo->currentText().toStdString();
    string message = (boost::format("sudo ip link set dev %s down;"
                                    "sudo rmmod ifb;")
                % ifbName.c_str()
                ).str();
    onCommandExecute(message);
}


void MainWindowImpl::onTCExecute()
{
    string ifcName = ifcCombo->currentText().toStdString();
    string ifbName = ifbCombo->currentText().toStdString();

    if(ifcName.empty()) {
        return;
    }

    double inboundDelayTime = inboundDelayTimeSpin->value();
    double inboundLossPercent = inboundLossPercentSpin->value();
    double inboundRateRate = inboundRateRateSpin->value();

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

    cout << inboundLimitPackets << endl;
    cout << inboundDelayJitter << endl;

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

    double outboundDelayTime = outboundDelayTimeSpin->value();
    double outboundLossPercent = outboundLossPercentSpin->value();
    double outboundRateRate = outboundRateRateSpin->value();

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

    string srcipName = srcLine->text().toStdString();
    string dstipName = dstLine->text().toStdString();
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
