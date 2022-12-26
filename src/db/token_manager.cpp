/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#include "jwt-cpp/jwt.h"

#include "core/data_type.h"
#include "db/token_manager.h"

lgraph::TokenManager::TokenManager(const std::string& secret_key, const int& valid_time)
    : secret_key_(secret_key),
      valid_time_(valid_time),
      verifier_(
          jwt::verify().allow_algorithm(jwt::algorithm::hs256{secret_key_}).with_issuer("fma.ai")) {
}

void lgraph::TokenManager::ModifyValidTime(const int& valid_time) {
    valid_time_ = valid_time;
}

std::string lgraph::TokenManager::IssueFirstToken() const {
    return jwt::create()
    .set_type("JWT")
    .set_issuer("fma.ai")
    .set_payload_claim("refresh_time", jwt::claim(std::to_string(fma_common::GetTime())))
    .set_payload_claim("first_login_time", jwt::claim(std::to_string(fma_common::GetTime())))

    .sign(jwt::algorithm::hs256{secret_key_});
}

std::string lgraph::TokenManager::IssueRefreshToken(const double& first_login_time) const {
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
    if ((fma_common::GetTime() - stod(first_login_time)) <= TOKEN_EXPIRE_TIME) {
        return IssueRefreshToken(stod(first_login_time));
    } else {
        return "";
    }
}

bool lgraph::TokenManager::JudgeRefreshTime(const std::string& token) {
    auto decode_token = jwt::decode(token);
    verifier_.verify(decode_token);
    auto refresh_time = decode_token.get_payload_claim("refresh_time").as_string();
    if ((fma_common::GetTime() - stod(refresh_time)) <= valid_time_) {
        return true;
    } else {
        return false;
    }
}
