/**
* Copyright 2024 AntGroup CO., Ltd.
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

/*
 * written by botu.wzy
 */

#pragma once
#include <string>
namespace common {
class LocalDateTime {
private:
    int64_t nanoseconds_since_epoch_ = 0;
public:
    LocalDateTime();
    explicit LocalDateTime(const std::string& str);
    explicit LocalDateTime(int64_t nanoseconds)
        : nanoseconds_since_epoch_(nanoseconds) {};
    [[nodiscard]] int64_t GetStorage() const {return nanoseconds_since_epoch_;}
    [[nodiscard]] std::string ToString() const;
    bool operator<(const LocalDateTime& rhs) const noexcept;
    bool operator<=(const LocalDateTime& rhs) const noexcept;
    bool operator>(const LocalDateTime& rhs) const noexcept;
    bool operator>=(const LocalDateTime& rhs) const noexcept;
    bool operator==(const LocalDateTime& rhs) const noexcept;
    bool operator!=(const LocalDateTime& rhs) const noexcept;
};

class Date {
   private:
    int64_t days_since_epoch_ = 0;
   public:
    Date();
    explicit Date(const std::string& str);
    explicit Date(int64_t days)
        : days_since_epoch_(days) {};
    [[nodiscard]] int64_t GetStorage() const {return days_since_epoch_;}
    [[nodiscard]] std::string ToString() const;
    bool operator<(const Date& rhs) const noexcept;
    bool operator<=(const Date& rhs) const noexcept;
    bool operator>(const Date& rhs) const noexcept;
    bool operator>=(const Date& rhs) const noexcept;
    bool operator==(const Date& rhs) const noexcept;
    bool operator!=(const Date& rhs) const noexcept;
};

}