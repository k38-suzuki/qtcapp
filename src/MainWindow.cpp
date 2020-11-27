#include "MainWindow.h"
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
#include <boost/format.hpp>
#include <iostream>
#include <sstream>
#include <stdlib.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

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
    QWidget* widget;

    QComboBox* ifcCombo;
    QComboBox* ifbCombo;
    QLineEdit* srcLine;
    QLineEdit* dstLine;

    QSpinBox* idelaySpin;
    QSpinBox* irateSpin;
    QSpinBox* ilossSpin;
    QSpinBox* odelaySpin;
    QSpinBox* orateSpin;
    QSpinBox* olossSpin;

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

    widget = new QWidget();

    QVBoxLayout* vbox = new QVBoxLayout();
    ifcCombo = new QComboBox();

    struct ifreq ifr[IFR_MAX];
    struct ifconf ifc;
    int fd = socket(AF_INET, SOCK_DGRAM, 0);

    ifc.ifc_len = sizeof(ifr);
    ifc.ifc_ifcu.ifcu_buf = (char*)ifr;
    ioctl(fd, SIOCGIFCONF, &ifc);
    int nifs = ifc.ifc_len / sizeof(struct ifreq);
    for (int j = 0; j < nifs; j++) {
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
    ilossSpin = new QSpinBox();
    ilossSpin->setRange(0, LOSS_MAX);
    odelaySpin = new QSpinBox();
    odelaySpin->setRange(0, DELAY_MAX);
    orateSpin = new QSpinBox();
    orateSpin->setRange(0, RATE_MAX);
    olossSpin = new QSpinBox();
    olossSpin->setRange(0, LOSS_MAX);

    QGridLayout* pbox = new QGridLayout();
    pbox->addWidget(new QLabel(QLabel::tr("Inbound Dealy [ms]")), 0, 0);
    pbox->addWidget(idelaySpin, 0, 1);
    pbox->addWidget(new QLabel(QLabel::tr("Inbound Rate [kbit/s]")), 1, 0);
    pbox->addWidget(irateSpin, 1, 1);
    pbox->addWidget(new QLabel(QLabel::tr("Inbound Loss [%]")), 2, 0);
    pbox->addWidget(ilossSpin, 2, 1);
    pbox->addWidget(new QLabel(QLabel::tr("Outbound Dealy [ms]")), 3, 0);
    pbox->addWidget(odelaySpin, 3, 1);
    pbox->addWidget(new QLabel(QLabel::tr("Outbound Rate [kbit/s]")), 4, 0);
    pbox->addWidget(orateSpin, 4, 1);
    pbox->addWidget(new QLabel(QLabel::tr("Outbound Loss [%]")), 5, 0);
    pbox->addWidget(olossSpin, 5, 1);

    QHBoxLayout* tbox = new QHBoxLayout();
    clrButton = new QPushButton(QPushButton::tr("Clear"));
    aplButton = new QPushButton(QPushButton::tr("Apply"));
    aplButton->setCheckable(true);
    tbox->addWidget(clrButton);
    tbox->addWidget(aplButton);

    vbox->addWidget(makeSeparator("Settings"));
    vbox->addLayout(sbox);
    vbox->addWidget(makeSeparator("Parameters"));
    vbox->addLayout(pbox);
    vbox->addWidget(makeSeparator(""));
    vbox->addLayout(tbox);
    widget->setLayout(vbox);
    self->setCentralWidget(widget);

    QObject::connect(clrButton, SIGNAL(clicked()), this->self, SLOT(onClearButtonClicked()));
    QObject::connect(aplButton, SIGNAL(toggled(bool)), this->self, SLOT(onApplyButtonClicked(bool)));
}


MainWindow::~MainWindow()
{
    impl->onTCFinalize();
    delete impl;
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
    impl->idelaySpin->setValue(0.0);
    impl->irateSpin->setValue(0);
    impl->ilossSpin->setValue(0);
    impl->odelaySpin->setValue(0);
    impl->orateSpin->setValue(0);
    impl->olossSpin->setValue(0);
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
    impl->aplButton->setPalette(palette);
}


void MainWindowImpl::onTCInitialize()
{
    string message = (
                boost::format("sudo modprobe ifb;\n"
                              "sudo modprobe act_mirred;\n"
                              "sudo ip link set dev %s up;\n")
                % ifbCombo->currentText().toStdString().c_str()).str();
    onCommandExecute(message);
}


void MainWindowImpl::onTCClear()
{
    string message = (
                boost::format("sudo tc qdisc del dev %s ingress;\n"
                              "sudo tc qdisc del dev %s root;\n"
                              "sudo tc qdisc del dev %s root;\n")
                % ifcCombo->currentText().toStdString().c_str()
                % ifbCombo->currentText().toStdString().c_str()
                % ifcCombo->currentText().toStdString().c_str()
                ).str();
    onCommandExecute(message);
}


void MainWindowImpl::onTCFinalize()
{
    string message = (
                boost::format("sudo ip link set dev %s down;\n"
                              "sudo rmmod ifb;\n")
                % ifbCombo->currentText().toStdString().c_str()
                ).str();
    onCommandExecute(message);
}


void MainWindowImpl::onTCExecute()
{
    if(ifcCombo->currentText().toStdString().empty()) {
        return;
    }

    string head[] = { " delay ", " rate ", " loss " };
    string unit[] = { "ms", "kbps", "%" };
    vector<double> value;
    value.push_back(idelaySpin->value());
    value.push_back(irateSpin->value());
    value.push_back(ilossSpin->value());
    value.push_back(odelaySpin->value());
    value.push_back(orateSpin->value());
    value.push_back(olossSpin->value());

    string effects[6];
    for(int i = 0; i < 6; i++) {
        if(value[i] > 0.0) {
            int index = i % 3;
            effects[i] = (boost::format("%s%3.2lf%s")
                          % head[index].c_str() % value[i] % unit[index].c_str()
                        ).str();
        }
        else {
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
                    boost::format("sudo tc qdisc add dev %s parent 1:2 handle 20: netem limit 2000%s%s%s;\n"
                                  "sudo tc filter add dev %s protocol ip parent 1: prio 2 u32 match ip src %s match ip dst %s flowid 1:2;\n")
                    % ifbCombo->currentText().toStdString().c_str() % effects[0].c_str() % effects[1].c_str() % effects[2].c_str()
                    % ifbCombo->currentText().toStdString().c_str() % dstipName.c_str() % srcipName.c_str()
                    ).str();
        srcMessage = (
                    boost::format("sudo tc qdisc add dev %s parent 1:2 handle 20: netem limit 2000%s%s%s;\n"
                                  "sudo tc filter add dev %s protocol ip parent 1: prio 2 u32 match ip src %s match ip dst %s flowid 1:2;\n")
                    % ifcCombo->currentText().toStdString().c_str() % effects[3].c_str() % effects[4].c_str() % effects[5].c_str()
                    % ifcCombo->currentText().toStdString().c_str() % srcipName.c_str() % dstipName.c_str()
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
                % ifcCombo->currentText().toStdString().c_str()
                % ifcCombo->currentText().toStdString().c_str() % ifbCombo->currentText().toStdString().c_str()
                % ifbCombo->currentText().toStdString().c_str()
                % ifbCombo->currentText().toStdString().c_str()
                % dstMessage.c_str()
                % ifcCombo->currentText().toStdString().c_str()
                % ifcCombo->currentText().toStdString().c_str()
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
        exit(EXIT_SUCCESS);
    }
}
