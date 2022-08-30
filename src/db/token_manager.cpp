/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#include "jwt-cpp/jwt.h"

#include "core/data_type.h"
#include "db/token_manager.h"

lgraph::TokenManager::TokenManager(const std::string& secret_key)
    : secret_key_(secret_key),
      verifier_(
          jwt::verify().allow_algorithm(jwt::algorithm::hs256{secret_key_}).with_issuer("fma.ai")) {
}

std::string lgraph::TokenManager::IssueToken(const std::string& user,
                                             const std::string& password) const {
    return jwt::create()
        .set_type("JWT")
        .set_issuer("fma.ai")
        .set_payload_claim("user", jwt::claim(user))
        .set_payload_claim("password", jwt::claim(password))
        .sign(jwt::algorithm::hs256{secret_key_});
}

bool lgraph::TokenManager::DecipherToken(const std::string& token, std::string& user,
                                         std::string& password) const {
    auto decode_token = jwt::decode(token);
    verifier_.verify(decode_token);
    user = decode_token.get_payload_claim("user").as_string();
    password = decode_token.get_payload_claim("password").as_string();
    return true;
}
