/*!
  @file
  @author Kenta Suzuki
*/

#include "MainWindow.h"
#include <QCheckBox>
#include <QComboBox>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMenuBar>
#include <QVBoxLayout>
#include <QPalette>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QTabWidget>
#include <boost/format.hpp>
#include <sstream>
#include <stdlib.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>

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
    ~MainWindowImpl();
    MainWindow* self;

    QComboBox* ifcCombo;
    QComboBox* ifbCombo;
    QLineEdit* srcLine;
    QLineEdit* dstLine;

    QDoubleSpinBox* inboundLimitPacketsSpin;
    QDoubleSpinBox* inboundDelayTimeSpin;
    QDoubleSpinBox* inboundDelayJitterSpin;
    QDoubleSpinBox* inboundDelayCorrelationSpin;
    QCheckBox* inboundDelayDistributionCheck;
    QComboBox* inboundDelayDistributionCombo;
    QCheckBox* inboundLossRandomCheck;
    QDoubleSpinBox* inboundLossPercentSpin;
    QDoubleSpinBox* inboundLossCorrelationSpin;
    QDoubleSpinBox* inboundCorruptPercentSpin;
    QDoubleSpinBox* inboundCorruptCorrelationSpin;
    QDoubleSpinBox* inboundDuplicationPercentSpin;
    QDoubleSpinBox* inboundDuplicationCorrelationSpin;
    QDoubleSpinBox* inboundReorderingPercentSpin;
    QDoubleSpinBox* inboundReorderingCorrelationSpin;
    QDoubleSpinBox* inboundReorderingDistanceSpin;
    QDoubleSpinBox* inboundRateRateSpin;
    QDoubleSpinBox* inboundRatePacketOverheadSpin;
    QDoubleSpinBox* inboundRateCellSizeSpin;
    QDoubleSpinBox* inboundRateCellOverheadSpin;

    QDoubleSpinBox* outboundLimitPacketsSpin;
    QDoubleSpinBox* outboundDelayTimeSpin;
    QDoubleSpinBox* outboundDelayJitterSpin;
    QDoubleSpinBox* outboundDelayCorrelationSpin;
    QCheckBox* outboundDelayDistributionCheck;
    QComboBox* outboundDelayDistributionCombo;
    QCheckBox* outboundLossRandomCheck;
    QDoubleSpinBox* outboundLossPercentSpin;
    QDoubleSpinBox* outboundLossCorrelationSpin;
    QDoubleSpinBox* outboundCorruptPercentSpin;
    QDoubleSpinBox* outboundCorruptCorrelationSpin;
    QDoubleSpinBox* outboundDuplicationPercentSpin;
    QDoubleSpinBox* outboundDuplicationCorrelationSpin;
    QDoubleSpinBox* outboundReorderingPercentSpin;
    QDoubleSpinBox* outboundReorderingCorrelationSpin;
    QDoubleSpinBox* outboundReorderingDistanceSpin;
    QDoubleSpinBox* outboundRateRateSpin;
    QDoubleSpinBox* outboundRatePacketOverheadSpin;
    QDoubleSpinBox* outboundRateCellSizeSpin;
    QDoubleSpinBox* outboundRateCellOverheadSpin;

    QAction* showAct;
    bool showCommands;

    QPushButton* clrButton;
    QPushButton* aplButton;

    QWidget* makeSeparator(QString text);

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
    QMenu* toolMenu = self->menuBar()->addMenu(QObject::tr("&Tool"));
    QMenu* helpMenu = self->menuBar()->addMenu(QObject::tr("&Help"));

    QAction* showAct = new QAction(QObject::tr("Show commands"));
    showAct->setCheckable(true);
    showAct->setChecked(false);
    editMenu->addAction(showAct);
    showCommands = false;

    QAction* quitAct = new QAction(QObject::tr("Quit"));
    fileMenu->addAction(quitAct);

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
    sbox->addWidget(ifcCombo, stindex++, 1);
    sbox->addWidget(new QLabel(QObject::tr("IFB")), stindex, 0);
    sbox->addWidget(ifbCombo, stindex++, 1);
    sbox->addWidget(new QLabel(QObject::tr("Source IP")), stindex, 0);
    sbox->addWidget(srcLine, stindex++, 1);
    sbox->addWidget(new QLabel(QObject::tr("Destination IP")), stindex, 0);
    sbox->addWidget(dstLine, stindex++, 1);

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
    bsbox->addWidget(new QLabel(QObject::tr("Inbound Dealy [ms]")), bsindex, 0);
    bsbox->addWidget(inboundDelayTimeSpin, bsindex++, 1);
    bsbox->addWidget(new QLabel(QObject::tr("Inbound Loss [%]")), bsindex, 0);
    bsbox->addWidget(inboundLossPercentSpin, bsindex++, 1);
    bsbox->addWidget(new QLabel(QObject::tr("Inbound Rate [kbit/s]")), bsindex, 0);
    bsbox->addWidget(inboundRateRateSpin, bsindex++, 1);
    bsbox->addWidget(new QLabel(QObject::tr("Outbound Dealy [ms]")), bsindex, 0);
    bsbox->addWidget(outboundDelayTimeSpin, bsindex++, 1);
    bsbox->addWidget(new QLabel(QObject::tr("Outbound Loss [%]")), bsindex, 0);
    bsbox->addWidget(outboundLossPercentSpin, bsindex++, 1);
    bsbox->addWidget(new QLabel(QObject::tr("Outbound Rate [kbit/s]")), bsindex, 0);
    bsbox->addWidget(outboundRateRateSpin, bsindex++, 1);

    bsvbox->addWidget(makeSeparator("Settings"));
    bsvbox->addLayout(sbox);
    bsvbox->addWidget(makeSeparator("Parameters"));
    bsvbox->addLayout(bsbox);
    bsvbox->addStretch();
    bswidget->setLayout(bsvbox);

    // Advanced Tab
    inboundLimitPacketsSpin = new QDoubleSpinBox();
    inboundLimitPacketsSpin->setRange(0.0, 10000.0);
    inboundLimitPacketsSpin->setValue(2000.0);
    inboundDelayJitterSpin = new QDoubleSpinBox();
    inboundDelayJitterSpin->setRange(0.0, DELAY_MAX);
    inboundDelayCorrelationSpin = new QDoubleSpinBox();
    inboundDelayCorrelationSpin->setRange(0.0, 100.0);
    inboundDelayDistributionCheck = new QCheckBox();
    inboundDelayDistributionCheck->setChecked(false);
    inboundDelayDistributionCombo = new QComboBox();
    inboundDelayDistributionCombo->setCurrentIndex(0);
    inboundLossRandomCheck = new QCheckBox();
    inboundLossRandomCheck->setChecked(false);
    inboundLossCorrelationSpin = new QDoubleSpinBox();
    inboundLossCorrelationSpin->setRange(0.0, LOSS_MAX);
    inboundCorruptPercentSpin = new QDoubleSpinBox();
    inboundCorruptPercentSpin->setRange(0.0, 100.0);
    inboundCorruptCorrelationSpin = new QDoubleSpinBox();
    inboundCorruptCorrelationSpin->setRange(0.0, 100.0);
    inboundDuplicationPercentSpin = new QDoubleSpinBox();
    inboundDuplicationPercentSpin->setRange(0.0, 100.00);
    inboundDuplicationCorrelationSpin = new QDoubleSpinBox();
    inboundDuplicationCorrelationSpin->setRange(0.0, 100.00);
    inboundReorderingPercentSpin = new QDoubleSpinBox();
    inboundReorderingPercentSpin->setRange(0.0, 100.0);
    inboundReorderingCorrelationSpin = new QDoubleSpinBox();
    inboundReorderingCorrelationSpin->setRange(0.0, 100.0);
    inboundReorderingDistanceSpin = new QDoubleSpinBox();
    inboundReorderingDistanceSpin->setRange(0.0, 100);
    inboundRatePacketOverheadSpin = new QDoubleSpinBox();
    inboundRatePacketOverheadSpin->setRange(0.0, 100.0);
    inboundRateCellSizeSpin = new QDoubleSpinBox();
    inboundRateCellSizeSpin->setRange(0.0, 100.0);
    inboundRateCellOverheadSpin = new QDoubleSpinBox();
    inboundRateCellOverheadSpin->setRange(0.0, 100.0);

    outboundLimitPacketsSpin = new QDoubleSpinBox();
    outboundLimitPacketsSpin->setRange(0.0, 10000.0);
    outboundLimitPacketsSpin->setValue(2000.0);
    outboundDelayJitterSpin = new QDoubleSpinBox();
    outboundDelayJitterSpin->setRange(0.0, DELAY_MAX);
    outboundDelayCorrelationSpin = new QDoubleSpinBox();
    outboundDelayCorrelationSpin->setRange(0.0, 100.0);
    outboundDelayDistributionCheck = new QCheckBox();
    outboundDelayDistributionCheck->setChecked(false);
    outboundDelayDistributionCombo = new QComboBox();
    outboundDelayDistributionCombo->setCurrentIndex(0);
    outboundLossRandomCheck = new QCheckBox();
    outboundLossRandomCheck->setChecked(false);
    outboundLossCorrelationSpin = new QDoubleSpinBox();
    outboundLossCorrelationSpin->setRange(0.0, LOSS_MAX);
    outboundCorruptPercentSpin = new QDoubleSpinBox();
    outboundCorruptPercentSpin->setRange(0.0, 100.0);
    outboundCorruptCorrelationSpin = new QDoubleSpinBox();
    outboundCorruptCorrelationSpin->setRange(0.0, 100.0);
    outboundDuplicationPercentSpin = new QDoubleSpinBox();
    outboundDuplicationPercentSpin->setRange(0.0, 100.00);
    outboundDuplicationCorrelationSpin = new QDoubleSpinBox();
    outboundDuplicationCorrelationSpin->setRange(0.0, 100.00);
    outboundReorderingPercentSpin = new QDoubleSpinBox();
    outboundReorderingPercentSpin->setRange(0.0, 100.0);
    outboundReorderingCorrelationSpin = new QDoubleSpinBox();
    outboundReorderingCorrelationSpin->setRange(0.0, 100.0);
    outboundReorderingDistanceSpin = new QDoubleSpinBox();
    outboundReorderingDistanceSpin->setRange(0.0, 100);
    outboundRatePacketOverheadSpin = new QDoubleSpinBox();
    outboundRatePacketOverheadSpin->setRange(0.0, 100.0);
    outboundRateCellSizeSpin = new QDoubleSpinBox();
    outboundRateCellSizeSpin->setRange(0.0, 100.0);
    outboundRateCellOverheadSpin = new QDoubleSpinBox();
    outboundRateCellOverheadSpin->setRange(0.0, 100.0);

    QGridLayout* adbox = new QGridLayout();
    int adindex = 0;
    adbox->addWidget(new QLabel(QObject::tr("Inbound Jitter [ms]")), adindex, 0);
    adbox->addWidget(inboundDelayJitterSpin, adindex++, 1);
    adbox->addWidget(new QLabel(QObject::tr("Inbound Duplication [%]")), adindex, 0);
    adbox->addWidget(inboundDuplicationPercentSpin, adindex++, 1);
    adbox->addWidget(new QLabel(QObject::tr("Inbound Corruption [%]")), adindex, 0);
    adbox->addWidget(inboundCorruptPercentSpin, adindex++, 1);
    adbox->addWidget(new QLabel(QObject::tr("Inbound Re-ordering [%]")), adindex, 0);
    adbox->addWidget(inboundReorderingPercentSpin, adindex++, 1);
    adbox->addWidget(new QLabel(QObject::tr("Inbound Correlation [%]")), adindex, 0);
    adbox->addWidget(inboundReorderingCorrelationSpin, adindex++, 1);
    adbox->addWidget(new QLabel(QObject::tr("Inbound Gap [distance]")), adindex, 0);
    adbox->addWidget(inboundReorderingDistanceSpin, adindex++, 1);
    adbox->addWidget(new QLabel(QObject::tr("Inbound Burst Loss [%]")), adindex, 0);
    adbox->addWidget(inboundLossCorrelationSpin, adindex++, 1);
    adbox->addWidget(new QLabel(QObject::tr("Outbound Jitter [ms]")), adindex, 0);
    adbox->addWidget(outboundDelayJitterSpin, adindex++, 1);
    adbox->addWidget(new QLabel(QObject::tr("Outbound Duplication [%]")), adindex, 0);
    adbox->addWidget(outboundDuplicationPercentSpin, adindex++, 1);
    adbox->addWidget(new QLabel(QObject::tr("Outbound Corruption [%]")), adindex, 0);
    adbox->addWidget(outboundCorruptPercentSpin, adindex++, 1);
    adbox->addWidget(new QLabel(QObject::tr("Outbound Re-ordering [%]")), adindex, 0);
    adbox->addWidget(outboundReorderingPercentSpin, adindex++, 1);
    adbox->addWidget(new QLabel(QObject::tr("Outbound Correlation [%]")), adindex, 0);
    adbox->addWidget(outboundReorderingCorrelationSpin, adindex++, 1);
    adbox->addWidget(new QLabel(QObject::tr("Outbound  Gap [distance]")), adindex, 0);
    adbox->addWidget(outboundReorderingDistanceSpin, adindex++, 1);
    adbox->addWidget(new QLabel(QObject::tr("Outbound Burst Loss [%]")), adindex, 0);
    adbox->addWidget(outboundLossCorrelationSpin, adindex++, 1);

    QWidget* adwidget = new QWidget();
    QVBoxLayout* advbox = new QVBoxLayout();
    advbox->addWidget(makeSeparator("Advanced Parameters"));
    advbox->addLayout(adbox);
    advbox->addStretch();
    adwidget->setLayout(advbox);

    QHBoxLayout* tbox = new QHBoxLayout();
    clrButton = new QPushButton(QObject::tr("Clear"));
    aplButton = new QPushButton(QObject::tr("Apply"));
    aplButton->setCheckable(true);
    tbox->addWidget(clrButton);
    tbox->addWidget(aplButton);

    QTabWidget* tab = new QTabWidget();
    tab->addTab(bswidget, QObject::tr("Basic"));
    tab->addTab(adwidget, QObject::tr("Advanced"));

    QWidget* central = new QWidget();
    QVBoxLayout* vbox = new QVBoxLayout();
    vbox->addWidget(tab);
    vbox->addLayout(tbox);
    central->setLayout(vbox);

    self->setCentralWidget(central);

    QObject::connect(clrButton, SIGNAL(clicked()), self, SLOT(onClearButtonClicked()));
    QObject::connect(aplButton, SIGNAL(toggled(bool)), self, SLOT(onApplyButtonToggled(bool)));
    QObject::connect(showAct, SIGNAL(triggered(bool)), self, SLOT(onShowActionTriggered(bool)));
    QObject::connect(ifbCombo, SIGNAL(currentTextChanged(QString)), self, SLOT(onCurrentIFBChanged(QString)));
    QObject::connect(quitAct, SIGNAL(triggered()), self, SLOT(close()));
}


MainWindow::~MainWindow()
{
    impl->onTCFinalize();
    delete impl;
}


MainWindowImpl::~MainWindowImpl()
{

}


QWidget* MainWindowImpl::makeSeparator(QString text)
{
    QWidget* separator = new QWidget();
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
    separator->setLayout(hbox);

    return separator;
}


void MainWindow::onClearButtonClicked()
{
    impl->ifcCombo->setCurrentIndex(0);
    impl->ifbCombo->setCurrentIndex(0);
    impl->srcLine->setText("0.0.0.0/0");
    impl->dstLine->setText("0.0.0.0/0");

    impl->inboundLimitPacketsSpin->setValue(0.0);
    impl->inboundDelayTimeSpin->setValue(0.0);
    impl->inboundDelayJitterSpin->setValue(0.0);
    impl->inboundDelayCorrelationSpin->setValue(0.0);
    impl->inboundDelayDistributionCheck->setChecked(false);
    impl->inboundDelayDistributionCombo->setCurrentIndex(0);
    impl->inboundLossRandomCheck->setChecked(false);
    impl->inboundLossPercentSpin->setValue(0.0);
    impl->inboundLossCorrelationSpin->setValue(0.0);
    impl->inboundCorruptPercentSpin->setValue(0.0);
    impl->inboundCorruptCorrelationSpin->setValue(0.0);
    impl->inboundDuplicationPercentSpin->setValue(0.0);
    impl->inboundDuplicationCorrelationSpin->setValue(0.0);
    impl->inboundReorderingPercentSpin->setValue(0.0);
    impl->inboundReorderingCorrelationSpin->setValue(0.0);
    impl->inboundReorderingDistanceSpin->setValue(0.0);
    impl->inboundRateRateSpin->setValue(0.0);
    impl->inboundRatePacketOverheadSpin->setValue(0.0);
    impl->inboundRateCellSizeSpin->setValue(0.0);
    impl->inboundRateCellOverheadSpin->setValue(0.0);

    impl->outboundLimitPacketsSpin->setValue(0.0);
    impl->outboundDelayTimeSpin->setValue(0.0);
    impl->outboundDelayJitterSpin->setValue(0.0);
    impl->outboundDelayCorrelationSpin->setValue(0.0);
    impl->outboundDelayDistributionCheck->setChecked(false);
    impl->outboundDelayDistributionCombo->setCurrentIndex(0);
    impl->outboundLossRandomCheck->setChecked(false);
    impl->outboundLossPercentSpin->setValue(0.0);
    impl->outboundLossCorrelationSpin->setValue(0.0);
    impl->outboundCorruptPercentSpin->setValue(0.0);
    impl->outboundCorruptCorrelationSpin->setValue(0.0);
    impl->outboundDuplicationPercentSpin->setValue(0.0);
    impl->outboundDuplicationCorrelationSpin->setValue(0.0);
    impl->outboundReorderingPercentSpin->setValue(0.0);
    impl->outboundReorderingCorrelationSpin->setValue(0.0);
    impl->outboundReorderingDistanceSpin->setValue(0.0);
    impl->outboundRateRateSpin->setValue(0.0);
    impl->outboundRatePacketOverheadSpin->setValue(0.0);
    impl->outboundRateCellSizeSpin->setValue(0.0);
    impl->outboundRateCellOverheadSpin->setValue(0.0);
}


void MainWindow::onApplyButtonToggled(bool on)
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


void MainWindow::onShowActionTriggered(bool on)
{
    impl->showCommands = on;
}


void MainWindow::onCurrentIFBChanged(QString ifbName)
{
    impl->onTCInitialize();
}


void MainWindowImpl::onTCInitialize()
{
    string ifbName = ifbCombo->currentText().toStdString();
    string message = (boost::format("sudo modprobe ifb;\n"
                                    "sudo modprobe act_mirred;\n"
                                    "sudo ip link set dev %s up;\n")
                % ifbName.c_str()).str();
    onCommandExecute(message);
}


void MainWindowImpl::onTCClear()
{
    string ifcName = ifcCombo->currentText().toStdString();
    string ifbName = ifbCombo->currentText().toStdString();
    string message = (boost::format("sudo tc qdisc del dev %s ingress;\n"
                                    "sudo tc qdisc del dev %s root;\n"
                                    "sudo tc qdisc del dev %s root;\n")
                % ifcName.c_str()
                % ifbName.c_str()
                % ifcName.c_str()
                ).str();
    onCommandExecute(message);
}


void MainWindowImpl::onTCFinalize()
{
    string ifbName = ifbCombo->currentText().toStdString();
    string message = (boost::format("sudo ip link set dev %s down;\n"
                                    "sudo rmmod ifb;\n")
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

    double inboundLimitPackets = inboundLimitPacketsSpin->value();
    double inboundDelayTime = inboundDelayTimeSpin->value();
    double inboundDelayJitter = inboundDelayJitterSpin->value();
    double inboundDelayCorrelation = inboundDelayCorrelationSpin->value();
    bool isCheckedInboundDelayDistribution = inboundDelayDistributionCheck->isChecked();
    string inboundDelayDistribution = inboundDelayDistributionCombo->currentText().toStdString();
    bool isCheckedInboundLossRandom = inboundLossRandomCheck->isChecked();
    double inboundLossPercent = inboundLossPercentSpin->value();
    double inboundLossCorrelation = inboundLossCorrelationSpin->value();
    double inboundCorruptPercent = inboundCorruptPercentSpin->value();
    double inboundCorruptCorrelation = inboundCorruptCorrelationSpin->value();
    double inboundDuplicationPercent = inboundDuplicationPercentSpin->value();
    double inboundDuplicationCorrelation = inboundDuplicationCorrelationSpin->value();
    double inboundReorderingPercent = inboundReorderingPercentSpin->value();
    double inboundReorderingCorrelation = inboundReorderingCorrelationSpin->value();
    double inboundReorderingDistance = inboundReorderingDistanceSpin->value();
    double inboundRateRate = inboundRateRateSpin->value();
    double inboundRatePacketOverhead = inboundRatePacketOverheadSpin->value();
    double inboundRateCellSize = inboundRateCellSizeSpin->value();
    double inboundRateCellOverhead = inboundRateCellOverheadSpin->value();

    double outboundLimitPackets = outboundLimitPacketsSpin->value();
    double outboundDelayTime = outboundDelayTimeSpin->value();
    double outboundDelayJitter = outboundDelayJitterSpin->value();
    double outboundDelayCorrelation = outboundDelayCorrelationSpin->value();
    bool isCheckedOutboundDelayDistribution = outboundDelayDistributionCheck->isChecked();
    string outboundDelayDistribution = outboundDelayDistributionCombo->currentText().toStdString();
    bool isCheckedOutboundLossRandom = outboundLossRandomCheck->isChecked();
    double outboundLossPercent = outboundLossPercentSpin->value();
    double outboundLossCorrelation = outboundLossCorrelationSpin->value();
    double outboundCorruptPercent = outboundCorruptPercentSpin->value();
    double outboundCorruptCorrelation = outboundCorruptCorrelationSpin->value();
    double outboundDuplicationPercent = outboundDuplicationPercentSpin->value();
    double outboundDuplicationCorrelation = outboundDuplicationCorrelationSpin->value();
    double outboundReorderingPercent = outboundReorderingPercentSpin->value();
    double outboundReorderingCorrelation = outboundReorderingCorrelationSpin->value();
    double outboundReorderingDistance = outboundReorderingDistanceSpin->value();
    double outboundRateRate = outboundRateRateSpin->value();
    double outboundRatePacketOverhead = outboundRatePacketOverheadSpin->value();
    double outboundRateCellSize = outboundRateCellSizeSpin->value();
    double outboundRateCellOverhead = outboundRateCellOverheadSpin->value();

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
                    boost::format("sudo tc qdisc add dev %s parent 1:2 handle 20: netem %s;\n"
                                  "sudo tc filter add dev %s protocol ip parent 1: prio 2 u32 match ip src %s match ip dst %s flowid 1:2;\n")
                    % ifbName.c_str() % inboundEffects.c_str()
                    % ifbName.c_str() % dstipName.c_str() % srcipName.c_str()
                    ).str();
        srcMessage = (
                    boost::format("sudo tc qdisc add dev %s parent 1:2 handle 20: netem %s;\n"
                                  "sudo tc filter add dev %s protocol ip parent 1: prio 2 u32 match ip src %s match ip dst %s flowid 1:2;\n")
                    % ifcName.c_str() % outboundEffects.c_str()
                    % ifcName.c_str() % srcipName.c_str() % dstipName.c_str()
                    ).str();
    }
    else {
        dstMessage.clear();
        srcMessage.clear();
    }

    string message = (
                boost::format("sudo tc qdisc add dev %s ingress handle ffff:;\n"
                              "sudo tc filter add dev %s parent ffff: protocol ip u32 match u32 0 0 action mirred egress redirect dev %s;\n"
                              "sudo tc qdisc add dev %s root handle 1: "
                              "prio bands 16 priomap 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0;\n"
                              "sudo tc qdisc add dev %s parent 1:1 handle 10: netem limit 2000;\n"
                              "%s"
                              "sudo tc qdisc add dev %s root handle 1: "
                              "prio bands 16 priomap 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0;\n"
                              "sudo tc qdisc add dev %s parent 1:1 handle 10: netem limit 2000;\n"
                              "%s")
                % ifcName.c_str()
                % ifcName.c_str() % ifbName.c_str()
                % ifbName.c_str()
                % ifbName.c_str()
                % dstMessage.c_str()
                % ifcName.c_str()
                % ifcName.c_str()
                % srcMessage.c_str()
                ).str();

    onCommandExecute(message);
}


void MainWindowImpl::onCommandExecute(const string& message)
{
    pid_t pid = fork();
    if(pid == -1) {
        exit(EXIT_FAILURE);
    } else if(pid == 0) {
//        int ret = system(message.c_str());
        if(showCommands) {
            cout << message << endl;
        }
        exit(EXIT_SUCCESS);
    }
}
