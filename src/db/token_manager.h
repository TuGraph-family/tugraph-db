/**
 * Copyright 2022 AntGroup CO., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 */

#pragma once

#include <atomic>
#include <cstdint>
#include <string>
#include <limits>

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
#define TOKEN_MAX_TIME std::numeric_limits<int>::max()

class TokenManager {
    std::string secret_key_;
    int refresh_time_;
    int expire_time_;
    // verifier
    jwt::verifier<jwt::default_clock, jwt::traits::kazuho_picojson> verifier_;

 public:
    /*!
    @param[in] secret_key    key for encryption
    @param[in] refresh_time    Time to judge whether it is expired
    */
    explicit TokenManager(const std::string& secret_key,
                    const int refresh_time = TOKEN_REFRESH_TIME);

    /*!
    @param[in] token         jwt token
    @param[in] refresh_time    Time to judge whether it is expired
    */
    void ModifyRefreshTime(const std::string& token, const int refresh_time);

    /*!
    @param[in] token         jwt token
    @return  refresh_time, expire_time
    */
    std::pair<int, int> GetTokenTime(const std::string& token);

    /*!
    @param[in] token          jwt token
    @param[in] expire_time    Time to judge whether it is expired
    */
    void ModifyExpireTime(const std::string& token, const int expire_time);

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
    std::string IssueRefreshToken(const double first_login_time) const;

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
