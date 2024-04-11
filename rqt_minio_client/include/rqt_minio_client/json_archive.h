/**
   @author Kenta Suzuki
*/

#ifndef rqt_minio_client__json_archive_H
#define rqt_minio_client__json_archive_H

#include <QJsonObject>
#include <QVector>

namespace rqt_minio_client {

class JsonArchive
{
public:
    JsonArchive();
    ~JsonArchive();

    bool loadFile(const QString& fileName);
    bool saveFile(const QString& fileName);
    
    virtual void read(const QJsonObject& json) = 0;
    virtual void write(QJsonObject& json) = 0;

    int get(const QJsonObject& json, const QString& key, const int& init) const;
    double get(const QJsonObject& json, const QString& key, const double& init) const;
    bool get(const QJsonObject& json, const QString& key, const bool& init) const;
    QString get(const QJsonObject& json, const QString& key, const char* init) const;

    bool get(const QJsonObject& json, const QString& key, QVector<int>& vector);
    bool get(const QJsonObject& json, const QString& key, QVector<double>& vector);
    bool get(const QJsonObject& json, const QString& key, QVector<bool>& vector);
    bool get(const QJsonObject& json, const QString& key, QVector<QString>& vector);

private:
    class Impl;
    Impl* impl;
};

}

#endif // rqt_minio_client__json_archive_H
