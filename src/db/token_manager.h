/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#pragma once

#include <atomic>
#include <cstdint>
#include <string>

#include "jwt-cpp/jwt.h"
#include "jwt-cpp/traits/kazuho-picojson/traits.h"

namespace lgraph {
/**
 * Manages tokens issued to clients. A client has to get token before it
 * can issue DB operations. A token is valid for a period of time. Operation
 * with an invalid or expired token will be rejected. Admin can revoke all
 * existing tokens by rotating the valid period.
 */

#define TOKEN_REFRESH_TIME 3600
#define TOKEN_EXPIRE_TIME 3600 * 24

class TokenManager {
    std::string secret_key_;
    int valid_time_;
    // verifier
    jwt::verifier<jwt::default_clock, jwt::traits::kazuho_picojson> verifier_;

 public:
    /*!
    @param[in] secret_key    key for encryption
    @param[in] valid_time    Time to judge whether it is expired
    */
    explicit TokenManager(const std::string& secret_key,
                    const int& valid_time = TOKEN_REFRESH_TIME);

    /*!
    @param[in] valid_time    Time to judge whether it is expired
    */
    void ModifyValidTime(const int& valid_time);

    /*!
    @brief  Generate token when logging in for the first time
    @return token
    */
    std::string IssueFirstToken() const;

    /*!
    @brief  
    @param[in] first_login_time   first_login_time is passed in when the token is refreshed
    @return  token
    */
    std::string IssueRefreshToken(const double& first_login_time) const;

    /*!
    @param[in] token     refresh token
    @return new_token
    */
    std::string UpdateToken(const std::string& token) const;

    /*!
    @param[in] token      judge token is valid
    @return   true->valid    false->unvalid
    */
    bool JudgeRefreshTime(const std::string& token);
};
}  // namespace lgraph
