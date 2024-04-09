/**
   @author Kenta Suzuki
*/

#include "rqt_ping/ping.h"

#include <QByteArray>
#include <QProcess>
#include <QStringList>
#include <QVector>
#include <QtMath>

#include <algorithm>

namespace rqt_ping {

class Ping::Impl
{
public:
    Ping* self;

    Impl(Ping* self);

    void start();
    void close();
    void write();

    void on_process_finished(const int& exitCode, QProcess::ExitStatus exitStatus);
    void on_process_readyReadStandardError();
    void on_process_readyReadStandardOutput();
    void on_process_stateChanged(QProcess::ProcessState newState);

    QProcess process;
    QString address;
    QVector<double> times;

    int count;
    double second;
    bool is_started;
    double min;
    double avg;
    double max;
    double mdev;
    double loss;
    int transmitted_packets;
    int received_packets;
};


Ping::Ping(QObject* parent)
    : QObject(parent)
{
    impl = new Impl(this);
}

Ping::Impl::Impl(Ping* self)
    : self(self)
{
    count = 0;
    second = 1.0;
    is_started = false;
    times.clear();
    min = 0.0;
    avg = 0.0;
    max = 0.0;
    mdev = 0.0;
    loss = 0.0;
    transmitted_packets = 0;
    received_packets = 0;

    self->connect(&process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
        [=](int exitCode, QProcess::ExitStatus exitStatus){ on_process_finished(exitCode, exitStatus); });
    self->connect(&process, &QProcess::readyReadStandardError, [&](){ on_process_readyReadStandardError(); });
    self->connect(&process, &QProcess::readyReadStandardOutput, [&](){ on_process_readyReadStandardOutput(); });
    self->connect(&process, &QProcess::stateChanged, [&](QProcess::ProcessState newState){ on_process_stateChanged(newState); });
}

Ping::~Ping()
{
    delete impl;
}

void Ping::setAddress(const QString& address)
{
    impl->address = address;
}

void Ping::setCount(const int& count)
{
    impl->count = count;
}

void Ping::setWait(const double& second)
{
    if(second >= 0.2) {
        impl->second = second;
    }
}

void Ping::start()
{
    impl->start();
}

void Ping::Impl::start()
{
    is_started = true;
    times.clear();
    min = 0.0;
    avg = 0.0;
    max = 0.0;
    mdev = 0.0;
    loss = 0.0;
    transmitted_packets = 0;
    received_packets = 0;
    process.start("bash");
}

void Ping::stop()
{
    impl->close();
}

void Ping::Impl::close()
{
    is_started = false;
    process.close();
}

double Ping::min() const
{
    return impl->min;
}

double Ping::avg() const
{
    return impl->avg;
}

double Ping::max() const
{
    return impl->max;
}

double Ping::mdev() const
{
    return impl->mdev;
}

double Ping::loss() const
{
    return impl->loss;
}

int Ping::transmittedPackets() const
{
    return impl->transmitted_packets;
}

int Ping::receivedPackets() const
{
    return impl->received_packets;
}

void Ping::Impl::write()
{
    if(address.isEmpty()) {
        return;
    }
    QByteArray data;
    data.append("LANG=C ping " + address);
    if(count > 0) {
        data.append(QString(" -c %1").arg(count));
    }
    data.append(QString(" -i %1").arg(second));
    data.append("\n");
    process.write(data);
}

bool Ping::started()
{
    return impl->is_started;
}

void Ping::Impl::on_process_finished(const int& exitCode, QProcess::ExitStatus exitStatus)
{
    QString text = process.readAllStandardOutput().trimmed();
    emit self->output(text);
    // emit self->output("Finished");
}

void Ping::Impl::on_process_readyReadStandardError()
{
    QString text = process.readAllStandardError();
    emit self->output(text);
}

void Ping::Impl::on_process_readyReadStandardOutput()
{
    QString text = process.readAllStandardOutput().trimmed();
    emit self->output(text);

    QStringList list = text.split(" ");
    for(int i = 0; i < list.size(); ++i) {
        QString element = list.at(i);
        if(element.contains("time=")) {
            QStringList list2 = element.split("=");
            if(list2.size() == 2) {
                times.push_back(list2[1].toDouble());
                min = *std::min_element(times.constBegin(), times.constEnd());
                avg = std::accumulate(times.constBegin(), times.constEnd(), 0.0) / (double)times.size();
                max = *std::max_element(times.constBegin(), times.constEnd());
                double sum = 0.0;
                for(int j = 0; j < times.size(); ++j) {
                    double time = times[j];
                    sum += (time - avg) * (time - avg);
                }
                mdev = qSqrt(sum / (double)times.size());
            }
        }
    }

    for(int i = 0; i < list.size(); ++i) {
        QString element = list.at(i);
        if(element.contains("icmp_seq=")) {
            QStringList list2 = element.split("=");
            if(list2.size() == 2) {
                int seq = list2[1].toInt();
                transmitted_packets = seq;
                received_packets = seq - (seq - times.size());
                loss = ((double)seq - (double)times.size()) / (double)seq * 100.0;
            }
        }
    }
}

void Ping::Impl::on_process_stateChanged(QProcess::ProcessState newState)
{
    switch(newState) {
        case QProcess::NotRunning:
            // emit self->output("Not runnging");
            break;
        case QProcess::Starting:
            // emit self->output("Starting");
            break;
        case QProcess::Running:
            // emit self->output("Running");
            write();
            break;
        default:
            break;
    }
}

}
