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

#include "jwt-cpp/jwt.h"

#include "core/data_type.h"
#include "db/token_manager.h"

lgraph::TokenManager::TokenManager(const std::string& secret_key, const int refresh_time)
    : secret_key_(secret_key),
      refresh_time_(refresh_time),
      expire_time_(TOKEN_EXPIRE_TIME),
      verifier_(
          jwt::verify().allow_algorithm(jwt::algorithm::hs256{secret_key_}).with_issuer("fma.ai")) {
}

void lgraph::TokenManager::ModifyRefreshTime(const std::string& token, const int refresh_time) {
    auto decode_token = jwt::decode(token);
    verifier_.verify(decode_token);
    if (refresh_time == 0) {
        refresh_time_ = TOKEN_MAX_TIME;
    } else {
        refresh_time_ = refresh_time;
    }
}

std::pair<int, int> lgraph::TokenManager::GetTokenTime(const std::string& token) {
    auto decode_token = jwt::decode(token);
    verifier_.verify(decode_token);
    auto refresh_time = refresh_time_;
    auto first_login_time = expire_time_;
    return std::make_pair(refresh_time, first_login_time);
}

void lgraph::TokenManager::ModifyExpireTime(const std::string& token, const int expire_time) {
    auto decode_token = jwt::decode(token);
    verifier_.verify(decode_token);
    if (expire_time == 0) {
        expire_time_ = TOKEN_MAX_TIME;
    } else {
        expire_time_ = expire_time;
    }
}

std::string lgraph::TokenManager::IssueFirstToken() const {
    return jwt::create()
    .set_type("JWT")
    .set_issuer("fma.ai")
    .set_payload_claim("refresh_time", jwt::claim(std::to_string(fma_common::GetTime())))
    .set_payload_claim("first_login_time", jwt::claim(std::to_string(fma_common::GetTime())))

    .sign(jwt::algorithm::hs256{secret_key_});
}

std::string lgraph::TokenManager::IssueRefreshToken(const double first_login_time) const {
    return jwt::create()
    .set_type("JWT")
    .set_issuer("fma.ai")
    .set_payload_claim("refresh_time", jwt::claim(std::to_string(fma_common::GetTime())))
    .set_payload_claim("first_login_time", jwt::claim(std::to_string(first_login_time)))

    .sign(jwt::algorithm::hs256{secret_key_});
}

std::string lgraph::TokenManager::UpdateToken(const std::string& token) const {
    auto decode_token = jwt::decode(token);
    verifier_.verify(decode_token);
    auto first_login_time = decode_token.get_payload_claim("first_login_time").as_string();
    if ((fma_common::GetTime() - stod(first_login_time)) <= expire_time_) {
        return IssueRefreshToken(stod(first_login_time));
    } else {
        return "";
    }
}

bool lgraph::TokenManager::JudgeRefreshTime(const std::string& token) {
    auto decode_token = jwt::decode(token);
    verifier_.verify(decode_token);
    auto refresh_time = decode_token.get_payload_claim("refresh_time").as_string();
    if ((fma_common::GetTime() - stod(refresh_time)) <= refresh_time_) {
        return true;
    } else {
        return false;
    }
}
