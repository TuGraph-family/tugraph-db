/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#pragma once

#include <iostream>
#include <utility>
#include <map>
#include <boost/any.hpp>

#ifndef _WIN32
#include "brpc/server.h"
#include "gflags/gflags.h"
#include "butil/logging.h"
#include "butil/time.h"
#include "brpc/channel.h"
#include "protobuf/ha.pb.h"
#endif

namespace lgraph_rpc {
class m_channel : public brpc::Channel {};
class m_controller : public brpc::Controller {};
class m_channel_options : public brpc::ChannelOptions {};
}  // namespace lgraph_rpc
