/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#pragma once

#include <atomic>
#include <cstdint>
#include <string>

#include "jwt-cpp/jwt.h"

namespace lgraph {
/**
 * Manages tokens issued to clients. A client has to get token before it
 * can issue DB operations. A token is valid for a period of time. Operation
 * with an invalid or expired token will be rejected. Admin can revoke all
 * existing tokens by rotating the valid period.
 */
class TokenManager {
    std::string secret_key_;
    // verifier
    jwt::verifier<jwt::default_clock> verifier_;

 public:
    // @param secret_key    key for encryption
    // @param valid_period_seconds  valid period in seconds for token
    // @param clock_skew_tolerance_us   clock skew tolerance in microseconds
    explicit TokenManager(const std::string& secret_key);

    // get a token using this user name
    // returns the token
    std::string IssueToken(const std::string& user, const std::string& password) const;

    // decipher a token to get user and password
    // returns true if success, otherwise false
    bool DecipherToken(const std::string& token, std::string& user, std::string& password) const;
};
}  // namespace lgraph
