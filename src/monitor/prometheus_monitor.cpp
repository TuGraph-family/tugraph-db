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

#include "monitor/prometheus_monitor.h"
#include "prometheus/registry.h"
#include "fma-common/hardware_info.h"
#include "fma-common/file_system.h"
#include "tools/json.hpp"

namespace lgraph {
namespace monitor {

ResourceMonitor::ResourceMonitor(const std::string& host)
    : exposer(host)
    , registry(std::make_shared<prometheus::Registry>()) {
    prometheus::Family<prometheus::Gauge>& gf = prometheus::BuildGauge().Name("resources_report")
            .Help("tugraph resources monitor gauge").Register(*registry);
    cpu_total = &gf.Add({{"resouces_type", "cpu"}, {"type", "total"}});
    cpu_total->SetToCurrentTime();

    cpu_self = &gf.Add({{"resouces_type", "cpu"}, {"type", "self"}});
    cpu_self->SetToCurrentTime();

    mem_total = &gf.Add({{"resouces_type", "memory"}, {"type", "total"}});
    mem_total->SetToCurrentTime();
    mem_available = &gf.Add({{"resouces_type", "memory"}, {"type", "available"}});
    mem_available->SetToCurrentTime();
    mem_self = &gf.Add({{"resouces_type", "memory"}, {"type", "self"}});
    mem_self->SetToCurrentTime();

    disk_read_rate = &gf.Add({{"resouces_type", "disk_rate"}, {"type", "read"}});
    disk_read_rate->SetToCurrentTime();
    disk_write_rate = &gf.Add({{"resouces_type", "disk_rate"}, {"type", "write"}});
    disk_write_rate->SetToCurrentTime();

    disk_total = &gf.Add({{"resouces_type", "disk"}, {"type", "total"}});
    disk_total->SetToCurrentTime();
    disk_available = &gf.Add({{"resouces_type", "disk"}, {"type", "available"}});
    disk_available->SetToCurrentTime();
    disk_self = &gf.Add({{"resouces_type", "disk"}, {"type", "self"}});
    disk_self->SetToCurrentTime();

    total_request = &gf.Add({{"resouces_type", "request"}, {"type", "total"}});
    total_request->SetToCurrentTime();
    write_request = &gf.Add({{"resouces_type", "request"}, {"type", "write"}});
    write_request->SetToCurrentTime();
    exposer.RegisterCollectable(registry);
}

void ResourceMonitor::report_server_info(const std::string &info) {
    nlohmann::json value = nlohmann::json::parse(info);
    cpu_total->Set(value["cpu"]["server"]);
    cpu_self->Set(value["cpu"]["self"]);
    mem_total->Set(value["memory"]["total"]);
    mem_available->Set(value["memory"]["available"]);
    mem_self->Set(value["memory"]["self"]);
    disk_read_rate->Set(value["disk_rate"]["read"]);
    disk_write_rate->Set(value["disk_rate"]["write"]);
    disk_total->Set(value["disk_storage"]["total"]);
    disk_available->Set(value["disk_storage"]["available"]);
    disk_self->Set(value["disk_storage"]["self"]);
}

void ResourceMonitor::report_tugraph_info(const std::string& info) {
    nlohmann::json value = nlohmann::json::parse(info);
    total_request->Set(value["request"]["requests/second"]);
    write_request->Set(value["request"]["writes/second"]);
}

}  // end of namespace monitor

}  // end of namespace lgraph
