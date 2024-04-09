/**
   @author Kenta Suzuki
*/

#ifndef rqt_netem__bash_H
#define rqt_netem__bash_H

#include <QObject>

namespace rqt_netem {

class Bash : public QObject
{
    Q_OBJECT
public:
    Bash(QObject* parent = nullptr);
    ~Bash();

    void start();
    void stop();
    void write(const QString& program);

    bool started() const;

signals:
    void started();
    void stopped();
    void errored(QString error);
    void output(QString output);

private:
    class Impl;
    Impl* impl;
};

}

#endif // rqt_netem__bash_H
