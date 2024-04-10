/**
   @author Kenta Suzuki
*/

#include "rqt_netem/bash.h"

#include <QByteArray>
#include <QProcess>

namespace rqt_netem {

class Bash::Impl
{
public:
    Bash* self;

    Impl(Bash* self);

    void start();
    void close();
    void write();

    void on_process_readyReadStandardError();
    void on_process_readyReadStandardOutput();
    void on_process_stateChanged(QProcess::ProcessState newState);

    QProcess process;
    QString program;

    bool is_started;
};

Bash::Bash(QObject* parent)
    : QObject(parent)
{
    impl = new Impl(this);
}

Bash::Impl::Impl(Bash* self)
    : self(self)
{
    is_started = false;

    self->connect(&process, &QProcess::readyReadStandardError,
        [&](){ on_process_readyReadStandardError(); });
    self->connect(&process, &QProcess::readyReadStandardOutput,
        [&](){ on_process_readyReadStandardOutput(); });
    self->connect(&process, &QProcess::stateChanged,
        [&](QProcess::ProcessState newState){ on_process_stateChanged(newState); });
}

Bash::~Bash()
{
    delete impl;
}

void Bash::start()
{
    impl->start();
}

void Bash::Impl::start()
{
    is_started = true;
    process.start("bash");
    emit self->started();
}

void Bash::stop()
{
    impl->close();
}

void Bash::Impl::close()
{
    is_started = false;
    process.close();
    emit self->stopped();
}

void Bash::write(const QString& program)
{
    impl->program = program;
    impl->write();
}

void Bash::Impl::write()
{
    QByteArray data;
    data.append(program);
    data.append("\n");
    process.write(data);
}

bool Bash::started() const
{
    return impl->is_started;
}

void Bash::Impl::on_process_readyReadStandardError()
{
    emit self->errored(process.readAllStandardError());
}

void Bash::Impl::on_process_readyReadStandardOutput()
{
    emit self->output(process.readAllStandardOutput());
}

void Bash::Impl::on_process_stateChanged(QProcess::ProcessState newState)
{
    switch(newState) {
        case QProcess::NotRunning:
            break;
        case QProcess::Starting:
            break;
        case QProcess::Running:
            // write();
            break;
        default:
            break;
    }
}

}
