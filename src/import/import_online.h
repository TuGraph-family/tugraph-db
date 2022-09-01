/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#pragma once

#include <exception>
#include <vector>
#include <thread>

#include "core/data_type.h"
#include "core/value.h"
#include "import/import_v2.h"

namespace lgraph {
class HaStateMachine;

namespace import_v2 {

class ImportOnline {
 public:
    struct Config {
        bool continue_on_error = false;
        size_t n_threads = 8;
        std::string delimiter = ",";
    };

    // main entrance of online import
    // this is called by StateMachine when received the request
    // this function is called in each HA worker
    static std::string HandleOnlineTextPackage(std::string&& desc, std::string&& data,
                                               LightningGraph* db, const Config& config);

    static std::string HandleOnlineSchema(std::string&& desc, AccessControlledDB& db);

 private:
    // during online import, all functions are static
    // at present, no ImportOnline instance should be created
    ImportOnline() = delete;

    static std::string ImportVertexes(LightningGraph* db, Transaction& txn, const CsvDesc& fd,
                                      std::vector<std::vector<FieldData>>&& data,
                                      const Config& config);

    static std::string ImportEdges(LightningGraph* db, Transaction& txn, const CsvDesc& fd,
                                   std::vector<std::vector<FieldData>>&& data,
                                   const Config& config);
};
}  // namespace import_v2
}  // namespace lgraph
