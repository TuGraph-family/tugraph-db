/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#pragma once

#include <sstream>
#include <unordered_map>
#include "core/data_type.h"
#include "lgraph/lgraph_date_time.h"
#include "fma-common/file_system.h"
#include "fma-common/fma_stream.h"

namespace lgraph {

class FieldSpecSerializer {
 public:
    static std::string FormatBoolean(bool val) {
        if (val) return "true";
        return "false";
    }

    static bool FileReader(const std::string& path, std::string& res) {
        auto& fs = fma_common::FileSystem::GetFileSystem(path);
        if (fs.FileExists(path)) {
            fma_common::InputFmaStream ifs(path, 0);
            size_t sz = ifs.Size();
            res.resize(sz);
            size_t ssz = ifs.Read(&res[0], sz);
            if (ssz != sz) {
                res = FMA_FMT("Failed to read plugin file {}", path);
                return false;
            }
            return true;
        }
        res = FMA_FMT("Failed to find plugin file {}", path);
        return false;
    }
};

}  // end of namespace lgraph
