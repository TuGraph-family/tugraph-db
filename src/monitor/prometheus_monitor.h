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

#include "prometheus/exposer.h"
#include "prometheus/gauge.h"

namespace lgraph {
namespace monitor {

class ResourceMonitor {
 public:
    explicit ResourceMonitor(const std::string &host);

    void report_server_info(const std::string& info);

    void report_tugraph_info(const std::string& info);

 private:
    prometheus::Exposer exposer;
    std::shared_ptr<prometheus::Registry> registry;
    prometheus::Gauge *cpu_total;
    prometheus::Gauge *cpu_self;

    prometheus::Gauge *mem_total;
    prometheus::Gauge *mem_available;
    prometheus::Gauge *mem_self;

    prometheus::Gauge *disk_read_rate;
    prometheus::Gauge *disk_write_rate;

    prometheus::Gauge *disk_total;
    prometheus::Gauge *disk_available;
    prometheus::Gauge *disk_self;

    prometheus::Gauge *total_request;
    prometheus::Gauge *write_request;
};

}  // end of namespace monitor

}  // end of namespace lgraph
