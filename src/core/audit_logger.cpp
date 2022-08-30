/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#include "core/audit_logger.h"

namespace lgraph {
std::atomic<bool> AuditLogger::enabled_(false);

thread_local AuditLogTLS AuditLogTLS::instance_;
}  // namespace lgraph
