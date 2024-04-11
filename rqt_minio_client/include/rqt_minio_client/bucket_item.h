/**
   @author Kenta Suzuki
*/

#ifndef rqt_minio_client__bucket_item_H
#define rqt_minio_client__bucket_item_H

#include <aws/core/Aws.h>
#include <aws/s3/S3Client.h>

#include <QObject>

#include <vector>

namespace rqt_minio_client {

class BucketItem : public QObject
{
    Q_OBJECT
public:
    BucketItem(QObject* parent = nullptr);
    BucketItem(const Aws::String& bucketName);
    ~BucketItem();

    void setName(const Aws::String& bucketName);
    Aws::String name() const;

    bool createBucket(const Aws::Client::ClientConfiguration& clientConfig);
    bool deleteBucket(const Aws::Client::ClientConfiguration& clientConfig);
    bool headBucket(const Aws::Client::ClientConfiguration& clientConfig);
    static bool listBuckets(const Aws::String& accessKeyId,
        const Aws::String& secretKey, const Aws::Client::ClientConfiguration& clientConfig);
    static Aws::Vector<Aws::String> getBuckets(const Aws::String& accessKeyId,
        const Aws::String& secretKey, const Aws::Client::ClientConfiguration& clientConfig);

    bool copyObject(const Aws::String& objectKey,
        const Aws::String& toBucket, const Aws::Client::ClientConfiguration& clientConfig);
    bool deleteObject(const Aws::String& objectKey,
        const Aws::Client::ClientConfiguration& clientConfig);
    bool deleteObjects(std::vector<Aws::String>& objectKeys,
        const Aws::Client::ClientConfiguration& clientConfig);
    bool getObject(const Aws::String& objectKey,
        const Aws::String& fileName, const Aws::Client::ClientConfiguration& clientConfig);
    bool headObject(const Aws::String& objectKey,
        const Aws::Client::ClientConfiguration& clientConfig);
    bool listObjects(const Aws::Client::ClientConfiguration& clientConfig);
    bool listObjectsV2(const Aws::Client::ClientConfiguration& clientConfig);
    bool putObject(const Aws::String& objectKey,
        const Aws::String& fileName, const Aws::Client::ClientConfiguration& clientConfig);

    bool deleteBucketTagging(const Aws::Client::ClientConfiguration& clientConfig);
    bool getBucketTagging(const Aws::Client::ClientConfiguration& clientConfig);
    bool headBucketTagging(const Aws::String& key,
        const Aws::String& value, const Aws::Client::ClientConfiguration& clientConfig);
    bool putBucketTagging(const Aws::String& key,
        const Aws::String& value, const Aws::Client::ClientConfiguration& clientConfig);

    bool deleteObjectTagging(const Aws::String& objectKey,
        const Aws::Client::ClientConfiguration& clientConfig);
    bool getObjectTagging(const Aws::String& objectKey,
        const Aws::Client::ClientConfiguration& clientConfig);
    bool headObjectTagging(const Aws::String& objectKey,
        const Aws::String& key, const Aws::String& value, const Aws::Client::ClientConfiguration& clientConfig);
    bool putObjectTagging(const Aws::String& objectKey,
        const Aws::String& key, const Aws::String& value, const Aws::Client::ClientConfiguration& clientConfig);

    bool deleteBucketPolicy(const Aws::Client::ClientConfiguration& clientConfig);
    bool getBucketPolicy(const Aws::Client::ClientConfiguration& clientConfig);
    // bool putBucketPolicy(const Aws::String& policyBody,
    //     const Aws::Client::ClientConfiguration& clientConfig);

    Aws::Vector<Aws::S3::Model::Tag> getObjectsTagging(const Aws::String& objectKey,
        const Aws::Client::ClientConfiguration& clientConfig);
    Aws::Vector<Aws::String> getObjects(const Aws::Client::ClientConfiguration& clientConfig);
    long long getContentLength(const Aws::String& objectKey,
        const Aws::Client::ClientConfiguration& clientConfig);
    Aws::Utils::DateTime getLastModified(const Aws::String& objectKey,
        const Aws::Client::ClientConfiguration& clientConfig);
    Aws::String getETag(const Aws::String& objectKey,
        const Aws::Client::ClientConfiguration& clientConfig);

    void createCredentials(const Aws::String& accessKeyId, const Aws::String& secretKey);

private:
    class Impl;
    Impl* impl;
};

}

#endif // rqt_minio_client__bucket_item_H
