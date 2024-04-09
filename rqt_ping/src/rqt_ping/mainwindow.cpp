/**
   @author Kenta Suzuki
*/

#include "rqt_ping/mainwindow.h"

#include <QAction>
#include <QBoxLayout>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QTextEdit>
#include <QToolBar>

#include "rqt_ping/ping.h"

namespace rqt_ping {

class PingConfigDialog : public QDialog
{
public:
    PingConfigDialog(QWidget* parent = nullptr);

    void setCount(const int& count) { countSpin->setValue(count); }
    int count() const { return countSpin->value(); }
    void setWait(const double& wait) { waitSpin->setValue(wait); }
    double wait() const { return waitSpin->value(); }

private:

    QSpinBox* countSpin;
    QDoubleSpinBox* waitSpin;
    QDialogButtonBox* buttonBox;
};

class MainWindow::Impl
{
public:
    MainWindow* self;

    Impl(MainWindow* self);
    ~Impl();

    void start();
    void stop();
    void config();
    void print(const QString& text);

    void createActions();
    void createToolBars();

    QAction* startAct;
    QAction* stopAct;
    QAction* configAct;

    QLineEdit* addressLine;
    QTextEdit* messageText;

    int count;
    double wait;

    Ping* ping;
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

    self->setWindowTitle("Ping");

    count = 0;
    wait = 1.0;

    messageText = new QTextEdit;

    ping = new Ping;
    self->connect(ping, &Ping::output, [&](QString text){ print(text); });

    auto layout = new QVBoxLayout;
    layout->addWidget(messageText);
    widget->setLayout(layout);
}

MainWindow::~MainWindow()
{
    delete impl;
}

MainWindow::Impl::~Impl()
{
    stop();
}

void MainWindow::Impl::start()
{
    stop();

    QString address = addressLine->text();
    ping->setAddress(address);
    ping->setCount(count);
    ping->setWait(wait);
    ping->start();
}

void MainWindow::Impl::stop()
{
    if(ping->started()) {
        ping->stop();
        const QString text = QString("--- %1 ping statistics ---").arg(addressLine->text());
        print(text);

        const QString text2 = QString("%1 packets transmitted, %2 received, %3\% packet loss, ")
            .arg(ping->transmittedPackets()).arg(ping->receivedPackets()).arg(ping->loss());
        print(text2);

        const QString text3 = QString("rtt min/avg/max/mdev = %1/%2/%3/%4 ms")
            .arg(ping->min()).arg(ping->avg()).arg(ping->max()).arg(ping->mdev());
        print(text3);
    }
}

void MainWindow::Impl::config()
{
    PingConfigDialog dialog(self);
    dialog.setCount(count);
    dialog.setWait(wait);

    if(dialog.exec()) {
        count = dialog.count();
        wait = dialog.wait();
    }
}

void MainWindow::Impl::print(const QString& text)
{
    messageText->append(text);
}

void MainWindow::Impl::createActions()
{
    const QIcon startIcon = QIcon::fromTheme("media-playback-start");
    startAct = new QAction(startIcon, "&Start", self);
    startAct->setStatusTip("Start ping");
    self->connect(startAct, &QAction::triggered, [&](){ start(); });

    const QIcon stopIcon = QIcon::fromTheme("media-playback-stop");
    stopAct = new QAction(stopIcon, "&Stop", self);
    stopAct->setStatusTip("Stop ping");
    self->connect(stopAct, &QAction::triggered, [&](){ stop(); });

    const QIcon configIcon = QIcon::fromTheme("preferences-system");
    configAct = new QAction(configIcon, "&Config", self);
    configAct->setStatusTip("Show the config dialog");
    self->connect(configAct, &QAction::triggered, [&](){ config(); });
}

void MainWindow::Impl::createToolBars()
{
    QToolBar* pingToolBar = self->addToolBar("Ping");
    addressLine = new QLineEdit;
    addressLine->setPlaceholderText("xxx.xxx.xxx.xxx");

    pingToolBar->addWidget(addressLine);
    pingToolBar->addAction(startAct);
    pingToolBar->addAction(stopAct);
    pingToolBar->addAction(configAct);
}

PingConfigDialog::PingConfigDialog(QWidget* parent)
    : QDialog(parent)
{
    countSpin = new QSpinBox;
    countSpin->setRange(0, 9999);

    waitSpin = new QDoubleSpinBox;

    QFormLayout* layout = new QFormLayout;
    layout->addRow("Count [-]", countSpin);
    layout->addRow("Wait [s]", waitSpin);

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                     | QDialogButtonBox::Cancel);

    connect(buttonBox, &QDialogButtonBox::accepted, [&](){ accept(); });
    connect(buttonBox, &QDialogButtonBox::rejected, [&](){ reject(); });

    auto mainLayout = new QVBoxLayout;
    mainLayout->addLayout(layout);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);
    setWindowTitle("Ping Config");
}

}
