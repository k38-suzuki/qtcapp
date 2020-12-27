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
#include <QVBoxLayout>
#include <QPalette>
#include <QPushButton>
#include <QSpinBox>
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

    QSpinBox* idelaySpin;
    QSpinBox* irateSpin;
    QDoubleSpinBox* ilossSpin;
    QSpinBox* odelaySpin;
    QSpinBox* orateSpin;
    QDoubleSpinBox* olossSpin;

    QSpinBox* ijitterSpin;
    QDoubleSpinBox* iduplicateSpin;
    QDoubleSpinBox* icorruptSpin;
    QSpinBox* igapSpin;
    QDoubleSpinBox* ireorderingSpin;
    QDoubleSpinBox* icorrelationSpin;
    QSpinBox* ibLossSpin;
    QSpinBox* ojitterSpin;
    QSpinBox* obLossSpin;
    QDoubleSpinBox* oduplicateSpin;
    QDoubleSpinBox* ocorruptSpin;
    QSpinBox* ogapSpin;
    QDoubleSpinBox* oreorderingSpin;
    QDoubleSpinBox* ocorrelationSpin;

    QCheckBox* showCheck;

    QPushButton* settingsButton;
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

    // Base Tab
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
    ifbCombo->setEnabled(false);

    srcLine = new QLineEdit();
    srcLine->setText("0.0.0.0/0");

    dstLine = new QLineEdit();
    dstLine->setText("0.0.0.0/0");

    QGridLayout* sbox = new QGridLayout();
    sbox->addWidget(new QLabel(QLabel::tr("Interface")), 0, 0);
    sbox->addWidget(ifcCombo, 0, 1);
    sbox->addWidget(new QLabel(QLabel::tr("IFB")), 1, 0);
    sbox->addWidget(ifbCombo, 1, 1);
    sbox->addWidget(new QLabel(QLabel::tr("Source IP")), 2, 0);
    sbox->addWidget(srcLine, 2, 1);
    sbox->addWidget(new QLabel(QLabel::tr("Destination IP")), 3, 0);
    sbox->addWidget(dstLine, 3, 1);

    idelaySpin = new QSpinBox();
    idelaySpin->setRange(0, DELAY_MAX);
    irateSpin = new QSpinBox();
    irateSpin->setRange(0, RATE_MAX);
    ilossSpin = new QDoubleSpinBox();
    ilossSpin->setRange(0, LOSS_MAX);
    odelaySpin = new QSpinBox();
    odelaySpin->setRange(0, DELAY_MAX);
    orateSpin = new QSpinBox();
    orateSpin->setRange(0, RATE_MAX);
    olossSpin = new QDoubleSpinBox();
    olossSpin->setRange(0, LOSS_MAX);

    QGridLayout* bsbox = new QGridLayout();
    int bsindex = 0;
    bsbox->addWidget(new QLabel(QLabel::tr("Inbound Dealy [ms]")), bsindex, 0);
    bsbox->addWidget(idelaySpin, bsindex++, 1);
    bsbox->addWidget(new QLabel(QLabel::tr("Inbound Rate [kbit/s]")), bsindex, 0);
    bsbox->addWidget(irateSpin, bsindex++, 1);
    bsbox->addWidget(new QLabel(QLabel::tr("Inbound Loss [%]")), bsindex, 0);
    bsbox->addWidget(ilossSpin, bsindex++, 1);
    bsbox->addWidget(new QLabel(QLabel::tr("Outbound Dealy [ms]")), bsindex, 0);
    bsbox->addWidget(odelaySpin, bsindex++, 1);
    bsbox->addWidget(new QLabel(QLabel::tr("Outbound Rate [kbit/s]")), bsindex, 0);
    bsbox->addWidget(orateSpin, bsindex++, 1);
    bsbox->addWidget(new QLabel(QLabel::tr("Outbound Loss [%]")), bsindex, 0);
    bsbox->addWidget(olossSpin, bsindex++, 1);

    QHBoxLayout* tbox = new QHBoxLayout();
    clrButton = new QPushButton(QPushButton::tr("Clear"));
    aplButton = new QPushButton(QPushButton::tr("Apply"));
    aplButton->setCheckable(true);
    tbox->addWidget(clrButton);
    tbox->addWidget(aplButton);

    showCheck = new QCheckBox();
    showCheck->setText(QCheckBox::tr("Show commands"));
    showCheck->setChecked(true);

    QHBoxLayout* asbox = new QHBoxLayout();
    asbox->addStretch();
    asbox->addWidget(showCheck);

    bsvbox->addWidget(makeSeparator("Settings"));
    bsvbox->addLayout(sbox);
    bsvbox->addWidget(makeSeparator("Parameters"));
    bsvbox->addLayout(bsbox);
    bsvbox->addLayout(asbox);
    bsvbox->addWidget(makeSeparator(""));
    bsvbox->addLayout(tbox);
    bswidget->setLayout(bsvbox);

    // Advanced Tab
    ijitterSpin = new QSpinBox();
    ijitterSpin->setRange(0, DELAY_MAX);
    iduplicateSpin = new QDoubleSpinBox();
    iduplicateSpin->setRange(0, 100.00);
    icorruptSpin = new QDoubleSpinBox();
    icorruptSpin->setRange(0, 100.0);
    ireorderingSpin = new QDoubleSpinBox();
    ireorderingSpin->setRange(0, 100.0);
    icorrelationSpin = new QDoubleSpinBox();
    icorrelationSpin->setRange(0, 100.0);
    igapSpin = new QSpinBox();
    igapSpin->setRange(0, 100);
    ibLossSpin = new QSpinBox();
    ibLossSpin->setRange(0, LOSS_MAX);
    ojitterSpin = new QSpinBox();
    ojitterSpin->setRange(0, DELAY_MAX);
    obLossSpin = new QSpinBox();
    obLossSpin->setRange(0, LOSS_MAX);
    oduplicateSpin = new QDoubleSpinBox();
    oduplicateSpin->setRange(0, 100.0);
    ocorruptSpin = new QDoubleSpinBox();
    ocorruptSpin->setRange(0, 100.0);
    oreorderingSpin = new QDoubleSpinBox();
    oreorderingSpin->setRange(0, 100.0);
    ocorrelationSpin = new QDoubleSpinBox();
    ocorrelationSpin->setRange(0, 100.0);
    ogapSpin = new QSpinBox();
    ogapSpin->setRange(0, 100);

    QGridLayout* adbox = new QGridLayout();
    int adindex = 0;
    adbox->addWidget(new QLabel(QLabel::tr("Inbound Jitter [ms]")), adindex, 0);
    adbox->addWidget(ijitterSpin, adindex++, 1);
    adbox->addWidget(new QLabel(QLabel::tr("Inbound Duplication [%]")), adindex, 0);
    adbox->addWidget(iduplicateSpin, adindex++, 1);
    adbox->addWidget(new QLabel(QLabel::tr("Inbound Corruption [%]")), adindex, 0);
    adbox->addWidget(icorruptSpin, adindex++, 1);
    adbox->addWidget(new QLabel(QLabel::tr("Inbound Re-ordering [%]")), adindex, 0);
    adbox->addWidget(ireorderingSpin, adindex++, 1);
    adbox->addWidget(new QLabel(QLabel::tr("Inbound Correlation [%]")), adindex, 0);
    adbox->addWidget(icorrelationSpin, adindex++, 1);
    adbox->addWidget(new QLabel(QLabel::tr("Inbound Gap [distance]")), adindex, 0);
    adbox->addWidget(igapSpin, adindex++, 1);
    adbox->addWidget(new QLabel(QLabel::tr("Inbound Burst Loss [%]")), adindex, 0);
    adbox->addWidget(ibLossSpin, adindex++, 1);
    adbox->addWidget(new QLabel(QLabel::tr("Outbound Jitter [ms]")), adindex, 0);
    adbox->addWidget(ojitterSpin, adindex++, 1);
    adbox->addWidget(new QLabel(QLabel::tr("Outbound Duplication [%]")), adindex, 0);
    adbox->addWidget(oduplicateSpin, adindex++, 1);
    adbox->addWidget(new QLabel(QLabel::tr("Outbound Corruption [%]")), adindex, 0);
    adbox->addWidget(ocorruptSpin, adindex++, 1);
    adbox->addWidget(new QLabel(QLabel::tr("Outbound Re-ordering [%]")), adindex, 0);
    adbox->addWidget(oreorderingSpin, adindex++, 1);
    adbox->addWidget(new QLabel(QLabel::tr("Outbound Correlation [%]")), adindex, 0);
    adbox->addWidget(ocorrelationSpin, adindex++, 1);
    adbox->addWidget(new QLabel(QLabel::tr("Outbound  Gap [distance]")), adindex, 0);
    adbox->addWidget(ogapSpin, adindex++, 1);
    adbox->addWidget(new QLabel(QLabel::tr("Outbound Burst Loss [%]")), adindex, 0);
    adbox->addWidget(obLossSpin, adindex++, 1);

    QWidget* adwidget = new QWidget();
    QVBoxLayout* advbox = new QVBoxLayout();
    advbox->addWidget(makeSeparator("Advanced Parameters"));
    advbox->addLayout(adbox);
    advbox->addStretch();
    adwidget->setLayout(advbox);

    QTabWidget* tab = new QTabWidget();
    tab->addTab(bswidget, QTabWidget::tr("Base"));
    tab->addTab(adwidget, QTabWidget::tr("Advanced"));

    self->setCentralWidget(tab);

    QObject::connect(clrButton, SIGNAL(clicked()), self, SLOT(onClearButtonClicked()));
    QObject::connect(aplButton, SIGNAL(toggled(bool)), self, SLOT(onApplyButtonClicked(bool)));
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
    impl->idelaySpin->setValue(0);
    impl->irateSpin->setValue(0);
    impl->ilossSpin->setValue(0.0);
    impl->odelaySpin->setValue(0);
    impl->orateSpin->setValue(0);
    impl->olossSpin->setValue(0.0);

    impl->ijitterSpin->setValue(0);
    impl->iduplicateSpin->setValue(0.0);
    impl->icorruptSpin->setValue(0.0);
    impl->ireorderingSpin->setValue(0.0);
    impl->icorrelationSpin->setValue(0.0);
    impl->igapSpin->setValue(0);
    impl->ibLossSpin->setValue(0);
    impl->ojitterSpin->setValue(0);
    impl->oduplicateSpin->setValue(0.0);
    impl->ocorruptSpin->setValue(0.0);
    impl->oreorderingSpin->setValue(0.0);
    impl->ocorrelationSpin->setValue(0.0);
    impl->ogapSpin->setValue(0);
    impl->obLossSpin->setValue(0);
}


void MainWindow::onApplyButtonClicked(bool on)
{
    QPalette palette;

    if(on) {
        palette.setColor(QPalette::Button, QColor(Qt::red));
        impl->onTCExecute();
    } else {
        impl->onTCClear();
    }
    impl->ifcCombo->setEnabled(!on);
    impl->aplButton->setPalette(palette);
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

    string head[] = { " delay ", " rate ", " loss ", " duplicate ", " corrupt " };
    string unit[] = { "ms", "kbps", "%", "%", "%" };
    vector<double> value;
    value.push_back(idelaySpin->value());
    value.push_back(irateSpin->value());
    value.push_back(ilossSpin->value());
    value.push_back(iduplicateSpin->value());
    value.push_back(icorruptSpin->value());
    value.push_back(odelaySpin->value());
    value.push_back(orateSpin->value());
    value.push_back(olossSpin->value());
    value.push_back(oduplicateSpin->value());
    value.push_back(ocorruptSpin->value());

    string effects[10];
    for(int i = 0; i < 10; i++) {
        if(value[i] > 0.0) {
            int index = i % 5;
            effects[i] = (boost::format("%s%3.2lf%s")
                          % head[index].c_str() % value[i] % unit[index].c_str()
                        ).str();
            if(i == 0) {
                double idelay = idelaySpin->value();
                double ijitter = ijitterSpin->value();
                double ireordering = ireorderingSpin->value();
                double icorrelation = icorrelationSpin->value();
                int igap = igapSpin->value();
                if((idelay > 0.0) && (ijitter > 0.0) && (ijitter <= idelay)) {
                    effects[i] += (boost::format(" %dms")
                                   % ijitter
                                ).str();
                }
                if((idelay > 0.0) && (ireordering > 0.0) && (icorrelation > 0.0)) {
                    effects[i] += (boost::format(" reorder %d%s %d%s")
                                   % ireordering  % ("%") % icorrelation % ("%")
                                   ).str();
                }
                if((idelay > 0.0) && (ireordering > 0.0) && (icorrelation > 0.0) && (igap > 0)) {
                    effects[i] += (boost::format(" gap %d")
                                   % igap
                                ).str();
                }
            } else if(i == 2) {
                double iloss = ilossSpin->value();
                double ibloss = ibLossSpin->value();

                if((iloss > 0.0) && (ibloss > 0.0)) {
                    effects[i] += (boost::format(" %d%s")
                                   % ibloss % ("%")
                                ).str();
                }

            } else if(i == 5) {
                double odelay = odelaySpin->value();
                double ojitter = ojitterSpin->value();
                double oreordering = oreorderingSpin->value();
                double ocorrelation = ocorrelationSpin->value();
                int ogap = ogapSpin->value();
                if((odelay > 0.0) && (ojitter > 0.0) && (ojitter <= odelay)) {
                    effects[i] += (boost::format(" %dms")
                                   % ojitter
                                ).str();                }
                if((odelay > 0.0) && (oreordering > 0.0) && (ocorrelation > 0.0)) {
                    effects[i] += (boost::format(" reorder %d%s %d%s")
                                   % oreordering  % ("%") % ocorrelation % ("%")
                                   ).str();
                }
                if((odelay > 0.0) && (oreordering > 0.0) && (ocorrelation > 0.0) && (ogap > 0)) {
                    effects[i] += (boost::format(" gap %d")
                                   % ogap
                                ).str();
                }
            } else if(i == 7) {
                double oloss = olossSpin->value();
                double obloss = obLossSpin->value();

                if((oloss > 0.0) && (obloss > 0.0)) {
                    effects[i] += (boost::format(" %d%s")
                                   % obloss % ("%")
                                ).str();
                }
            }
        } else {
            effects[i].clear();
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
                    boost::format("sudo tc qdisc add dev %s parent 1:2 handle 20: netem limit 2000%s%s%s%s%s;\n"
                                  "sudo tc filter add dev %s protocol ip parent 1: prio 2 u32 match ip src %s match ip dst %s flowid 1:2;\n")
                    % ifbName.c_str() % effects[0].c_str() % effects[1].c_str() % effects[2].c_str() % effects[3].c_str() % effects[4].c_str()
                    % ifbName.c_str() % dstipName.c_str() % srcipName.c_str()
                    ).str();
        srcMessage = (
                    boost::format("sudo tc qdisc add dev %s parent 1:2 handle 20: netem limit 2000%s%s%s%s%s;\n"
                                  "sudo tc filter add dev %s protocol ip parent 1: prio 2 u32 match ip src %s match ip dst %s flowid 1:2;\n")
                    % ifcName.c_str() % effects[5].c_str() % effects[6].c_str() % effects[7].c_str() % effects[8].c_str() % effects[9].c_str()
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
        int ret = system(message.c_str());
        bool on = showCheck->isChecked();
        if(on) {
            cout << message << endl;
        }
        exit(EXIT_SUCCESS);
    }
}
