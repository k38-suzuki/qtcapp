/**
   @author Kenta Suzuki
*/

#include "rqt_minio_client/bucket_item.h"

#include <aws/s3/model/CopyObjectRequest.h>
#include <aws/s3/model/DeleteObjectRequest.h>
#include <aws/s3/model/DeleteObjectsRequest.h>
#include <aws/s3/model/DeleteObjectTaggingRequest.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <aws/s3/model/GetObjectTaggingRequest.h>
#include <aws/s3/model/HeadObjectRequest.h>
#include <aws/s3/model/ListObjectsRequest.h>
#include <aws/s3/model/ListObjectsV2Request.h>
#include <aws/s3/model/PutObjectRequest.h>
#include <aws/s3/model/PutObjectTaggingRequest.h>
#include <aws/s3/model/Object.h>

#include <aws/s3/model/CreateBucketRequest.h>
#include <aws/s3/model/DeleteBucketRequest.h>
#include <aws/s3/model/DeleteBucketTaggingRequest.h>
#include <aws/s3/model/GetBucketTaggingRequest.h>
#include <aws/s3/model/HeadBucketRequest.h>
#include <aws/s3/model/PutBucketTaggingRequest.h>
#include <aws/s3/model/BucketLocationConstraint.h>
#include <aws/core/utils/UUID.h>
#include <aws/core/utils/StringUtils.h>
#include <aws/s3/model/Bucket.h>

#include <aws/s3/model/DeleteBucketPolicyRequest.h>
#include <aws/s3/model/GetBucketPolicyRequest.h>
// #include <aws/s3/model/PutBucketPolicyRequest.h>
// #include <aws/sts/STSClient.h>
// #include <aws/sts/model/GetCallerIdentityRequest.h>

#include <aws/core/auth/AWSAuthSigner.h>
#include <aws/core/auth/AWSCredentialsProvider.h>

#include <QMimeType>
#include <QMimeDatabase>

// #include <cstdio>
#include <fstream>
#include <sstream>
#include <sys/stat.h>

#define DEBUG

namespace {

Aws::Vector<Aws::S3::Model::Bucket> bucketList;

}

namespace rqt_minio_client {

class BucketItem::Impl
{
public:
    BucketItem* self;

    Impl(BucketItem* self);

    Aws::String bucketName;
    Aws::String token;
    Aws::Auth::AWSCredentials credentials;

    Aws::Vector<Aws::S3::Model::Tag> bucketTags;
    Aws::Vector<Aws::S3::Model::Tag> objectTags;
    Aws::Vector<Aws::S3::Model::Object> objectList;

    long long contentLength;
    Aws::Utils::DateTime lastModified;
    Aws::String eTag;

#ifdef DEBUG
    std::stringstream cout;
#endif

    bool createBucket(const Aws::String& bucketName,
        const Aws::Client::ClientConfiguration& clientConfig);
    bool deleteBucket(const Aws::String& bucketName,
        const Aws::Client::ClientConfiguration& clientConfig);
    bool headBucket(const Aws::String& bucketName,
        const Aws::Client::ClientConfiguration& clientConfig);
    bool copyObject(const Aws::String& objectKey, const Aws::String& fromBucket, const Aws::String& toBucket,
        const Aws::Client::ClientConfiguration& clientConfig);
    bool deleteObject(const Aws::String& objectKey,
        const Aws::String& fromBucket, const Aws::Client::ClientConfiguration& clientConfig);
    bool deleteObjects(const std::vector<Aws::String>& objectKeys,
        const Aws::String& fromBucket, const Aws::Client::ClientConfiguration& clientConfig);
    bool getObject(const Aws::String& objectKey,
        const Aws::String& fromBucket, const Aws::String& fileName, const Aws::Client::ClientConfiguration& clientConfig);
    bool headObject(const Aws::String& objectKey,
        const Aws::String& fromBucket, const Aws::Client::ClientConfiguration& clientConfig);
    bool listObjects(const Aws::String& bucketName,
        const Aws::Client::ClientConfiguration& clientConfig);
    bool listObjectsV2(const Aws::String& bucketName,
        const Aws::Client::ClientConfiguration& clientConfig);
    bool putObject(const Aws::String& objectKey,
        const Aws::String& bucketName, const Aws::String& fileName, const Aws::Client::ClientConfiguration& clientConfig);
    bool deleteBucketTagging(const Aws::String& bucketName,
        const Aws::Client::ClientConfiguration& clientConfig);
    bool getBucketTagging(const Aws::String& bucketName,
        const Aws::Client::ClientConfiguration& clientConfig);
    bool headBucketTagging(const Aws::String& bucketName,
        const Aws::String& key, const Aws::String& value, const Aws::Client::ClientConfiguration& clientConfig);
    bool putBucketTagging(const Aws::String& bucketName,
        const Aws::String& key, const Aws::String& value, const Aws::Client::ClientConfiguration& clientConfig);
    bool deleteObjectTagging(const Aws::String& bucketName,
        const Aws::String& objectKey, const Aws::Client::ClientConfiguration& clientConfig);
    bool getObjectTagging(const Aws::String& bucketName,
        const Aws::String& objectKey, const Aws::Client::ClientConfiguration& clientConfig);
    bool headObjectTagging(const Aws::String& bucketName,
        const Aws::String& objectKey, const Aws::String& key, const Aws::String& value, const Aws::Client::ClientConfiguration& clientConfig);
    bool putObjectTagging(const Aws::String& bucketName,
        const Aws::String& objectKey, const Aws::String& key, const Aws::String& value, const Aws::Client::ClientConfiguration& clientConfig);
    bool deleteBucketPolicy(const Aws::String& bucketName,
        const Aws::Client::ClientConfiguration& clientConfig);
    bool getBucketPolicy(const Aws::String& bucketName,
        const Aws::Client::ClientConfiguration& clientConfig);
    // bool putBucketPolicy(const Aws::String& bucketName,
    //     const Aws::String& policyBody, const Aws::Client::ClientConfiguration& clientConfig);
};

BucketItem::BucketItem(QObject* parent)
    : QObject(parent)
{
    impl = new Impl(this);
}

BucketItem::BucketItem(const Aws::String& bucketName)
    : impl(new Impl(this))
{
    impl->bucketName = bucketName;
}

BucketItem::Impl::Impl(BucketItem* self)
    : self(self)
{
    bucketName.clear();
    token.clear();
    bucketTags.clear();
    objectTags.clear();
    objectList.clear();
}

BucketItem::~BucketItem()
{
    delete impl;
}

void BucketItem::setName(const Aws::String& bucketName)
{
    impl->bucketName = bucketName;
}

Aws::String BucketItem::name() const
{
    return impl->bucketName;
}

bool BucketItem::createBucket(const Aws::Client::ClientConfiguration& clientConfig)
{
    return impl->createBucket(impl->bucketName, clientConfig);
}

bool BucketItem::Impl::createBucket(const Aws::String& bucketName,
    const Aws::Client::ClientConfiguration& clientConfig)
{
    // Aws::S3::S3Client client(clientConfig);
    Aws::S3::S3Client client(credentials, clientConfig,
        Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy::Never, false);
    Aws::S3::Model::CreateBucketRequest request;
    request.SetBucket(bucketName);

    //TODO(user): Change the bucket location constraint enum to your target Region.
    if(clientConfig.region != "us-east-1") {
        Aws::S3::Model::CreateBucketConfiguration createBucketConfig;
        createBucketConfig.SetLocationConstraint(
            Aws::S3::Model::BucketLocationConstraintMapper::GetBucketLocationConstraintForName(
                clientConfig.region));
        request.SetCreateBucketConfiguration(createBucketConfig);
    }

    Aws::S3::Model::CreateBucketOutcome outcome = client.CreateBucket(request);
    if(!outcome.IsSuccess()) {
        auto err = outcome.GetError();
        std::cerr << "Error: CreateBucket: " <<
            err.GetExceptionName() << ": " << err.GetMessage() << std::endl;
    } else {
        std::cout << "Created bucket " << bucketName <<
            " in the specified AWS Region." << std::endl;
    }

    return outcome.IsSuccess();
}

bool BucketItem::deleteBucket(const Aws::Client::ClientConfiguration& clientConfig)
{
    return impl->deleteBucket(impl->bucketName, clientConfig);
}

bool BucketItem::Impl::deleteBucket(const Aws::String& bucketName,
    const Aws::Client::ClientConfiguration& clientConfig)
{
    // Aws::S3::S3Client client(clientConfig);
    Aws::S3::S3Client client(credentials, clientConfig,
        Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy::Never, false);

    Aws::S3::Model::DeleteBucketRequest request;
    request.SetBucket(bucketName);

    Aws::S3::Model::DeleteBucketOutcome outcome =
        client.DeleteBucket(request);

    if(!outcome.IsSuccess()) {
        const Aws::S3::S3Error& err = outcome.GetError();
        std::cerr << "Error: DeleteBucket: " <<
            err.GetExceptionName() << ": " << err.GetMessage() << std::endl;
    } else {
        std::cout << "The bucket was deleted" << std::endl;
    }

    return outcome.IsSuccess();
}

bool BucketItem::headBucket(const Aws::Client::ClientConfiguration& clientConfig)
{
    return impl->headBucket(impl->bucketName, clientConfig);
}

bool BucketItem::Impl::headBucket(const Aws::String& bucketName,
    const Aws::Client::ClientConfiguration& clientConfig)
{
    // Aws::S3::S3Client client(clientConfig);
    Aws::S3::S3Client client(credentials, clientConfig,
        Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy::Never, false);

    Aws::S3::Model::HeadBucketRequest request;
    request.SetBucket(bucketName);

    Aws::S3::Model::HeadBucketOutcome outcome = client.HeadBucket(request);

    if(!outcome.IsSuccess()) {
        const Aws::S3::S3Error& err = outcome.GetError();
        std::cerr << "Error: HeadBucket: " <<
            err.GetExceptionName() << ": " << err.GetMessage() << std::endl;
    } else {
        std::cout << "The bucket is found" << std::endl;
    }

    return outcome.IsSuccess();
}

bool BucketItem::listBuckets(const Aws::String& accessKeyId,
    const Aws::String& secretKey, const Aws::Client::ClientConfiguration& clientConfig)
{
    // Aws::S3::S3Client client(clientConfig);
    Aws::Auth::AWSCredentials credentials;
    credentials.SetAWSAccessKeyId(accessKeyId);
    credentials.SetAWSSecretKey(secretKey);
    Aws::S3::S3Client client(credentials, clientConfig,
        Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy::Never, false);

    auto outcome = client.ListBuckets();

    bool result = true;
    if(!outcome.IsSuccess()) {
        std::cerr << "Failed with error: " << outcome.GetError() << std::endl;
        result = false;
    } else {
        std::cout << "Found " << outcome.GetResult().GetBuckets().size() << " buckets\n";
        for(auto&& b: outcome.GetResult().GetBuckets()) {
            std::cout << b.GetName() << std::endl;
        }

        bucketList.clear();
        bucketList = outcome.GetResult().GetBuckets();
    }

    return result;
}

Aws::Vector<Aws::String> BucketItem::getBuckets(const Aws::String& accessKeyId,
    const Aws::String& secretKey, const Aws::Client::ClientConfiguration& clientConfig)
{
    Aws::Vector<Aws::String> buckets;
    bucketList.clear();
    if(!BucketItem::listBuckets(accessKeyId, secretKey, clientConfig)) {
        return buckets;
    }

    for(int i = 0; i < bucketList.size(); ++i) {
        buckets.push_back(bucketList[i].GetName());
    }

    return buckets;
}

bool BucketItem::copyObject(const Aws::String& objectKey,
    const Aws::String& toBucket, const Aws::Client::ClientConfiguration& clientConfig)
{
    return impl->copyObject(objectKey, impl->bucketName, toBucket, clientConfig);
}

bool BucketItem::Impl::copyObject(const Aws::String& objectKey, const Aws::String& fromBucket, const Aws::String& toBucket,
    const Aws::Client::ClientConfiguration& clientConfig)
{
    // Aws::S3::S3Client client(clientConfig);
    Aws::S3::S3Client client(credentials, clientConfig,
        Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy::Never, false);
    Aws::S3::Model::CopyObjectRequest request;

    request.WithCopySource(fromBucket + "/" + objectKey)
        .WithKey(objectKey)
        .WithBucket(toBucket);

    Aws::S3::Model::CopyObjectOutcome outcome = client.CopyObject(request);
    if(!outcome.IsSuccess()) {
        const Aws::S3::S3Error& err = outcome.GetError();
        std::cerr << "Error: CopyObject: " <<
            err.GetExceptionName() << ": " << err.GetMessage() << std::endl;

    } else {
        std::cout << "Successfully copied " << objectKey << " from " << fromBucket <<
            " to " << toBucket << "." << std::endl;
    }

    return outcome.IsSuccess();
}

bool BucketItem::deleteObject(const Aws::String& objectKey,
    const Aws::Client::ClientConfiguration& clientConfig)
{
    if(!impl->deleteObject(objectKey, impl->bucketName, clientConfig)) {
        return false;
    }
    return true;
}

bool BucketItem::Impl::deleteObject(const Aws::String& objectKey,
    const Aws::String& fromBucket,
    const Aws::Client::ClientConfiguration& clientConfig)
{
    // Aws::S3::S3Client client(clientConfig);
    Aws::S3::S3Client client(credentials, clientConfig,
        Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy::Never, false);
    Aws::S3::Model::DeleteObjectRequest request;

    request.WithKey(objectKey)
        .WithBucket(fromBucket);

    Aws::S3::Model::DeleteObjectOutcome outcome =
        client.DeleteObject(request);

    if(!outcome.IsSuccess()) {
        auto err = outcome.GetError();
        std::cerr << "Error: DeleteObject: " <<
            err.GetExceptionName() << ": " << err.GetMessage() << std::endl;
    } else {
        std::cout << "Successfully deleted the object." << std::endl;
    }

    return outcome.IsSuccess();
}

bool BucketItem::deleteObjects(std::vector<Aws::String>& objectKeys,
    const Aws::Client::ClientConfiguration& clientConfig)
{
    return impl->deleteObjects(objectKeys, impl->bucketName, clientConfig);
}

bool BucketItem::Impl::deleteObjects(const std::vector<Aws::String>& objectKeys,
    const Aws::String& fromBucket,
    const Aws::Client::ClientConfiguration& clientConfig)
{
    // Aws::S3::S3Client client(clientConfig);
    Aws::S3::S3Client client(credentials, clientConfig,
        Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy::Never, false);
    Aws::S3::Model::DeleteObjectsRequest request;

    Aws::S3::Model::Delete deleteObject;
    for(const Aws::String& objectKey : objectKeys) {
        deleteObject.AddObjects(Aws::S3::Model::ObjectIdentifier().WithKey(objectKey));
    }

    request.SetDelete(deleteObject);
    request.SetBucket(fromBucket);

    Aws::S3::Model::DeleteObjectsOutcome outcome =
        client.DeleteObjects(request);

    if(!outcome.IsSuccess()) {
        auto err = outcome.GetError();
        std::cerr << "Error deleting objects. " <<
            err.GetExceptionName() << ": " << err.GetMessage() << std::endl;
    } else {
        std::cout << "Successfully deleted the objects.";
        for(size_t i = 0; i < objectKeys.size(); ++i) {
            std::cout << objectKeys[i];
            if(i < objectKeys.size() - 1) {
                std::cout << ", ";
            }
        }

        std::cout << " from bucket " << fromBucket << "." << std::endl;
    }

    return outcome.IsSuccess();
}

bool BucketItem::getObject(const Aws::String& objectKey,
    const Aws::String& fileName, const Aws::Client::ClientConfiguration& clientConfig)
{
    if(!impl->getObject(objectKey, impl->bucketName, fileName, clientConfig)) {
        return false;
    }
    return true;
}

bool BucketItem::Impl::getObject(const Aws::String& objectKey,
    const Aws::String& fromBucket, const Aws::String& fileName, const Aws::Client::ClientConfiguration& clientConfig)
{
    // Aws::S3::S3Client client(clientConfig);
    Aws::S3::S3Client client(credentials, clientConfig,
        Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy::Never, false);

    Aws::S3::Model::GetObjectRequest request;
    request.SetBucket(fromBucket);
    request.SetKey(objectKey);

    Aws::S3::Model::GetObjectOutcome outcome =
        client.GetObject(request);

    if(!outcome.IsSuccess()) {
        const Aws::S3::S3Error& err = outcome.GetError();
        std::cerr << "Error: GetObject: " <<
            err.GetExceptionName() << ": " << err.GetMessage() << std::endl;
    } else {
        // std::cout << "Successfully retrieved '" << objectKey << "' from '"
        //     << fromBucket << "'." << std::endl;

        // std::cout << "Downloaded the object with the key, '" << objectKey << "', in the bucket, '"
        //     << fromBucket << "'." << std::endl;

        contentLength = outcome.GetResult().GetContentLength();
        lastModified = outcome.GetResult().GetLastModified();
        eTag = outcome.GetResult().GetETag();

        if(!fileName.empty()) {
            Aws::IOStream &ioStream = outcome.GetResultWithOwnership().
                    GetBody();
            Aws::OFStream outStream(fileName);
            if(!outStream.is_open()) {
                std::cerr << "Error: unable to open file, '" << fileName << "'." << std::endl;
            } else {
                outStream << ioStream.rdbuf();
                std::cout << "Wrote the downloaded object to the file '"
                    << fileName << "'." << std::endl;
            }
        }
    }

    return outcome.IsSuccess();
}

bool BucketItem::headObject(const Aws::String& objectKey,
    const Aws::Client::ClientConfiguration& clientConfig)
{
    return impl->headObject(objectKey, impl->bucketName, clientConfig);
}

bool BucketItem::Impl::headObject(const Aws::String& objectKey,
    const Aws::String& fromBucket, const Aws::Client::ClientConfiguration& clientConfig)
{
    // Aws::S3::S3Client client(clientConfig);
    Aws::S3::S3Client client(credentials, clientConfig,
        Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy::Never, false);

    Aws::S3::Model::HeadObjectRequest request;
    request.SetBucket(fromBucket);

    Aws::S3::Model::HeadObjectOutcome outcome =
        client.HeadObject(request);

    if(!outcome.IsSuccess()) {
        const Aws::S3::S3Error& err = outcome.GetError();
        std::cerr << "Error: HeadObject: " <<
            err.GetExceptionName() << ": " << err.GetMessage() << std::endl;
    } else {
        std::cout << "The object is found" << std::endl;
    }

    return outcome.IsSuccess();
}

bool BucketItem::listObjects(const Aws::Client::ClientConfiguration& clientConfig)
{
    return impl->listObjects(impl->bucketName, clientConfig);
}

bool BucketItem::listObjectsV2(const Aws::Client::ClientConfiguration& clientConfig)
{
    impl->token.clear();
    return impl->listObjectsV2(impl->bucketName, clientConfig);
}

bool BucketItem::Impl::listObjects(const Aws::String& bucketName,
    const Aws::Client::ClientConfiguration& clientConfig)
{
    // Aws::S3::S3Client s3_client(clientConfig);
    Aws::S3::S3Client s3_client(credentials, clientConfig,
        Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy::Never, false);

    Aws::S3::Model::ListObjectsRequest request;
    request.WithBucket(bucketName);

    auto outcome = s3_client.ListObjects(request);

    if(!outcome.IsSuccess()) {
        std::cerr << "Error: ListObjects: " <<
            outcome.GetError().GetMessage() << std::endl;
    } else {
        Aws::Vector<Aws::S3::Model::Object> objects =
            outcome.GetResult().GetContents();

        for(Aws::S3::Model::Object& object: objects) {
            std::cout << object.GetKey() << std::endl;
        }

        objectList.clear();
        objectList = objects;
    }

    return outcome.IsSuccess();
}

bool BucketItem::Impl::listObjectsV2(const Aws::String& bucketName,
    const Aws::Client::ClientConfiguration& clientConfig)
{
    // Aws::S3::S3Client s3_client(clientConfig);
    Aws::S3::S3Client s3_client(credentials, clientConfig,
        Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy::Never, false);

    Aws::S3::Model::ListObjectsV2Request request;
    request.WithBucket(bucketName);
    if(!token.empty()) {
        request.SetContinuationToken(token);
    } else {
        objectList.clear();
    }

    auto outcome = s3_client.ListObjectsV2(request);

    if(!outcome.IsSuccess()) {
        std::cerr << "Error: ListObjectsV2: " <<
            outcome.GetError().GetMessage() << std::endl;
    } else {
        Aws::Vector<Aws::S3::Model::Object> objects =
            outcome.GetResult().GetContents();

        for(Aws::S3::Model::Object& object: objects) {
            std::cout << object.GetKey() << std::endl;
            objectList.push_back(object);
        }

        if(outcome.GetResult().GetIsTruncated()) {
            token = outcome.GetResult().GetNextContinuationToken();
            listObjectsV2(bucketName, clientConfig);
        }
    }

    return outcome.IsSuccess();
}

bool BucketItem::putObject(const Aws::String& objectKey, const Aws::String& fileName,
    const Aws::Client::ClientConfiguration& clientConfig)
{
    if(!impl->putObject(objectKey, impl->bucketName, fileName, clientConfig)) {
        return false;
    }
    return true;
}

bool BucketItem::Impl::putObject(const Aws::String& objectKey,
    const Aws::String& bucketName, const Aws::String& fileName, const Aws::Client::ClientConfiguration& clientConfig)
{
    // Aws::S3::S3Client s3_client(clientConfig);
    Aws::S3::S3Client s3_client(credentials, clientConfig,
        Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy::Never, false);

    Aws::S3::Model::PutObjectRequest request;
    request.SetBucket(bucketName);
    //We are using the name of the file as the key for the object in the bucket.
    //However, this is just a std::string and can be set according to your retrieval needs.
    request.SetKey(objectKey);

    QMimeDatabase database;
    QMimeType mimeType = database.mimeTypeForFile(fileName.c_str());
    Aws::String name = mimeType.name().toStdString();
    request.SetContentType(name.c_str());

    std::shared_ptr<Aws::IOStream> inputData =
        Aws::MakeShared<Aws::FStream>("SampleAllocationTag",
            fileName.c_str(),
            std::ios_base::in | std::ios_base::binary);

    if(!*inputData) {
        std::cerr << "Error unable to read file " << fileName << std::endl;
        return false;
    }

    request.SetBody(inputData);

    Aws::S3::Model::PutObjectOutcome outcome =
        s3_client.PutObject(request);

    if(!outcome.IsSuccess()) {
        std::cerr << "Error: PutObject: " <<
            outcome.GetError().GetMessage() << std::endl;
    } else {
        std::cout << "Added object '" << fileName << "' to bucket '"
            << bucketName << "'." << std::endl;
    }

    return outcome.IsSuccess();
}

bool BucketItem::deleteBucketTagging(const Aws::Client::ClientConfiguration& clientConfig)
{
    return impl->deleteBucketTagging(impl->bucketName, clientConfig);
}

bool BucketItem::Impl::deleteBucketTagging(const Aws::String& bucketName,
    const Aws::Client::ClientConfiguration& clientConfig)
{
    // Aws::S3::S3Client client(clientConfig);
    Aws::S3::S3Client client(credentials, clientConfig,
        Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy::Never, false);

    Aws::S3::Model::DeleteBucketTaggingRequest request;
    request.SetBucket(bucketName);

    Aws::S3::Model::DeleteBucketTaggingOutcome outcome = client.DeleteBucketTagging(request);
    if(!outcome.IsSuccess()) {
        auto err = outcome.GetError();
        std::cerr << "Error: DeleteBucketTagging: " << 
            err.GetExceptionName() << ": " << err.GetMessage() << std::endl;
    } else {
        std::cout << "Successfully delete a tag from the bucket '" << bucketName
            << "'." << std::endl;
    }

    return outcome.IsSuccess();
}

bool BucketItem::getBucketTagging(const Aws::Client::ClientConfiguration& clientConfig)
{
    return impl->getBucketTagging(impl->bucketName, clientConfig);
}

bool BucketItem::Impl::getBucketTagging(const Aws::String& bucketName,
    const Aws::Client::ClientConfiguration& clientConfig)
{
    // Aws::S3::S3Client client(clientConfig);
    Aws::S3::S3Client client(credentials, clientConfig,
        Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy::Never, false);

    Aws::S3::Model::GetBucketTaggingRequest request;
    request.SetBucket(bucketName);

    Aws::S3::Model::GetBucketTaggingOutcome outcome = client.GetBucketTagging(request);
    if(!outcome.IsSuccess()) {
        auto err = outcome.GetError();
        std::cerr << "Error: GetBucketTagging: " << 
                  err.GetExceptionName() << ": " << err.GetMessage() << std::endl;
    } else {
        std::cout << "Successfully get tags from the bucket '" << bucketName
            << "'." << std::endl;

        Aws::Vector<Aws::S3::Model::Tag> tags =
            outcome.GetResult().GetTagSet();

        for(Aws::S3::Model::Tag& tag : tags) {
            std::cout << tag.GetKey() << " : " << tag.GetValue() << std::endl;
        }

        bucketTags.clear();
        bucketTags = tags;
    }

    return outcome.IsSuccess();
}

bool BucketItem::headBucketTagging(const Aws::String& key,
    const Aws::String& value, const Aws::Client::ClientConfiguration& clientConfig)
{
    return impl->headBucketTagging(impl->bucketName, key, value, clientConfig);
}

bool BucketItem::Impl::headBucketTagging(const Aws::String& bucketName,
    const Aws::String& key,
    const Aws::String& value,
    const Aws::Client::ClientConfiguration& clientConfig)
{
    bool result = false;
    if(!getBucketTagging(bucketName, clientConfig)) {
        result = false;
    }

    for(Aws::S3::Model::Tag& tag : bucketTags) {
        if(tag.GetKey() == key && tag.GetValue() == value) {
            result = true;
        }
    }

    return result;
}

bool BucketItem::putBucketTagging(const Aws::String& key,
    const Aws::String& value, const Aws::Client::ClientConfiguration& clientConfig)
{
    return impl->putBucketTagging(impl->bucketName, key, value, clientConfig);
}

bool BucketItem::Impl::putBucketTagging(const Aws::String& bucketName,
    const Aws::String& key,
    const Aws::String& value, 
    const Aws::Client::ClientConfiguration& clientConfig)
{
    // Aws::S3::S3Client client(clientConfig);
    Aws::S3::S3Client client(credentials, clientConfig,
        Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy::Never, false);

    if(!getBucketTagging(key, clientConfig)) {
        return false;
    }

    Aws::S3::Model::PutBucketTaggingRequest request;
    request.SetBucket(bucketName);

    Aws::S3::Model::Tag tag;
    tag.SetKey(key);
    tag.SetValue(value);
    bucketTags.push_back(tag);
    Aws::S3::Model::Tagging tagging;
    tagging.SetTagSet(bucketTags);
    request.SetTagging(tagging);

    Aws::S3::Model::PutBucketTaggingOutcome outcome = client.PutBucketTagging(request);
    if(!outcome.IsSuccess()) {
        auto err = outcome.GetError();
        std::cerr << "Error: PutBucketTagging: " << 
            err.GetExceptionName() << ": " << err.GetMessage() << std::endl;
    } else {
        std::cout << "Successfully put a tag to the bucket '" << bucketName
            << "'." << std::endl;
    }

    return outcome.IsSuccess(); 
}

bool BucketItem::deleteObjectTagging(const Aws::String& objectKey,
    const Aws::Client::ClientConfiguration& clientConfig)
{
    return impl->deleteObjectTagging(impl->bucketName, objectKey, clientConfig);
}

bool BucketItem::Impl::deleteObjectTagging(const Aws::String& bucketName,
    const Aws::String& objectKey, const Aws::Client::ClientConfiguration& clientConfig)
{
    // Aws::S3::S3Client client(clientConfig);
    Aws::S3::S3Client client(credentials, clientConfig,
        Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy::Never, false);

    Aws::S3::Model::DeleteObjectTaggingRequest request;
    request.SetBucket(bucketName);
    request.SetKey(objectKey);

    Aws::S3::Model::DeleteObjectTaggingOutcome outcome = client.DeleteObjectTagging(request);
    if(!outcome.IsSuccess()) {
        auto err = outcome.GetError();
        std::cerr << "Error: DeleteObjectTagging: " << 
            err.GetExceptionName() << ": " << err.GetMessage() << std::endl;
    } else {
        std::cout << "Successfully delete a tag from the object '" << objectKey
            << "'." << std::endl;
    }

    return outcome.IsSuccess();
}

bool BucketItem::getObjectTagging(const Aws::String& objectKey,
    const Aws::Client::ClientConfiguration& clientConfig)
{
    return impl->getObjectTagging(impl->bucketName, objectKey, clientConfig);
}

bool BucketItem::Impl::getObjectTagging(const Aws::String& bucketName,
    const Aws::String& objectKey, const Aws::Client::ClientConfiguration& clientConfig)
{
    // Aws::S3::S3Client client(clientConfig);
    Aws::S3::S3Client client(credentials, clientConfig,
        Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy::Never, false);

    Aws::S3::Model::GetObjectTaggingRequest request;
    request.SetBucket(bucketName);
    request.SetKey(objectKey);

    Aws::S3::Model::GetObjectTaggingOutcome outcome = client.GetObjectTagging(request);
    if(!outcome.IsSuccess()) {
        auto err = outcome.GetError();
        std::cerr << "Error: GetObjectTagging: " << 
            err.GetExceptionName() << ": " << err.GetMessage() << std::endl;
    } else {
        std::cout << "Successfully get tags from the object '" << objectKey
            << "'." << std::endl;

        Aws::Vector<Aws::S3::Model::Tag> tags =
            outcome.GetResult().GetTagSet();

        for(Aws::S3::Model::Tag& tag : tags) {
            // std::cout << tag.GetKey() << " : " << tag.GetValue() << std::endl;
        }

        objectTags.clear();
        objectTags = tags;
    }

    return outcome.IsSuccess();
}

Aws::Vector<Aws::S3::Model::Tag> BucketItem::getObjectsTagging(const Aws::String& objectKey,
    const Aws::Client::ClientConfiguration& clientConfig)
{
    if(!impl->getObjectTagging(impl->bucketName, objectKey, clientConfig)) {
        return impl->objectTags;
    }
    return impl->objectTags;
}

bool BucketItem::headObjectTagging(const Aws::String& objectKey,
    const Aws::String& key, const Aws::String& value, const Aws::Client::ClientConfiguration& clientConfig)
{
    return impl->headObjectTagging(impl->bucketName, objectKey,
        key, value, clientConfig);
}

bool BucketItem::Impl::headObjectTagging(const Aws::String& bucketName,
    const Aws::String& objectKey, const Aws::String& key, const Aws::String& value, const Aws::Client::ClientConfiguration& clientConfig)
{
    bool result = false;
    if(!getObjectTagging(bucketName, objectKey, clientConfig)) {
        result = false;
    }

    for(Aws::S3::Model::Tag& tag : objectTags) {
        if(tag.GetKey() == key && tag.GetValue() == value) {
            result = true;
        }
    }

    return result;
}

bool BucketItem::putObjectTagging(const Aws::String& objectKey,
    const Aws::String& key, const Aws::String& value, const Aws::Client::ClientConfiguration& clientConfig)
{
    return impl->putObjectTagging(impl->bucketName, objectKey,
        key, value, clientConfig);
}

bool BucketItem::Impl::putObjectTagging(const Aws::String& bucketName,
    const Aws::String& objectKey, const Aws::String& key, const Aws::String& value, const Aws::Client::ClientConfiguration& clientConfig)
{
    // Aws::S3::S3Client client(clientConfig);
    Aws::S3::S3Client client(credentials, clientConfig,
        Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy::Never, false);

    Aws::S3::Model::PutObjectTaggingRequest request;
    request.SetBucket(bucketName);
    request.SetKey(objectKey);

    Aws::S3::Model::Tag tag;
    tag.SetKey(key);
    tag.SetValue(value);
    objectTags.push_back(tag);
    Aws::S3::Model::Tagging tagging;
    tagging.SetTagSet(objectTags);
    request.SetTagging(tagging);

    Aws::S3::Model::PutObjectTaggingOutcome outcome = client.PutObjectTagging(request);
    if(!outcome.IsSuccess()) {
        auto err = outcome.GetError();
        std::cerr << "Error: PutObjectTagging: " << 
            err.GetExceptionName() << ": " << err.GetMessage() << std::endl;
    } else {
        std::cout << "Successfully put a tag to the object '" << objectKey
            << "'." << std::endl;
    }

    return outcome.IsSuccess();
}

bool BucketItem::deleteBucketPolicy(const Aws::Client::ClientConfiguration& clientConfig)
{
    return impl->deleteBucketPolicy(impl->bucketName, clientConfig);
}

bool BucketItem::Impl::deleteBucketPolicy(const Aws::String& bucketName,
    const Aws::Client::ClientConfiguration& clientConfig)
{
    // Aws::S3::S3Client client(clientConfig);
    Aws::S3::S3Client client(credentials, clientConfig,
        Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy::Never, false);

    Aws::S3::Model::DeleteBucketPolicyRequest request;
    request.SetBucket(bucketName);

    Aws::S3::Model::DeleteBucketPolicyOutcome outcome = client.DeleteBucketPolicy(request);

    if(!outcome.IsSuccess()) {
        const Aws::S3::S3Error &err = outcome.GetError();
        std::cerr << "Error: DeleteBucketPolicy: " <<
            err.GetExceptionName() << ": " << err.GetMessage() << std::endl;
    } else {
        std::cout << "Policy was deleted from the bucket." << std::endl;
    }

    return outcome.IsSuccess();
}

bool BucketItem::getBucketPolicy(const Aws::Client::ClientConfiguration& clientConfig)
{
    return impl->getBucketPolicy(impl->bucketName, clientConfig);
}

bool BucketItem::Impl::getBucketPolicy(const Aws::String& bucketName,
    const Aws::Client::ClientConfiguration& clientConfig)
{
    // Aws::S3::S3Client s3_client(clientConfig);
    Aws::S3::S3Client s3_client(credentials, clientConfig,
        Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy::Never, false);

    Aws::S3::Model::GetBucketPolicyRequest request;
    request.SetBucket(bucketName);

    Aws::S3::Model::GetBucketPolicyOutcome outcome =
            s3_client.GetBucketPolicy(request);

    if(!outcome.IsSuccess()) {
        const Aws::S3::S3Error &err = outcome.GetError();
        std::cerr << "Error: GetBucketPolicy: "
            << err.GetExceptionName() << ": " << err.GetMessage() << std::endl;
    } else {
        Aws::StringStream policy_stream;
        Aws::String line;

        outcome.GetResult().GetPolicy() >> line;
        policy_stream << line;

        std::cout << "Retrieve the policy for bucket '" << bucketName << "':\n\n" <<
            policy_stream.str() << std::endl;
    }

    return outcome.IsSuccess();
}

// bool BucketItem::putBucketPolicy(const Aws::String& policyBody,
//     const Aws::Client::ClientConfiguration& clientConfig)
// {
//     return impl->putBucketPolicy(impl->bucketName, policyBody, clientConfig);
// }

// bool BucketItem::Impl::putBucketPolicy(const Aws::String& bucketName,
//     const Aws::String& policyBody, const Aws::Client::ClientConfiguration& clientConfig)
// {
//     // Aws::S3::S3Client s3_client(clientConfig);
//     Aws::S3::S3Client s3_client(credentials, clientConfig,
//         Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy::Never, false);

//     std::shared_ptr<Aws::StringStream> request_body =
//             MakeShared<Aws::StringStream>("");
//     *request_body << policyBody;

//     Aws::S3::Model::PutBucketPolicyRequest request;
//     request.SetBucket(bucketName);
//     request.SetBody(request_body);

//     Aws::S3::Model::PutBucketPolicyOutcome outcome =
//             s3_client.PutBucketPolicy(request);

//     if(!outcome.IsSuccess()) {
//         std::cerr << "Error: PutBucketPolicy: "
//             << outcome.GetError().GetMessage() << std::endl;
//     } else {
//         std::cout << "Set the following policy body for the bucket '" <<
//             bucketName << "':" << std::endl << std::endl;
//         std::cout << policyBody << std::endl;
//     }

//     return outcome.IsSuccess();
// }

Aws::Vector<Aws::String> BucketItem::getObjects(const Aws::Client::ClientConfiguration& clientConfig)
{
    Aws::Vector<Aws::String> objects;
    impl->objectList.clear();
    impl->token.clear();
    if(!impl->listObjectsV2(impl->bucketName, clientConfig)) {
        return objects;
    }

    for(int i = 0; i < impl->objectList.size(); ++i) {
        objects.push_back(impl->objectList[i].GetKey());
    }

    return objects;
}

long long BucketItem::getContentLength(const Aws::String& objectKey,
    const Aws::Client::ClientConfiguration& clientConfig)
{
    if(!impl->getObject(objectKey, impl->bucketName, "", clientConfig)) {
        return impl->contentLength;
    }
    return impl->contentLength;
}

Aws::Utils::DateTime BucketItem::getLastModified(const Aws::String& objectKey,
    const Aws::Client::ClientConfiguration& clientConfig)
{
    if(!impl->getObject(objectKey, impl->bucketName, "", clientConfig)) {
        return impl->lastModified;
    }
    return impl->lastModified;
}

Aws::String BucketItem::getETag(const Aws::String& objectKey,
    const Aws::Client::ClientConfiguration& clientConfig)
{
    if(!impl->getObject(objectKey, impl->bucketName, "", clientConfig)) {
        return impl->eTag;
    }
    return impl->eTag;
}

void BucketItem::createCredentials(const Aws::String& accessKeyId, const Aws::String& secretKey)
{
    if(!accessKeyId.empty() && !secretKey.empty()) {
        impl->credentials.SetAWSAccessKeyId(accessKeyId);
        impl->credentials.SetAWSSecretKey(secretKey);
    } else {
        // std::cout << "" << std::endl;
    }
}

}
