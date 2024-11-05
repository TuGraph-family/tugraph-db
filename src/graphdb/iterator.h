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

//
// Created by botu.wzy
//

#pragma once

namespace txn {
class Transaction;
}
namespace graphdb {
class Iterator {
   public:
    explicit Iterator(txn::Transaction* txn) : txn_(txn) {}
    // No copying allowed
    Iterator(const Iterator&) = delete;
    void operator=(const Iterator&) = delete;

    txn::Transaction* GetTxn() { return txn_; }
    virtual bool Valid() { return valid_; };
    virtual void Next() = 0;
    virtual ~Iterator() = default;

   protected:
    txn::Transaction* txn_ = nullptr;
    bool valid_ = false;
};
}