#include <gflags/gflags.h>
#include <filesystem>
#include <shared_mutex>
#include <boost/endian/conversion.hpp>
#include <boost/lexical_cast.hpp>
#include <nlohmann/json.hpp>
#include <boost/asio.hpp>
#include "raft_driver.h"
#include "logger.h"

using boost::asio::async_write;
using boost::asio::ip::tcp;
#define ASSERT(cond) if (!(cond)) std::abort()

extern std::shared_mutex promise_mutex;
extern std::unordered_map<uint64_t, std::promise<std::string>> pending_promise;

void NodeClient::reconnect() {
    if (has_closed_) {
        return;
    }
    boost::system::error_code ec;
    socket_.close(ec);
    if (ec) {
        LOG_WARN("socket close error: {}", ec.message());
    }
    connected_ = false;
    send_buffers_.clear();
    msg_queue_.clear();
    timer_.expires_from_now(interval_);
    timer_.async_wait([this, self = shared_from_this()](const boost::system::error_code &ec) {
        if (ec == boost::asio::error::operation_aborted) {
            return;
        }
        Connect();
    });
}

void NodeClient::Close() {
    has_closed_ = true;
    io_service_.post([this, self = shared_from_this()]() {
        boost::system::error_code ec;
        socket_.close(ec);
        timer_.cancel(ec);
    });
}

void NodeClient::Send(std::string str) {
    if (has_closed_) {
        LOG_DEBUG("connection[{}:{}] is closed, drop this message", endpoint_.address().to_string(), endpoint_.port());
        return;
    }
    io_service_.post([this, self = shared_from_this(), msg = std::move(str)]() mutable  {
        if (!connected_) {
            LOG_DEBUG("connection[{}:{}] is not available, drop this message", endpoint_.address().to_string(), endpoint_.port());
            return;
        }
        bool need_invoke = msg_queue_.empty();
        msg_queue_.emplace_back(std::move(msg));
        if (need_invoke) {
            do_send();
        }
    });
}

void NodeClient::send_magic_code() {
    async_write(socket_, boost::asio::buffer(magic_code),
        [this, self = shared_from_this()](const boost::system::error_code &ec, size_t) {
            if (ec) {
                LOG_WARN("async write error {}", ec.message().c_str());
                if (ec == boost::asio::error::operation_aborted) {
                    return;
                }
                reconnect();
                return;
            }
            connected_ = true;
            do_read_some();
        }
    );
}

void NodeClient::do_send() {
    for (size_t i = 0; i < msg_queue_.size(); i++) {
        send_buffers_.emplace_back(boost::asio::buffer(msg_queue_[i]));
        if (send_buffers_.size() >= 5) {
            break;
        }
    }
    async_write(socket_, send_buffers_,
        [this, self = shared_from_this()](const boost::system::error_code &ec, std::size_t) {
            if (ec) {
                LOG_WARN("async write error {}", ec.message().c_str());
                if (ec == boost::asio::error::operation_aborted) {
                    return;
                }
                reconnect();
                return;
            }
            assert(msg_queue_.size() >= send_buffers_.size());
            msg_queue_.erase(msg_queue_.begin(), msg_queue_.begin() + send_buffers_.size());
            send_buffers_.clear();
            if (!msg_queue_.empty()) {
                do_send();
            }
        }
    );
}

void NodeClient::do_read_some() {
    socket_.async_read_some(boost::asio::buffer(buffer4_),
        [this, self = shared_from_this()](const boost::system::error_code &ec, size_t) {
            if (ec) {
                LOG_WARN("async_read_some error: {}", ec.message().c_str());
                if (ec == boost::asio::error::operation_aborted) {
                    return;
                }
                reconnect();
            } else {
                LOG_ERROR("unexpected read");
                reconnect();
            }
        });
}

void socket_set_options(tcp::socket& socket) {
    try {
        socket.set_option(tcp::no_delay(true));
        socket.set_option(boost::asio::socket_base::keep_alive(true));
        socket.set_option(tcp::socket::reuse_address(true));
    } catch (const boost::system::system_error& e) {
        LOG_ERROR("socket_set_options error: {}", e.what());
    }
}

void NodeClient::Connect() {
    if (has_closed_) {
        return;
    }
    boost::system::error_code ec;
    socket_.open(tcp::v4(), ec);
    if (ec) {
        LOG_WARN("socket open error: {}", ec.message().c_str());
    }
    socket_.async_connect(endpoint_,
        [this, self = shared_from_this()](const boost::system::error_code &ec) {
            if (ec) {
                if (ec == boost::asio::error::operation_aborted) {
                    return;
                }
                LOG_WARN("async connect {} error: {}", boost::lexical_cast<std::string>(endpoint_).c_str(), ec.message().c_str());
                reconnect();
            } else {
                LOG_INFO("connect {} success", boost::lexical_cast<std::string>(endpoint_).c_str());
                socket_set_options(socket_);
                send_magic_code();
            }
        });
}

std::string MessageToNetString(const google::protobuf::Message& msg) {
    std::string str;
    msg.SerializeToString(&str);
    uint32_t size = str.size();
    boost::endian::native_to_big_inplace(size);
    str.insert(0, (const char*)&size, sizeof(size));
    return str;
}

eraft::Error RaftDriver::Run() {
    rocksdb::Options options;
    options.create_if_missing = true;
    options.create_missing_column_families = true;
    std::vector<rocksdb::ColumnFamilyDescriptor> cfs;
    cfs.emplace_back(rocksdb::kDefaultColumnFamilyName, options);
    cfs.emplace_back("meta", options);
    std::vector<rocksdb::ColumnFamilyHandle*> cf_handles;
    rocksdb::DB* db;
    std::filesystem::create_directories(log_path_);
    auto s = rocksdb::DB::Open(options, log_path_, cfs, &cf_handles, &db);
    if (!s.ok()) {
        return eraft::Error("Open raft db error: " + s.ToString());
    }
    storage_ = std::make_shared<RaftLogStorage>(db, cf_handles[0], cf_handles[1]);
    auto applied = std::max(apply_id_, storage_->GetApplyIndex());
    auto nodes_info = storage_->GetNodesInfo();
    auto json_obj = nlohmann::json::parse(nodes_info);
    for (auto& item : json_obj) {
        auto client = std::make_shared<NodeClient>(client_service_, item["ip"].get<std::string>(), item["port"].get<int>());
        client->Connect();
        node_clients_.emplace(item["node_id"].get<uint64_t>(), std::move(client));
    }
    LOG_INFO("Raft nodes info: {}", nodes_info);
    bool exist = storage_->Init();
    eraft::Config config;
    config.id_ = node_id_;
    config.applied_ = applied;
    config.electionTick_ = 10;
    config.heartbeatTick_ = 1;
    config.storage_ = storage_;
    config.maxSizePerMsg_ = 1024 * 1024;
    config.maxInflightMsgs_ = 256;
    config.maxUncommittedEntriesSize_ = 1 << 30;
    config.stepDownOnRemoval_ = true;
    eraft::Error err;
    std::tie(rn_, err) = eraft::NewRawNode(config);
    if (err != nullptr) {
        return err;
    }
    if (!exist) {
        err = rn_->Bootstrap(init_peers_);
        if (err != nullptr) {
            return err;
        }
    }
    tick_timer_.async_wait([this](const boost::system::error_code& ec) {
        if (ec) {
            LOG_WARN("tick_timer async_wait error: {}", ec.message().c_str());
            return;
        }
        tick();
    });
    return {};
}

void RaftDriver::Message(raftpb::Message msg) {
    // Ignore unexpected local messages receiving over network.
    if (eraft::IsLocalMsg(msg.type()) && !eraft::IsLocalMsgTarget(msg.from())) {
        return;
    }
    raft_service_.post([this, msg = std::move(msg)]() mutable {
        if (eraft::IsResponseMsg(msg.type()) &&
            !eraft::IsLocalMsgTarget(msg.from()) &&
            rn_->raft_->trk_.progress_.data().count(msg.from()) == 0) {
            // Filter out response message from unknown From.
            return;
        }
        auto err = rn_->Step(std::move(msg));
        if (err != nullptr) {
            LOG_WARN("Step return err: {}",err.String().c_str());
        }
        check_ready();
    });
}

eraft::Error RaftDriver::PostMessage(raftpb::Message msg) {
    std::promise<eraft::Error> promise;
    std::future<eraft::Error> future = promise.get_future();
    raft_service_.post([this, msg = std::move(msg), &promise]() mutable {
        if (rn_->raft_->id_ != rn_->raft_->lead_) {
            promise.set_value(eraft::Error("not leader"));
            return;
        }
        msg.set_from(rn_->raft_->id_);
        auto err = rn_->Step(std::move(msg));
        if (err != nullptr) {
            LOG_WARN("Proposal return err: {}",err.String().c_str());
        }
        promise.set_value(std::move(err));
        check_ready();
    });
    return future.get();
}

eraft::Error RaftDriver::ProposeConfChange(const raftpb::ConfChange& cc) {
    raftpb::Message msg;
    auto entry = msg.add_entries();
    entry->set_type(raftpb::EntryType::EntryConfChange);
    entry->set_data(cc.SerializeAsString());
    msg.set_type(raftpb::MessageType::MsgProp);
    return PostMessage(std::move(msg));
}

eraft::Error RaftDriver::Proposal(std::string data) {
    raftpb::Message msg;
    auto entry = msg.add_entries();
    entry->set_type(raftpb::EntryType::EntryNormal);
    entry->set_data(std::move(data));
    msg.set_type(raftpb::MessageType::MsgProp);
    return PostMessage(std::move(msg));
}

void RaftDriver::check_ready() {
    if (advance_) {
        return;
    }
    if (!rn_->HasReady()) {
        return;
    }
    auto ready = rn_->GetReady();
    ready_service_.post([this, ready = std::move(ready)]() mutable {
        on_ready(std::move(ready));
        advance();
    });
    advance_ = true;
}

std::string RaftDriver::nodes_info() {
    auto array = nlohmann::json::array();
    for (auto& [id, node] : node_clients_) {
        auto obj = nlohmann::json::object();
        obj["node_id"] = id;
        obj["ip"] = node->ip();
        obj["port"] = node->port();
        array.emplace_back(std::move(obj));
    }
    return array.dump();
}


void RaftDriver::on_ready(eraft::Ready ready) {
    if (ready.softState_) {
        LOG_INFO("soft state change, state:{}, lead:{}",
                 eraft::ToString(ready.softState_->raftState_).c_str(), ready.softState_->lead_);
    }
    rocksdb::WriteBatch batch;
    if (!ready.entries_.empty()) {
        ASSERT(storage_->Append(std::move(ready.entries_), batch) == nullptr);
    }
    if (!eraft::IsEmptyHardState(ready.hardState_)) {
        ASSERT(storage_->SetHardState(ready.hardState_, batch) == nullptr);
    }
    if (!eraft::IsEmptySnap(ready.snapshot_)) {
        LOG_FATAL("Snapshot should be empty");
        //ASSERT(storage_->SetSnapshot(ready.snapshot_, batch) == nullptr);
    }
    storage_->WriteBatch(batch);
    for (const auto& msg : ready.messages_) {
        auto iter = node_clients_.find(msg.to());
        if (iter != node_clients_.end()) {
            iter->second->Send(MessageToNetString(msg));
        } else {
            LOG_WARN("send msg, but peer client id {} not exists", msg.to());
        }
    }
    if (!eraft::IsEmptySnap(ready.snapshot_)) {
        LOG_FATAL("Snapshot should be empty");
        //ASSERT(storage_->ApplySnapshot(std::move(ready.snapshot_), batch) == nullptr);
    }
    for (const auto& entry : ready.committedEntries_) {
        switch (entry.type()) {
            case raftpb::EntryNormal: {
                if (entry.data().empty()) {
                    continue;
                }
                auto propose = nlohmann::json::parse(entry.data());
                auto uid = propose["uid"].get<uint64_t>();
                auto data = propose["data"].get<std::string>();
                apply_(entry.index(), data);
                {
                    std::shared_lock lock(promise_mutex);
                    auto iter = pending_promise.find(uid);
                    if (iter != pending_promise.end()) {
                        iter->second.set_value(data);
                    }
                }
                break;
            }
            case raftpb::EntryConfChange: {
                raftpb::ConfChange cc;
                ASSERT(cc.ParseFromString(entry.data()));
                auto confstate = rn_->ApplyConfChange(raftpb::ConfChangeWrap(cc));
                auto info = nlohmann::json::parse(cc.context());
                switch (cc.type()) {
                    case  raftpb::ConfChangeType::ConfChangeAddLearnerNode:
                    case  raftpb::ConfChangeType::ConfChangeAddNode: {
                        if (cc.type() == raftpb::ConfChangeType::ConfChangeAddNode) {
                            LOG_INFO("Add node: {}", cc.ShortDebugString());
                        } else {
                            LOG_INFO("Add learner: {}", cc.ShortDebugString());
                        }
                        if (!node_clients_.count(cc.node_id())) {
                            auto ip = info["ip"].get<std::string>();
                            auto port = info["port"].get<int>();
                            auto client = std::make_shared<NodeClient>(client_service_, ip, port);
                            client->Connect();
                            node_clients_.emplace(cc.node_id(), std::move(client));
                        } else {
                            LOG_ERROR("peer client id %d has already existed", cc.node_id());
                        }
                        break;
                    }
                    case  raftpb::ConfChangeType::ConfChangeRemoveNode: {
                        LOG_INFO("Remove node: {}", cc.ShortDebugString());
                        if (node_clients_.count(cc.node_id())) {
                            node_clients_.at(cc.node_id())->Close();
                            node_clients_.erase(cc.node_id());
                            LOG_INFO("erase id {} from peer client pool", cc.node_id());
                        }
                        break;
                    }
                    case  raftpb::ConfChangeType::ConfChangeUpdateNode: {
                        LOG_INFO("Update node: %s", cc.ShortDebugString());
                        break;
                    }
                    default: {
                        break;
                    }
                }
                LOG_INFO("New ConfState: {}", confstate->ShortDebugString());
                rocksdb::WriteBatch wb;
                storage_->SetConfState(*confstate, wb);
                storage_->SetNodesInfo(nodes_info(), wb);
                storage_->SetApplyIndex(entry.index(), wb);
                storage_->WriteBatch(wb);
                if (cc.id() > 0) {
                    std::shared_lock lock(promise_mutex);
                    auto iter = pending_promise.find(cc.id());
                    if (iter != pending_promise.end()) {
                        iter->second.set_value(cc.context());
                    }
                }
                break;
            }
            default: {
                LOG_ERROR("Unhandled entry : {}", entry.ShortDebugString());
            }
        }
    }
}