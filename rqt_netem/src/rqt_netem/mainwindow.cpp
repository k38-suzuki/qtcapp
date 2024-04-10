/**
   @author Kenta Suzuki
*/

#include "rqt_netem/mainwindow.h"

#include <QAction>
#include <QBoxLayout>
#include <QByteArray>
#include <QCheckBox>
#include <QComboBox>
#include <QDebug>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QGridLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QProcess>
#include <QPushButton>
#include <QToolBar>
#include <QValidator>

#include <boost/format.hpp>
#include <stdlib.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include "rqt_netem/bash.h"

namespace {

bool checkAddress(const QString& address)
{
    QString input = address;
    QStringList list = input.split("/");
    if(list.size() != 2) {
        return false;
    }
    QStringList list2 = list.at(0).split(".");
    if(list2.size() != 4) {
        return false;
    }
    for(int i = 0; i < 4; ++i) {
        int element = list2.at(i).toInt();
        if(element < 0 || element > 255) {
            return false;
        }
    }
    int mask = list.at(1).toInt();
    if(mask < 0 || mask > 32) {
        return false;
    }

    return true;    
}

class IPv4Validator : public QValidator
{
public:
    IPv4Validator(QObject* parent = nullptr)
        : QValidator(parent)
    {

    }

    virtual QValidator::State validate(QString& input, int& pos) const override
    {
        return checkAddress(input) ? QValidator::Acceptable : QValidator::Invalid;
    }
};

const QStringList nameList = {
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

struct OptionInfo {
    double limit_packets = 2000.0;
    double delay_jitter = 0.0;
    double delay_correlation = 0.0;
    QString delay_distribution = "disabled";
    QString loss_random = "disabled";
    double loss_correlation = 0.0;
    double duplication_percent = 0.0;
    double duplication_correlation = 0.0;
    double corruption_percent = 0.0;
    double corruption_correlation = 0.0;
    double reordering_percent = 0.0;
    double reordering_correlation = 0.0;
    double reordering_distance = 0.0;
    double rate_packet_overhead = 0.0;
    double rate_cell_size = 0.0;
    double rate_cell_overhead = 0.0;
    double slot_min_delay = 0.0;
    double slot_max_delay = 0.0;
    QString slot_distribution = "disabled";
};

struct ComboInfo {
    const QString label;
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
    const QString label;
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

DoubleSpinInfo spinInfo[] = {
    { 1, 1, 0.0,   100000.0, 0.0 }, { 1, 2, 0.0,   100000.0, 0.0 },
    { 2, 1, 0.0,      100.0, 0.0 }, { 2, 2, 0.0,      100.0, 0.0 },
    { 3, 1, 0.0, 11000000.0, 0.0 }, { 3, 2, 0.0, 11000000.0, 0.0 },
};

DoubleSpinInfo spinInfo2[] = {
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

namespace rqt_netem {

class AdvancedConfigDialog : public QDialog
{
public:
    AdvancedConfigDialog(QWidget* parent = nullptr);

    OptionInfo inboundInfo() { return this->inInfo; }
    OptionInfo outboundInfo() { return this->outInfo; }

    void update(const OptionInfo& inInfo, const OptionInfo& outInfo);
    void make();

private:
    void on_resetButton_clicked();

    enum {
        Inbound_LimitPackets, Outbound_LimitPackets,
        Inbound_DelayJitter, Outbound_DelayJitter,
        Inbound_DelayCorrelation, Outbound_DelayCorrelation,
        Inbound_LossCorrelation, Outbound_LossCorrelation,
        Inbound_DuplicationPercent, Outbound_DuplicationPercent,
        Inbound_DuplicationCorrelation, Outbound_DuplicationCorrelation,
        Inbound_CorruptionPercent, Outbound_CorruptionPercent,
        Inbound_CorruptionCorrelation, Outbound_CorruptionCorrelation,
        Inbound_ReorderingPercent, Outbound_ReorderingPercent,
        Inbound_ReorderingCorrelation, Outbound_ReorderingCorrelation,
        Inbound_ReorderingDistance, Outbound_ReorderingDistance,
        Inbound_RatePacketOverhead, Outbound_RatePacketOverhead,
        Inbound_RateCellSize, Outbound_RateCellSize,
        Inbound_RateCellOverhead, Outbound_RateCellOverhead,
        Inbound_SlotMinDelay, Outbound_SlotMinDelay,
        Inbound_SlotMaxDelay, Outbound_SlotMaxDelay,
        NumAdvSpins
    };

    enum {
        Inbound_DelayDistribution, Outbound_DelayDistribution,
        Inbound_LossRandom, Outbound_LossRandom,
        Inbound_SlotDistribution, Outbound_SlotDistribution,
        NumAdvCombos
    };

    QDoubleSpinBox* spins[NumAdvSpins];
    QComboBox* combos[NumAdvCombos];
    QDialogButtonBox* buttonBox;

    OptionInfo inInfo;
    OptionInfo outInfo;
};

class MainWindow::Impl
{
public:
    MainWindow* self;

    Impl(MainWindow* self);

    void open();
    void save();
    void start();
    void stop();
    void clear();
    void config();

    void startBash();
    void stopBash();
    void close();
    void write();
    void execute(const QString& text);

    bool save(const QString& fileName);
    bool load(const QString& fileName);
    void read(const QJsonObject& json);
    void write(QJsonObject& json) const;

    void on_process_stateChanged(QProcess::ProcessState newState);

    void createActions();
    void createToolBars();

    enum { Interface, IntermediateFunctionalBlock, NumCombos };
    enum { Source, Destination, NumLines };
    enum {
        Inbound_DelayTIme, Outbound_DelayTIme,
        Inbound_LossPercent, Outbound_LossPercent,
        Inbound_RateRate, Outbound_RateRate,
        NumSpins
    };

    QAction* openAct;
    QAction* saveAct;
    QAction* startAct;
    QAction* stopAct;
    QAction* configAct;
    QAction* clearAct;

    QComboBox* combos[NumCombos];
    QLineEdit* lines[NumLines];
    QDoubleSpinBox* spins[NumSpins];
    QProcess process;

    bool is_started;

    OptionInfo inInfo;
    OptionInfo outInfo;
    Bash* bash;
};

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    impl = new Impl(this);
}

MainWindow::Impl::Impl(MainWindow* self)
    : self(self)
{
    QWidget* widget = new QWidget;
    self->setCentralWidget(widget);

    createActions();
    createToolBars();

    self->setWindowTitle("Network Emulator");

    is_started = false;
    bash = new Bash;

    QGridLayout* gridLayout = new QGridLayout;
    for(int i = 0; i < NumCombos; ++i) {
        QComboBox* combo = new QComboBox;
        combos[i] = combo;
        ComboInfo& info = comboInfo[i];
        gridLayout->addWidget(new QLabel(info.label), info.row, info.cln - 1);
        gridLayout->addWidget(combo, info.row, info.cln);
    }

    for(int i = 0; i < NumLines; ++i) {
        QLineEdit* line = new QLineEdit;
        lines[i] = line;
        line->setText("0.0.0.0/0");
        line->setValidator(new IPv4Validator(self));
        LineInfo& info = lineInfo[i];
        gridLayout->addWidget(new QLabel(info.label), info.row, info.cln - 1);
        gridLayout->addWidget(line, info.row, info.cln);
    }

    QComboBox* ifcCombo = combos[Interface];
    const int IFR_MAX = 10;
    struct ifreq ifr[IFR_MAX];
    struct ifconf ifc;
    int fd = socket(AF_INET, SOCK_DGRAM, 0);

    ifc.ifc_len = sizeof(ifr);
    ifc.ifc_ifcu.ifcu_buf = (char*)ifr;
    ioctl(fd, SIOCGIFCONF, &ifc);
    int nifs = ifc.ifc_len / sizeof(struct ifreq);
    for(int j = 0; j < nifs; j++) {
        QString interfaceName = ifr[j].ifr_name;
        ifcCombo->addItem(interfaceName);
    }
    ::close(fd);

    QComboBox* ifbCombo = combos[IntermediateFunctionalBlock];
    ifbCombo->addItems(QStringList() << "ifb0" << "ifb1");

    QGridLayout* gridLayout2 = new QGridLayout;
    gridLayout2->addWidget(new QLabel("Inbound"), 0, 1);
    gridLayout2->addWidget(new QLabel("Outbound"), 0, 2);

    const QStringList list = { "Dealy Time [ms]", "Loss Percent [%]", "Rate Rate [kbit/s]" };
    for(int i = 0; i < 3; ++i) {
        gridLayout2->addWidget(new QLabel(list.at(i)), i + 1, 0);
    }

    for(int i = 0; i < NumSpins; ++i) {
        QDoubleSpinBox* spin = new QDoubleSpinBox;
        spins[i] = spin;
        DoubleSpinInfo& info = spinInfo[i];
        spin->setRange(info.lower, info.upper);
        spin->setValue(info.value);
        gridLayout2->addWidget(spin, info.row, info.cln);
    }

    self->connect(&process, &QProcess::stateChanged, [&](QProcess::ProcessState newState){ on_process_stateChanged(newState); });

    auto layout = new QVBoxLayout;
    layout->addLayout(gridLayout);
    layout->addLayout(gridLayout2);
    layout->addStretch();
    widget->setLayout(layout);
}

MainWindow::~MainWindow()
{
    delete impl;
}

void MainWindow::Impl::open()
{
    static QString dir = "/home";
    QString fileName = QFileDialog::getOpenFileName(self, "Open File",
        dir,
        "JSON Files (*.json);;All Files (*)");

    if(fileName.isEmpty()) {
        return;
    } else {
        QFileInfo info(fileName);
        dir = info.absolutePath();
        load(fileName);
    }
}

void MainWindow::Impl::save()
{
    static QString dir = "/home";
    QString fileName = QFileDialog::getSaveFileName(self, "Save File",
        dir,
        "JSON Files (*.json);;All Files (*)");

    if(fileName.isEmpty()) {
        return;
    } else {
        QFileInfo info(fileName);
        dir = info.absolutePath();
        save(fileName);
    }
}

void MainWindow::Impl::start()
{
    stop();
    startBash();
}

void MainWindow::Impl::stop()
{
    close();
    stopBash();
}

void MainWindow::Impl::clear()
{
    for(int i = 0; i < NumCombos; ++i) {
        combos[i]->setCurrentIndex(0);
    }
    for(int i = 0; i < NumLines; ++i) {
        lines[i]->setText("0.0.0.0/0");
    }
    for(int i = 0; i < NumSpins; ++i) {
        spins[i]->setValue(0.0);
    }
}

void MainWindow::Impl::config()
{
    AdvancedConfigDialog dialog(self);
    dialog.update(inInfo, outInfo);

    if(dialog.exec()) {
        dialog.make();
        inInfo = dialog.inboundInfo();
        outInfo = dialog.outboundInfo();
    }
}

void MainWindow::Impl::startBash()
{
    is_started = true;
    process.start("bash");
}

void MainWindow::Impl::stopBash()
{
    is_started = false;
    process.close();
}

void MainWindow::Impl::close()
{
    QString ifc_name = combos[Interface]->currentText();
    QString ifb_name = combos[IntermediateFunctionalBlock]->currentText();

    if(is_started) {
        QStringList list;
        // clear settings
        list << QString("sudo tc qdisc del dev %1 ingress").arg(ifc_name);
        list << QString("sudo tc qdisc del dev %1 root").arg(ifb_name);
        list << QString("sudo tc qdisc del dev %1 root").arg(ifc_name);

        // finalize
        list << QString("sudo ip link set dev %1 down").arg(ifb_name);
        list << "sudo rmmod ifb";
        for(int i = 0; i < list.size(); ++i) {
            execute(list.at(i));
        }
    }
}

void MainWindow::Impl::write()
{
    double in_delay_time = spins[Inbound_DelayTIme]->value();
    double in_loss_percent = spins[Inbound_LossPercent]->value();
    double in_rate_rate = spins[Inbound_RateRate]->value();
    double out_delay_time = spins[Outbound_DelayTIme]->value();
    double out_loss_percent = spins[Outbound_LossPercent]->value();
    double out_rate_rate = spins[Outbound_RateRate]->value();

    QString src_ip_name = lines[Source]->text();
    QString dst_ip_name = lines[Destination]->text();
    QString ifc_name = combos[Interface]->currentText();
    QString ifb_name = combos[IntermediateFunctionalBlock]->currentText();

    QString in_delay_distribution = inInfo.delay_distribution;
    QString in_loss_random = inInfo.loss_random;
    QString in_slot_distribution = inInfo.slot_distribution;
    QString out_delay_distribution  = outInfo.delay_distribution;
    QString out_loss_random = outInfo.loss_random;
    QString out_slot_distribution = outInfo.slot_distribution;

    QString in_text;
    QString out_text;

    // inbound commands
    if(inInfo.limit_packets > 0.0) {
        in_text += QString("limit %1").arg((int)inInfo.limit_packets);
    }

    if(in_delay_time > 0.0) {
        in_text += QString(" delay %1ms").arg((int)in_delay_time);
        if(inInfo.delay_jitter > 0.0) {
            in_text += QString(" %1ms").arg((int)inInfo.delay_jitter);
            if(inInfo.delay_correlation > 0.0) {
                in_text += QString(" %1\%").arg((int)inInfo.delay_correlation);
            }
        }
        if(in_delay_distribution != "disabled") {
            in_text += QString(" distribution %1").arg(in_delay_distribution);
        }
    }

    if(in_loss_percent > 0.0) {
        in_text += " loss";
        if(in_loss_random != "disabled") {
            in_text += " random";
        }
        in_text += QString(" %1\%").arg((int)in_loss_percent);
        if(inInfo.loss_correlation > 0.0) {
            in_text += QString(" %1\%").arg((int)inInfo.loss_correlation);
        }
    }

    if(inInfo.corruption_percent > 0.0) {
        in_text += QString(" corrupt %1\%").arg((int)inInfo.corruption_percent);
        if(inInfo.corruption_correlation > 0.0) {
            in_text += QString(" %1\%").arg((int)inInfo.corruption_correlation);
        }
    }

    if(inInfo.duplication_percent > 0.0) {
        in_text += QString(" duplicate %1\%").arg((int)inInfo.duplication_percent);
        if(inInfo.duplication_correlation > 0.0) {
            in_text += QString(" %1\%").arg((int)inInfo.duplication_correlation);
        }
    }

    if(inInfo.reordering_percent > 0.0) {
        in_text += QString(" reorder %1\%").arg((int)inInfo.reordering_percent);
        if(inInfo.reordering_correlation > 0.0) {
            in_text += QString(" %1\%").arg((int)inInfo.reordering_correlation);
        }
        if(inInfo.reordering_distance > 0.0) {
            in_text += QString(" gap %1").arg((int)inInfo.reordering_distance);
        }
    }

    if(in_rate_rate > 0.0) {
        in_text += QString(" rate %1kbps").arg((int)in_rate_rate);
        if(inInfo.rate_packet_overhead > 0.0) {
            in_text += QString(" %1").arg((int)inInfo.rate_packet_overhead);
            if(inInfo.rate_cell_size > 0.0) {
                in_text += QString(" %1").arg((int)inInfo.rate_cell_size);
                if(inInfo.rate_cell_overhead > 0.0) {
                    in_text += QString(" %1").arg((int)inInfo.rate_cell_overhead);
                }
            }
        }
    }

    if(in_slot_distribution != "disabled") {
        in_text += QString(" slot %1").arg(in_slot_distribution);
    } else if(inInfo.slot_min_delay > 0.0) {
        in_text += QString(" slot %1ms").arg((int)inInfo.slot_min_delay);
        if(inInfo.slot_max_delay > 0.0) {
            in_text += QString(" %1ms").arg((int)inInfo.slot_max_delay);
        }
    }

    // outbound commands
    if(outInfo.limit_packets > 0.0) {
        out_text += QString("limit %1").arg((int)outInfo.limit_packets);
    }

    if(out_delay_time > 0.0) {
        out_text += QString(" delay %1ms").arg((int)out_delay_time);
        if(outInfo.delay_jitter > 0.0) {
            out_text += QString(" %1ms").arg((int)outInfo.delay_jitter);
            if(outInfo.delay_correlation > 0.0) {
                out_text += QString(" %1\%").arg((int)outInfo.delay_correlation);
            }
        }
        if(out_delay_distribution != "disabled") {
            out_text += QString(" distribution %1").arg(out_delay_distribution);
        }
    }

    if(out_loss_percent > 0.0) {
        out_text += " loss";
        if(out_loss_random != "disabled") {
            out_text += " random";
        }
        out_text += QString(" %1\%").arg((int)out_loss_percent);
        if(outInfo.loss_correlation > 0.0) {
            out_text += QString(" %1\%").arg((int)outInfo.loss_correlation);
        }
    }

    if(outInfo.corruption_percent > 0.0) {
        out_text += QString(" corrupt %1\%").arg((int)outInfo.corruption_percent);
        if(outInfo.corruption_correlation > 0.0) {
            out_text += QString(" %1\%").arg((int)outInfo.corruption_correlation);
        }
    }

    if(outInfo.duplication_percent > 0.0) {
        out_text += QString(" duplicate %1\%").arg((int)outInfo.duplication_percent);
        if(outInfo.duplication_correlation > 0.0) {
            out_text += QString(" %1\%").arg((int)outInfo.duplication_correlation);
        }
    }

    if(outInfo.reordering_percent > 0.0) {
        out_text += QString(" reorder %1\%").arg((int)outInfo.reordering_percent);
        if(outInfo.reordering_correlation > 0.0) {
            out_text += QString(" %1\%").arg((int)outInfo.reordering_correlation);
        }
        if(outInfo.reordering_distance > 0.0) {
            out_text += QString(" gap %1").arg((int)outInfo.reordering_distance);
        }
    }

    if(out_rate_rate > 0.0) {
        out_text += QString(" rate %1kbps").arg((int)out_rate_rate);
        if(outInfo.rate_packet_overhead > 0.0) {
            out_text += QString(" %1").arg((int)outInfo.rate_packet_overhead);
            if(outInfo.rate_cell_size > 0.0) {
                out_text += QString(" %1").arg((int)outInfo.rate_cell_size);
                if(outInfo.rate_cell_overhead > 0.0) {
                    out_text += QString(" %1").arg((int)outInfo.rate_cell_overhead);
                }
            }
        }
    }

    if(out_slot_distribution != "disabled") {
        out_text += QString(" slot %1").arg(out_slot_distribution);
    } else if(outInfo.slot_min_delay > 0.0) {
        out_text += QString(" slot %1ms").arg((int)outInfo.slot_min_delay);
        if(outInfo.slot_max_delay > 0.0) {
            out_text += QString(" %1ms").arg((int)outInfo.slot_max_delay);
        }
    }

    if(!checkAddress(src_ip_name)) {
        src_ip_name = "0.0.0.0/0";
    }
    if(!checkAddress(dst_ip_name)) {
        dst_ip_name = "0.0.0.0/0";
    }

    QStringList list;
    // initialize
    list << "sudo modprobe ifb";
    list << "sudo modprobe act_mirred";
    list << QString("sudo ip link set dev %1 up").arg(ifb_name);

    // apply settings
    list << QString("sudo tc qdisc add dev %1 ingress handle ffff:")
        .arg(ifc_name);
    list << QString("sudo tc filter add dev %1 parent ffff: protocol ip u32 match u32 0 0 action mirred egress redirect dev %2")
        .arg(ifc_name).arg(ifb_name);
    list << QString("sudo tc qdisc add dev %1 root handle 1: prio bands 16 priomap 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0")
        .arg(ifb_name);
    list << QString("sudo tc qdisc add dev %1 parent 1:1 handle 10: netem limit %2")
        .arg(ifb_name).arg((int)inInfo.limit_packets);

    if((!src_ip_name.isEmpty()) && (!dst_ip_name.isEmpty())) {
        list << QString("sudo tc qdisc add dev %1 parent 1:2 handle 20: netem %2")
            .arg(ifb_name).arg(in_text);
        list << QString("sudo tc filter add dev %1 protocol ip parent 1: prio 2 u32 match ip src %2 match ip dst %3 flowid 1:2")
            .arg(ifb_name).arg(dst_ip_name).arg(src_ip_name);
    }
    list << QString("sudo tc qdisc add dev %1 root handle 1: prio bands 16 priomap 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0")
        .arg(ifc_name);
    list << QString("sudo tc qdisc add dev %1 parent 1:1 handle 10: netem limit %2")
        .arg(ifc_name).arg((int)outInfo.limit_packets);
    if((!src_ip_name.isEmpty()) && (!dst_ip_name.isEmpty())) {
        list << QString("sudo tc qdisc add dev %1 parent 1:2 handle 20: netem %2")
            .arg(ifc_name).arg(out_text);
        list << QString("sudo tc filter add dev %1 protocol ip parent 1: prio 2 u32 match ip src %2 match ip dst %3 flowid 1:2")
            .arg(ifc_name).arg(src_ip_name).arg(dst_ip_name);
    }
    for(int i = 0; i < list.size(); ++i) {
        execute(list.at(i));
    }
}

void MainWindow::Impl::execute(const QString& text)
{
    // QByteArray data;
    // data.append(QString(program.c_str()));
    // data.append("\n");
    // process.write(data);
    int ret = system(text.toStdString().c_str());
    // qDebug() << text;
}

bool MainWindow::Impl::save(const QString& fileName)
{
    QFile saveFile(fileName);
    if(!saveFile.open(QIODevice::WriteOnly)) {
        qWarning("Couldn't open save file.");
        return false;
    }

    QJsonObject object;
    write(object);
    saveFile.write(QJsonDocument(object).toJson());

    return true;
}

bool MainWindow::Impl::load(const QString& fileName)
{
    QFile loadFile(fileName);
    if(!loadFile.open(QIODevice::ReadOnly)) {
        qWarning("Couldn't open save file.");
        return false;
    }

    QByteArray saveData = loadFile.readAll();

    QJsonDocument loadDoc(QJsonDocument::fromJson(saveData));

    read(loadDoc.object());

    return true;
}

void MainWindow::Impl::read(const QJsonObject& json)
{
    if(json.contains("config") && json["config"].isArray()) {
        const QJsonArray configArray = json["config"].toArray();
            combos[0]->setCurrentIndex(configArray[0].toInt());
            combos[1]->setCurrentIndex(configArray[1].toInt());
            lines[0]->setText(configArray[2].toString());
            lines[1]->setText(configArray[3].toString());
            spins[0]->setValue(configArray[4].toDouble());
            spins[1]->setValue(configArray[5].toDouble());
            spins[2]->setValue(configArray[6].toDouble());
            spins[3]->setValue(configArray[7].toDouble());
            spins[4]->setValue(configArray[8].toDouble());
            spins[5]->setValue(configArray[9].toDouble());
    }

    if(json.contains("adv_config") && json["adv_config"].isArray()) {
        const QJsonArray advConfigArray = json["adv_config"].toArray();
        inInfo.limit_packets = advConfigArray[0].toDouble();
        inInfo.delay_jitter = advConfigArray[1].toDouble();
        inInfo.delay_correlation = advConfigArray[2].toDouble();
        inInfo.delay_distribution = advConfigArray[3].toString();
        inInfo.loss_random = advConfigArray[4].toString();
        inInfo.loss_correlation = advConfigArray[5].toDouble();
        inInfo.duplication_percent = advConfigArray[6].toDouble();
        inInfo.duplication_correlation = advConfigArray[7].toDouble();
        inInfo.corruption_percent = advConfigArray[8].toDouble();
        inInfo.corruption_correlation = advConfigArray[9].toDouble();
        inInfo.reordering_percent = advConfigArray[10].toDouble();
        inInfo.reordering_correlation = advConfigArray[11].toDouble();
        inInfo.reordering_distance = advConfigArray[12].toDouble();
        inInfo.rate_packet_overhead = advConfigArray[13].toDouble();
        inInfo.rate_cell_size = advConfigArray[14].toDouble();
        inInfo.rate_cell_overhead = advConfigArray[15].toDouble();
        inInfo.slot_min_delay = advConfigArray[16].toDouble();
        inInfo.slot_max_delay = advConfigArray[17].toDouble();
        inInfo.slot_distribution = advConfigArray[18].toString();

        outInfo.limit_packets = advConfigArray[19].toDouble();
        outInfo.delay_jitter = advConfigArray[20].toDouble();
        outInfo.delay_correlation = advConfigArray[21].toDouble();
        outInfo.delay_distribution = advConfigArray[22].toString();
        outInfo.loss_random = advConfigArray[23].toString();
        outInfo.loss_correlation = advConfigArray[24].toDouble();
        outInfo.duplication_percent = advConfigArray[25].toDouble();
        outInfo.duplication_correlation = advConfigArray[26].toDouble();
        outInfo.corruption_percent = advConfigArray[27].toDouble();
        outInfo.corruption_correlation = advConfigArray[28].toDouble();
        outInfo.reordering_percent = advConfigArray[29].toDouble();
        outInfo.reordering_correlation = advConfigArray[30].toDouble();
        outInfo.reordering_distance = advConfigArray[31].toDouble();
        outInfo.rate_packet_overhead = advConfigArray[32].toDouble();
        outInfo.rate_cell_size = advConfigArray[33].toDouble();
        outInfo.rate_cell_overhead = advConfigArray[34].toDouble();
        outInfo.slot_min_delay = advConfigArray[35].toDouble();
        outInfo.slot_max_delay = advConfigArray[36].toDouble();
        outInfo.slot_distribution = advConfigArray[37].toString();
    }
}

void MainWindow::Impl::write(QJsonObject& json) const
{
    QJsonArray configArray;
    for(int i = 0; i < NumCombos; ++i) {
        configArray.append(combos[i]->currentIndex());
    }
    for(int i = 0; i < NumLines; ++i) {
        configArray.append(lines[i]->text());
    }
    for(int i = 0; i < NumSpins; ++i) {
        configArray.append(spins[i]->value());
    }
    json["config"] = configArray;

    QJsonArray advConfigArray;
    advConfigArray.append(inInfo.limit_packets);
    advConfigArray.append(inInfo.delay_jitter);
    advConfigArray.append(inInfo.delay_correlation);
    advConfigArray.append(inInfo.delay_distribution);
    advConfigArray.append(inInfo.loss_random);
    advConfigArray.append(inInfo.loss_correlation);
    advConfigArray.append(inInfo.duplication_percent);
    advConfigArray.append(inInfo.duplication_correlation);
    advConfigArray.append(inInfo.corruption_percent);
    advConfigArray.append(inInfo.corruption_correlation);
    advConfigArray.append(inInfo.reordering_percent);
    advConfigArray.append(inInfo.reordering_correlation);
    advConfigArray.append(inInfo.reordering_distance);
    advConfigArray.append(inInfo.rate_packet_overhead);
    advConfigArray.append(inInfo.rate_cell_size);
    advConfigArray.append(inInfo.rate_cell_overhead);
    advConfigArray.append(inInfo.slot_min_delay);
    advConfigArray.append(inInfo.slot_max_delay);
    advConfigArray.append(inInfo.slot_distribution);

    advConfigArray.append(outInfo.limit_packets);
    advConfigArray.append(outInfo.delay_jitter);
    advConfigArray.append(outInfo.delay_correlation);
    advConfigArray.append(outInfo.delay_distribution);
    advConfigArray.append(outInfo.loss_random);
    advConfigArray.append(outInfo.loss_correlation);
    advConfigArray.append(outInfo.duplication_percent);
    advConfigArray.append(outInfo.duplication_correlation);
    advConfigArray.append(outInfo.corruption_percent);
    advConfigArray.append(outInfo.corruption_correlation);
    advConfigArray.append(outInfo.reordering_percent);
    advConfigArray.append(outInfo.reordering_correlation);
    advConfigArray.append(outInfo.reordering_distance);
    advConfigArray.append(outInfo.rate_packet_overhead);
    advConfigArray.append(outInfo.rate_cell_size);
    advConfigArray.append(outInfo.rate_cell_overhead);
    advConfigArray.append(outInfo.slot_min_delay);
    advConfigArray.append(outInfo.slot_max_delay);
    advConfigArray.append(outInfo.slot_distribution);
    json["adv_config"] = advConfigArray;
}

void MainWindow::Impl::on_process_stateChanged(QProcess::ProcessState newState)
{
    switch(newState) {
        case QProcess::NotRunning:
            // emit self->output("Not runnging");
            combos[Interface]->setEnabled(true);
            combos[IntermediateFunctionalBlock]->setEnabled(true);
            break;
        case QProcess::Starting:
            // emit self->output("Starting");
            combos[Interface]->setEnabled(false);
            combos[IntermediateFunctionalBlock]->setEnabled(false);
            break;
        case QProcess::Running:
            // emit self->output("Running");
            write();
            break;
        default:
            break;
    }
}

void MainWindow::Impl::createActions()
{
    const QIcon openIcon = QIcon::fromTheme("document-open");
    openAct = new QAction(openIcon, "&Open...", self);
    openAct->setShortcuts(QKeySequence::Open);
    openAct->setStatusTip("Open an existing file");
    self->connect(openAct, &QAction::triggered, [&](){ open(); });

    const QIcon saveIcon = QIcon::fromTheme("document-save");
    saveAct = new QAction(saveIcon, "&Save", self);
    saveAct->setShortcuts(QKeySequence::Save);
    saveAct->setStatusTip("Save the advanced configurations to disk");
    self->connect(saveAct, &QAction::triggered, [&](){ save(); });

    const QIcon startIcon = QIcon::fromTheme("media-playback-start");
    startAct = new QAction(startIcon, "&Start", self);
    startAct->setStatusTip("Start netem");
    self->connect(startAct, &QAction::triggered, [&](){ start(); });

    const QIcon stopIcon = QIcon::fromTheme("media-playback-stop");
    stopAct = new QAction(stopIcon, "&Stop", self);
    stopAct->setStatusTip("Stop netem");
    self->connect(stopAct, &QAction::triggered, [&](){ stop(); });

    const QIcon clearIcon = QIcon::fromTheme("edit-clear");
    clearAct = new QAction(clearIcon, "&Clear", self);
    clearAct->setStatusTip("Clear the configurations");
    self->connect(clearAct, &QAction::triggered, [&](){ clear(); });

    const QIcon configIcon = QIcon::fromTheme("preferences-system");
    configAct = new QAction(configIcon, "&Config", self);
    configAct->setStatusTip("Show the config dialog");
    self->connect(configAct, &QAction::triggered, [&](){ config(); });
}

void MainWindow::Impl::createToolBars()
{
    QToolBar* emulatorToolBar = self->addToolBar("Network Emulator");
    emulatorToolBar->addAction(openAct);
    emulatorToolBar->addAction(saveAct);
    emulatorToolBar->addSeparator();
    emulatorToolBar->addAction(startAct);
    emulatorToolBar->addAction(stopAct);
    emulatorToolBar->addAction(clearAct);
    emulatorToolBar->addAction(configAct);
}

AdvancedConfigDialog::AdvancedConfigDialog(QWidget* parent)
    : QDialog(parent)
{
    QGridLayout* layout = new QGridLayout;
    layout->addWidget(new QLabel("Inbound"), 0, 1);
    layout->addWidget(new QLabel("Outbound"), 0, 2);

    for(int i = 0; i < 19; ++i) {
        QLabel* label = new QLabel(nameList.at(i));
        layout->addWidget(label, i + 1, 0);
    }

    for(int i = 0; i < NumAdvSpins; ++i) {
        QDoubleSpinBox* spin = new QDoubleSpinBox;
        spins[i] = spin;
        DoubleSpinInfo& info = spinInfo2[i];
        spin->setRange(info.lower, info.upper);
        spin->setValue(info.value);
        layout->addWidget(spin, info.row, info.cln);
    }

    const QStringList list = { "disabled", "uniform", "normal", "pareto", "paretonormal" };
    const QStringList list2 = { "disabled", "enabled" };
    for(int i = 0; i < NumAdvCombos; ++i) {
        QComboBox* combo = new QComboBox;
        combos[i] = combo;
        if(i == 2 || i == 3) {
            combo->addItems(list2);
        } else {
            combo->addItems(list);
        }
        combo->setCurrentIndex(0);
        ComboInfo& info = comboInfo2[i];
        layout->addWidget(combo, info.row, info.cln);
    }

    QPushButton* resetButton = new QPushButton("&Reset");
    connect(resetButton, &QPushButton::clicked, [&](){ on_resetButton_clicked(); });

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                     | QDialogButtonBox::Cancel);

    connect(buttonBox, &QDialogButtonBox::accepted, [&](){ accept(); });
    connect(buttonBox, &QDialogButtonBox::rejected, [&](){ reject(); });

    buttonBox->addButton(resetButton, QDialogButtonBox::ResetRole);

    auto mainLayout = new QVBoxLayout;
    mainLayout->addLayout(layout);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);
    setWindowTitle("Advanced Config");
}

void AdvancedConfigDialog::on_resetButton_clicked()
{
    for(int i = 0; i < NumAdvSpins; ++i) {
        QDoubleSpinBox* spin = spins[i];
        DoubleSpinInfo& info = spinInfo2[i];
        spin->setRange(info.lower, info.upper);
        spin->setValue(info.value);
    }

    for(int i = 0; i < NumAdvCombos; ++i) {
        combos[i]->setCurrentIndex(0);
    }
}

void AdvancedConfigDialog::update(const OptionInfo& inInfo, const OptionInfo& outInfo)
{
    // inbound
    spins[Inbound_LimitPackets]->setValue(inInfo.limit_packets);
    spins[Inbound_DelayJitter]->setValue(inInfo.delay_jitter);
    spins[Inbound_DelayCorrelation]->setValue(inInfo.delay_correlation);
    spins[Inbound_LossCorrelation]->setValue(inInfo.loss_correlation);
    spins[Inbound_DuplicationPercent]->setValue(inInfo.duplication_percent);
    spins[Inbound_DuplicationCorrelation]->setValue(inInfo.duplication_correlation);
    spins[Inbound_CorruptionPercent]->setValue(inInfo.corruption_percent);
    spins[Inbound_CorruptionCorrelation]->setValue(inInfo.corruption_correlation);
    spins[Inbound_ReorderingPercent]->setValue(inInfo.reordering_percent);
    spins[Inbound_ReorderingCorrelation]->setValue(inInfo.reordering_correlation);
    spins[Inbound_ReorderingDistance]->setValue(inInfo.reordering_distance);
    spins[Inbound_RatePacketOverhead]->setValue(inInfo.rate_packet_overhead);
    spins[Inbound_RateCellSize]->setValue(inInfo.rate_cell_size);
    spins[Inbound_RateCellOverhead]->setValue(inInfo.rate_cell_overhead);
    spins[Inbound_SlotMinDelay]->setValue(inInfo.slot_min_delay);
    spins[Inbound_SlotMaxDelay]->setValue(inInfo.slot_max_delay);

    combos[Inbound_DelayDistribution]->setCurrentText(inInfo.delay_distribution);
    combos[Inbound_LossRandom]->setCurrentText(inInfo.loss_random);
    combos[Inbound_SlotDistribution]->setCurrentText(inInfo.slot_distribution);

    // outbound
    spins[Outbound_LimitPackets]->setValue(outInfo.limit_packets);
    spins[Outbound_DelayJitter]->setValue(outInfo.delay_jitter);
    spins[Outbound_DelayCorrelation]->setValue(outInfo.delay_correlation);
    spins[Outbound_LossCorrelation]->setValue(outInfo.loss_correlation);
    spins[Outbound_DuplicationPercent]->setValue(outInfo.duplication_percent);
    spins[Outbound_DuplicationCorrelation]->setValue(outInfo.duplication_correlation);
    spins[Outbound_CorruptionPercent]->setValue(outInfo.corruption_percent);
    spins[Outbound_CorruptionCorrelation]->setValue(outInfo.corruption_correlation);
    spins[Outbound_ReorderingPercent]->setValue(outInfo.reordering_percent);
    spins[Outbound_ReorderingCorrelation]->setValue(outInfo.reordering_correlation);
    spins[Outbound_ReorderingDistance]->setValue(outInfo.reordering_distance);
    spins[Outbound_RatePacketOverhead]->setValue(outInfo.rate_packet_overhead);
    spins[Outbound_RateCellSize]->setValue(outInfo.rate_cell_size);
    spins[Outbound_RateCellOverhead]->setValue(outInfo.rate_cell_overhead);
    spins[Outbound_SlotMinDelay]->setValue(outInfo.slot_min_delay);
    spins[Outbound_SlotMaxDelay]->setValue(outInfo.slot_max_delay);

    combos[Outbound_DelayDistribution]->setCurrentText(outInfo.delay_distribution);
    combos[Outbound_LossRandom]->setCurrentText(outInfo.loss_random);
    combos[Outbound_SlotDistribution]->setCurrentText(outInfo.slot_distribution);
}

void AdvancedConfigDialog::make()
{
    // inbound
    inInfo.limit_packets = spins[Inbound_LimitPackets]->value();
    inInfo.delay_jitter = spins[Inbound_DelayJitter]->value();
    inInfo.delay_correlation = spins[Inbound_DelayCorrelation]->value();
    inInfo.loss_correlation = spins[Inbound_LossCorrelation]->value();
    inInfo.duplication_percent = spins[Inbound_DuplicationPercent]->value();
    inInfo.duplication_correlation = spins[Inbound_DuplicationCorrelation]->value();
    inInfo.corruption_percent = spins[Inbound_CorruptionPercent]->value();
    inInfo.corruption_correlation = spins[Inbound_CorruptionCorrelation]->value();
    inInfo.reordering_percent = spins[Inbound_ReorderingPercent]->value();
    inInfo.reordering_correlation = spins[Inbound_ReorderingCorrelation]->value();
    inInfo.reordering_distance = spins[Inbound_ReorderingDistance]->value();
    inInfo.rate_packet_overhead = spins[Inbound_RatePacketOverhead]->value();
    inInfo.rate_cell_size = spins[Inbound_RateCellSize]->value();
    inInfo.rate_cell_overhead = spins[Inbound_RateCellOverhead]->value();
    inInfo.slot_min_delay = spins[Inbound_SlotMinDelay]->value();
    inInfo.slot_max_delay = spins[Inbound_SlotMaxDelay]->value();

    inInfo.delay_distribution = combos[Inbound_DelayDistribution]->currentText();
    inInfo.loss_random = combos[Inbound_LossRandom]->currentText();
    inInfo.slot_distribution = combos[Inbound_SlotDistribution]->currentText();

    // outbound
    outInfo.limit_packets = spins[Outbound_LimitPackets]->value();
    outInfo.delay_jitter = spins[Outbound_DelayJitter]->value();
    outInfo.delay_correlation = spins[Outbound_DelayCorrelation]->value();
    outInfo.loss_correlation = spins[Outbound_LossCorrelation]->value();
    outInfo.duplication_percent = spins[Outbound_DuplicationPercent]->value();
    outInfo.duplication_correlation = spins[Outbound_DuplicationCorrelation]->value();
    outInfo.corruption_percent = spins[Outbound_CorruptionPercent]->value();
    outInfo.corruption_correlation = spins[Outbound_CorruptionCorrelation]->value();
    outInfo.reordering_percent = spins[Outbound_ReorderingPercent]->value();
    outInfo.reordering_correlation = spins[Outbound_ReorderingCorrelation]->value();
    outInfo.reordering_distance = spins[Outbound_ReorderingDistance]->value();
    outInfo.rate_packet_overhead = spins[Outbound_RatePacketOverhead]->value();
    outInfo.rate_cell_size = spins[Outbound_RateCellSize]->value();
    outInfo.rate_cell_overhead = spins[Outbound_RateCellOverhead]->value();
    outInfo.slot_min_delay = spins[Outbound_SlotMinDelay]->value();
    outInfo.slot_max_delay = spins[Outbound_SlotMaxDelay]->value();

    outInfo.delay_distribution = combos[Outbound_DelayDistribution]->currentText();
    outInfo.loss_random = combos[Outbound_LossRandom]->currentText();
    outInfo.slot_distribution = combos[Outbound_SlotDistribution]->currentText();
}

}
