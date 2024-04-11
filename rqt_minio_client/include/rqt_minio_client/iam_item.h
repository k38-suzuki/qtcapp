/*!
    \author Kenta Suzuki
*/

#ifndef rqt_minio_client__iam_item_H
#define rqt_minio_client__iam_item_H

#include <aws/core/Aws.h>
#include <aws/iam/IAMClient.h>

namespace rqt_minio_client {

class IAMItem
{
public:
    IAMItem();
    ~IAMItem();

    bool createUser(const Aws::String& userName,
        const Aws::Client::ClientConfiguration& clientConfig);
    bool deleteUser(const Aws::String& userName,
        const Aws::Client::ClientConfiguration& clientConfig);
    bool listUsers(const Aws::Client::ClientConfiguration& clientConfig);
    bool updateUser(const Aws::String& currentUserName,
        const Aws::String& newUserName,
        const Aws::Client::ClientConfiguration& clientConfig);

    Aws::String createAccessKey(const Aws::String& userName,
        const Aws::Client::ClientConfiguration& clientConfig);
    bool deleteAccessKey(const Aws::String& userName,
        const Aws::String& accessKeyID,
        const Aws::Client::ClientConfiguration& clientConfig);
    bool listAccessKeys(const Aws::String& userName,
        const Aws::Client::ClientConfiguration& clientConfig);
    bool updateAccessKey(const Aws::String& userName,
        const Aws::String& accessKeyID,
        Aws::IAM::Model::StatusType status,
        const Aws::Client::ClientConfiguration& clientConfig);
    bool accessKeyLastUsed(const Aws::String& secretKeyID,
        const Aws::Client::ClientConfiguration& clientConfig);

    Aws::String createPolicy(const Aws::String& policyName,
        const Aws::String& rsrcArn,
        const Aws::Client::ClientConfiguration& clientConfig);
    bool deletePolicy(const Aws::String& policyArn,
        const Aws::Client::ClientConfiguration& clientConfig);
    bool getPolicy(const Aws::String& policyArn,
        const Aws::Client::ClientConfiguration& clientConfig);
    bool listPolicies(const Aws::Client::ClientConfiguration& clientConfig);

    bool attachRolePolicy(const Aws::String& roleName,
        const Aws::String& policyArn,
        const Aws::Client::ClientConfiguration& clientConfig);
    bool detachRolePolicy(const Aws::String& roleName,
        const Aws::String& policyArn,
        const Aws::Client::ClientConfiguration& clientConfig);
    bool putRolePolicy(const Aws::String& roleName,
        const Aws::String& policyName,
        const Aws::String& policyDocument,
        const Aws::Client::ClientConfiguration& clientConfig);

    bool deleteServerCertificate(const Aws::String& certificateName,
        const Aws::Client::ClientConfiguration& clientConfig);
    bool getServerCertificate(const Aws::String& certificateName,
        const Aws::Client::ClientConfiguration& clientConfig);
    bool listServerCertificates(
        const Aws::Client::ClientConfiguration& clientConfig);
    bool updateServerCertificate(const Aws::String& currentCertificateName,
        const Aws::String& newCertificateName,
        const Aws::Client::ClientConfiguration& clientConfig);

    bool createAccountAlias(const Aws::String& aliasName,
        const Aws::Client::ClientConfiguration& clientConfig);
    bool deleteAccountAlias(const Aws::String& accountAlias,
        const Aws::Client::ClientConfiguration& clientConfig);
    bool listAccountAliases(const Aws::Client::ClientConfiguration& clientConfig);

private:
    class Impl;
    Impl* impl;
};

}

#endif // rqt_minio_client__iam_item_H
