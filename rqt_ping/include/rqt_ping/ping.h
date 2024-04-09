/**
   @author Kenta Suzuki
*/

#ifndef rqt_ping__ping_H
#define rqt_ping__ping_H

#include <QObject>

namespace rqt_ping {

class Ping : public QObject
{
    Q_OBJECT
public:
    Ping(QObject* parent = nullptr);
    ~Ping();

    void setAddress(const QString& address);
    void setCount(const int& count);
    void setWait(const double& second);

    void start();
    void stop();

    double min() const;
    double avg() const;
    double max() const;
    double mdev() const;
    double loss() const;

    int transmittedPackets() const;
    int receivedPackets() const;

    bool started();

signals:
    void output(QString text);

private:
    class Impl;
    Impl* impl;
};

}

#endif // rqt_ping__ping_H
