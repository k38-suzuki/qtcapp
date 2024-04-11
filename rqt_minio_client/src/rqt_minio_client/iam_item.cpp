/*!
    \author Kenta Suzuki
*/

#include "rqt_minio_client/iam_item.h"

#include <aws/iam/model/CreateUserRequest.h>
#include <aws/iam/model/DeleteUserRequest.h>
#include <aws/iam/model/GetUserRequest.h>
#include <aws/iam/model/GetUserResult.h>
#include <aws/iam/model/ListUsersRequest.h>
#include <aws/iam/model/UpdateUserRequest.h>

#include <aws/iam/model/CreateAccessKeyRequest.h>
#include <aws/iam/model/CreateAccessKeyResult.h>
#include <aws/iam/model/DeleteAccessKeyRequest.h>
#include <aws/iam/model/GetAccessKeyLastUsedRequest.h>
#include <aws/iam/model/GetAccessKeyLastUsedResult.h>
#include <aws/iam/model/ListAccessKeysRequest.h>
#include <aws/iam/model/ListAccessKeysResult.h>
#include <aws/iam/model/UpdateAccessKeyRequest.h>

#include <aws/iam/model/CreatePolicyRequest.h>
#include <aws/iam/model/CreatePolicyResult.h>
#include <aws/iam/model/DeletePolicyRequest.h>
#include <aws/iam/model/GetPolicyRequest.h>
#include <aws/iam/model/GetPolicyResult.h>
#include <aws/iam/model/ListPoliciesRequest.h>
#include <aws/iam/model/ListPoliciesResult.h>

#include <aws/iam/model/AttachRolePolicyRequest.h>
#include <aws/iam/model/DetachRolePolicyRequest.h>
#include <aws/iam/model/ListAttachedRolePoliciesRequest.h>
#include <aws/iam/model/ListAttachedRolePoliciesResult.h>
#include <aws/iam/model/PutRolePolicyRequest.h>

#include <aws/iam/model/DeleteServerCertificateRequest.h>
#include <aws/iam/model/GetServerCertificateRequest.h>
#include <aws/iam/model/GetServerCertificateResult.h>
#include <aws/iam/model/ListServerCertificatesRequest.h>
#include <aws/iam/model/UpdateServerCertificateRequest.h>

#include <aws/iam/model/CreateAccountAliasRequest.h>
#include <aws/iam/model/DeleteAccountAliasRequest.h>
#include <aws/iam/model/ListAccountAliasesRequest.h>
#include <aws/iam/model/ListAccountAliasesResult.h>

#include <iomanip>
#include <iostream>

namespace {

Aws::String BuildSamplePolicyDocument(const Aws::String &rsrc_arn)
{
    std::stringstream stringStream;
    stringStream << "{"
                 << "  \"Version\": \"2012-10-17\","
                 << "  \"Statement\": ["
                 << "    {"
                 << "        \"Effect\": \"Allow\","
                 << "        \"Action\": \"logs:CreateLogGroup\","
                 << "        \"Resource\": \""
                 << rsrc_arn
                 << "\""
                 << "    },"
                 << "    {"
                 << "        \"Effect\": \"Allow\","
                 << "        \"Action\": ["
                 << "            \"dynamodb:DeleteItem\","
                 << "            \"dynamodb:GetItem\","
                 << "            \"dynamodb:PutItem\","
                 << "            \"dynamodb:Scan\","
                 << "            \"dynamodb:UpdateItem\""
                 << "       ],"
                 << "       \"Resource\": \""
                 << rsrc_arn
                 << "\""
                 << "    }"
                 << "   ]"
                 << "}";

    return stringStream.str();
}

}

namespace rqt_minio_client {

class IAMItem::Impl
{
public:
    IAMItem* self;

    Impl(IAMItem* self);

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
};

IAMItem::IAMItem()
{
    impl = new Impl(this);
}

IAMItem::Impl::Impl(IAMItem* self)
    : self(self)
{

}

IAMItem::~IAMItem()
{
    delete impl;
}

bool IAMItem::createUser(const Aws::String& userName,
    const Aws::Client::ClientConfiguration& clientConfig)
{
    return impl->createUser(userName, clientConfig);
}

bool IAMItem::Impl::createUser(const Aws::String& userName,
    const Aws::Client::ClientConfiguration& clientConfig)
{
    Aws::IAM::IAMClient iam(clientConfig);
    Aws::IAM::Model::GetUserRequest get_request;
    get_request.SetUserName(userName);

    auto get_outcome = iam.GetUser(get_request);
    if(get_outcome.IsSuccess()) {
        std::cout << "IAM user " << userName << " already exists" << std::endl;
        return true;
    } else if(get_outcome.GetError().GetErrorType() !=
             Aws::IAM::IAMErrors::NO_SUCH_ENTITY) {
        std::cerr << "Error checking existence of IAM user " << userName << ":"
            << get_outcome.GetError().GetMessage() << std::endl;
        return false;
    }

    Aws::IAM::Model::CreateUserRequest create_request;
    create_request.SetUserName(userName);

    auto create_outcome = iam.CreateUser(create_request);
    if(!create_outcome.IsSuccess()) {
        std::cerr << "Error creating IAM user " << userName << ":" <<
            create_outcome.GetError().GetMessage() << std::endl;
    } else {
        std::cout << "Successfully created IAM user " << userName << std::endl;
    }

    return create_outcome.IsSuccess();
}

bool IAMItem::deleteUser(const Aws::String& userName,
    const Aws::Client::ClientConfiguration& clientConfig)
{
    return impl->deleteUser(userName, clientConfig);
}

bool IAMItem::Impl::deleteUser(const Aws::String& userName,
    const Aws::Client::ClientConfiguration& clientConfig)
{
    Aws::IAM::IAMClient iam(clientConfig);
    Aws::IAM::Model::GetUserRequest get_request;
    get_request.SetUserName(userName);

    auto get_outcome = iam.GetUser(get_request);
    if(!get_outcome.IsSuccess()) {
        if(get_outcome.GetError().GetErrorType() ==
            Aws::IAM::IAMErrors::NO_SUCH_ENTITY) {
            std::cout << "IAM user " << userName << " does not exist" <<
                std::endl;
        } else {
            std::cerr << "Error checking existence of IAM user " << userName <<
                ": " << get_outcome.GetError().GetMessage() << std::endl;
        }
        return false;
    }

    Aws::IAM::Model::DeleteUserRequest request;
    request.SetUserName(userName);
    auto outcome = iam.DeleteUser(request);
    if(!outcome.IsSuccess()) {
        std::cerr << "Error deleting IAM user " << userName << ": " <<
            outcome.GetError().GetMessage() << std::endl;;
    } else {
        std::cout << "Successfully deleted IAM user " << userName << std::endl;
    }

    return outcome.IsSuccess();
}

bool IAMItem::listUsers(const Aws::Client::ClientConfiguration& clientConfig)
{
    return impl->listUsers(clientConfig);
}

bool IAMItem::Impl::listUsers(const Aws::Client::ClientConfiguration& clientConfig)
{
    const Aws::String DATE_FORMAT = "%Y-%m-%d";
    Aws::IAM::IAMClient iam(clientConfig);
    Aws::IAM::Model::ListUsersRequest request;

    bool done = false;
    bool header = false;
    while(!done) {
        auto outcome = iam.ListUsers(request);
        if(!outcome.IsSuccess()) {
            std::cerr << "Failed to list iam users:" <<
                outcome.GetError().GetMessage() << std::endl;
            return false;
        }

        if(!header) {
            std::cout << std::left << std::setw(32) << "Name" <<
                std::setw(30) << "ID" << std::setw(64) << "Arn" <<
                std::setw(20) << "CreateDate" << std::endl;
            header = true;
        }

        const auto &users = outcome.GetResult().GetUsers();
        for(const auto &user: users) {
            std::cout << std::left << std::setw(32) << user.GetUserName() <<
                std::setw(30) << user.GetUserId() << std::setw(64) <<
                user.GetArn() << std::setw(20) <<
                user.GetCreateDate().ToGmtString(DATE_FORMAT.c_str())
                << std::endl;
        }

        if(outcome.GetResult().GetIsTruncated()) {
            request.SetMarker(outcome.GetResult().GetMarker());
        } else {
            done = true;
        }
    }

    return true;
}

bool IAMItem::updateUser(const Aws::String& currentUserName,
    const Aws::String& newUserName,
    const Aws::Client::ClientConfiguration& clientConfig)
{
    return impl->updateUser(currentUserName, newUserName, clientConfig);
}

bool IAMItem::Impl::updateUser(const Aws::String& currentUserName,
    const Aws::String& newUserName,
    const Aws::Client::ClientConfiguration& clientConfig)
{
    Aws::IAM::IAMClient iam(clientConfig);

    Aws::IAM::Model::UpdateUserRequest request;
    request.SetUserName(currentUserName);
    request.SetNewUserName(newUserName);

    auto outcome = iam.UpdateUser(request);
    if(outcome.IsSuccess()) {
        std::cout << "IAM user " << currentUserName <<
            " successfully updated with new user name " << newUserName <<
            std::endl;
    } else {
        std::cerr << "Error updating user name for IAM user " << currentUserName <<
            ":" << outcome.GetError().GetMessage() << std::endl;
    }

    return outcome.IsSuccess();
}

Aws::String IAMItem::createAccessKey(const Aws::String& userName,
    const Aws::Client::ClientConfiguration& clientConfig)
{
    return impl->createAccessKey(userName, clientConfig);
}

Aws::String IAMItem::Impl::createAccessKey(const Aws::String& userName,
    const Aws::Client::ClientConfiguration& clientConfig)
{
    Aws::IAM::IAMClient iam(clientConfig);

    Aws::IAM::Model::CreateAccessKeyRequest request;
    request.SetUserName(userName);

    Aws::String result;
    Aws::IAM::Model::CreateAccessKeyOutcome outcome = iam.CreateAccessKey(request);
    if(!outcome.IsSuccess()) {
        std::cerr << "Error creating access key for IAM user " << userName
            << ":" << outcome.GetError().GetMessage() << std::endl;
    } else {
        const auto &accessKey = outcome.GetResult().GetAccessKey();
        std::cout << "Successfully created access key for IAM user " <<
            userName << std::endl << "  aws_access_key_id = " <<
            accessKey.GetAccessKeyId() << std::endl <<
            " aws_secret_access_key = " << accessKey.GetSecretAccessKey() <<
            std::endl;
        result = accessKey.GetAccessKeyId();
    }

    return result;
}

bool IAMItem::deleteAccessKey(const Aws::String& userName,
    const Aws::String& accessKeyID,
    const Aws::Client::ClientConfiguration& clientConfig)
{
    return impl->deleteAccessKey(userName, accessKeyID, clientConfig);
}

bool IAMItem::Impl::deleteAccessKey(const Aws::String& userName,
    const Aws::String& accessKeyID,
    const Aws::Client::ClientConfiguration& clientConfig)
{
    Aws::IAM::IAMClient iam(clientConfig);

    Aws::IAM::Model::DeleteAccessKeyRequest request;
    request.SetUserName(userName);
    request.SetAccessKeyId(accessKeyID);

    auto outcome = iam.DeleteAccessKey(request);

    if(!outcome.IsSuccess()) {
        std::cerr << "Error deleting access key " << accessKeyID << " from user "
            << userName << ": " << outcome.GetError().GetMessage() <<
            std::endl;
    } else {
        std::cout << "Successfully deleted access key " << accessKeyID
            << " for IAM user " << userName << std::endl;
    }

    return outcome.IsSuccess();
}

bool IAMItem::listAccessKeys(const Aws::String& userName,
    const Aws::Client::ClientConfiguration& clientConfig)
{
    return impl->listAccessKeys(userName, clientConfig);
}

bool IAMItem::Impl::listAccessKeys(const Aws::String& userName,
    const Aws::Client::ClientConfiguration& clientConfig)
{
    Aws::IAM::IAMClient iam(clientConfig);
    Aws::IAM::Model::ListAccessKeysRequest request;
    request.SetUserName(userName);

    bool done = false;
    bool header = false;
    while(!done) {
        auto outcome = iam.ListAccessKeys(request);
        if(!outcome.IsSuccess()) {
            std::cerr << "Failed to list access keys for user " << userName
                << ": " << outcome.GetError().GetMessage() << std::endl;
            return false;
        }

        if(!header) {
            std::cout << std::left << std::setw(32) << "UserName" <<
                std::setw(30) << "KeyID" << std::setw(20) << "Status" <<
                std::setw(20) << "CreateDate" << std::endl;
            header = true;
        }

        const auto &keys = outcome.GetResult().GetAccessKeyMetadata();
        const Aws::String DATE_FORMAT = "%Y-%m-%d";

        for(const auto &key: keys) {
            Aws::String statusString =
                Aws::IAM::Model::StatusTypeMapper::GetNameForStatusType(
                    key.GetStatus());
            std::cout << std::left << std::setw(32) << key.GetUserName() <<
                std::setw(30) << key.GetAccessKeyId() << std::setw(20) <<
                statusString << std::setw(20) <<
                key.GetCreateDate().ToGmtString(DATE_FORMAT.c_str()) << std::endl;
        }

        if(outcome.GetResult().GetIsTruncated()) {
            request.SetMarker(outcome.GetResult().GetMarker());
        } else {
            done = true;
        }
    }

    return true;
}

bool IAMItem::updateAccessKey(const Aws::String& userName,
    const Aws::String& accessKeyID,
    Aws::IAM::Model::StatusType status,
    const Aws::Client::ClientConfiguration& clientConfig)
{
    return impl->updateAccessKey(userName, accessKeyID, status, clientConfig);
}

bool IAMItem::Impl::updateAccessKey(const Aws::String& userName,
    const Aws::String& accessKeyID,
    Aws::IAM::Model::StatusType status,
    const Aws::Client::ClientConfiguration& clientConfig)
{
    Aws::IAM::IAMClient iam(clientConfig);
    Aws::IAM::Model::UpdateAccessKeyRequest request;
    request.SetUserName(userName);
    request.SetAccessKeyId(accessKeyID);
    request.SetStatus(status);

    auto outcome = iam.UpdateAccessKey(request);
    if(outcome.IsSuccess()) {
        std::cout << "Successfully updated status of access key "
            << accessKeyID << " for user " << userName << std::endl;
    } else {
        std::cerr << "Error updated status of access key " << accessKeyID <<
            " for user " << userName << ": " <<
            outcome.GetError().GetMessage() << std::endl;
    }

    return outcome.IsSuccess();
}

bool IAMItem::accessKeyLastUsed(const Aws::String& secretKeyID,
    const Aws::Client::ClientConfiguration& clientConfig)
{
    return impl->accessKeyLastUsed(secretKeyID, clientConfig);
}

bool IAMItem::Impl::accessKeyLastUsed(const Aws::String& secretKeyID,
    const Aws::Client::ClientConfiguration& clientConfig)
{
    Aws::IAM::IAMClient iam(clientConfig);
    Aws::IAM::Model::GetAccessKeyLastUsedRequest request;

    request.SetAccessKeyId(secretKeyID);

    Aws::IAM::Model::GetAccessKeyLastUsedOutcome outcome = iam.GetAccessKeyLastUsed(
        request);

    if(!outcome.IsSuccess()) {
        std::cerr << "Error querying last used time for access key " <<
            secretKeyID << ":" << outcome.GetError().GetMessage() << std::endl;
    } else {
        Aws::String lastUsedTimeString =
            outcome.GetResult()
                    .GetAccessKeyLastUsed()
                    .GetLastUsedDate()
                    .ToGmtString(Aws::Utils::DateFormat::ISO_8601);
        std::cout << "Access key " << secretKeyID << " last used at time " <<
            lastUsedTimeString << std::endl;
    }

    return outcome.IsSuccess();
}

Aws::String IAMItem::createPolicy(const Aws::String& policyName,
    const Aws::String& rsrcArn,
    const Aws::Client::ClientConfiguration& clientConfig)
{
    return impl->createPolicy(policyName, rsrcArn, clientConfig);
}

Aws::String IAMItem::Impl::createPolicy(const Aws::String& policyName,
    const Aws::String& rsrcArn,
    const Aws::Client::ClientConfiguration& clientConfig)
{
    Aws::IAM::IAMClient iam(clientConfig);

    Aws::IAM::Model::CreatePolicyRequest request;
    request.SetPolicyName(policyName);
    request.SetPolicyDocument(BuildSamplePolicyDocument(rsrcArn));

    Aws::IAM::Model::CreatePolicyOutcome outcome = iam.CreatePolicy(request);
    Aws::String result;
    if(!outcome.IsSuccess()) {
        std::cerr << "Error creating policy " << policyName << ": " <<
            outcome.GetError().GetMessage() << std::endl;
    } else {
        result = outcome.GetResult().GetPolicy().GetArn();
        std::cout << "Successfully created policy " << policyName <<
            std::endl;
    }

    return result;
}

bool IAMItem::deletePolicy(const Aws::String& policyArn,
    const Aws::Client::ClientConfiguration& clientConfig)
{
    return impl->deletePolicy(policyArn, clientConfig);
}

bool IAMItem::Impl::deletePolicy(const Aws::String& policyArn,
    const Aws::Client::ClientConfiguration& clientConfig)
{
    Aws::IAM::IAMClient iam(clientConfig);
    Aws::IAM::Model::DeletePolicyRequest request;
    request.SetPolicyArn(policyArn);

    auto outcome = iam.DeletePolicy(request);
    if(!outcome.IsSuccess()) {
        std::cerr << "Error deleting policy with arn " << policyArn << ": "
            << outcome.GetError().GetMessage() << std::endl;
    } else {
        std::cout << "Successfully deleted policy with arn " << policyArn
            << std::endl;
    }

    return outcome.IsSuccess();
}

bool IAMItem::getPolicy(const Aws::String& policyArn,
    const Aws::Client::ClientConfiguration& clientConfig)
{
    return impl->getPolicy(policyArn, clientConfig);
}

bool IAMItem::Impl::getPolicy(const Aws::String& policyArn,
    const Aws::Client::ClientConfiguration& clientConfig)
{
    Aws::IAM::IAMClient iam(clientConfig);
    Aws::IAM::Model::GetPolicyRequest request;
    request.SetPolicyArn(policyArn);

    auto outcome = iam.GetPolicy(request);
    if(!outcome.IsSuccess()) {
        std::cerr << "Error getting policy " << policyArn << ": " <<
            outcome.GetError().GetMessage() << std::endl;
    } else {
        const auto &policy = outcome.GetResult().GetPolicy();
        std::cout << "Name: " << policy.GetPolicyName() << std::endl <<
            "ID: " << policy.GetPolicyId() << std::endl << "Arn: " <<
            policy.GetArn() << std::endl << "Description: " <<
            policy.GetDescription() << std::endl << "CreateDate: " <<
            policy.GetCreateDate().ToGmtString(Aws::Utils::DateFormat::ISO_8601)
            << std::endl;
    }

    return outcome.IsSuccess();
}

bool IAMItem::listPolicies(const Aws::Client::ClientConfiguration& clientConfig)
{
    return impl->listPolicies(clientConfig);
}

bool IAMItem::Impl::listPolicies(const Aws::Client::ClientConfiguration& clientConfig)
{
    const Aws::String DATE_FORMAT("%Y-%m-%d");
    Aws::IAM::IAMClient iam(clientConfig);
    Aws::IAM::Model::ListPoliciesRequest request;

    bool done = false;
    bool header = false;
    while(!done) {
        auto outcome = iam.ListPolicies(request);
        if(!outcome.IsSuccess()) {
            std::cerr << "Failed to list iam policies: " <<
                outcome.GetError().GetMessage() << std::endl;
            return false;
        }

        if(!header) {
            std::cout << std::left << std::setw(55) << "Name" <<
                std::setw(30) << "ID" << std::setw(80) << "Arn" <<
                std::setw(64) << "Description" << std::setw(12) <<
                "CreateDate" << std::endl;
            header = true;
        }

        const auto &policies = outcome.GetResult().GetPolicies();
        for(const auto &policy: policies) {
            std::cout << std::left << std::setw(55) <<
                policy.GetPolicyName() << std::setw(30) <<
                policy.GetPolicyId() << std::setw(80) << policy.GetArn() <<
                std::setw(64) << policy.GetDescription() << std::setw(12) <<
                policy.GetCreateDate().ToGmtString(DATE_FORMAT.c_str()) <<
                std::endl;
        }

        if(outcome.GetResult().GetIsTruncated()) {
            request.SetMarker(outcome.GetResult().GetMarker());
        } else {
            done = true;
        }
    }

    return true;
}

bool IAMItem::attachRolePolicy(const Aws::String& roleName,
    const Aws::String& policyArn,
    const Aws::Client::ClientConfiguration& clientConfig)
{
    return impl->attachRolePolicy(roleName, policyArn, clientConfig);
}

bool IAMItem::Impl::attachRolePolicy(const Aws::String& roleName,
    const Aws::String& policyArn,
    const Aws::Client::ClientConfiguration& clientConfig)
{
    Aws::IAM::IAMClient iam(clientConfig);

    Aws::IAM::Model::ListAttachedRolePoliciesRequest list_request;
    list_request.SetRoleName(roleName);

    bool done = false;
    while(!done) {
        auto list_outcome = iam.ListAttachedRolePolicies(list_request);
        if(!list_outcome.IsSuccess()) {
            std::cerr << "Failed to list attached policies of role " <<
                roleName << ": " << list_outcome.GetError().GetMessage() <<
                std::endl;
            return false;
        }

        const auto &policies = list_outcome.GetResult().GetAttachedPolicies();
        if(std::any_of(policies.cbegin(), policies.cend(),
            [=](const Aws::IAM::Model::AttachedPolicy &policy) {
                    return policy.GetPolicyArn() == policyArn;
            })) {
            std::cout << "Policy " << policyArn <<
                " is already attached to role " << roleName << std::endl;
            return true;
        }

        done = !list_outcome.GetResult().GetIsTruncated();
        list_request.SetMarker(list_outcome.GetResult().GetMarker());
    }

    Aws::IAM::Model::AttachRolePolicyRequest request;
    request.SetRoleName(roleName);
    request.SetPolicyArn(policyArn);

    Aws::IAM::Model::AttachRolePolicyOutcome outcome = iam.AttachRolePolicy(request);
    if(!outcome.IsSuccess()) {
        std::cerr << "Failed to attach policy " << policyArn << " to role " <<
            roleName << ": " << outcome.GetError().GetMessage() << std::endl;
    } else {
        std::cout << "Successfully attached policy " << policyArn << " to role " <<
            roleName << std::endl;
    }

    return outcome.IsSuccess();
}

bool IAMItem::detachRolePolicy(const Aws::String& roleName,
    const Aws::String& policyArn,
    const Aws::Client::ClientConfiguration& clientConfig)
{
    return impl->detachRolePolicy(roleName, policyArn, clientConfig);
}

bool IAMItem::Impl::detachRolePolicy(const Aws::String& roleName,
    const Aws::String& policyArn,
    const Aws::Client::ClientConfiguration& clientConfig)
{
    Aws::IAM::IAMClient iam(clientConfig);

    Aws::IAM::Model::ListAttachedRolePoliciesRequest list_request;
    list_request.SetRoleName(roleName);

    bool done = false;
    bool attached = false;
    while(!done) {
        auto listOutcome = iam.ListAttachedRolePolicies(list_request);
        if(!listOutcome.IsSuccess()) {
            std::cerr << "Failed to list attached policies of role " <<
                roleName << ": " << listOutcome.GetError().GetMessage() <<
                std::endl;
            return false;
        }

        const auto &policies = listOutcome.GetResult().GetAttachedPolicies();
        attached = std::any_of(
            policies.cbegin(), policies.cend(),
            [=](const Aws::IAM::Model::AttachedPolicy &policy) {
                    return policy.GetPolicyArn() == policyArn;
            });
        if(attached) {
            break;
        }

        done = !listOutcome.GetResult().GetIsTruncated();
        list_request.SetMarker(listOutcome.GetResult().GetMarker());
    }

    if(!attached) {
        std::cerr << "Policy " << policyArn << " is not attached to role " <<
            roleName << std::endl;
        return false;
    }

    Aws::IAM::Model::DetachRolePolicyRequest detachRequest;
    detachRequest.SetRoleName(roleName);
    detachRequest.SetPolicyArn(policyArn);

    auto detachOutcome = iam.DetachRolePolicy(detachRequest);
    if(!detachOutcome.IsSuccess()) {
        std::cerr << "Failed to detach policy " << policyArn << " from role "
            << roleName << ": " << detachOutcome.GetError().GetMessage() <<
            std::endl;
    } else {
        std::cout << "Successfully detached policy " << policyArn << " from role "
            << roleName << std::endl;
    }

    return detachOutcome.IsSuccess();
}

bool IAMItem::putRolePolicy(const Aws::String& roleName,
    const Aws::String& policyName,
    const Aws::String& policyDocument,
    const Aws::Client::ClientConfiguration& clientConfig)
{
    return impl->putRolePolicy(roleName, policyName, policyDocument, clientConfig);
}

bool IAMItem::Impl::putRolePolicy(const Aws::String& roleName,
    const Aws::String& policyName,
    const Aws::String& policyDocument,
    const Aws::Client::ClientConfiguration& clientConfig)
{
    Aws::IAM::IAMClient iamClient(clientConfig);
    Aws::IAM::Model::PutRolePolicyRequest request;

    request.SetRoleName(roleName);
    request.SetPolicyName(policyName);
    request.SetPolicyDocument(policyDocument);

    Aws::IAM::Model::PutRolePolicyOutcome outcome = iamClient.PutRolePolicy(request);
    if(!outcome.IsSuccess()) {
        std::cerr << "Error putting policy on role. " <<
            outcome.GetError().GetMessage() << std::endl;
    } else {
        std::cout << "Successfully put the role policy." << std::endl;
    }

    return outcome.IsSuccess();
}

bool IAMItem::deleteServerCertificate(const Aws::String& certificateName,
    const Aws::Client::ClientConfiguration& clientConfig)
{
    return impl->deleteServerCertificate(certificateName, clientConfig);
}

bool IAMItem::Impl::deleteServerCertificate(const Aws::String& certificateName,
    const Aws::Client::ClientConfiguration& clientConfig)
{
    Aws::IAM::IAMClient iam(clientConfig);
    Aws::IAM::Model::DeleteServerCertificateRequest request;
    request.SetServerCertificateName(certificateName);

    const auto outcome = iam.DeleteServerCertificate(request);
    bool result = true;
    if(!outcome.IsSuccess()) {
        if(outcome.GetError().GetErrorType() != Aws::IAM::IAMErrors::NO_SUCH_ENTITY) {
            std::cerr << "Error deleting server certificate " << certificateName <<
                ": " << outcome.GetError().GetMessage() << std::endl;
            result = false;
        } else {
            std::cout << "Certificate '" << certificateName
                << "' not found." << std::endl;
        }
    } else {
        std::cout << "Successfully deleted server certificate " << certificateName
            << std::endl;
    }

    return result;
}

bool IAMItem::getServerCertificate(const Aws::String& certificateName,
    const Aws::Client::ClientConfiguration& clientConfig)
{
    return impl->getServerCertificate(certificateName, clientConfig);
}

bool IAMItem::Impl::getServerCertificate(const Aws::String& certificateName,
    const Aws::Client::ClientConfiguration& clientConfig)
{
    Aws::IAM::IAMClient iam(clientConfig);
    Aws::IAM::Model::GetServerCertificateRequest request;
    request.SetServerCertificateName(certificateName);

    auto outcome = iam.GetServerCertificate(request);
    bool result = true;
    if(!outcome.IsSuccess()) {
        if(outcome.GetError().GetErrorType() != Aws::IAM::IAMErrors::NO_SUCH_ENTITY) {
            std::cerr << "Error getting server certificate " << certificateName <<
                ": " << outcome.GetError().GetMessage() << std::endl;
            result = false;
        } else {
            std::cout << "Certificate '" << certificateName
                << "' not found." << std::endl;
        }
    } else {
        const auto &certificate = outcome.GetResult().GetServerCertificate();
        std::cout << "Name: " <<
            certificate.GetServerCertificateMetadata().GetServerCertificateName()
            << std::endl << "Body: " << certificate.GetCertificateBody() <<
            std::endl << "Chain: " << certificate.GetCertificateChain() <<
            std::endl;
    }

    return result;
}

bool IAMItem::listServerCertificates(
    const Aws::Client::ClientConfiguration& clientConfig)
{
    return impl->listServerCertificates(clientConfig);
}

bool IAMItem::Impl::listServerCertificates(
    const Aws::Client::ClientConfiguration& clientConfig)
{
    const Aws::String DATE_FORMAT = "%Y-%m-%d";

    Aws::IAM::IAMClient iam(clientConfig);
    Aws::IAM::Model::ListServerCertificatesRequest request;

    bool done = false;
    bool header = false;
    while (!done) {
        auto outcome = iam.ListServerCertificates(request);
        if(!outcome.IsSuccess()) {
            std::cerr << "Failed to list server certificates: " <<
                outcome.GetError().GetMessage() << std::endl;
            return false;
        }

        if(!header) {
            std::cout << std::left << std::setw(55) << "Name" <<
                std::setw(30) << "ID" << std::setw(80) << "Arn" <<
                std::setw(14) << "UploadDate" << std::setw(14) <<
                "ExpirationDate" << std::endl;
            header = true;
        }

        const auto &certificates =
            outcome.GetResult().GetServerCertificateMetadataList();

        for(const auto &certificate: certificates) {
            std::cout << std::left << std::setw(55) <<
                certificate.GetServerCertificateName() << std::setw(30) <<
                certificate.GetServerCertificateId() << std::setw(80) <<
                certificate.GetArn() << std::setw(14) <<
                certificate.GetUploadDate().ToGmtString(DATE_FORMAT.c_str()) <<
                std::setw(14) <<
                certificate.GetExpiration().ToGmtString(DATE_FORMAT.c_str()) <<
                std::endl;
        }

        if(outcome.GetResult().GetIsTruncated()) {
            request.SetMarker(outcome.GetResult().GetMarker());
        } else {
            done = true;
        }
    }

    return true;
}

bool IAMItem::updateServerCertificate(const Aws::String& currentCertificateName,
    const Aws::String& newCertificateName,
    const Aws::Client::ClientConfiguration& clientConfig)
{
    return impl->updateServerCertificate(currentCertificateName, newCertificateName, clientConfig);
}

bool IAMItem::Impl::updateServerCertificate(const Aws::String& currentCertificateName,
    const Aws::String& newCertificateName,
    const Aws::Client::ClientConfiguration& clientConfig)
{
    Aws::IAM::IAMClient iam(clientConfig);
    Aws::IAM::Model::UpdateServerCertificateRequest request;
    request.SetServerCertificateName(currentCertificateName);
    request.SetNewServerCertificateName(newCertificateName);

    auto outcome = iam.UpdateServerCertificate(request);
    bool result = true;
    if(outcome.IsSuccess()) {
        std::cout << "Server certificate " << currentCertificateName
            << " successfully renamed as " << newCertificateName
            << std::endl;
    } else {
        if(outcome.GetError().GetErrorType() != Aws::IAM::IAMErrors::NO_SUCH_ENTITY) {
            std::cerr << "Error changing name of server certificate " <<
                currentCertificateName << " to " << newCertificateName << ":" <<
                outcome.GetError().GetMessage() << std::endl;
            result = false;
        } else {
            std::cout << "Certificate '" << currentCertificateName
                << "' not found." << std::endl;
        }
    }

    return result;
}

bool IAMItem::createAccountAlias(const Aws::String& aliasName,
    const Aws::Client::ClientConfiguration& clientConfig)
{
    return impl->createAccountAlias(aliasName, clientConfig);
}

bool IAMItem::Impl::createAccountAlias(const Aws::String& aliasName,
    const Aws::Client::ClientConfiguration& clientConfig)
{
    Aws::IAM::IAMClient iam(clientConfig);
    Aws::IAM::Model::CreateAccountAliasRequest request;
    request.SetAccountAlias(aliasName);

    Aws::IAM::Model::CreateAccountAliasOutcome outcome = iam.CreateAccountAlias(
        request);
    if(!outcome.IsSuccess()) {
        std::cerr << "Error creating account alias " << aliasName << ": "
            << outcome.GetError().GetMessage() << std::endl;
    } else {
        std::cout << "Successfully created account alias " << aliasName <<
            std::endl;
    }

    return outcome.IsSuccess();
}

bool IAMItem::deleteAccountAlias(const Aws::String& accountAlias,
    const Aws::Client::ClientConfiguration& clientConfig)
{
    return impl->deleteAccountAlias(accountAlias, clientConfig);
}

bool IAMItem::Impl::deleteAccountAlias(const Aws::String& accountAlias,
    const Aws::Client::ClientConfiguration& clientConfig)
{
    Aws::IAM::IAMClient iam(clientConfig);

    Aws::IAM::Model::DeleteAccountAliasRequest request;
    request.SetAccountAlias(accountAlias);

    const auto outcome = iam.DeleteAccountAlias(request);
    if(!outcome.IsSuccess()) {
        std::cerr << "Error deleting account alias " << accountAlias << ": "
            << outcome.GetError().GetMessage() << std::endl;
    } else {
        std::cout << "Successfully deleted account alias " << accountAlias <<
            std::endl;
    }

    return outcome.IsSuccess();
}

bool IAMItem::listAccountAliases(const Aws::Client::ClientConfiguration& clientConfig)
{
    return impl->listAccountAliases(clientConfig);
}

bool IAMItem::Impl::listAccountAliases(const Aws::Client::ClientConfiguration& clientConfig)
{
    Aws::IAM::IAMClient iam(clientConfig);
    Aws::IAM::Model::ListAccountAliasesRequest request;

    bool done = false;
    bool header = false;
    while (!done) {
        auto outcome = iam.ListAccountAliases(request);
        if(!outcome.IsSuccess()) {
            std::cerr << "Failed to list account aliases: " <<
                outcome.GetError().GetMessage() << std::endl;
            return false;
        }

        const auto &aliases = outcome.GetResult().GetAccountAliases();
        if(!header) {
            if(aliases.size() == 0) {
                std::cout << "Account has no aliases" << std::endl;
                break;
            }
            std::cout << std::left << std::setw(32) << "Alias" << std::endl;
            header = true;
        }

        for (const auto &alias: aliases) {
            std::cout << std::left << std::setw(32) << alias << std::endl;
        }

        if(outcome.GetResult().GetIsTruncated()) {
            request.SetMarker(outcome.GetResult().GetMarker());
        } else {
            done = true;
        }
    }

    return true;
}

}
